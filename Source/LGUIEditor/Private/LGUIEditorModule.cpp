// Copyright 2019 LexLiu. All Rights Reserved.

#include "LGUIEditorModule.h"
#include "SlateExtras.h"
#include "LevelEditor.h"
#include "Modules/ModuleManager.h"

#include "LGUIEditorStyle.h"
#include "LGUIEditorCommands.h"
#include "Thumbnail/LGUIPrefabThumbnailRenderer.h"
#include "Thumbnail/LGUISpriteThumbnailRenderer.h"
#include "ContentBrowserExtensions/LGUIContentBrowserExtensions.h"
#include "LevelEditorMenuExtensions/LGUILevelEditorExtensions.h"
#include "Window/LGUIAtlasViewer.h"
#include "DetailCustomization/LGUIDrawableEventOneParamCustomization.h"

#include "Core/LGUISettings.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"

#include "Window/LGUIEditorTools.h"
#include "SceneOutliner/LGUISceneOutlinerInfoColumn.h"
#include "SceneOutlinerModule.h"
#include "SceneOutlinerPublicTypes.h"
#include "SceneOutliner/LGUINativeSceneOutlinerExtension.h"
#include "AssetToolsModule.h"

const FName FLGUIEditorModule::LGUIEditorToolsTabName(TEXT("LGUIEditorTools"));
const FName FLGUIEditorModule::LGUIEventComponentSelectorName(TEXT("LGUIEventComponentSelector"));
const FName FLGUIEditorModule::LGUIEventFunctionSelectorName(TEXT("LGUIEventFunctionSelector"));
const FName FLGUIEditorModule::LGUIAtlasViewerName(TEXT("LGUIAtlasViewerName"));

FLGUIEditorModule* FLGUIEditorModule::Instance = nullptr;

#define LOCTEXT_NAMESPACE "FLGUIEditorModule"
DEFINE_LOG_CATEGORY(LGUIEditor);

void FLGUIEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FLGUIEditorStyle::Initialize();
	FLGUIEditorStyle::ReloadTextures();

	FLGUIEditorCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);
		
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

	//Editor tools
	{
		auto editorCommand = FLGUIEditorCommands::Get();
		//PluginCommands->MapAction(
		//	editorCommand.OpenEditorToolsWindow,
		//	FExecuteAction::CreateRaw(this, &FLGUIEditorModule::EditorToolButtonClicked),
		//	FCanExecuteAction());

		//actor action
		PluginCommands->MapAction(
			editorCommand.CopyActor,
			FExecuteAction::CreateStatic(&ULGUIEditorToolsAgentObject::CopySelectedActors_Impl),
			FCanExecuteAction::CreateLambda([] {return GEditor->GetSelectedActorCount() > 0; })
		);
		PluginCommands->MapAction(
			editorCommand.PasteActor,
			FExecuteAction::CreateStatic(&ULGUIEditorToolsAgentObject::PasteSelectedActors_Impl),
			FCanExecuteAction::CreateLambda([] {return ULGUIEditorToolsAgentObject::HaveValidCopiedActors(); })
		);
		PluginCommands->MapAction(
			editorCommand.DuplicateActor,
			FExecuteAction::CreateStatic(&ULGUIEditorToolsAgentObject::DuplicateSelectedActors_Impl),
			FCanExecuteAction::CreateLambda([] {return GEditor->GetSelectedActorCount() > 0; })
		);
		PluginCommands->MapAction(
			editorCommand.DeleteActor,
			FExecuteAction::CreateStatic(&ULGUIEditorToolsAgentObject::DeleteSelectedActors_Impl),
			FCanExecuteAction::CreateLambda([] {return GEditor->GetSelectedActorCount() > 0; })
		);

		//component action
		PluginCommands->MapAction(
			editorCommand.CopyComponentValues,
			FExecuteAction::CreateStatic(&ULGUIEditorToolsAgentObject::CopyComponentValues_Impl),
			FCanExecuteAction::CreateLambda([] {return GEditor->GetSelectedComponentCount() > 0; })
		);
		PluginCommands->MapAction(
			editorCommand.PasteComponentValues,
			FExecuteAction::CreateStatic(&ULGUIEditorToolsAgentObject::PasteComponentValues_Impl),
			FCanExecuteAction::CreateLambda([] {return ULGUIEditorToolsAgentObject::HaveValidCopiedComponent(); })
		);

		PluginCommands->MapAction(
			editorCommand.PreserveSceneoutlinerHierarchy,
			FExecuteAction::CreateLambda([]{ULGUINativeSceneOutlinerExtension::active = !ULGUINativeSceneOutlinerExtension::active; GEditor->RedrawAllViewports(); }),
			FCanExecuteAction(),
			FIsActionChecked::CreateLambda([] {return ULGUINativeSceneOutlinerExtension::active; }));
		
		PluginCommands->MapAction(
			editorCommand.OpenAtlasViewer,
			FExecuteAction::CreateStatic(&ULGUIEditorToolsAgentObject::OpenAtlasViewer_Impl),
			FCanExecuteAction());

		TSharedPtr<FExtender> toolbarExtender = MakeShareable(new FExtender);
		toolbarExtender->AddToolBarExtension("Game", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FLGUIEditorModule::AddEditorToolsToToolbarExtension));
		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(toolbarExtender);
	}
	//register SceneOutliner ColumnInfo
	{
		FSceneOutlinerModule& SceneOutlinerModule = FModuleManager::LoadModuleChecked< FSceneOutlinerModule >("SceneOutliner");
		SceneOutliner::FColumnInfo ColumnInfo(SceneOutliner::EColumnVisibility::Visible, 15, FCreateSceneOutlinerColumn::CreateStatic(&LGUISceneOutliner::FLGUISceneOutlinerInfoColumn::MakeInstance));
		SceneOutlinerModule.RegisterDefaultColumnType<LGUISceneOutliner::FLGUISceneOutlinerInfoColumn>(SceneOutliner::FDefaultColumnInfo(ColumnInfo));
		//SceneOutliner extension
		auto sceneOutlinerExtensionObject = GetMutableDefault<ULGUINativeSceneOutlinerExtension>();
		sceneOutlinerExtensionObject->AddToRoot();
		sceneOutlinerExtensionObject->Init();
	}
	//register window
	{
		//Editor tools
		//FGlobalTabmanager::Get()->RegisterNomadTabSpawner(LGUIEditorToolsTabName, FOnSpawnTab::CreateRaw(this,
		//	&FLGUIEditorModule::HandleSpawnEditorToolsTab))
		//	.SetDisplayName(LOCTEXT("LGUIEditorToolsTitle", "LGUI Editor Tools"))
		//	.SetMenuType(ETabSpawnerMenuType::Hidden);
		//event component selector
		FGlobalTabmanager::Get()->RegisterNomadTabSpawner(LGUIEventComponentSelectorName, FOnSpawnTab::CreateRaw(this, &FLGUIEditorModule::HandleSpawnEventComponentSelectorTab))
			.SetDisplayName(LOCTEXT("LGUIEventComponentSelector", "LGUI Event Component Selector"))
			.SetMenuType(ETabSpawnerMenuType::Hidden);
		//event function selector
		FGlobalTabmanager::Get()->RegisterNomadTabSpawner(LGUIEventFunctionSelectorName, FOnSpawnTab::CreateRaw(this, &FLGUIEditorModule::HandleSpawnEventFunctionSelectorTab))
			.SetDisplayName(LOCTEXT("LGUIEventFuntionSelector", "LGUI Event Funtion Selector"))
			.SetMenuType(ETabSpawnerMenuType::Hidden);
		//atlas texture viewer
		FGlobalTabmanager::Get()->RegisterNomadTabSpawner(LGUIAtlasViewerName, FOnSpawnTab::CreateRaw(this, &FLGUIEditorModule::HandleSpawnAtlasViewerTab))
			.SetDisplayName(LOCTEXT("LGUIAtlasTextureViewerName", "LGUI Atlas Texture Viewer"))
			.SetMenuType(ETabSpawnerMenuType::Hidden);
	}
	//register component editor
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.RegisterCustomClassLayout(UUIItem::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUIItemCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(UUISpriteBase::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUISpriteBaseCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(UUISprite::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUISpriteCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(ULGUICanvas::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FLGUICanvasCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(UUIText::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUITextCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(UUITextureBase::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUITextureBaseCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(UUIPanel::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUIPanelCustomization::MakeInstance));

		PropertyModule.RegisterCustomClassLayout(ULGUISpriteData::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FLGUISpriteDataCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(ULGUIFontData::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FLGUIFontDataCustomization::MakeInstance));
		
		PropertyModule.RegisterCustomClassLayout(UUISelectableComponent::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUISelectableCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(UUIToggleComponent::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUIToggleCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(UUITextInputComponent::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUITextInputCustomization::MakeInstance));

		PropertyModule.RegisterCustomClassLayout(UUILayoutBase::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUILayoutBaseCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(UUIVerticalLayout::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUIVerticalLayoutCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(UUIHorizontalLayout::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUIHorizontalLayoutCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(UUIGridLayout::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUIGridLayoutCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(UUILayoutElement::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUILayoutElementCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(ULGUICanvasScaler::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUICanvasScalerCustomization::MakeInstance));

		PropertyModule.RegisterCustomClassLayout(ULGUIPrefabHelperComponent::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FLGUIPrefabHelperComponentCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(ULGUIPrefab::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FLGUIPrefabCustomization::MakeInstance));

		PropertyModule.RegisterCustomClassLayout(ULGUIEditorToolsAgentObject::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FEditorToolsCustomization::MakeInstance));

		PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIDrawableEvent::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FLGUIDrawableEventOneParamCustomization::MakeInstance));
		//PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIDrawableEventTwoParam::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FLGUIDrawableEventTwoParamCustomization::MakeInstance));

		PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIEditHelperButton::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FLGUIEditHelperButtonCustomization::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIComponentReference::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FLGUIComponentRefereceCustomization::MakeInstance));
	}
	//register asset
	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		//register AssetCategory
		EAssetTypeCategories::Type LGUIAssetCategoryBit = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("LGUI")), LOCTEXT("LGUIAssetCategory", "LGUI"));

		TSharedRef<IAssetTypeActions> spriteAction = MakeShareable(new FLGUISpriteDataTypeAction(LGUIAssetCategoryBit));
		TSharedRef<IAssetTypeActions> fontAction = MakeShareable(new FLGUIFontDataTypeAction(LGUIAssetCategoryBit));
		TSharedRef<IAssetTypeActions> prefabAction = MakeShareable(new FLGUIPrefabTypeAction(LGUIAssetCategoryBit));
		AssetTools.RegisterAssetTypeActions(spriteAction);
		AssetTools.RegisterAssetTypeActions(fontAction);
		AssetTools.RegisterAssetTypeActions(prefabAction);
	}
	//register Thumbnail
	{
		UThumbnailManager::Get().RegisterCustomRenderer(ULGUIPrefab::StaticClass(), ULGUIPrefabThumbnailRenderer::StaticClass());
		UThumbnailManager::Get().RegisterCustomRenderer(ULGUISpriteData::StaticClass(), ULGUISpriteThumbnailRenderer::StaticClass());
	}
	//register right mout button in content browser
	{
		if (!IsRunningCommandlet())
		{
			FLGUIContentBrowserExtensions::InstallHooks();
			FLGUILevelEditorExtensions::InstallHooks();
		}
	}
	//register setting
	{
		if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
		{
			SettingsModule->RegisterSettings("Project", "Plugins", "LGUISprite",
				LOCTEXT("LGUISpriteSettingsName", "LGUI"),
				LOCTEXT("LGUISpriteSettingsDescription", "LGUISpriteSettings"),
				GetMutableDefault<ULGUISettings>());
		}
	}

	Instance = this;
}

