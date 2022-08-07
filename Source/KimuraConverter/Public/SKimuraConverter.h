//
// Copyright (c) Alexandre Hetu.
// Licensed under the MIT License.
//
// https://github.com/ahetu04
//

#pragma once

#include "CoreMinimal.h"
#include "Layout/Visibility.h"
#include "Input/Reply.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SWidget.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Views/STableViewBase.h"
#include "SourceControlOperationBase.h"
#include "ISourceControlProvider.h"
#include "IKimuraConverter.h"
#include "Widgets/SWindow.h"

class SButton;


class SKimuraConverter : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SKimuraConverter) 
		:
		_OwnerWindow()
		{ }

		SLATE_ARGUMENT(TSharedPtr<SWindow>, OwnerWindow)

	SLATE_END_ARGS()

public:

	void Construct(const FArguments& InArgs);

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	bool IsConvertButtonEnabled() const;
	FReply OnConvertPressed();

	bool IsCancelButtonEnabled() const;
	FReply OnCancelPressed();

	bool IsCloseButtonEnabled() const;
	FReply OnClosePressed();

	void TerminateConverter();

private:

/*

			std::string input = TryParseArgument(argument, "i:");
			std::string output = TryParseArgument(argument, "o:");
			std::string opt = TryParseArgument(argument, "opt:");
			std::string scale = TryParseArgument(argument, "scale:");
			std::string startFrame = TryParseArgument(argument, "start:");
			std::string endFrame = TryParseArgument(argument, "end:");
			std::string posFormat = TryParseArgument(argument, "pFmt:");
			std::string normalFormat = TryParseArgument(argument, "nFmt:");
			std::string velocityFormat = TryParseArgument(argument, "vFmt:");
			std::string texcoordFormat = TryParseArgument(argument, "tFmt:");
			std::string colorFormat = TryParseArgument(argument, "cFmt:");
			std::string swizzle = TryParseArgument(argument, "swizzle:");
			std::string flipOrder = TryParseArgument(argument, "flip:");
			std::string flipUV = TryParseArgument(argument, "flipUV:");
			std::string preset = TryParseArgument(argument, "preset:");
			std::string savePreset = TryParseArgument(argument, "bind:");

			image sequence
			image mipmaps 
			image max size
			image format



*/

	float OnGetProgression() const;

	TWeakPtr<SWindow>					OwnerWindow;

	TSharedPtr<IDetailsView>			PropertiesDetailView;

	TSharedPtr<SButton>					ConvertButton;
	TSharedPtr<SButton>					CancelButton;

	Kimura::IKimuraConverter*			ConverterInstance = nullptr;




};
