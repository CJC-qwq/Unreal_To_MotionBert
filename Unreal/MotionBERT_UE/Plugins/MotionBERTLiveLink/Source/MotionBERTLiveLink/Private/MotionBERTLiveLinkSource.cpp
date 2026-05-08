#include "MotionBERTLiveLinkSource.h"

#include "MotionBERTLiveLink.h"

#include "Async/Async.h"
#include "Common/UdpSocketBuilder.h"
#include "ILiveLinkClient.h"
#include "JsonObjectConverter.h"
#include "Misc/CoreDelegates.h"
#include "Roles/LiveLinkAnimationRole.h"
#include "Roles/LiveLinkAnimationTypes.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

#define LOCTEXT_NAMESPACE "MotionBERTLiveLinkSource"

namespace MotionBERTLiveLinkDefaults
{
	static const TCHAR* SubjectName = TEXT("MotionBERTPose");
	static const TCHAR* BoneNames[] =
	{
		TEXT("pelvis"),
		TEXT("right_hip"),
		TEXT("right_knee"),
		TEXT("right_ankle"),
		TEXT("left_hip"),
		TEXT("left_knee"),
		TEXT("left_ankle"),
		TEXT("spine"),
		TEXT("thorax"),
		TEXT("nose"),
		TEXT("head"),
		TEXT("left_shoulder"),
		TEXT("left_elbow"),
		TEXT("left_wrist"),
		TEXT("right_shoulder"),
		TEXT("right_elbow"),
		TEXT("right_wrist")
	};

	static const int32 BoneParents[] =
	{
		-1,
		0, 1, 2,
		0, 4, 5,
		0, 7, 8, 9,
		8, 11, 12,
		8, 14, 15
	};
}

FMotionBERTLiveLinkSource::FMotionBERTLiveLinkSource(const FMotionBERTLiveLinkConnectionSettings& InConnectionSettings)
	: ConnectionSettings(InConnectionSettings)
{
	SourceStatus = LOCTEXT("SourceStatus_NoData", "No data");
	SourceType = LOCTEXT("SourceType", "MotionBERT UDP");
	SourceMachineName = FText::Format(
		LOCTEXT("MachineName", "{0}:{1}"),
		FText::FromString(ConnectionSettings.BindAddress),
		FText::AsNumber(ConnectionSettings.UdpPort, &FNumberFormattingOptions::DefaultNoGrouping())
	);

	if (!FIPv4Address::Parse(ConnectionSettings.BindAddress, DeviceEndpoint.Address))
	{
		DeviceEndpoint.Address = FIPv4Address::Any;
	}
	DeviceEndpoint.Port = ConnectionSettings.UdpPort;

	Socket = FUdpSocketBuilder(TEXT("MotionBERTLiveLinkSocket"))
		.AsNonBlocking()
		.AsReusable()
		.BoundToEndpoint(DeviceEndpoint)
		.WithReceiveBufferSize(ReceiveBufferSize);

	if (Socket != nullptr && Socket->GetSocketType() == SOCKTYPE_Datagram)
	{
		SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
		ReceiveBuffer.SetNumUninitialized(ReceiveBufferSize);
		DeferredStartDelegateHandle = FCoreDelegates::OnEndFrame.AddRaw(this, &FMotionBERTLiveLinkSource::Start);
		UE_LOG(LogMotionBERTLiveLink, Log, TEXT("MotionBERTLiveLink: Listening on %s"), *DeviceEndpoint.ToString());
	}
	else
	{
		UE_LOG(LogMotionBERTLiveLink, Error, TEXT("MotionBERTLiveLink: Failed to open UDP socket on %s"), *DeviceEndpoint.ToString());
	}
}

FMotionBERTLiveLinkSource::~FMotionBERTLiveLinkSource()
{
	if (DeferredStartDelegateHandle.IsValid())
	{
		FCoreDelegates::OnEndFrame.Remove(DeferredStartDelegateHandle);
	}

	Stop();

	if (Thread != nullptr)
	{
		Thread->WaitForCompletion();
		delete Thread;
		Thread = nullptr;
	}

	if (Socket != nullptr)
	{
		Socket->Close();
		if (SocketSubsystem != nullptr)
		{
			SocketSubsystem->DestroySocket(Socket);
		}
		Socket = nullptr;
	}
}