void FLGUIEditorModule::ShutdownModule()
{
	Instance = nullptr;
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FLGUIEditorStyle::Shutdown();

	FLGUIEditorCommands::Unregister();

	//FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(LGUIEditorToolsTabName);
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(LGUIEventComponentSelectorName);
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(LGUIEventFunctionSelectorName);
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(LGUIAtlasViewerName);

	FSceneOutlinerModule& SceneOutlinerModule = FModuleManager::LoadModuleChecked< FSceneOutlinerModule >("SceneOutliner");
	SceneOutlinerModule.UnRegisterColumnType<LGUISceneOutliner::FLGUISceneOutlinerInfoColumn>();

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.UnregisterCustomClassLayout(UUIItem::StaticClass()->GetFName());
	PropertyModule.UnregisterCustomClassLayout(UUISpriteBase::StaticClass()->GetFName());
	PropertyModule.UnregisterCustomClassLayout(UUISprite::StaticClass()->GetFName());
	PropertyModule.UnregisterCustomClassLayout(ULGUICanvas::StaticClass()->GetFName());
	PropertyModule.UnregisterCustomClassLayout(UUIText::StaticClass()->GetFName());
	PropertyModule.UnregisterCustomClassLayout(UUITextureBase::StaticClass()->GetFName());
	PropertyModule.UnregisterCustomClassLayout(UUIPanel::StaticClass()->GetFName());

	PropertyModule.UnregisterCustomClassLayout(ULGUISpriteData::StaticClass()->GetFName());
	PropertyModule.UnregisterCustomClassLayout(ULGUIFontData::StaticClass()->GetFName());

	PropertyModule.UnregisterCustomClassLayout(UUISelectableComponent::StaticClass()->GetFName());
	PropertyModule.UnregisterCustomClassLayout(UUIToggleComponent::StaticClass()->GetFName());
	PropertyModule.UnregisterCustomClassLayout(UUITextInputComponent::StaticClass()->GetFName());

	PropertyModule.UnregisterCustomClassLayout(UUILayoutBase::StaticClass()->GetFName());
	PropertyModule.UnregisterCustomClassLayout(UUIVerticalLayout::StaticClass()->GetFName());
	PropertyModule.UnregisterCustomClassLayout(UUIHorizontalLayout::StaticClass()->GetFName());
	PropertyModule.UnregisterCustomClassLayout(UUIGridLayout::StaticClass()->GetFName());
	PropertyModule.UnregisterCustomClassLayout(UUILayoutElement::StaticClass()->GetFName());

	PropertyModule.UnregisterCustomClassLayout(ULGUIPrefabHelperComponent::StaticClass()->GetFName());
	PropertyModule.UnregisterCustomClassLayout(ULGUIPrefab::StaticClass()->GetFName());

	PropertyModule.UnregisterCustomClassLayout(ULGUIEditorToolsAgentObject::StaticClass()->GetFName());

	PropertyModule.UnregisterCustomPropertyTypeLayout("LGUIDrawableEvent");
	PropertyModule.UnregisterCustomPropertyTypeLayout("LGUIEditHelperButton");
	PropertyModule.UnregisterCustomPropertyTypeLayout("LGUIComponentReference");

	FLGUIContentBrowserExtensions::RemoveHooks();
	FLGUILevelEditorExtensions::RemoveHooks();

	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "LGUISprite");
	}
}

