// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "LevelEditorMenuExtensions/LGUILevelEditorExtensions.h"
#include "EngineModule.h"
#include "Engine/EngineTypes.h"
#include "LGUIEditorStyle.h"
#include "LevelEditor.h"
#include "Core/ActorComponent/UIItem.h"

#define LOCTEXT_NAMESPACE "FLGUILevelEditorExtensions"

//////////////////////////////////////////////////////////////////////////

FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors LevelEditorMenuExtenderDelegate;
FDelegateHandle LevelEditorMenuExtenderDelegateHandle;

//////////////////////////////////////////////////////////////////////////

class FLGUILevelEditorExtensions_Impl
{
private:
	static void Deplicate(AActor* InActor)
	{

	}
	static void CreateButton(UUIItem* InUIItem)
	{
		
	}
public:
	static void CreateHelperButtons(FMenuBuilder& MenuBuilder, UUIItem* InUIItem)
	{
		return;
		//@todo: make this work!
		MenuBuilder.BeginSection("LGUI", LOCTEXT("LGUILevelEditorHeading", "LGUI"));
		if (InUIItem)
		{
			FUIAction Action_CreateButton(FExecuteAction::CreateStatic(&FLGUILevelEditorExtensions_Impl::CreateButton, InUIItem));
			MenuBuilder.AddMenuEntry(
				LOCTEXT("CreateButton", "CreateButton"),
				LOCTEXT("CreateButton_Tooltip", "Create a button under this UI object"),
				FSlateIcon(),
				Action_CreateButton,
				NAME_None,
				EUserInterfaceActionType::Button);

			FUIAction Action_Duplicate(FExecuteAction::CreateStatic(&FLGUILevelEditorExtensions_Impl::Deplicate, InUIItem->GetOwner()));
			MenuBuilder.AddMenuEntry(
				LOCTEXT("DuplicateActor", "DuplicateActor"),
				LOCTEXT("CreateButton_Tooltip", "Duplicate this actor"),
				FSlateIcon(),
				Action_Duplicate,
				NAME_None,
				EUserInterfaceActionType::Button);
		}
	}
	static TSharedRef<FExtender> OnExtendLevelEditorMenu(const TSharedRef<FUICommandList> CommandList, TArray<AActor*> SelectedActors)
	{
		TSharedRef<FExtender> Extender(new FExtender());

		if (SelectedActors.Num() == 1)//only support one selection
		{
			if (auto uiItem = SelectedActors[0]->FindComponentByClass<UUIItem>())
			{
				Extender->AddMenuExtension(
					"ActorType",
					EExtensionHook::Before,
					nullptr,
					FMenuExtensionDelegate::CreateStatic(&FLGUILevelEditorExtensions_Impl::CreateHelperButtons, uiItem)
				);
			}
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