void FMotionBERTLiveLinkSource::ReceiveClient(ILiveLinkClient* InClient, FGuid InSourceGuid)
{
	FScopeLock Lock(&ClientAccessMutex);
	Client = InClient;
	SourceGuid = InSourceGuid;
}

bool FMotionBERTLiveLinkSource::IsSourceStillValid() const
{
	return !Stopping && Thread != nullptr && Socket != nullptr;
}

bool FMotionBERTLiveLinkSource::RequestSourceShutdown()
{
	Stop();
	return true;
}

FText FMotionBERTLiveLinkSource::GetSourceType() const
{
	return SourceType;
}

FText FMotionBERTLiveLinkSource::GetSourceMachineName() const
{
	return SourceMachineName;
}

FText FMotionBERTLiveLinkSource::GetSourceStatus() const
{
	return SourceStatus;
}

void FMotionBERTLiveLinkSource::Start()
{
	check(DeferredStartDelegateHandle.IsValid());

	FCoreDelegates::OnEndFrame.Remove(DeferredStartDelegateHandle);
	DeferredStartDelegateHandle.Reset();

	SourceStatus = LOCTEXT("SourceStatus_Listening", "Listening");
	ThreadName = TEXT("MotionBERTLiveLink Receiver ");
	ThreadName.AppendInt(FAsyncThreadIndex::GetNext());
	Thread = FRunnableThread::Create(this, *ThreadName, 128 * 1024, TPri_AboveNormal, FPlatformAffinity::GetPoolThreadMask());
}

void FMotionBERTLiveLinkSource::Stop()
{
	Stopping = true;
}

uint32 FMotionBERTLiveLinkSource::Run()
{
	const FTimespan SocketTimeout = FTimespan::FromMilliseconds(1);

	while (!Stopping)
	{
		if (Socket != nullptr && Socket->Wait(ESocketWaitConditions::WaitForRead, SocketTimeout))
		{
			uint32 PendingDataSize = 0;
			FString LatestPacketString;
			bool bReceivedPacket = false;
			while (Socket != nullptr && Socket->HasPendingData(PendingDataSize))
			{
				int32 ReceivedDataSize = 0;
				if (Socket != nullptr && Socket->Recv(ReceiveBuffer.GetData(), ReceiveBufferSize, ReceivedDataSize) && ReceivedDataSize > 0)
				{
					LatestPacketString = FString(UTF8_TO_TCHAR(reinterpret_cast<const char*>(ReceiveBuffer.GetData()))).Left(ReceivedDataSize);
					bReceivedPacket = true;
				}
			}

			if (bReceivedPacket)
			{
				FSubjectPacket Packet;
				if (ParsePacket(LatestPacketString, Packet))
				{
					EnsureStaticDataSent(Packet);
					SendFrameData(Packet);
					SourceStatus = LOCTEXT("SourceStatus_Active", "Active");
				}
				else
				{
					SourceStatus = LOCTEXT("SourceStatus_BadPacket", "Bad packet");
				}
			}
		}
	}

	return 0;
}