void FLGUIEditorModule::RefreshSceneOutliner()
{
	GEngine->BroadcastLevelActorListChanged();
}

TSharedRef<SDockTab> FLGUIEditorModule::HandleSpawnEditorToolsTab(const FSpawnTabArgs& SpawnTabArgs)
{
	TSharedRef<SDockTab> ResultTab = SNew(SDockTab).TabRole(ETabRole::NomadTab);
	auto TabContentWidget = SNew(SLGUIEditorTools, ResultTab);
	ResultTab->SetContent(TabContentWidget);
	return ResultTab;
}
TSharedRef<SDockTab> FLGUIEditorModule::HandleSpawnEventComponentSelectorTab(const FSpawnTabArgs& SpawnTabArgs)
{
	auto ResultTab = SNew(SDockTab).TabRole(ETabRole::NomadTab);
	auto TabContentWidget = SNew(SLGUIEventComponentSelector, ResultTab);
	ResultTab->SetContent(TabContentWidget);
	return ResultTab;
}
TSharedRef<SDockTab> FLGUIEditorModule::HandleSpawnEventFunctionSelectorTab(const FSpawnTabArgs& SpawnTabArgs)
{
	auto ResultTab = SNew(SDockTab).TabRole(ETabRole::NomadTab);
	auto TabContentWidget = SNew(SLGUIEventFunctionSelector, ResultTab);
	ResultTab->SetContent(TabContentWidget);
	return ResultTab;
}
TSharedRef<SDockTab> FLGUIEditorModule::HandleSpawnAtlasViewerTab(const FSpawnTabArgs& SpawnTabArgs)
{
	auto ResultTab = SNew(SDockTab).TabRole(ETabRole::NomadTab);
	auto TabContentWidget = SNew(SLGUIAtlasViewer, ResultTab);
	ResultTab->SetContent(TabContentWidget);
	return ResultTab;
}

void FLGUIEditorModule::EditorToolButtonClicked()
{
	FGlobalTabmanager::Get()->InvokeTab(LGUIEditorToolsTabName);
}
bool FLGUIEditorModule::CanEditActorForPrefab()
{
	return GEditor->GetSelectedActorCount() == 1 && ULGUIEditorToolsAgentObject::GetPrefabActor_WhichManageThisActor(ULGUIEditorToolsAgentObject::GetFirstSelectedActor()) != nullptr;
}

void FLGUIEditorModule::AddEditorToolsToToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.AddComboButton(
		FUIAction(),
		FOnGetContent::CreateRaw(this, &FLGUIEditorModule::MakeEditorToolsMenu, false),
		LOCTEXT("LGUITools", "LGUI Tools"),
		LOCTEXT("LGUIEditorTools", "LGUI Editor Tools"),
		FSlateIcon(FLGUIEditorStyle::GetStyleSetName(), "LGUIEditor.EditorTools")
	);
}

TSharedRef<SWidget> FLGUIEditorModule::MakeEditorToolsMenu(bool IsSceneOutlineMenu)
{
	FMenuBuilder MenuBuilder(true, PluginCommands);
	auto commandList = FLGUIEditorCommands::Get();

	MenuBuilder.BeginSection("Prefab", LOCTEXT("Prefab", "Prefab"));
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("CreatePrefab", "Create Prefab"),
			LOCTEXT("Create_Tooltip", "Use selected actor to create a new prefab"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&ULGUIEditorToolsAgentObject::CreatePrefabAsset)
				, FCanExecuteAction::CreateLambda([] {return GEditor->GetSelectedActorCount() > 0; }))
		);
		MenuBuilder.AddMenuEntry(
			LOCTEXT("ApplyPrefab", "Apply Prefab"),
			LOCTEXT("Apply_Tooltip", "Apply change and save prefab to asset"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&ULGUIEditorToolsAgentObject::ApplyPrefab)
				, FCanExecuteAction::CreateRaw(this, &FLGUIEditorModule::CanEditActorForPrefab)
				, FGetActionCheckState()
				, FIsActionButtonVisible::CreateRaw(this, &FLGUIEditorModule::CanEditActorForPrefab))
		);
		MenuBuilder.AddMenuEntry(
			LOCTEXT("RevertPrefab", "Revert Prefab"),
			LOCTEXT("Revert_Tooltip", "Revert any changes"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&ULGUIEditorToolsAgentObject::RevertPrefab)
				, FCanExecuteAction::CreateRaw(this, &FLGUIEditorModule::CanEditActorForPrefab)
				, FGetActionCheckState()
				, FIsActionButtonVisible::CreateRaw(this, &FLGUIEditorModule::CanEditActorForPrefab))
		);
		MenuBuilder.AddMenuEntry(
			LOCTEXT("DeletePrefab", "Delete this Prefab instance"),
			LOCTEXT("Delete_Tooltip", "Delete actors created by this prefab with hierarchy"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&ULGUIEditorToolsAgentObject::DeletePrefab)
				, FCanExecuteAction::CreateRaw(this, &FLGUIEditorModule::CanEditActorForPrefab)
				, FGetActionCheckState()
				, FIsActionButtonVisible::CreateRaw(this, &FLGUIEditorModule::CanEditActorForPrefab))
		);
		MenuBuilder.AddMenuEntry(
			LOCTEXT("SelectPrefabAsset", "Browse to Prefab asset"),
			LOCTEXT("SelectPrefabAsset_Tooltip", "Browse to Prefab asset in Content Browser"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&ULGUIEditorToolsAgentObject::SelectPrefabAsset)
				, FCanExecuteAction::CreateRaw(this, &FLGUIEditorModule::CanEditActorForPrefab)
				, FGetActionCheckState()
				, FIsActionButtonVisible::CreateRaw(this, &FLGUIEditorModule::CanEditActorForPrefab))
		);
	}
	MenuBuilder.EndSection();

	MenuBuilder.BeginSection("Create", LOCTEXT("Create", "Create"));
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("EmptyActor", "Empty Actor"),
			LOCTEXT("EmptyActor_Tooltip", "Create an empty actor"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&ULGUIEditorToolsAgentObject::CreateEmptyActor))
		);
		MenuBuilder.AddSubMenu(
			LOCTEXT("CreateUIElementSubMenu", "Create UI Element"),
			LOCTEXT("CreateUIElementSubMenu_Tooltip", "Create UI Element"),
			FNewMenuDelegate::CreateRaw(this, &FLGUIEditorModule::CreateUIElementSubMenu)
		);
		MenuBuilder.AddSubMenu(
			LOCTEXT("CreateUIExtensionSubMenu", "Create UI Extension Element"),
			LOCTEXT("CreateUIExtensionSubMenu_Tooltip", "Create UI Extension Element"),
			FNewMenuDelegate::CreateRaw(this, &FLGUIEditorModule::CreateUIExtensionSubMenu)
		);
		MenuBuilder.AddSubMenu(
			LOCTEXT("BasicSetup", "Basic Setup"),
			LOCTEXT("BasicSetup", "Basic Setup"),
			FNewMenuDelegate::CreateRaw(this, &FLGUIEditorModule::BasicSetupSubMenu)
		);
		MenuBuilder.AddSubMenu(
			LOCTEXT("ReplaceUIElement", "Replace this by..."),
			LOCTEXT("ReplaceUIElement_Tooltip", "Replace UI Element with..."),
			FNewMenuDelegate::CreateRaw(this, &FLGUIEditorModule::ReplaceUIElementSubMenu),
			FUIAction(FExecuteAction(), FCanExecuteAction::CreateLambda([] {return GEditor->GetSelectedActorCount() > 0; })),
			NAME_None,
			EUserInterfaceActionType::Button
		);
	}
	MenuBuilder.EndSection();

	MenuBuilder.BeginSection("ActorAction", LOCTEXT("ActorAction", "Edit Actor With Hierarchy"));
	{
		MenuBuilder.AddMenuEntry(commandList.CopyActor);
		MenuBuilder.AddMenuEntry(commandList.PasteActor);
		MenuBuilder.AddMenuEntry(commandList.DuplicateActor);
		MenuBuilder.AddMenuEntry(commandList.DeleteActor);
		MenuBuilder.AddSubMenu(
			LOCTEXT("ChangeCollisionChannelSubMenu", "Change Trace Channel"),
			LOCTEXT("ChangeCollisionChannelSubMenu_Tooltip", "Change a UI element's trace channel to selected channel, with hierarchy"),
			FNewMenuDelegate::CreateRaw(this, &FLGUIEditorModule::ChangeTraceChannelSubMenu),
			FUIAction(FExecuteAction(), FCanExecuteAction::CreateLambda([] {return GEditor->GetSelectedActorCount() > 0; })),
			NAME_None,
			EUserInterfaceActionType::Button
		);
	}
	MenuBuilder.EndSection();

	if (!IsSceneOutlineMenu)
	{
		MenuBuilder.BeginSection("ComponentAction", LOCTEXT("ComponentAction", "Edit Component"));
		{
			MenuBuilder.AddMenuEntry(commandList.CopyComponentValues);
			MenuBuilder.AddMenuEntry(commandList.PasteComponentValues);
		}
		MenuBuilder.EndSection();

		MenuBuilder.BeginSection("Toggles", LOCTEXT("Toggles", "Toggles"));
		{
			MenuBuilder.AddMenuEntry(FLGUIEditorCommands::Get().PreserveSceneoutlinerHierarchy);
		}
		MenuBuilder.EndSection();

		MenuBuilder.BeginSection("OpenWindow", LOCTEXT("OpenWindow", "Open Window"));
		{
			MenuBuilder.AddMenuEntry(FLGUIEditorCommands::Get().OpenAtlasViewer);
			//MenuBuilder.AddMenuEntry(FLGUIEditorCommands::Get().OpenScreenSpaceUIViewer);
		}
		MenuBuilder.EndSection();
	}
	return MenuBuilder.MakeWidget();
}

