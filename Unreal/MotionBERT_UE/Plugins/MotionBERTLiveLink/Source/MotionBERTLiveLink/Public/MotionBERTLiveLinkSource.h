#pragma once

#include "ILiveLinkSource.h"
#include "MotionBERTLiveLinkConnectionSettings.h"

#include "Delegates/IDelegateInstance.h"
#include "HAL/Runnable.h"
#include "HAL/ThreadSafeBool.h"
#include "Interfaces/IPv4/IPv4Endpoint.h"
#include "Sockets.h"

class ILiveLinkClient;
class ISocketSubsystem;
class FRunnableThread;

class MOTIONBERTLIVELINK_API FMotionBERTLiveLinkSource : public ILiveLinkSource, public FRunnable, public TSharedFromThis<FMotionBERTLiveLinkSource>
{
public:
	explicit FMotionBERTLiveLinkSource(const FMotionBERTLiveLinkConnectionSettings& InConnectionSettings);
	virtual ~FMotionBERTLiveLinkSource();

	virtual void ReceiveClient(ILiveLinkClient* InClient, FGuid InSourceGuid) override;
	virtual bool IsSourceStillValid() const override;
	virtual bool RequestSourceShutdown() override;
	virtual FText GetSourceType() const override;
	virtual FText GetSourceMachineName() const override;
	virtual FText GetSourceStatus() const override;

	virtual bool Init() override { return true; }
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override {}

private:
	struct FSubjectPacket
	{
		FName SubjectName;
		TArray<FName> BoneNames;
		TArray<int32> BoneParents;
		TArray<FTransform> BoneTransforms;
		double TimestampSeconds = 0.0;
	};

	void Start();
	bool ParsePacket(const FString& PacketString, FSubjectPacket& OutPacket) const;
	bool ParseTransformArray(const TArray<TSharedPtr<FJsonValue>>& TransformValues, TArray<FTransform>& OutTransforms) const;
	bool ParsePositionArray(const TArray<TSharedPtr<FJsonValue>>& PositionValues, TArray<FTransform>& OutTransforms) const;
	void EnsureStaticDataSent(const FSubjectPacket& Packet);
	void SendFrameData(const FSubjectPacket& Packet);
	void FillDefaultSkeleton(FSubjectPacket& InOutPacket) const;
	bool ParseFloatArray(const TArray<TSharedPtr<FJsonValue>>& Values, int32 ExpectedNum, TArray<double>& OutValues) const;

private:
	ILiveLinkClient* Client = nullptr;
	FGuid SourceGuid;

	FMotionBERTLiveLinkConnectionSettings ConnectionSettings;
	FText SourceType;
	FText SourceMachineName;
	FText SourceStatus;

	FThreadSafeBool Stopping = false;
	FRunnableThread* Thread = nullptr;
	FString ThreadName;

	FSocket* Socket = nullptr;
	ISocketSubsystem* SocketSubsystem = nullptr;
	FIPv4Endpoint DeviceEndpoint;
	TArray<uint8> ReceiveBuffer;
	const uint32 ReceiveBufferSize = 1024 * 256;
	FDelegateHandle DeferredStartDelegateHandle;

	mutable FCriticalSection ClientAccessMutex;
	TSet<FName> EncounteredSubjects;
};