bool FMotionBERTLiveLinkSource::ParsePacket(const FString& PacketString, FSubjectPacket& OutPacket) const
{
	TSharedPtr<FJsonObject> JsonObject;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(PacketString);
	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		UE_LOG(LogMotionBERTLiveLink, Warning, TEXT("MotionBERTLiveLink: Failed to parse JSON packet."));
		return false;
	}

	FString SubjectNameString;
	if (JsonObject->TryGetStringField(TEXT("subject_name"), SubjectNameString))
	{
		OutPacket.SubjectName = FName(*SubjectNameString);
	}
	if (OutPacket.SubjectName.IsNone())
	{
		OutPacket.SubjectName = FName(*ConnectionSettings.DefaultSubjectName);
	}

	if (JsonObject->HasTypedField<EJson::Array>(TEXT("bone_names")))
	{
		const TArray<TSharedPtr<FJsonValue>>& BoneNamesJson = JsonObject->GetArrayField(TEXT("bone_names"));
		for (const TSharedPtr<FJsonValue>& Value : BoneNamesJson)
		{
			OutPacket.BoneNames.Add(FName(*Value->AsString()));
		}
	}

	if (JsonObject->HasTypedField<EJson::Array>(TEXT("bone_parents")))
	{
		const TArray<TSharedPtr<FJsonValue>>& BoneParentsJson = JsonObject->GetArrayField(TEXT("bone_parents"));
		for (const TSharedPtr<FJsonValue>& Value : BoneParentsJson)
		{
			OutPacket.BoneParents.Add(static_cast<int32>(Value->AsNumber()));
		}
	}

	if (JsonObject->HasTypedField<EJson::Number>(TEXT("timestamp")))
	{
		OutPacket.TimestampSeconds = JsonObject->GetNumberField(TEXT("timestamp"));
	}

	if (JsonObject->HasTypedField<EJson::Array>(TEXT("transforms")))
	{
		if (!ParseTransformArray(JsonObject->GetArrayField(TEXT("transforms")), OutPacket.BoneTransforms))
		{
			return false;
		}
	}
	else if (JsonObject->HasTypedField<EJson::Array>(TEXT("positions")))
	{
		if (!ParsePositionArray(JsonObject->GetArrayField(TEXT("positions")), OutPacket.BoneTransforms))
		{
			return false;
		}
	}
	else
	{
		UE_LOG(LogMotionBERTLiveLink, Warning, TEXT("MotionBERTLiveLink: Packet missing transforms or positions."));
		return false;
	}

	if (OutPacket.BoneNames.Num() == 0 || OutPacket.BoneParents.Num() == 0)
	{
		FillDefaultSkeleton(OutPacket);
	}

	if (OutPacket.BoneNames.Num() != OutPacket.BoneParents.Num() || OutPacket.BoneNames.Num() != OutPacket.BoneTransforms.Num())
	{
		UE_LOG(
			LogMotionBERTLiveLink,
			Warning,
			TEXT("MotionBERTLiveLink: Skeleton mismatch names=%d parents=%d transforms=%d"),
			OutPacket.BoneNames.Num(),
			OutPacket.BoneParents.Num(),
			OutPacket.BoneTransforms.Num()
		);
		return false;
	}

	return true;
}

bool FMotionBERTLiveLinkSource::ParseTransformArray(const TArray<TSharedPtr<FJsonValue>>& TransformValues, TArray<FTransform>& OutTransforms) const
{
	OutTransforms.Reserve(TransformValues.Num());
	for (const TSharedPtr<FJsonValue>& TransformValue : TransformValues)
	{
		const TSharedPtr<FJsonObject> TransformObject = TransformValue->AsObject();
		if (!TransformObject.IsValid())
		{
			return false;
		}

		TArray<double> TranslationValues;
		TArray<double> RotationValues;
		TArray<double> ScaleValues;

		if (!ParseFloatArray(TransformObject->GetArrayField(TEXT("translation")), 3, TranslationValues))
		{
			return false;
		}
		if (!ParseFloatArray(TransformObject->GetArrayField(TEXT("rotation")), 4, RotationValues))
		{
			return false;
		}
		if (TransformObject->HasTypedField<EJson::Array>(TEXT("scale")))
		{
			if (!ParseFloatArray(TransformObject->GetArrayField(TEXT("scale")), 3, ScaleValues))
			{
				return false;
			}
		}
		else
		{
			ScaleValues = {1.0, 1.0, 1.0};
		}

		const FVector Translation(static_cast<float>(TranslationValues[0]), static_cast<float>(TranslationValues[1]), static_cast<float>(TranslationValues[2]));
		const FQuat Rotation(static_cast<float>(RotationValues[0]), static_cast<float>(RotationValues[1]), static_cast<float>(RotationValues[2]), static_cast<float>(RotationValues[3]));
		const FVector Scale(static_cast<float>(ScaleValues[0]), static_cast<float>(ScaleValues[1]), static_cast<float>(ScaleValues[2]));
		OutTransforms.Add(FTransform(Rotation, Translation, Scale));
	}

	return true;
}