void FLGUIEditorModule::CreateUIElementSubMenu(FMenuBuilder& MenuBuilder)
{
	struct FunctionContainer
	{
		static void CreateUIBaseElementMenuEntry(FMenuBuilder& InBuilder, UClass* InClass)
		{
			auto UIItemName = InClass->GetName();
			auto ShotName = UIItemName;
			ShotName.RemoveFromEnd(TEXT("Actor"));
			InBuilder.AddMenuEntry(
				FText::FromString(ShotName),
				FText::FromString(FString::Printf(TEXT("Create %s"), *(UIItemName))),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&ULGUIEditorToolsAgentObject::CreateUIItemActor, InClass))
			);
		}
		static void CreateUIControlMenuEntry(FMenuBuilder& InBuilder, const FString& InControlName, const FString& InPrefabPath)
		{
			InBuilder.AddMenuEntry(
				FText::FromString(InControlName),
				FText::FromString(FString::Printf(TEXT("Create %s"), *InControlName)),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&ULGUIEditorToolsAgentObject::CreateUIControls, InPrefabPath))
			);
		}
	};

	MenuBuilder.BeginSection("UIElement");
	{
		FunctionContainer::CreateUIBaseElementMenuEntry(MenuBuilder, AUIContainerActor::StaticClass());
		FunctionContainer::CreateUIBaseElementMenuEntry(MenuBuilder, AUISpriteActor::StaticClass());
		FunctionContainer::CreateUIBaseElementMenuEntry(MenuBuilder, AUITextActor::StaticClass());
		FunctionContainer::CreateUIBaseElementMenuEntry(MenuBuilder, AUITextureActor::StaticClass());

		FunctionContainer::CreateUIControlMenuEntry(MenuBuilder, TEXT("Button"), "/LGUI/Prefabs/DefaultButton");
		FunctionContainer::CreateUIControlMenuEntry(MenuBuilder, TEXT("Toggle"), "/LGUI/Prefabs/DefaultToggle");
		FunctionContainer::CreateUIControlMenuEntry(MenuBuilder, TEXT("HorizontalSlider"), "/LGUI/Prefabs/DefaultHorizontalSlider");
		FunctionContainer::CreateUIControlMenuEntry(MenuBuilder, TEXT("VerticalSlider"), "/LGUI/Prefabs/DefaultVerticalSlider");
		FunctionContainer::CreateUIControlMenuEntry(MenuBuilder, TEXT("HorizontalScrollbar"), "/LGUI/Prefabs/DefaultHorizontalScrollbar");
		FunctionContainer::CreateUIControlMenuEntry(MenuBuilder, TEXT("VerticalScrollbar"), "/LGUI/Prefabs/DefaultVerticalScrollbar");
		FunctionContainer::CreateUIControlMenuEntry(MenuBuilder, TEXT("FlyoutMenuButton"), "/LGUI/Prefabs/DefaultFlyoutMenuButton");
		FunctionContainer::CreateUIControlMenuEntry(MenuBuilder, TEXT("ComboBoxButton"), "/LGUI/Prefabs/DefaultComboBoxButton");
		FunctionContainer::CreateUIControlMenuEntry(MenuBuilder, TEXT("TextInput"), "/LGUI/Prefabs/DefaultTextInput");
		FunctionContainer::CreateUIControlMenuEntry(MenuBuilder, TEXT("ScrollView"), "/LGUI/Prefabs/DefaultScrollView");
	}
	MenuBuilder.EndSection();
}

