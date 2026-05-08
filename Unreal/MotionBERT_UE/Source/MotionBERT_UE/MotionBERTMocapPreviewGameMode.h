// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MotionBERT_UEGameMode.h"

#include "MotionBERTMocapPreviewGameMode.generated.h"

class AMotionBERTDebugSkeletonActor;

UCLASS()
class AMotionBERTMocapPreviewGameMode : public AMotionBERT_UEGameMode
{
	GENERATED_BODY()

public:
	AMotionBERTMocapPreviewGameMode();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "MotionBERT|Debug")
	TSubclassOf<AMotionBERTDebugSkeletonActor> DebugSkeletonActorClass;

	UPROPERTY(EditAnywhere, Category = "MotionBERT|Debug")
	FTransform DebugSkeletonSpawnTransform;
};
