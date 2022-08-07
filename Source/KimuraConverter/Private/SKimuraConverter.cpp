//
// Copyright (c) Alexandre Hetu.
// Licensed under the MIT License.
//
// https://github.com/ahetu04
//

#include "SKimuraConverter.h"
#include "Widgets/Views/STableRow.h"
#include "Modules/ModuleManager.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Input/SComboBox.h"
#include "EditorStyleSet.h"
#include "Widgets/Images/SThrobber.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Templates/SharedPointer.h"
#include "EditorDirectories.h"
#include "EditorStyleSet.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SFilePathPicker.h"
#include "Widgets/Notifications/SProgressBar.h"

#include "KimuraConverterProperties.h"
#include <vector>
#include <string>

#define LOCTEXT_NAMESPACE "SKimuraConverter"

#if WITH_EDITOR
	PRAGMA_DISABLE_OPTIMIZATION
#endif

//-----------------------------------------------------------------------------
// SKimuraConverter::Construct
//-----------------------------------------------------------------------------
void SKimuraConverter::Construct(const FArguments& InArgs)
{

	OwnerWindow = InArgs._OwnerWindow;


	FSlateFontInfo Font = FEditorStyle::GetFontStyle(TEXT("SourceControl.LoginWindow.Font"));

	UKimuraConverterProperties* pProperties = UKimuraConverterProperties::GetDefault();

	// setup detail view 
	{
		FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		FDetailsViewArgs DetailsViewArgs;
		DetailsViewArgs.bAllowSearch = false;
		DetailsViewArgs.bUpdatesFromSelection = false;
		DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
		this->PropertiesDetailView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
		this->PropertiesDetailView->SetObject(pProperties);
	}

	this->ChildSlot
		[
			// display all the properties from ConverterSettings
			SNew(SVerticalBox)

			// show all properties as part of a detail view
			+ SVerticalBox::Slot()
				.Padding(2)
				.AutoHeight()
				.MaxHeight(800.0f)
				[
					this->PropertiesDetailView->AsShared()
				]

			// progress bar
			+ SVerticalBox::Slot()
				.Padding(2)
				.MaxHeight(15.0f)
				[
					SNew(SProgressBar)
					.RefreshRate(0.016)
					.Visibility(EVisibility::All)
					.ForceVolatile(true)
					.Percent_Lambda([&]()
					{
						return OnGetProgression();
					})
					.Visibility_Lambda([this]() -> EVisibility
					{
						return EVisibility::Visible;
					})
				]

			+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Right)
				.Padding(2)
				[
					SNew(SUniformGridPanel)
					.SlotPadding(1)
					+ SUniformGridPanel::Slot(0, 0)
					[
						SAssignNew(ConvertButton, SButton)
						.HAlign(HAlign_Center)
						.Text(LOCTEXT("convert!", "Convert!"))
						.IsEnabled(this, &SKimuraConverter::IsConvertButtonEnabled)
						.OnClicked(this, &SKimuraConverter::OnConvertPressed)
					]

					+ SUniformGridPanel::Slot(1, 0)
					[
						SNew(SButton)
						.HAlign(HAlign_Center)
						.Text(LOCTEXT("Cancel", "Cancel"))
						.IsEnabled(this, &SKimuraConverter::IsCancelButtonEnabled)
						.OnClicked(this, &SKimuraConverter::OnCancelPressed)
					]

					+ SUniformGridPanel::Slot(2, 0)
					[
						SNew(SButton)
						.HAlign(HAlign_Center)
						.Text(LOCTEXT("Close", "Close"))
						.IsEnabled(this, &SKimuraConverter::IsCloseButtonEnabled)
						.OnClicked(this, &SKimuraConverter::OnClosePressed)
					]
				]
		];

}


//-----------------------------------------------------------------------------
// SKimuraConverter::Tick
//-----------------------------------------------------------------------------
void SKimuraConverter::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	if (this->ConverterInstance)
	{
		// check if converter is still working
		if (!this->ConverterInstance->IsWorking())
		{
			//UE_LOG(KimuraLogConverter, Log, TEXT("DONE!"));

			delete this->ConverterInstance;
			this->ConverterInstance = nullptr;
		}
		else
		{
			//UE_LOG(KimuraLogConverter, Log, TEXT("Converting... "));

		}
	}
}


//-----------------------------------------------------------------------------
// SKimuraConverter::IsConvertButtonEnabled
//-----------------------------------------------------------------------------
bool SKimuraConverter::IsConvertButtonEnabled() const
{
	return this->ConverterInstance == nullptr;
}


//-----------------------------------------------------------------------------
// SKimuraConverter::OnConvertPressed
//-----------------------------------------------------------------------------
FReply SKimuraConverter::OnConvertPressed()
{
	UKimuraConverterProperties* pProperties = UKimuraConverterProperties::GetDefault();

	// if the output file valid?
	if (!pProperties->OutputFile.FilePath.EndsWith(".k"))
	{
		UE_LOG(KimuraConverter, Error, TEXT("Invalid output filename. Output file must have the .k extension"));
		return FReply::Handled();
	}

	// get the arguments to pass to the converter
	std::vector<std::string> argv;
 	pProperties->GenerateArgs(argv);

	this->ConverterInstance = Kimura::CreateConverter(argv);

	if (!this->ConverterInstance)
	{
		return FReply::Handled();
	}

	this->ConverterInstance->Start();

	return FReply::Handled();
}


//-----------------------------------------------------------------------------
// SKimuraConverter::IsCancelButtonEnabled
//-----------------------------------------------------------------------------
bool SKimuraConverter::IsCancelButtonEnabled() const
{
	return this->ConverterInstance != nullptr;
}


//-----------------------------------------------------------------------------
// SKimuraConverter::OnCancelPressed
//-----------------------------------------------------------------------------
FReply SKimuraConverter::OnCancelPressed()
{
	this->TerminateConverter();

	return FReply::Handled();
}


//-----------------------------------------------------------------------------
// SKimuraConverter::IsCloseButtonEnabled
//-----------------------------------------------------------------------------
bool SKimuraConverter::IsCloseButtonEnabled() const
{
	return true;
}


//-----------------------------------------------------------------------------
// SKimuraConverter::OnClosePressed
//-----------------------------------------------------------------------------
FReply SKimuraConverter::OnClosePressed()
{
	this->TerminateConverter();

	// close parent window
	if (this->OwnerWindow.IsValid())
	{
		this->OwnerWindow.Pin()->RequestDestroyWindow();
	}

	return FReply::Handled();
}


//-----------------------------------------------------------------------------
// SKimuraConverter::OnGetProgression
//-----------------------------------------------------------------------------
float SKimuraConverter::OnGetProgression() const
{
	if (this->ConverterInstance)
	{
		int NumFrames = 0;
		int NumFramesProcessed = 0;
		this->ConverterInstance->GetConversionProgress(NumFramesProcessed, NumFrames);

		if (NumFrames > 0)
		{
			return (float)NumFramesProcessed / (float)NumFrames;
		}

	}

	return 0.0f;
}


//-----------------------------------------------------------------------------
// SKimuraConverter::TerminateConverter
//-----------------------------------------------------------------------------
void SKimuraConverter::TerminateConverter()
{
	if (this->ConverterInstance != nullptr &&
		this->ConverterInstance->IsWorking())
	{
		this->ConverterInstance->Stop();

		delete this->ConverterInstance;
		this->ConverterInstance = nullptr;

	}
}


#undef LOCTEXT_NAMESPACE

#if WITH_EDITOR
	PRAGMA_ENABLE_OPTIMIZATION
#endif