void FLGUIEditorModule::CreateUIExtensionSubMenu(FMenuBuilder& MenuBuilder)
{
	struct FunctionContainer
	{
		static void CreateUIExtensionElementMenuEntry(FMenuBuilder& InBuilder, UClass* InClass)
		{
			auto UIItemName = InClass->GetName();
			InBuilder.AddMenuEntry(
				FText::FromString(UIItemName),
				FText::FromString(FString::Printf(TEXT("Create %s"), *UIItemName)),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&ULGUIEditorToolsAgentObject::CreateUIItemActor, InClass))
			);
		}
	};

	MenuBuilder.BeginSection("UIExtension");
	{
		for (TObjectIterator<UClass> ClassItr; ClassItr; ++ClassItr)
		{
			if (ClassItr->IsChildOf(AUIBaseActor::StaticClass()))
			{
				if (*ClassItr != AUIContainerActor::StaticClass()
					&& *ClassItr != AUISpriteActor::StaticClass()
					&& *ClassItr != AUITextActor::StaticClass()
					&& *ClassItr != AUITextureActor::StaticClass()
					&& *ClassItr != AUIBaseActor::StaticClass())
				{
					bool isBlueprint = ClassItr->HasAnyClassFlags(CLASS_CompiledFromBlueprint);
					if (isBlueprint)
					{
						if (ClassItr->GetName().StartsWith(TEXT("SKEL_")))
						{
							continue;
						}
					}
					FunctionContainer::CreateUIExtensionElementMenuEntry(MenuBuilder, *ClassItr);
				}
			}
		}
	}
	MenuBuilder.EndSection();
}

