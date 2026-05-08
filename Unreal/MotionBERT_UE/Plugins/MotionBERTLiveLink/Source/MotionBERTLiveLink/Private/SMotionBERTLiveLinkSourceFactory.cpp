#include "SMotionBERTLiveLinkSourceFactory.h"

#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "SMotionBERTLiveLinkSourceFactory"

void SMotionBERTLiveLinkSourceFactory::Construct(const FArguments& InArgs)
{
	OnConnectionSettingsAccepted = InArgs._OnConnectionSettingsAccepted;

	ChildSlot
	[
		SNew(SBox)
		.WidthOverride(360.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Description", "Listen for MotionBERT UDP packets and publish them as a Live Link animation subject."))
				.AutoWrapText(true)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 6.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("BindAddressLabel", "Bind Address"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 10.0f)
			[
				SAssignNew(BindAddressTextBox, SEditableTextBox)
				.Text(FText::FromString(TEXT("0.0.0.0")))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 6.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("PortLabel", "UDP Port"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 10.0f)
			[
				SNew(SNumericEntryBox<int32>)
				.Value(this, &SMotionBERTLiveLinkSourceFactory::GetPort)
				.OnValueChanged(this, &SMotionBERTLiveLinkSourceFactory::OnPortChanged)
				.MinValue(1)
				.MaxValue(65535)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 6.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("SubjectLabel", "Default Subject Name"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 12.0f)
			[
				SAssignNew(SubjectNameTextBox, SEditableTextBox)
				.Text(FText::FromString(TEXT("MotionBERTPose")))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Right)
			[
				SNew(SUniformGridPanel)
				.SlotPadding(FMargin(6.0f, 0.0f))
				+ SUniformGridPanel::Slot(0, 0)
				[
					SNew(SButton)
					.Text(LOCTEXT("Cancel", "Cancel"))
					.OnClicked(this, &SMotionBERTLiveLinkSourceFactory::OnCancelClicked)
				]
				+ SUniformGridPanel::Slot(1, 0)
				[
					SNew(SButton)
					.Text(LOCTEXT("Ok", "Create Source"))
					.OnClicked(this, &SMotionBERTLiveLinkSourceFactory::OnOkClicked)
				]
			]
		]
	];
}

FReply SMotionBERTLiveLinkSourceFactory::OnOkClicked()
{
	if (OnConnectionSettingsAccepted.IsBound())
	{
		FMotionBERTLiveLinkConnectionSettings Settings;
		Settings.BindAddress = BindAddressTextBox.IsValid() ? BindAddressTextBox->GetText().ToString() : TEXT("0.0.0.0");
		Settings.UdpPort = PortValue;
		Settings.DefaultSubjectName = SubjectNameTextBox.IsValid() ? SubjectNameTextBox->GetText().ToString() : TEXT("MotionBERTPose");
		OnConnectionSettingsAccepted.Execute(Settings);
	}

	return FReply::Handled();
}

FReply SMotionBERTLiveLinkSourceFactory::OnCancelClicked()
{
	return FReply::Handled();
}

void SMotionBERTLiveLinkSourceFactory::OnPortChanged(int32 InValue)
{
	PortValue = InValue;
}

TOptional<int32> SMotionBERTLiveLinkSourceFactory::GetPort() const
{
	return PortValue;
}

#undef LOCTEXT_NAMESPACE
