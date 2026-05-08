// Copyright Epic Games, Inc. All Rights Reserved.

#include "MotionBERTDebugSkeletonActor.h"

#include "Components/BillboardComponent.h"
#include "DrawDebugHelpers.h"
#include "Features/IModularFeatures.h"
#include "ILiveLinkClient.h"
#include "MotionBERT_UE.h"
#include "Roles/LiveLinkAnimationRole.h"
#include "Roles/LiveLinkAnimationTypes.h"

namespace MotionBERTDebugSkeleton
{
	struct FBoneEdge
	{
		int32 StartIndex;
		int32 EndIndex;
		FColor Color;
	};

	static const FBoneEdge BoneEdges[] =
	{
		{0, 1, FColor(255, 160, 70)},
		{1, 2, FColor(255, 160, 70)},
		{2, 3, FColor(255, 160, 70)},
		{0, 4, FColor(80, 220, 140)},
		{4, 5, FColor(80, 220, 140)},
		{5, 6, FColor(80, 220, 140)},
		{0, 7, FColor(220, 220, 220)},
		{7, 8, FColor(220, 220, 220)},
		{8, 9, FColor(220, 220, 220)},
		{9, 10, FColor(220, 220, 220)},
		{8, 11, FColor(80, 220, 140)},
		{11, 12, FColor(80, 220, 140)},
		{12, 13, FColor(80, 220, 140)},
		{8, 14, FColor(255, 160, 70)},
		{14, 15, FColor(255, 160, 70)},
		{15, 16, FColor(255, 160, 70)},
	};

	static const int32 JointParents[] =
	{
		-1,
		0, 1, 2,
		0, 4, 5,
		0, 7, 8, 9,
		8, 11, 12,
		8, 14, 15
	};

	static const TCHAR* JointNames[] =
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
		TEXT("right_wrist"),
	};
}

AMotionBERTDebugSkeletonActor::AMotionBERTDebugSkeletonActor()
{
	PrimaryActorTick.bCanEverTick = true;

	BillboardComponent = CreateDefaultSubobject<UBillboardComponent>(TEXT("Billboard"));
	SetRootComponent(BillboardComponent);

	LiveLinkSubjectName = FLiveLinkSubjectName(FName(TEXT("MotionBERTPose")));
	JointRadius = 3.0f;
	BoneThickness = 2.5f;
	WorldOffset = FVector::ZeroVector;
	bCenterOnRoot = false;
	bDrawJointLabels = false;
	bLoggedMissingSubject = false;
}

void AMotionBERTDebugSkeletonActor::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogMotionBERT_UE, Log, TEXT("MotionBERT debug skeleton actor started. Subject=%s"), *LiveLinkSubjectName.Name.ToString());
}

void AMotionBERTDebugSkeletonActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	TArray<FVector> ComponentPositions;
	if (EvaluateLiveLinkFrame(ComponentPositions))
	{
		bLoggedMissingSubject = false;
		DrawSkeleton(ComponentPositions);
	}
	else
	{
		DrawWaitingState();
	}
}

