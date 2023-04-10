// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LevelEditorMenuExtensions/LGUILevelEditorExtensions.h"
#include "EngineModule.h"
#include "Engine/EngineTypes.h"
#include "LGUIEditorStyle.h"
#include "LevelEditor.h"
#include "Core/ActorComponent/UIItem.h"
#include "LGUIEditorModule.h"
#include "LGUIEditorTools.h"

#define LOCTEXT_NAMESPACE "FLGUILevelEditorExtensions"

//////////////////////////////////////////////////////////////////////////

FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors LevelEditorMenuExtenderDelegate;
FDelegateHandle LevelEditorMenuExtenderDelegateHandle;

//////////////////////////////////////////////////////////////////////////

class FLGUILevelEditorExtensions_Impl
{
public:
	static void CreateLGUISubMenu(FMenuBuilder& MenuBuilder)
	{
		MenuBuilder.AddWidget(
			FLGUIEditorModule::Get().MakeEditorToolsMenu(false, false, false, false, false, false)
			, FText::GetEmpty()
		);
	}
	static void CreateHelperButtons(FMenuBuilder& MenuBuilder)
	{
		MenuBuilder.BeginSection("LGUI", LOCTEXT("LGUILevelEditorHeading", "LGUI"));
		{
			MenuBuilder.AddSubMenu(
				LOCTEXT("LGUIEditorTools", "LGUI Editor Tools"),
				FText::GetEmpty(),
				FNewMenuDelegate::CreateStatic(&FLGUILevelEditorExtensions_Impl::CreateLGUISubMenu),
				FUIAction(),
				NAME_None, EUserInterfaceActionType::None
			);
		}
		MenuBuilder.EndSection();
	}
	static TSharedRef<FExtender> OnExtendLevelEditorMenu(const TSharedRef<FUICommandList> CommandList, TArray<AActor*> SelectedActors)
	{
		TSharedRef<FExtender> Extender(new FExtender());
		if (SelectedActors.Num() == 1//only support one selection
			&& IsValid(SelectedActors[0])
			&& LGUIEditorTools::IsActorCompatibleWithLGUIToolsMenu(SelectedActors[0])//only show menu with supported actor
			)
		{
			Extender->AddMenuExtension(
				"ActorType",
				EExtensionHook::Before,
				nullptr,
				FMenuExtensionDelegate::CreateStatic(&FLGUILevelEditorExtensions_Impl::CreateHelperButtons)
			);
		}
		return Extender;
	}
};

// FLGUILevelEditorExtensions

void FLGUILevelEditorExtensions::InstallHooks()
{
	LevelEditorMenuExtenderDelegate = FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors::CreateStatic(&FLGUILevelEditorExtensions_Impl::OnExtendLevelEditorMenu);

	FLevelEditorModule& LevelEditorModule = FModuleManager::Get().LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	auto& MenuExtenders = LevelEditorModule.GetAllLevelViewportContextMenuExtenders();
	MenuExtenders.Add(LevelEditorMenuExtenderDelegate);
	LevelEditorMenuExtenderDelegateHandle = MenuExtenders.Last().GetHandle();
}

void FLGUILevelEditorExtensions::RemoveHooks()
{
	if (FModuleManager::Get().IsModuleLoaded("LevelEditor"))
	{
		FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
		LevelEditorModule.GetAllLevelViewportContextMenuExtenders().RemoveAll([&](const FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors& Delegate) {
			return Delegate.GetHandle() == LevelEditorMenuExtenderDelegateHandle;
		});
	}
}

//////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE