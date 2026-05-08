#include "MotionBERTLiveLinkRetargetAsset.h"

#include "BonePose.h"
#include "Math/RotationMatrix.h"
#include "Roles/LiveLinkAnimationTypes.h"

namespace MotionBERTLiveLinkRetarget
{
	struct FDirectionMapping
	{
		FName TargetBone;
		FName TargetChildBone;
		FName SourceStart;
		FName SourceEnd;
		float RotationAlpha;
	};

	struct FEndBoneMapping
	{
		FName TargetBone;
		FName SourceStart;
		FName SourceEnd;
		float RotationAlpha;
	};

	struct FRetargetContext
	{
		TMap<FName, int32> SourceIndexByName;
		TMap<FName, FCompactPoseBoneIndex> TargetIndexByName;
		TArray<FTransform> SourceComponentTransforms;
		TArray<FTransform> ReferenceComponentTransforms;
		TArray<FTransform> OutputComponentTransforms;
	};

	static const FName PelvisName(TEXT("pelvis"));
	static const FName SpineName(TEXT("spine"));
	static const FName ThoraxName(TEXT("thorax"));
	static const FName NoseName(TEXT("nose"));
	static const FName HeadName(TEXT("head"));
	static const FName LeftHipName(TEXT("left_hip"));
	static const FName LeftKneeName(TEXT("left_knee"));
	static const FName LeftAnkleName(TEXT("left_ankle"));
	static const FName RightHipName(TEXT("right_hip"));
	static const FName RightKneeName(TEXT("right_knee"));
	static const FName RightAnkleName(TEXT("right_ankle"));
	static const FName LeftShoulderName(TEXT("left_shoulder"));
	static const FName LeftElbowName(TEXT("left_elbow"));
	static const FName LeftWristName(TEXT("left_wrist"));
	static const FName RightShoulderName(TEXT("right_shoulder"));
	static const FName RightElbowName(TEXT("right_elbow"));
	static const FName RightWristName(TEXT("right_wrist"));

	static const TArray<FDirectionMapping> DirectionMappings =
	{
		{FName(TEXT("spine_01")), FName(TEXT("spine_02")), PelvisName, SpineName, 0.35f},
		{FName(TEXT("spine_02")), FName(TEXT("spine_03")), SpineName, ThoraxName, 0.45f},
		{FName(TEXT("spine_03")), FName(TEXT("neck_01")), ThoraxName, HeadName, 0.55f},
		{FName(TEXT("neck_01")), FName(TEXT("head")), ThoraxName, HeadName, 0.65f},
		{FName(TEXT("clavicle_l")), FName(TEXT("upperarm_l")), ThoraxName, LeftShoulderName, 0.55f},
		{FName(TEXT("upperarm_l")), FName(TEXT("lowerarm_l")), LeftShoulderName, LeftElbowName, 1.0f},
		{FName(TEXT("lowerarm_l")), FName(TEXT("hand_l")), LeftElbowName, LeftWristName, 1.0f},
		{FName(TEXT("clavicle_r")), FName(TEXT("upperarm_r")), ThoraxName, RightShoulderName, 0.55f},
		{FName(TEXT("upperarm_r")), FName(TEXT("lowerarm_r")), RightShoulderName, RightElbowName, 1.0f},
		{FName(TEXT("lowerarm_r")), FName(TEXT("hand_r")), RightElbowName, RightWristName, 1.0f},
		{FName(TEXT("thigh_l")), FName(TEXT("calf_l")), LeftHipName, LeftKneeName, 1.0f},
		{FName(TEXT("calf_l")), FName(TEXT("foot_l")), LeftKneeName, LeftAnkleName, 1.0f},
		{FName(TEXT("thigh_r")), FName(TEXT("calf_r")), RightHipName, RightKneeName, 1.0f},
		{FName(TEXT("calf_r")), FName(TEXT("foot_r")), RightKneeName, RightAnkleName, 1.0f},
	};

	static const TArray<FEndBoneMapping> EndBoneMappings =
	{
		{FName(TEXT("head")), ThoraxName, HeadName, 0.75f},
		{FName(TEXT("hand_l")), LeftElbowName, LeftWristName, 1.0f},
		{FName(TEXT("hand_r")), RightElbowName, RightWristName, 1.0f},
		{FName(TEXT("foot_l")), LeftKneeName, LeftAnkleName, 0.85f},
		{FName(TEXT("foot_r")), RightKneeName, RightAnkleName, 0.85f},
	};

	FQuat ComputeBoneDirectionDelta(const FVector& ReferenceDirection, const FVector& DesiredDirection)
	{
		const FVector ReferenceNormal = ReferenceDirection.GetSafeNormal();
		const FVector DesiredNormal = DesiredDirection.GetSafeNormal();
		if (ReferenceNormal.IsNearlyZero() || DesiredNormal.IsNearlyZero())
		{
			return FQuat::Identity;
		}

		return FQuat::FindBetweenNormals(ReferenceNormal, DesiredNormal);
	}

	bool TryGetSourceComponentPosition(const FRetargetContext& Context, FName BoneName, FVector& OutPosition)
	{
		const int32* SourceIndex = Context.SourceIndexByName.Find(BoneName);
		if (SourceIndex == nullptr || !Context.SourceComponentTransforms.IsValidIndex(*SourceIndex))
		{
			return false;
		}

		OutPosition = Context.SourceComponentTransforms[*SourceIndex].GetTranslation();
		return true;
	}

	bool TryGetTargetIndex(const FRetargetContext& Context, FName BoneName, FCompactPoseBoneIndex& OutIndex)
	{
		const FCompactPoseBoneIndex* FoundIndex = Context.TargetIndexByName.Find(BoneName);
		if (FoundIndex == nullptr || *FoundIndex == INDEX_NONE)
		{
			return false;
		}

		OutIndex = *FoundIndex;
		return true;
	}

	bool TryApplyDirectedBone(
		const FDirectionMapping& Mapping,
		const FCompactPose& OutPose,
		const TArray<FTransform>& ReferenceLocalTransforms,
		FRetargetContext& Context
	)
	{
		FCompactPoseBoneIndex BoneIndex(INDEX_NONE);
		if (!TryGetTargetIndex(Context, Mapping.TargetBone, BoneIndex))
		{
			return false;
		}

		FVector SourceStart = FVector::ZeroVector;
		FVector SourceEnd = FVector::ZeroVector;
		if (!TryGetSourceComponentPosition(Context, Mapping.SourceStart, SourceStart) ||
			!TryGetSourceComponentPosition(Context, Mapping.SourceEnd, SourceEnd))
		{
			return false;
		}

		const FVector DesiredDirection = SourceEnd - SourceStart;
		if (DesiredDirection.SizeSquared() < KINDA_SMALL_NUMBER)
		{
			return false;
		}

		FCompactPoseBoneIndex ChildIndex(INDEX_NONE);
		if (!TryGetTargetIndex(Context, Mapping.TargetChildBone, ChildIndex))
		{
			return false;
		}

		const FVector ReferenceBonePosition = Context.ReferenceComponentTransforms[BoneIndex.GetInt()].GetTranslation();
		const FVector ReferenceChildPosition = Context.ReferenceComponentTransforms[ChildIndex.GetInt()].GetTranslation();
		const FVector ReferenceDirection = ReferenceChildPosition - ReferenceBonePosition;
		if (ReferenceDirection.SizeSquared() < KINDA_SMALL_NUMBER)
		{
			return false;
		}

		const FCompactPoseBoneIndex ParentIndex = OutPose.GetParentBoneIndex(BoneIndex);
		const FQuat ParentWorldRotation = ParentIndex != INDEX_NONE
			? Context.OutputComponentTransforms[ParentIndex.GetInt()].GetRotation()
			: FQuat::Identity;

		const FQuat ReferenceWorldRotation = Context.ReferenceComponentTransforms[BoneIndex.GetInt()].GetRotation();
		const FQuat DeltaRotation = ComputeBoneDirectionDelta(ReferenceDirection, DesiredDirection);
		const FQuat WeightedDeltaRotation = FQuat::Slerp(FQuat::Identity, DeltaRotation, Mapping.RotationAlpha).GetNormalized();
		const FQuat DesiredWorldRotation = WeightedDeltaRotation * ReferenceWorldRotation;
		FTransform NewLocalTransform = ReferenceLocalTransforms[BoneIndex.GetInt()];
		NewLocalTransform.SetRotation((DesiredWorldRotation * ParentWorldRotation.Inverse()).GetNormalized());
		Context.OutputComponentTransforms[BoneIndex.GetInt()] = ParentIndex != INDEX_NONE
			? NewLocalTransform * Context.OutputComponentTransforms[ParentIndex.GetInt()]
			: NewLocalTransform;
		return true;
	}

