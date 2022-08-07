//
// Copyright (c) Alexandre Hetu.
// Licensed under the MIT License.
//
// https://github.com/ahetu04
//

#include "KimuraConverter.h"
#include "Interfaces/IPluginManager.h"
#include "ShaderCore.h"

#include "LevelEditor.h"
#include "Modules/ModuleManager.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "SKimuraConverter.h"
#include "Interfaces/IMainFrameModule.h"

#define LOCTEXT_NAMESPACE "FKimuraConverterModule"

DEFINE_LOG_CATEGORY(KimuraConverter)

//-----------------------------------------------------------------------------
// FKimuraConverterModule::StartupModule
//-----------------------------------------------------------------------------
void FKimuraConverterModule::StartupModule()
{
	this->ExtendTheEditor();
}


//-----------------------------------------------------------------------------
// FKimuraConverterModule::ExtendTheEditor
//-----------------------------------------------------------------------------
void FKimuraConverterModule::ExtendTheEditor()
{
	// setup menu extender
	TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
	{
		MenuExtender->AddMenuExtension(
			"LevelEditor",
			EExtensionHook::After,
			NULL,
			FMenuExtensionDelegate::CreateRaw(this, &FKimuraConverterModule::SetupConverterMenu)
		);
	}
	
	// add it
	FLevelEditorModule& LevelEditorModule = FModuleManager::Get().LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);

}


//-----------------------------------------------------------------------------
// FKimuraConverterModule::SetupConverterMenu
//-----------------------------------------------------------------------------
void FKimuraConverterModule::SetupConverterMenu(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.BeginSection("CustomMenu", TAttribute<FText>(FText::FromString("Kimura Player")));
	{
		MenuBuilder.AddMenuEntry(
			FText::FromString("abcToKimura"),
			FText::FromString("Convert Alembic files (.abc) to Kimura (.k)"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateRaw(this, &FKimuraConverterModule::OnOpenConverterMenu))
		);

	}
	MenuBuilder.EndSection();
}


//-----------------------------------------------------------------------------
// FKimuraConverterModule::OnOpenConverterMenu
//-----------------------------------------------------------------------------
void FKimuraConverterModule::OnOpenConverterMenu()
{
	//UE_LOG(KimuraConverter, Log, TEXT("FKimuraConverterModule::OnOpenConverterMenu"));


	{

		TSharedRef<SWindow> Window = SNew(SWindow)
			.Title(LOCTEXT("WindowTitle", "abcToKimura"))
			.SizingRule(ESizingRule::Autosized)
			.MinWidth(600.0f)
			.Visibility(EVisibility::All)
			.ForceVolatile(true);
			//.HasCloseButton(false);

		TSharedPtr<SKimuraConverter> Options;


		Window->SetContent
		(
			SAssignNew(Options, SKimuraConverter)
			.OwnerWindow(Window)
		);

		TSharedPtr<SWindow> ParentWindow;
		if (FModuleManager::Get().IsModuleLoaded("MainFrame"))
		{
			IMainFrameModule& MainFrame = FModuleManager::LoadModuleChecked<IMainFrameModule>("MainFrame");
			ParentWindow = MainFrame.GetParentWindow();
		}

		FSlateApplication::Get().AddModalWindow(Window, ParentWindow, false);
	}


}


//-----------------------------------------------------------------------------
// FKimuraConverterModule::ShutdownModule
//-----------------------------------------------------------------------------
void FKimuraConverterModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FKimuraConverterModule, KimuraConverter)

