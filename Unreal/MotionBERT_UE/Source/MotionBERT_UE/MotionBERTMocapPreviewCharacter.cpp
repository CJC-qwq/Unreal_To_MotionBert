// Copyright Epic Games, Inc. All Rights Reserved.

#include "MotionBERTMocapPreviewCharacter.h"

#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputAction.h"
#include "LiveLinkComponent.h"
#include "LiveLinkInstance.h"
#include "MotionBERT_UE.h"
#include "MotionBERTLiveLinkRetargetAsset.h"
#include "UObject/ConstructorHelpers.h"

AMotionBERTMocapPreviewCharacter::AMotionBERTMocapPreviewCharacter()
{
	LiveLinkComponent = CreateDefaultSubobject<ULiveLinkComponent>(TEXT("LiveLinkComponent"));

	LiveLinkSubjectName = FLiveLinkSubjectName(FName(TEXT("MotionBERTPose")));
	LiveLinkRetargetAssetClass = UMotionBERTLiveLinkRetargetAsset::StaticClass();
	bLockRootTranslation = true;
	RootTranslationScale = 1.0f;
	bEnableManualInput = false;
	bLiveLinkConfigured = false;

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MannyMeshFinder(
		TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple")
	);
	static ConstructorHelpers::FObjectFinder<UInputAction> JumpActionFinder(
		TEXT("/Game/Input/Actions/IA_Jump.IA_Jump")
	);
	static ConstructorHelpers::FObjectFinder<UInputAction> MoveActionFinder(
		TEXT("/Game/Input/Actions/IA_Move.IA_Move")
	);
	static ConstructorHelpers::FObjectFinder<UInputAction> LookActionFinder(
		TEXT("/Game/Input/Actions/IA_Look.IA_Look")
	);
	static ConstructorHelpers::FObjectFinder<UInputAction> MouseLookActionFinder(
		TEXT("/Game/Input/Actions/IA_MouseLook.IA_MouseLook")
	);

	if (MannyMeshFinder.Succeeded())
	{
		GetMesh()->SetSkeletalMeshAsset(MannyMeshFinder.Object);
	}
	if (JumpActionFinder.Succeeded())
	{
		JumpAction = JumpActionFinder.Object;
	}
	if (MoveActionFinder.Succeeded())
	{
		MoveAction = MoveActionFinder.Object;
	}
	if (LookActionFinder.Succeeded())
	{
		LookAction = LookActionFinder.Object;
	}
	if (MouseLookActionFinder.Succeeded())
	{
		MouseLookAction = MouseLookActionFinder.Object;
	}

	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -97.0f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh"));
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);

	GetCharacterMovement()->bOrientRotationToMovement = false;
	bUseControllerRotationYaw = true;
	PrimaryActorTick.bCanEverTick = true;
}

void AMotionBERTMocapPreviewCharacter::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();
	ConfigurePreviewMesh();
	ConfigureLiveLink();
}

void AMotionBERTMocapPreviewCharacter::BeginPlay()
{
	Super::BeginPlay();
	ConfigurePreviewMesh();
	bLiveLinkConfigured = ConfigureLiveLink();
}

void AMotionBERTMocapPreviewCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (bEnableManualInput)
	{
		Super::SetupPlayerInputComponent(PlayerInputComponent);
	}
}

void AMotionBERTMocapPreviewCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bLiveLinkConfigured)
	{
		ConfigurePreviewMesh();
		bLiveLinkConfigured = ConfigureLiveLink();
	}
}

void AMotionBERTMocapPreviewCharacter::ConfigurePreviewMesh()
{
	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (MeshComponent == nullptr)
	{
		return;
	}

	if (MeshComponent->GetAnimClass() != ULiveLinkInstance::StaticClass())
	{
		MeshComponent->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		MeshComponent->SetAnimInstanceClass(ULiveLinkInstance::StaticClass());
	}
}

bool AMotionBERTMocapPreviewCharacter::ConfigureLiveLink()
{
	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (MeshComponent == nullptr)
	{
		return false;
	}

	ULiveLinkInstance* LiveLinkAnimInstance = Cast<ULiveLinkInstance>(MeshComponent->GetAnimInstance());
	if (LiveLinkAnimInstance == nullptr)
	{
		return false;
	}

	LiveLinkAnimInstance->SetSubject(LiveLinkSubjectName);
	LiveLinkAnimInstance->SetRetargetAsset(LiveLinkRetargetAssetClass);
	LiveLinkAnimInstance->EnableLiveLinkEvaluation(true);

	UE_LOG(
		LogMotionBERT_UE,
		Log,
		TEXT("MotionBERT mocap preview configured. Subject=%s Retarget=%s AnimClass=%s"),
		*LiveLinkSubjectName.Name.ToString(),
		*GetNameSafe(LiveLinkRetargetAssetClass.Get()),
		*GetNameSafe(MeshComponent->GetAnimClass())
	);

	return true;
}