	bool TryApplyEndBone(
		const FEndBoneMapping& Mapping,
		const FCompactPose& OutPose,
		const TArray<FTransform>& ReferenceLocalTransforms,
		FRetargetContext& Context
	)
	{
		FCompactPoseBoneIndex BoneIndex(INDEX_NONE);
		if (!TryGetTargetIndex(Context, Mapping.TargetBone, BoneIndex))
		{
			return false;
		}

		const FCompactPoseBoneIndex ParentIndex = OutPose.GetParentBoneIndex(BoneIndex);
		if (ParentIndex == INDEX_NONE)
		{
			return false;
		}

		FVector SourceStart = FVector::ZeroVector;
		FVector SourceEnd = FVector::ZeroVector;
		if (!TryGetSourceComponentPosition(Context, Mapping.SourceStart, SourceStart) ||
			!TryGetSourceComponentPosition(Context, Mapping.SourceEnd, SourceEnd))
		{
			return false;
		}

		const FVector DesiredDirection = SourceEnd - SourceStart;
		if (DesiredDirection.SizeSquared() < KINDA_SMALL_NUMBER)
		{
			return false;
		}

		const FVector ReferenceParentPosition = Context.ReferenceComponentTransforms[ParentIndex.GetInt()].GetTranslation();
		const FVector ReferenceBonePosition = Context.ReferenceComponentTransforms[BoneIndex.GetInt()].GetTranslation();
		const FVector ReferenceDirection = ReferenceBonePosition - ReferenceParentPosition;
		if (ReferenceDirection.SizeSquared() < KINDA_SMALL_NUMBER)
		{
			return false;
		}

		const FQuat ParentWorldRotation = Context.OutputComponentTransforms[ParentIndex.GetInt()].GetRotation();
		const FQuat ReferenceWorldRotation = Context.ReferenceComponentTransforms[BoneIndex.GetInt()].GetRotation();
		const FQuat DeltaRotation = ComputeBoneDirectionDelta(ReferenceDirection, DesiredDirection);
		const FQuat WeightedDeltaRotation = FQuat::Slerp(FQuat::Identity, DeltaRotation, Mapping.RotationAlpha).GetNormalized();
		const FQuat DesiredWorldRotation = WeightedDeltaRotation * ReferenceWorldRotation;

		FTransform NewLocalTransform = ReferenceLocalTransforms[BoneIndex.GetInt()];
		NewLocalTransform.SetRotation((DesiredWorldRotation * ParentWorldRotation.Inverse()).GetNormalized());
		Context.OutputComponentTransforms[BoneIndex.GetInt()] = NewLocalTransform * Context.OutputComponentTransforms[ParentIndex.GetInt()];
		return true;
	}

