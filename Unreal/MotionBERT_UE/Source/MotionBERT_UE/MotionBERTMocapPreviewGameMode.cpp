// Copyright Epic Games, Inc. All Rights Reserved.

#include "MotionBERTMocapPreviewGameMode.h"

#include "MotionBERTDebugSkeletonActor.h"
#include "MotionBERT_UEPlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/SpectatorPawn.h"
#include "MotionBERT_UE.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AMotionBERTMocapPreviewGameMode::AMotionBERTMocapPreviewGameMode()
{
	DefaultPawnClass = ASpectatorPawn::StaticClass();
	DebugSkeletonActorClass = AMotionBERTDebugSkeletonActor::StaticClass();
	DebugSkeletonSpawnTransform = FTransform(FRotator::ZeroRotator, FVector(0.0f, 0.0f, 120.0f), FVector::OneVector);

	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerClassFinder(
		TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonPlayerController")
	);
	if (PlayerControllerClassFinder.Succeeded())
	{
		PlayerControllerClass = PlayerControllerClassFinder.Class;
	}
	else
	{
		PlayerControllerClass = AMotionBERT_UEPlayerController::StaticClass();
	}
}

void AMotionBERTMocapPreviewGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (DebugSkeletonActorClass != nullptr && GetWorld() != nullptr)
	{
		FTransform SpawnTransform = DebugSkeletonSpawnTransform;

		if (AActor* PlayerStart = UGameplayStatics::GetActorOfClass(GetWorld(), APlayerStart::StaticClass()))
		{
			const FVector BaseLocation = PlayerStart->GetActorLocation();
			const FVector Forward = PlayerStart->GetActorForwardVector();
			SpawnTransform.SetLocation(BaseLocation + Forward * 250.0f + FVector(0.0f, 0.0f, 110.0f));
		}

		GetWorld()->SpawnActor<AMotionBERTDebugSkeletonActor>(DebugSkeletonActorClass, SpawnTransform);
		UE_LOG(LogMotionBERT_UE, Log, TEXT("MotionBERT mocap preview now uses the debug skeleton actor instead of Manny. Spawn=%s"), *SpawnTransform.GetLocation().ToString());
	}
}
