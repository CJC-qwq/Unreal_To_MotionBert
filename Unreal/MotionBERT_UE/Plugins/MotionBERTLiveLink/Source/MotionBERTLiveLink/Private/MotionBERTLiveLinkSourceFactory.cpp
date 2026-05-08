#include "MotionBERTLiveLinkSourceFactory.h"

#include "MotionBERTLiveLinkSource.h"
#include "SMotionBERTLiveLinkSourceFactory.h"

#define LOCTEXT_NAMESPACE "MotionBERTLiveLinkSourceFactory"

FText UMotionBERTLiveLinkSourceFactory::GetSourceDisplayName() const
{
	return LOCTEXT("SourceDisplayName", "MotionBERT Live Link");
}

FText UMotionBERTLiveLinkSourceFactory::GetSourceTooltip() const
{
	return LOCTEXT("SourceTooltip", "Create a Live Link source that listens for MotionBERT skeletal data over UDP.");
}

TSharedPtr<SWidget> UMotionBERTLiveLinkSourceFactory::BuildCreationPanel(FOnLiveLinkSourceCreated InOnLiveLinkSourceCreated) const
{
	return SNew(SMotionBERTLiveLinkSourceFactory)
		.OnConnectionSettingsAccepted(FOnMotionBERTConnectionSettingsAccepted::CreateUObject(this, &UMotionBERTLiveLinkSourceFactory::CreateSourceFromSettings, InOnLiveLinkSourceCreated));
}

TSharedPtr<ILiveLinkSource> UMotionBERTLiveLinkSourceFactory::CreateSource(const FString& ConnectionString) const
{
	FMotionBERTLiveLinkConnectionSettings ConnectionSettings;
	if (!ConnectionString.IsEmpty())
	{
		FMotionBERTLiveLinkConnectionSettings::StaticStruct()->ImportText(*ConnectionString, &ConnectionSettings, nullptr, PPF_None, GLog, TEXT("UMotionBERTLiveLinkSourceFactory"));
	}

	return MakeShared<FMotionBERTLiveLinkSource>(ConnectionSettings);
}

void UMotionBERTLiveLinkSourceFactory::CreateSourceFromSettings(FMotionBERTLiveLinkConnectionSettings ConnectionSettings, FOnLiveLinkSourceCreated OnSourceCreated) const
{
	FString ConnectionString;
	FMotionBERTLiveLinkConnectionSettings::StaticStruct()->ExportText(ConnectionString, &ConnectionSettings, nullptr, nullptr, PPF_None, nullptr);

	TSharedPtr<FMotionBERTLiveLinkSource> SharedSource = MakeShared<FMotionBERTLiveLinkSource>(ConnectionSettings);
	OnSourceCreated.ExecuteIfBound(SharedSource, MoveTemp(ConnectionString));
}

#undef LOCTEXT_NAMESPACE