	bool TryApplyPelvis(
		const FCompactPose& OutPose,
		const TArray<FTransform>& ReferenceLocalTransforms,
		FRetargetContext& Context,
		bool bLockRootTranslation,
		float RootTranslationScale
	)
	{
		FCompactPoseBoneIndex PelvisIndex(INDEX_NONE);
		if (!TryGetTargetIndex(Context, FName(TEXT("pelvis")), PelvisIndex))
		{
			return false;
		}

		FVector SourceLeftHip = FVector::ZeroVector;
		FVector SourceRightHip = FVector::ZeroVector;
		FVector SourceSpine = FVector::ZeroVector;
		if (!TryGetSourceComponentPosition(Context, LeftHipName, SourceLeftHip) ||
			!TryGetSourceComponentPosition(Context, RightHipName, SourceRightHip) ||
			!TryGetSourceComponentPosition(Context, SpineName, SourceSpine))
		{
			return false;
		}

		const FVector SourceUp = (SourceSpine - 0.5f * (SourceLeftHip + SourceRightHip)).GetSafeNormal();
		const FVector SourceRight = (SourceRightHip - SourceLeftHip).GetSafeNormal();
		const FVector SourceForward = FVector::CrossProduct(SourceUp, SourceRight).GetSafeNormal();
		if (SourceUp.IsNearlyZero() || SourceRight.IsNearlyZero() || SourceForward.IsNearlyZero())
		{
			return false;
		}

		FCompactPoseBoneIndex SpineIndex(INDEX_NONE);
		FCompactPoseBoneIndex LeftThighIndex(INDEX_NONE);
		FCompactPoseBoneIndex RightThighIndex(INDEX_NONE);
		if (!TryGetTargetIndex(Context, FName(TEXT("spine_01")), SpineIndex) ||
			!TryGetTargetIndex(Context, FName(TEXT("thigh_l")), LeftThighIndex) ||
			!TryGetTargetIndex(Context, FName(TEXT("thigh_r")), RightThighIndex))
		{
			return false;
		}

		const FVector ReferencePelvisPosition = Context.ReferenceComponentTransforms[PelvisIndex.GetInt()].GetTranslation();
		const FVector ReferenceSpinePosition = Context.ReferenceComponentTransforms[SpineIndex.GetInt()].GetTranslation();
		const FVector ReferenceLeftThighPosition = Context.ReferenceComponentTransforms[LeftThighIndex.GetInt()].GetTranslation();
		const FVector ReferenceRightThighPosition = Context.ReferenceComponentTransforms[RightThighIndex.GetInt()].GetTranslation();

		const FVector ReferenceUp = (ReferenceSpinePosition - ReferencePelvisPosition).GetSafeNormal();
		const FVector ReferenceRight = (ReferenceRightThighPosition - ReferenceLeftThighPosition).GetSafeNormal();
		const FVector ReferenceForward = FVector::CrossProduct(ReferenceUp, ReferenceRight).GetSafeNormal();
		if (ReferenceUp.IsNearlyZero() || ReferenceRight.IsNearlyZero() || ReferenceForward.IsNearlyZero())
		{
			return false;
		}

		const FMatrix ReferenceBasis = FRotationMatrix::MakeFromXZ(ReferenceForward, ReferenceUp);
		const FMatrix DesiredBasis = FRotationMatrix::MakeFromXZ(SourceForward, SourceUp);

		const FQuat ReferenceBasisQuat(ReferenceBasis);
		const FQuat DesiredBasisQuat(DesiredBasis);
		const FQuat PelvisWorldRotation = DesiredBasisQuat * ReferenceBasisQuat.Inverse() *
			Context.ReferenceComponentTransforms[PelvisIndex.GetInt()].GetRotation();

		FTransform PelvisLocalTransform = ReferenceLocalTransforms[PelvisIndex.GetInt()];
		PelvisLocalTransform.SetRotation(PelvisWorldRotation.GetNormalized());
		if (!bLockRootTranslation)
		{
			const FVector SourcePelvis = 0.5f * (SourceLeftHip + SourceRightHip);
			const FVector ReferenceSourcePelvis = Context.SourceComponentTransforms[Context.SourceIndexByName[PelvisName]].GetTranslation();
			const FVector Delta = (SourcePelvis - ReferenceSourcePelvis) * RootTranslationScale;
			PelvisLocalTransform.SetTranslation(PelvisLocalTransform.GetTranslation() + Delta);
		}

		Context.OutputComponentTransforms[PelvisIndex.GetInt()] = PelvisLocalTransform;
		return true;
	}

	void BuildSourceIndex(const FLiveLinkSkeletonStaticData* InSkeletonData, FRetargetContext& Context)
	{
		Context.SourceIndexByName.Reset();
		const TArray<FName>& BoneNames = InSkeletonData->GetBoneNames();
		for (int32 BoneIndex = 0; BoneIndex < BoneNames.Num(); ++BoneIndex)
		{
			Context.SourceIndexByName.Add(BoneNames[BoneIndex], BoneIndex);
		}
	}

	void BuildTargetIndex(const FCompactPose& OutPose, FRetargetContext& Context)
	{
		Context.TargetIndexByName.Reset();
		const FBoneContainer& BoneContainer = OutPose.GetBoneContainer();
		for (const FCompactPoseBoneIndex BoneIndex : OutPose.ForEachBoneIndex())
		{
			const FName BoneName = BoneContainer.GetReferenceSkeleton().GetBoneName(BoneContainer.MakeMeshPoseIndex(BoneIndex).GetInt());
			Context.TargetIndexByName.Add(BoneName, BoneIndex);
		}
	}

