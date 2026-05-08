// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LiveLinkTypes.h"

#include "MotionBERTDebugSkeletonActor.generated.h"

class UBillboardComponent;

UCLASS()
class AMotionBERTDebugSkeletonActor : public AActor
{
	GENERATED_BODY()

public:
	AMotionBERTDebugSkeletonActor();

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MotionBERT|Debug")
	TObjectPtr<UBillboardComponent> BillboardComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MotionBERT|Live Link")
	FLiveLinkSubjectName LiveLinkSubjectName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MotionBERT|Debug")
	float JointRadius;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MotionBERT|Debug")
	float BoneThickness;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MotionBERT|Debug")
	FVector WorldOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MotionBERT|Debug")
	bool bCenterOnRoot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MotionBERT|Debug")
	bool bDrawJointLabels;

private:
	bool EvaluateLiveLinkFrame(TArray<FVector>& OutComponentPositions);
	void DrawSkeleton(const TArray<FVector>& ComponentPositions) const;
	void DrawWaitingState() const;

	bool bLoggedMissingSubject;
};