void FLGUIEditorModule::BasicSetupSubMenu(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.BeginSection("UIBasicSetup");
	{
		MenuBuilder.AddMenuEntry(
			FText::FromString(TEXT("Screen Space UI")),
			FText::FromString(FString::Printf(TEXT("Create Screen Space UI"))),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&ULGUIEditorToolsAgentObject::CreateScreenSpaceUIBasicSetup))
		);
		MenuBuilder.AddMenuEntry(
			FText::FromString(TEXT("World Space UI")),
			FText::FromString(FString::Printf(TEXT("Create World Space UI"))),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&ULGUIEditorToolsAgentObject::CreateWorldSpaceUIBasicSetup))
		);
	}
	MenuBuilder.EndSection();
}

void FLGUIEditorModule::ChangeTraceChannelSubMenu(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.BeginSection("CollisionChannel");
	{
		auto CollisionProfile = UCollisionProfile::Get();
		for (int i = 0, count = (int)ETraceTypeQuery::TraceTypeQuery_MAX; i < count; i++)
		{
			auto collisionChannel = UEngineTypes::ConvertToCollisionChannel((ETraceTypeQuery)i);
			auto channelName = CollisionProfile->ReturnChannelNameFromContainerIndex(collisionChannel).ToString();
			if (channelName != TEXT("MAX"))
			{
				MenuBuilder.AddMenuEntry(
					FText::FromString(channelName),
					FText::FromString(FString::Printf(TEXT("Change selected UI item actor's trace channel to %s"), *channelName)),
					FSlateIcon(),
					FUIAction(FExecuteAction::CreateStatic(&ULGUIEditorToolsAgentObject::ChangeTraceChannel_Impl, (ETraceTypeQuery)i))
				);
			}
		}
	}
	MenuBuilder.EndSection();
}

void FLGUIEditorModule::ReplaceUIElementSubMenu(FMenuBuilder& MenuBuilder)
{
	struct FunctionContainer
	{
		static void ReplaceUIElement(FMenuBuilder& InBuilder, UClass* InClass)
		{
			auto UIItemName = InClass->GetName();
			auto ShotName = UIItemName;
			ShotName.RemoveFromEnd(TEXT("Actor"));
			InBuilder.AddMenuEntry(
				FText::FromString(ShotName),
				FText::FromString(FString::Printf(TEXT("ReplaceWith:%s"), *UIItemName)),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&ULGUIEditorToolsAgentObject::ReplaceUIElementWith, InClass))
			);
		}
	};

	MenuBuilder.BeginSection("Replace");
	{
		FunctionContainer::ReplaceUIElement(MenuBuilder, AUIContainerActor::StaticClass());
		FunctionContainer::ReplaceUIElement(MenuBuilder, AUISpriteActor::StaticClass());
		FunctionContainer::ReplaceUIElement(MenuBuilder, AUITextActor::StaticClass());
		FunctionContainer::ReplaceUIElement(MenuBuilder, AUITextureActor::StaticClass());
	}
	MenuBuilder.EndSection();
}

IMPLEMENT_MODULE(FLGUIEditorModule, LGUIEditor)

#undef LOCTEXT_NAMESPACE