	void BuildSourceComponentTransforms(
		const FLiveLinkSkeletonStaticData* InSkeletonData,
		const FLiveLinkAnimationFrameData* InFrameData,
		FRetargetContext& Context
	)
	{
		const TArray<FTransform>& LocalTransforms = InFrameData->Transforms;
		const TArray<int32>& Parents = InSkeletonData->GetBoneParents();

		Context.SourceComponentTransforms.SetNum(LocalTransforms.Num());
		for (int32 BoneIndex = 0; BoneIndex < LocalTransforms.Num(); ++BoneIndex)
		{
			const int32 ParentIndex = Parents.IsValidIndex(BoneIndex) ? Parents[BoneIndex] : INDEX_NONE;
			Context.SourceComponentTransforms[BoneIndex] = ParentIndex != INDEX_NONE
				? LocalTransforms[BoneIndex] * Context.SourceComponentTransforms[ParentIndex]
				: LocalTransforms[BoneIndex];
		}
	}

	void BuildReferenceTransforms(const FCompactPose& OutPose, TArray<FTransform>& OutReferenceLocal, TArray<FTransform>& OutReferenceComponent)
	{
		OutReferenceLocal.SetNum(OutPose.GetNumBones());
		OutReferenceComponent.SetNum(OutPose.GetNumBones());

		for (const FCompactPoseBoneIndex BoneIndex : OutPose.ForEachBoneIndex())
		{
			OutReferenceLocal[BoneIndex.GetInt()] = OutPose.GetRefPose(BoneIndex);
			const FCompactPoseBoneIndex ParentIndex = OutPose.GetParentBoneIndex(BoneIndex);
			OutReferenceComponent[BoneIndex.GetInt()] = ParentIndex != INDEX_NONE
				? OutReferenceLocal[BoneIndex.GetInt()] * OutReferenceComponent[ParentIndex.GetInt()]
				: OutReferenceLocal[BoneIndex.GetInt()];
		}
	}
}

UMotionBERTLiveLinkRetargetAsset::UMotionBERTLiveLinkRetargetAsset(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UMotionBERTLiveLinkRetargetAsset::BuildPoseFromAnimationData(
	float DeltaTime,
	const FLiveLinkSkeletonStaticData* InSkeletonData,
	const FLiveLinkAnimationFrameData* InFrameData,
	FCompactPose& OutPose
)
{
	check(InSkeletonData);
	check(InFrameData);

	using namespace MotionBERTLiveLinkRetarget;

	FRetargetContext Context;
	TArray<FTransform> ReferenceLocalTransforms;

	BuildSourceIndex(InSkeletonData, Context);
	BuildTargetIndex(OutPose, Context);
	BuildSourceComponentTransforms(InSkeletonData, InFrameData, Context);
	BuildReferenceTransforms(OutPose, ReferenceLocalTransforms, Context.ReferenceComponentTransforms);
	Context.OutputComponentTransforms = Context.ReferenceComponentTransforms;

	TryApplyPelvis(OutPose, ReferenceLocalTransforms, Context, bLockRootTranslation, RootTranslationScale);
	for (const FDirectionMapping& Mapping : DirectionMappings)
	{
		TryApplyDirectedBone(Mapping, OutPose, ReferenceLocalTransforms, Context);
	}
	for (const FEndBoneMapping& Mapping : EndBoneMappings)
	{
		TryApplyEndBone(Mapping, OutPose, ReferenceLocalTransforms, Context);
	}

	for (const FCompactPoseBoneIndex BoneIndex : OutPose.ForEachBoneIndex())
	{
		const FCompactPoseBoneIndex ParentIndex = OutPose.GetParentBoneIndex(BoneIndex);
		const FTransform LocalTransform = ParentIndex != INDEX_NONE
			? Context.OutputComponentTransforms[BoneIndex.GetInt()].GetRelativeTransform(Context.OutputComponentTransforms[ParentIndex.GetInt()])
			: Context.OutputComponentTransforms[BoneIndex.GetInt()];
		OutPose[BoneIndex] = LocalTransform;
	}
}