bool AMotionBERTDebugSkeletonActor::EvaluateLiveLinkFrame(TArray<FVector>& OutComponentPositions)
{
	IModularFeatures& ModularFeatures = IModularFeatures::Get();
	if (!ModularFeatures.IsModularFeatureAvailable(ILiveLinkClient::ModularFeatureName))
	{
		return false;
	}

	ILiveLinkClient& LiveLinkClient = ModularFeatures.GetModularFeature<ILiveLinkClient>(ILiveLinkClient::ModularFeatureName);
	if (!LiveLinkClient.DoesSubjectSupportsRole_AnyThread(LiveLinkSubjectName, ULiveLinkAnimationRole::StaticClass()))
	{
		if (!bLoggedMissingSubject)
		{
			UE_LOG(LogMotionBERT_UE, Warning, TEXT("MotionBERT debug skeleton: subject '%s' is not available yet."), *LiveLinkSubjectName.Name.ToString());
		}
		bLoggedMissingSubject = true;
		return false;
	}

	FLiveLinkSubjectFrameData SubjectFrameData;
	if (!LiveLinkClient.EvaluateFrame_AnyThread(LiveLinkSubjectName, ULiveLinkAnimationRole::StaticClass(), SubjectFrameData))
	{
		return false;
	}

	const FLiveLinkSkeletonStaticData* SkeletonData = SubjectFrameData.StaticData.Cast<FLiveLinkSkeletonStaticData>();
	const FLiveLinkAnimationFrameData* FrameData = SubjectFrameData.FrameData.Cast<FLiveLinkAnimationFrameData>();
	if (SkeletonData == nullptr || FrameData == nullptr)
	{
		return false;
	}

	const TArray<FTransform>& LocalTransforms = FrameData->Transforms;
	const TArray<int32>& Parents = SkeletonData->GetBoneParents();
	if (LocalTransforms.Num() == 0 || LocalTransforms.Num() != Parents.Num())
	{
		return false;
	}

	OutComponentPositions.SetNum(LocalTransforms.Num());
	for (int32 BoneIndex = 0; BoneIndex < LocalTransforms.Num(); ++BoneIndex)
	{
		const int32 ParentIndex = Parents[BoneIndex];
		const FVector LocalTranslation = LocalTransforms[BoneIndex].GetTranslation();
		OutComponentPositions[BoneIndex] = ParentIndex != INDEX_NONE
			? OutComponentPositions[ParentIndex] + LocalTranslation
			: LocalTranslation;
	}

	if (bCenterOnRoot && OutComponentPositions.Num() > 0)
	{
		const FVector RootPosition = OutComponentPositions[0];
		for (FVector& JointPosition : OutComponentPositions)
		{
			JointPosition -= RootPosition;
		}
	}

	return true;
}

void AMotionBERTDebugSkeletonActor::DrawSkeleton(const TArray<FVector>& ComponentPositions) const
{
	const UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	const FVector BaseOffset = GetActorLocation() + WorldOffset;

	for (const MotionBERTDebugSkeleton::FBoneEdge& Edge : MotionBERTDebugSkeleton::BoneEdges)
	{
		if (!ComponentPositions.IsValidIndex(Edge.StartIndex) || !ComponentPositions.IsValidIndex(Edge.EndIndex))
		{
			continue;
		}

		const FVector Start = BaseOffset + ComponentPositions[Edge.StartIndex];
		const FVector End = BaseOffset + ComponentPositions[Edge.EndIndex];
		DrawDebugLine(World, Start, End, Edge.Color, false, 0.0f, 1, BoneThickness);
	}

	for (int32 JointIndex = 0; JointIndex < ComponentPositions.Num(); ++JointIndex)
	{
		const FVector JointLocation = BaseOffset + ComponentPositions[JointIndex];
		const bool bIsRoot = MotionBERTDebugSkeleton::JointParents[JointIndex] == INDEX_NONE;
		const FColor JointColor = bIsRoot ? FColor(255, 90, 90) : FColor(240, 240, 240);
		const float Radius = bIsRoot ? JointRadius * 1.45f : JointRadius;
		DrawDebugSphere(World, JointLocation, Radius, 10, JointColor, false, 0.0f, 1, 1.0f);

		if (bDrawJointLabels)
		{
			DrawDebugString(
				World,
				JointLocation + FVector(0.0f, 0.0f, 8.0f),
				MotionBERTDebugSkeleton::JointNames[JointIndex],
				nullptr,
				FColor::White,
				0.0f,
				false
			);
		}
	}
}

void AMotionBERTDebugSkeletonActor::DrawWaitingState() const
{
	const UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	const FVector Anchor = GetActorLocation() + WorldOffset;
	DrawDebugSphere(World, Anchor, JointRadius * 1.5f, 12, FColor(180, 180, 180), false, 0.0f, 1, 1.0f);
	DrawDebugString(World, Anchor + FVector(0.0f, 0.0f, 20.0f), TEXT("Waiting for MotionBERTPose"), nullptr, FColor(200, 200, 200), 0.0f, false);
}
