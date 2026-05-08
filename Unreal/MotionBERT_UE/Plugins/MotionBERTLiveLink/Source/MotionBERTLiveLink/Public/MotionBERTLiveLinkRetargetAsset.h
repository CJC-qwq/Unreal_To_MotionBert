#pragma once

#include "CoreMinimal.h"
#include "LiveLinkRetargetAsset.h"

#include "MotionBERTLiveLinkRetargetAsset.generated.h"

struct FCompactPose;
struct FLiveLinkAnimationFrameData;
struct FLiveLinkSkeletonStaticData;

UCLASS(Blueprintable)
class MOTIONBERTLIVELINK_API UMotionBERTLiveLinkRetargetAsset : public ULiveLinkRetargetAsset
{
	GENERATED_BODY()

public:
	UMotionBERTLiveLinkRetargetAsset(const FObjectInitializer& ObjectInitializer);

	virtual void BuildPoseFromAnimationData(
		float DeltaTime,
		const FLiveLinkSkeletonStaticData* InSkeletonData,
		const FLiveLinkAnimationFrameData* InFrameData,
		FCompactPose& OutPose
	) override;

	UPROPERTY(EditAnywhere, Category = "MotionBERT")
	bool bLockRootTranslation = true;

	UPROPERTY(EditAnywhere, Category = "MotionBERT", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float RootTranslationScale = 1.0f;
};
