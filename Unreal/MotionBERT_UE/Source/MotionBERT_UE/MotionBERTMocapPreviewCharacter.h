// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LiveLinkTypes.h"
#include "MotionBERT_UECharacter.h"

#include "MotionBERTMocapPreviewCharacter.generated.h"

class ULiveLinkComponent;
class ULiveLinkRetargetAsset;

UCLASS()
class AMotionBERTMocapPreviewCharacter : public AMotionBERT_UECharacter
{
	GENERATED_BODY()

public:
	AMotionBERTMocapPreviewCharacter();

protected:
	virtual void PostRegisterAllComponents() override;
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MotionBERT|Live Link")
	TObjectPtr<ULiveLinkComponent> LiveLinkComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MotionBERT|Live Link")
	FLiveLinkSubjectName LiveLinkSubjectName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MotionBERT|Live Link")
	TSubclassOf<ULiveLinkRetargetAsset> LiveLinkRetargetAssetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MotionBERT|Live Link")
	bool bLockRootTranslation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MotionBERT|Live Link", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float RootTranslationScale;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MotionBERT|Preview")
	bool bEnableManualInput;

private:
	void ConfigurePreviewMesh();
	bool ConfigureLiveLink();

	bool bLiveLinkConfigured;
};