bool FMotionBERTLiveLinkSource::ParsePositionArray(const TArray<TSharedPtr<FJsonValue>>& PositionValues, TArray<FTransform>& OutTransforms) const
{
	OutTransforms.Reserve(PositionValues.Num());
	for (const TSharedPtr<FJsonValue>& PositionValue : PositionValues)
	{
		TArray<double> TranslationValues;
		if (!ParseFloatArray(PositionValue->AsArray(), 3, TranslationValues))
		{
			return false;
		}

		const FVector Translation(static_cast<float>(TranslationValues[0]), static_cast<float>(TranslationValues[1]), static_cast<float>(TranslationValues[2]));
		OutTransforms.Add(FTransform(FQuat::Identity, Translation, FVector::OneVector));
	}

	return true;
}

void FMotionBERTLiveLinkSource::EnsureStaticDataSent(const FSubjectPacket& Packet)
{
	FScopeLock Lock(&ClientAccessMutex);
	if (Stopping || Client == nullptr || EncounteredSubjects.Contains(Packet.SubjectName))
	{
		return;
	}

	FLiveLinkStaticDataStruct StaticDataStruct(FLiveLinkSkeletonStaticData::StaticStruct());
	FLiveLinkSkeletonStaticData& SkeletonData = *StaticDataStruct.Cast<FLiveLinkSkeletonStaticData>();
	SkeletonData.BoneNames = Packet.BoneNames;
	SkeletonData.BoneParents = Packet.BoneParents;

	Client->RemoveSubject_AnyThread({SourceGuid, Packet.SubjectName});
	Client->PushSubjectStaticData_AnyThread({SourceGuid, Packet.SubjectName}, ULiveLinkAnimationRole::StaticClass(), MoveTemp(StaticDataStruct));
	EncounteredSubjects.Add(Packet.SubjectName);
}

void FMotionBERTLiveLinkSource::SendFrameData(const FSubjectPacket& Packet)
{
	FScopeLock Lock(&ClientAccessMutex);
	if (Stopping || Client == nullptr)
	{
		return;
	}

	FLiveLinkFrameDataStruct FrameDataStruct(FLiveLinkAnimationFrameData::StaticStruct());
	FLiveLinkAnimationFrameData& AnimationFrameData = *FrameDataStruct.Cast<FLiveLinkAnimationFrameData>();
	AnimationFrameData.WorldTime = Packet.TimestampSeconds > 0.0 ? Packet.TimestampSeconds : FPlatformTime::Seconds();
	AnimationFrameData.Transforms = Packet.BoneTransforms;

	Client->PushSubjectFrameData_AnyThread({SourceGuid, Packet.SubjectName}, MoveTemp(FrameDataStruct));
}

void FMotionBERTLiveLinkSource::FillDefaultSkeleton(FSubjectPacket& InOutPacket) const
{
	if (InOutPacket.BoneNames.Num() == 0)
	{
		for (const TCHAR* BoneName : MotionBERTLiveLinkDefaults::BoneNames)
		{
			InOutPacket.BoneNames.Add(FName(BoneName));
		}
	}

	if (InOutPacket.BoneParents.Num() == 0)
	{
		for (const int32 ParentIndex : MotionBERTLiveLinkDefaults::BoneParents)
		{
			InOutPacket.BoneParents.Add(ParentIndex);
		}
	}
}

bool FMotionBERTLiveLinkSource::ParseFloatArray(const TArray<TSharedPtr<FJsonValue>>& Values, int32 ExpectedNum, TArray<double>& OutValues) const
{
	if (Values.Num() != ExpectedNum)
	{
		return false;
	}

	OutValues.Reserve(ExpectedNum);
	for (const TSharedPtr<FJsonValue>& Value : Values)
	{
		OutValues.Add(Value->AsNumber());
	}
	return true;
}

#undef LOCTEXT_NAMESPACE
