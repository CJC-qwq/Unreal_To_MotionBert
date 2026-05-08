#pragma once

#include "LiveLinkSourceFactory.h"
#include "MotionBERTLiveLinkConnectionSettings.h"
#include "MotionBERTLiveLinkSourceFactory.generated.h"

class ILiveLinkSource;
class SWidget;

UCLASS()
class MOTIONBERTLIVELINK_API UMotionBERTLiveLinkSourceFactory : public ULiveLinkSourceFactory
{
	GENERATED_BODY()

public:
	virtual FText GetSourceDisplayName() const override;
	virtual FText GetSourceTooltip() const override;
	virtual EMenuType GetMenuType() const override { return EMenuType::SubPanel; }
	virtual TSharedPtr<SWidget> BuildCreationPanel(FOnLiveLinkSourceCreated OnLiveLinkSourceCreated) const override;
	virtual TSharedPtr<ILiveLinkSource> CreateSource(const FString& ConnectionString) const override;

private:
	void CreateSourceFromSettings(FMotionBERTLiveLinkConnectionSettings ConnectionSettings, FOnLiveLinkSourceCreated OnSourceCreated) const;
};
