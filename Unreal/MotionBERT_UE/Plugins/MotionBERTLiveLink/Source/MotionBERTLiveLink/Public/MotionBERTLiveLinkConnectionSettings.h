#pragma once

#include "CoreMinimal.h"
#include "MotionBERTLiveLinkConnectionSettings.generated.h"

USTRUCT(BlueprintType)
struct FMotionBERTLiveLinkConnectionSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connection")
	FString BindAddress = TEXT("0.0.0.0");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connection")
	int32 UdpPort = 7001;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connection")
	FString DefaultSubjectName = TEXT("MotionBERTPose");
};
