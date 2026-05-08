#pragma once

#include "CoreMinimal.h"
#include "Input/Reply.h"
#include "Widgets/SCompoundWidget.h"
#include "MotionBERTLiveLinkConnectionSettings.h"

DECLARE_DELEGATE_OneParam(FOnMotionBERTConnectionSettingsAccepted, FMotionBERTLiveLinkConnectionSettings)

class SEditableTextBox;
template<typename NumericType>
class SNumericEntryBox;

class SMotionBERTLiveLinkSourceFactory : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMotionBERTLiveLinkSourceFactory) {}
		SLATE_EVENT(FOnMotionBERTConnectionSettingsAccepted, OnConnectionSettingsAccepted)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	FReply OnOkClicked();
	FReply OnCancelClicked();
	void OnPortChanged(int32 InValue);
	TOptional<int32> GetPort() const;

private:
	FOnMotionBERTConnectionSettingsAccepted OnConnectionSettingsAccepted;
	TSharedPtr<SEditableTextBox> BindAddressTextBox;
	TSharedPtr<SEditableTextBox> SubjectNameTextBox;
	int32 PortValue = 7001;
};
