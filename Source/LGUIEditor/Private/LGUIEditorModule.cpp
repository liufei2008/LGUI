// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "LGUIEditorModule.h"
#include "LevelEditor.h"
#include "Modules/ModuleManager.h"

#include "Core/LGUIBehaviour.h"
#include "Core/LGUISettings.h"
#include "Core/Actor/LGUIManagerActor.h"

#include "ISettingsModule.h"
#include "ISettingsSection.h"

#include "SceneOutliner/LGUISceneOutlinerInfoColumn.h"
#include "SceneOutlinerModule.h"
#include "SceneOutlinerPublicTypes.h"
#include "SceneOutliner/LGUINativeSceneOutlinerExtension.h"
#include "AssetToolsModule.h"
#include "SceneView.h"
#include "Kismet2/KismetEditorUtilities.h"

#include "Engine/CollisionProfile.h"

#include "LGUIEditorPCH.h"

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

		//actor action
		PluginCommands->MapAction(
			editorCommand.CopyActor,
			FExecuteAction::CreateStatic(&LGUIEditorTools::CopySelectedActors_Impl),
			FCanExecuteAction::CreateLambda([] {return GEditor->GetSelectedActorCount() > 0; })
		);
		PluginCommands->MapAction(
			editorCommand.PasteActor,
			FExecuteAction::CreateStatic(&LGUIEditorTools::PasteSelectedActors_Impl),
			FCanExecuteAction::CreateLambda([] {return LGUIEditorTools::HaveValidCopiedActors(); })
		);
		PluginCommands->MapAction(
			editorCommand.DuplicateActor,
			FExecuteAction::CreateStatic(&LGUIEditorTools::DuplicateSelectedActors_Impl),
			FCanExecuteAction::CreateLambda([] {return GEditor->GetSelectedActorCount() > 0; })
		);
		PluginCommands->MapAction(
			editorCommand.DestroyActor,
			FExecuteAction::CreateStatic(&LGUIEditorTools::DeleteSelectedActors_Impl),
			FCanExecuteAction::CreateLambda([] {return GEditor->GetSelectedActorCount() > 0; })
		);

		//component action
		PluginCommands->MapAction(
			editorCommand.CopyComponentValues,
			FExecuteAction::CreateStatic(&LGUIEditorTools::CopyComponentValues_Impl),
			FCanExecuteAction::CreateLambda([] {return GEditor->GetSelectedComponentCount() > 0; })
		);
		PluginCommands->MapAction(
			editorCommand.PasteComponentValues,
			FExecuteAction::CreateStatic(&LGUIEditorTools::PasteComponentValues_Impl),
			FCanExecuteAction::CreateLambda([] {return LGUIEditorTools::HaveValidCopiedComponent(); })
		);
		PluginCommands->MapAction(
			editorCommand.FocusToScreenSpaceUI,
			FExecuteAction::CreateStatic(&LGUIEditorTools::FocusToScreenSpaceUI)
		);
		PluginCommands->MapAction(
			editorCommand.FocusToSelectedUI,
			FExecuteAction::CreateStatic(&LGUIEditorTools::FocusToSelectedUI)
		);
		PluginCommands->MapAction(
			editorCommand.ActiveViewportAsLGUIPreview,
			FExecuteAction::CreateRaw(this, &FLGUIEditorModule::ToggleActiveViewportAsPreview),
			FCanExecuteAction(),
			FIsActionChecked::CreateLambda([this] {return this->bActiveViewportAsPreview; })
		);
		PluginCommands->MapAction(
			editorCommand.ToggleLGUIInfoColume,
			FExecuteAction::CreateRaw(this, &FLGUIEditorModule::ToggleLGUIColumnInfo),
			FCanExecuteAction(),
			FIsActionChecked::CreateRaw(this, &FLGUIEditorModule::LGUIColumnInfoChecked)
		);

		TSharedPtr<FExtender> toolbarExtender = MakeShareable(new FExtender);
		toolbarExtender->AddToolBarExtension("Play", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FLGUIEditorModule::AddEditorToolsToToolbarExtension));
		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(toolbarExtender);
		LevelEditorModule.GetGlobalLevelEditorActions()->Append(PluginCommands.ToSharedRef());
	}
	//register SceneOutliner ColumnInfo
	{
		ApplyLGUIColumnInfo(LGUIColumnInfoChecked(), false);
		//SceneOutliner extension
		NativeSceneOutlinerExtension = new FLGUINativeSceneOutlinerExtension();
	}
	//register window
	{
		//atlas texture viewer
		FGlobalTabmanager::Get()->RegisterNomadTabSpawner(LGUIAtlasViewerName, FOnSpawnTab::CreateRaw(this, &FLGUIEditorModule::HandleSpawnAtlasViewerTab))
			.SetDisplayName(LOCTEXT("LGUIAtlasTextureViewerName", "LGUI Atlas Texture Viewer"))
			.SetMenuType(ETabSpawnerMenuType::Hidden);
	}
	//register custom editor
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.RegisterCustomClassLayout(UUIItem::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUIItemCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(UUISpriteBase::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUISpriteBaseCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(UUISprite::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUISpriteCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(ULGUICanvas::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FLGUICanvasCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(UUIText::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUITextCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(UUITextureBase::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUITextureBaseCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(UUITexture::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUITextureCustomization::MakeInstance));

		PropertyModule.RegisterCustomClassLayout(ULGUISpriteData::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FLGUISpriteDataCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(ULGUIFontData::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FLGUIFontDataCustomization::MakeInstance));
		
		PropertyModule.RegisterCustomClassLayout(UUISelectableComponent::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUISelectableCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(UUIToggleComponent::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUIToggleCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(UUITextInputComponent::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUITextInputCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(UUIScrollViewWithScrollbarComponent::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUIScrollViewWithScrollBarCustomization::MakeInstance));

		PropertyModule.RegisterCustomClassLayout(UUILayoutBase::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUILayoutBaseCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(UUIVerticalLayout::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUIVerticalLayoutCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(UUIHorizontalLayout::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUIHorizontalLayoutCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(UUIGridLayout::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUIGridLayoutCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(UUILayoutElement::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUILayoutElementCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(ULGUICanvasScaler::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUICanvasScalerCustomization::MakeInstance));

		PropertyModule.RegisterCustomClassLayout(ULGUIPrefabHelperComponent::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FLGUIPrefabHelperComponentCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(ULGUIPrefab::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FLGUIPrefabCustomization::MakeInstance));

		PropertyModule.RegisterCustomClassLayout(UUIEffectTextAnimation::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUIEffectTextAnimationCustomization::MakeInstance));

		PropertyModule.RegisterCustomClassLayout(UUISpriteSequencePlayer::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUISpriteSequencePlayerCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(UUISpriteSheetTexturePlayer::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUISpriteSheetTexturePlayerCustomization::MakeInstance));

		PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIDrawableEvent::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FLGUIDrawableEventOneParamCustomization::MakeInstance));
		//PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIDrawableEventTwoParam::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FLGUIDrawableEventTwoParamCustomization::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIDrawableEvent_Empty::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIDrawableEventOnePresetParamCustomization::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIDrawableEvent_Bool::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIDrawableEventOnePresetParamCustomization::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIDrawableEvent_Float::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIDrawableEventOnePresetParamCustomization::MakeInstance));
		//PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIDrawableEvent_Double::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIDrawableEventOnePresetParamCustomization::MakeInstance));
		//PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIDrawableEvent_Int8::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIDrawableEventOnePresetParamCustomization::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIDrawableEvent_UInt8::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIDrawableEventOnePresetParamCustomization::MakeInstance));
		//PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIDrawableEvent_Int16::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIDrawableEventOnePresetParamCustomization::MakeInstance));
		//PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIDrawableEvent_UInt16::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIDrawableEventOnePresetParamCustomization::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIDrawableEvent_Int32::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIDrawableEventOnePresetParamCustomization::MakeInstance));
		//PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIDrawableEvent_UInt32::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIDrawableEventOnePresetParamCustomization::MakeInstance));
		//PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIDrawableEvent_Int64::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIDrawableEventOnePresetParamCustomization::MakeInstance));
		//PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIDrawableEvent_UInt64::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIDrawableEventOnePresetParamCustomization::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIDrawableEvent_Vector2::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIDrawableEventOnePresetParamCustomization::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIDrawableEvent_Vector3::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIDrawableEventOnePresetParamCustomization::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIDrawableEvent_Vector4::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIDrawableEventOnePresetParamCustomization::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIDrawableEvent_Color::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIDrawableEventOnePresetParamCustomization::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIDrawableEvent_LinearColor::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIDrawableEventOnePresetParamCustomization::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIDrawableEvent_Quaternion::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIDrawableEventOnePresetParamCustomization::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIDrawableEvent_String::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIDrawableEventOnePresetParamCustomization::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIDrawableEvent_Object::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIDrawableEventOnePresetParamCustomization::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIDrawableEvent_Actor::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIDrawableEventOnePresetParamCustomization::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIDrawableEvent_PointerEvent::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIDrawableEventOnePresetParamCustomization::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIDrawableEvent_Class::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIDrawableEventOnePresetParamCustomization::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIDrawableEvent_Rotator::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIDrawableEventOnePresetParamCustomization::MakeInstance));

		PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIComponentReference::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FLGUIComponentRefereceCustomization::MakeInstance));
	}
	//register asset
	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		//register AssetCategory
		EAssetTypeCategories::Type LGUIAssetCategoryBit = AssetTools.FindAdvancedAssetCategory(FName(TEXT("LGUI")));
		if (LGUIAssetCategoryBit == EAssetTypeCategories::Misc)
		{
			LGUIAssetCategoryBit = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("LGUI")), LOCTEXT("LGUIAssetCategory", "LGUI"));
		}

		TSharedPtr<FAssetTypeActions_Base> spriteDataAction = MakeShareable(new FLGUISpriteDataTypeAction(LGUIAssetCategoryBit));
		TSharedPtr<FAssetTypeActions_Base> fontDataAction = MakeShareable(new FLGUIFontDataTypeAction(LGUIAssetCategoryBit));
		AssetTools.RegisterAssetTypeActions(spriteDataAction.ToSharedRef());
		AssetTools.RegisterAssetTypeActions(fontDataAction.ToSharedRef());
		AssetTypeActionsArray.Add(spriteDataAction);
		AssetTypeActionsArray.Add(fontDataAction);
	}
	//register Thumbnail
	{
		UThumbnailManager::Get().RegisterCustomRenderer(ULGUIPrefab::StaticClass(), ULGUIPrefabThumbnailRenderer::StaticClass());
		UThumbnailManager::Get().RegisterCustomRenderer(ULGUISpriteData::StaticClass(), ULGUISpriteThumbnailRenderer::StaticClass());
		UThumbnailManager::Get().RegisterCustomRenderer(ULGUISpriteData_BaseObject::StaticClass(), ULGUISpriteDataBaseObjectThumbnailRenderer::StaticClass());
	}
	//register right mouse button in content browser
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
			SettingsModule->RegisterSettings("Project", "Plugins", "LGUI",
				LOCTEXT("LGUISettingsName", "LGUI"),
				LOCTEXT("LGUISettingsDescription", "LGUI Settings"),
				GetMutableDefault<ULGUISettings>());
			SettingsModule->RegisterSettings("Project", "Plugins", "LGUI Editor",
				LOCTEXT("LGUIEditorSettingsName", "LGUIEditor"),
				LOCTEXT("LGUIEditorSettingsDescription", "LGUI Editor Settings"),
				GetMutableDefault<ULGUIEditorSettings>());
		}
	}
	//blueprint
	{
		FKismetEditorUtilities::RegisterAutoGeneratedDefaultEvent(this, ULGUIBehaviour::StaticClass(), TEXT("AwakeBP"));
		FKismetEditorUtilities::RegisterAutoGeneratedDefaultEvent(this, ULGUIBehaviour::StaticClass(), TEXT("StartBP"));
		FKismetEditorUtilities::RegisterAutoGeneratedDefaultEvent(this, ULGUIBehaviour::StaticClass(), TEXT("UpdateBP"));

		FKismetEditorUtilities::RegisterAutoGeneratedDefaultEvent(this, UUISelectableTransitionComponent::StaticClass(), TEXT("OnNormalBP"));
		FKismetEditorUtilities::RegisterAutoGeneratedDefaultEvent(this, UUISelectableTransitionComponent::StaticClass(), TEXT("OnHighlightedBP"));
		FKismetEditorUtilities::RegisterAutoGeneratedDefaultEvent(this, UUISelectableTransitionComponent::StaticClass(), TEXT("OnPressedBP"));
		FKismetEditorUtilities::RegisterAutoGeneratedDefaultEvent(this, UUISelectableTransitionComponent::StaticClass(), TEXT("OnDisabledBP"));
		FKismetEditorUtilities::RegisterAutoGeneratedDefaultEvent(this, UUISelectableTransitionComponent::StaticClass(), TEXT("OnStartCustomTransitionBP"));
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

	//unregister SceneOutliner ColumnInfo
	{
		FSceneOutlinerModule& SceneOutlinerModule = FModuleManager::LoadModuleChecked< FSceneOutlinerModule >("SceneOutliner");
		SceneOutlinerModule.UnRegisterColumnType<LGUISceneOutliner::FLGUISceneOutlinerInfoColumn>();
		delete NativeSceneOutlinerExtension;
		NativeSceneOutlinerExtension = nullptr;
	}
	//unregister window
	{
		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(LGUIAtlasViewerName);
	}
	//unregister custom editor
	if (UObjectInitialized() && FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomClassLayout(UUIItem::StaticClass()->GetFName());
		PropertyModule.UnregisterCustomClassLayout(UUISpriteBase::StaticClass()->GetFName());
		PropertyModule.UnregisterCustomClassLayout(UUISprite::StaticClass()->GetFName());
		PropertyModule.UnregisterCustomClassLayout(ULGUICanvas::StaticClass()->GetFName());
		PropertyModule.UnregisterCustomClassLayout(UUIText::StaticClass()->GetFName());
		PropertyModule.UnregisterCustomClassLayout(UUITextureBase::StaticClass()->GetFName());

		PropertyModule.UnregisterCustomClassLayout(ULGUISpriteData::StaticClass()->GetFName());
		PropertyModule.UnregisterCustomClassLayout(ULGUIFontData::StaticClass()->GetFName());

		PropertyModule.UnregisterCustomClassLayout(UUISelectableComponent::StaticClass()->GetFName());
		PropertyModule.UnregisterCustomClassLayout(UUIToggleComponent::StaticClass()->GetFName());
		PropertyModule.UnregisterCustomClassLayout(UUITextInputComponent::StaticClass()->GetFName());
		PropertyModule.UnregisterCustomClassLayout(UUIScrollViewWithScrollbarComponent::StaticClass()->GetFName());

		PropertyModule.UnregisterCustomClassLayout(UUILayoutBase::StaticClass()->GetFName());
		PropertyModule.UnregisterCustomClassLayout(UUIVerticalLayout::StaticClass()->GetFName());
		PropertyModule.UnregisterCustomClassLayout(UUIHorizontalLayout::StaticClass()->GetFName());
		PropertyModule.UnregisterCustomClassLayout(UUIGridLayout::StaticClass()->GetFName());
		PropertyModule.UnregisterCustomClassLayout(UUILayoutElement::StaticClass()->GetFName());

		PropertyModule.UnregisterCustomClassLayout(ULGUIPrefabHelperComponent::StaticClass()->GetFName());
		PropertyModule.UnregisterCustomClassLayout(ULGUIPrefab::StaticClass()->GetFName());

		PropertyModule.UnregisterCustomClassLayout(UUIEffectTextAnimation_Property::StaticClass()->GetFName());
		PropertyModule.UnregisterCustomClassLayout(UUIEffectTextAnimation::StaticClass()->GetFName());

		PropertyModule.UnregisterCustomClassLayout(UUISpriteSequencePlayer::StaticClass()->GetFName());
		PropertyModule.UnregisterCustomClassLayout(UUISpriteSheetTexturePlayer::StaticClass()->GetFName());

		PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIDrawableEvent::StaticStruct()->GetFName());
		PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIDrawableEvent::StaticStruct()->GetFName());
		//PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIDrawableEventTwoParam::StaticStruct()->GetFName());
		PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIDrawableEvent_Empty::StaticStruct()->GetFName());
		PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIDrawableEvent_Bool::StaticStruct()->GetFName());
		PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIDrawableEvent_Float::StaticStruct()->GetFName());
		//PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIDrawableEvent_Double::StaticStruct()->GetFName());
		//PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIDrawableEvent_Int8::StaticStruct()->GetFName());
		PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIDrawableEvent_UInt8::StaticStruct()->GetFName());
		//PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIDrawableEvent_Int16::StaticStruct()->GetFName());
		//PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIDrawableEvent_UInt16::StaticStruct()->GetFName());
		PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIDrawableEvent_Int32::StaticStruct()->GetFName());
		//PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIDrawableEvent_UInt32::StaticStruct()->GetFName());
		//PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIDrawableEvent_Int64::StaticStruct()->GetFName());
		//PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIDrawableEvent_UInt64::StaticStruct()->GetFName());
		PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIDrawableEvent_Vector2::StaticStruct()->GetFName());
		PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIDrawableEvent_Vector3::StaticStruct()->GetFName());
		PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIDrawableEvent_Vector4::StaticStruct()->GetFName());
		PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIDrawableEvent_Color::StaticStruct()->GetFName());
		PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIDrawableEvent_LinearColor::StaticStruct()->GetFName());
		PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIDrawableEvent_Quaternion::StaticStruct()->GetFName());
		PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIDrawableEvent_String::StaticStruct()->GetFName());
		PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIDrawableEvent_Object::StaticStruct()->GetFName());
		PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIDrawableEvent_Actor::StaticStruct()->GetFName());
		PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIDrawableEvent_PointerEvent::StaticStruct()->GetFName());
		PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIDrawableEvent_Class::StaticStruct()->GetFName());
		PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIDrawableEvent_Rotator::StaticStruct()->GetFName());

		PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIComponentReference::StaticStruct()->GetFName());
	}
	//unregister asset
	{
		if (FModuleManager::Get().IsModuleLoaded(TEXT("AssetTools")))
		{
			IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
			for (TSharedPtr<FAssetTypeActions_Base>& AssetTypeActions : AssetTypeActionsArray)
			{
				AssetTools.UnregisterAssetTypeActions(AssetTypeActions.ToSharedRef());
			}
		}
		AssetTypeActionsArray.Empty();
	}
	//unregister thumbnail
	if (UObjectInitialized())
	{
		UThumbnailManager::Get().UnregisterCustomRenderer(ULGUIPrefab::StaticClass());
		UThumbnailManager::Get().UnregisterCustomRenderer(ULGUISpriteData::StaticClass());
		UThumbnailManager::Get().UnregisterCustomRenderer(ULGUISpriteData_BaseObject::StaticClass());
	}
	//unregister right mouse button in content browser
	{
		FLGUIContentBrowserExtensions::RemoveHooks();
		FLGUILevelEditorExtensions::RemoveHooks();
	}

	//unregister setting
	{
		if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
		{
			SettingsModule->UnregisterSettings("Project", "Plugins", "LGUI");
			SettingsModule->UnregisterSettings("Project", "Plugins", "LGUI Editor");
		}
	}

	FKismetEditorUtilities::UnregisterAutoBlueprintNodeCreation(this);
}

TSharedRef<SDockTab> FLGUIEditorModule::HandleSpawnAtlasViewerTab(const FSpawnTabArgs& SpawnTabArgs)
{
	auto ResultTab = SNew(SDockTab).TabRole(ETabRole::NomadTab);
	auto TabContentWidget = SNew(SLGUIAtlasViewer, ResultTab);
	ResultTab->SetContent(TabContentWidget);
	return ResultTab;
}

bool FLGUIEditorModule::CanEditActorForPrefab()
{
	return GEditor->GetSelectedActorCount() == 1 && LGUIEditorTools::GetPrefabActor_WhichManageThisActor(LGUIEditorTools::GetFirstSelectedActor()) != nullptr;
}

void FLGUIEditorModule::AddEditorToolsToToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.BeginSection("LGUI");
	{
		Builder.AddComboButton(
			FUIAction(),
			FOnGetContent::CreateRaw(this, &FLGUIEditorModule::MakeEditorToolsMenu, false),
			LOCTEXT("LGUITools", "LGUI Tools"),
			LOCTEXT("LGUIEditorTools", "LGUI Editor Tools"),
			FSlateIcon(FLGUIEditorStyle::GetStyleSetName(), "LGUIEditor.EditorTools")
		);
	}
	Builder.EndSection();
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
			FUIAction(FExecuteAction::CreateStatic(&LGUIEditorTools::CreatePrefabAsset)
				, FCanExecuteAction::CreateLambda([] {return GEditor->GetSelectedActorCount() > 0; }))
		);
		MenuBuilder.AddMenuEntry(
			LOCTEXT("ApplyPrefab", "Apply Prefab"),
			LOCTEXT("Apply_Tooltip", "Apply change and save prefab to asset"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&LGUIEditorTools::ApplyPrefab)
				, FCanExecuteAction::CreateRaw(this, &FLGUIEditorModule::CanEditActorForPrefab)
				, FGetActionCheckState()
				, FIsActionButtonVisible::CreateRaw(this, &FLGUIEditorModule::CanEditActorForPrefab))
		);
		MenuBuilder.AddMenuEntry(
			LOCTEXT("RevertPrefab", "Revert Prefab"),
			LOCTEXT("Revert_Tooltip", "Revert any changes"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&LGUIEditorTools::RevertPrefab)
				, FCanExecuteAction::CreateRaw(this, &FLGUIEditorModule::CanEditActorForPrefab)
				, FGetActionCheckState()
				, FIsActionButtonVisible::CreateRaw(this, &FLGUIEditorModule::CanEditActorForPrefab))
		);
		MenuBuilder.AddMenuEntry(
			LOCTEXT("DeletePrefab", "Delete this Prefab instance"),
			LOCTEXT("Delete_Tooltip", "Delete actors created by this prefab with hierarchy"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&LGUIEditorTools::DeletePrefab)
				, FCanExecuteAction::CreateRaw(this, &FLGUIEditorModule::CanEditActorForPrefab)
				, FGetActionCheckState()
				, FIsActionButtonVisible::CreateRaw(this, &FLGUIEditorModule::CanEditActorForPrefab))
		);
		MenuBuilder.AddMenuEntry(
			LOCTEXT("UnlinkPrefab", "Unlink this Prefab"),
			LOCTEXT("UnlinkPrefab_Tooltip", "Unlink the actor from related prefab asset"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&LGUIEditorTools::UnlinkPrefab)
				, FCanExecuteAction::CreateRaw(this, &FLGUIEditorModule::CanEditActorForPrefab)
				, FGetActionCheckState()
				, FIsActionButtonVisible::CreateRaw(this, &FLGUIEditorModule::CanEditActorForPrefab))
		);
		MenuBuilder.AddMenuEntry(
			LOCTEXT("SelectPrefabAsset", "Browse to Prefab asset"),
			LOCTEXT("SelectPrefabAsset_Tooltip", "Browse to Prefab asset in Content Browser"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&LGUIEditorTools::SelectPrefabAsset)
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
			FUIAction(FExecuteAction::CreateStatic(&LGUIEditorTools::CreateEmptyActor))
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
			LOCTEXT("CreateUIPostProcessSubMenu", "Create UI Post Process"),
			LOCTEXT("CreateUIPostProcessSubMenu_Tooltip", "Create UI Post Process"),
			FNewMenuDelegate::CreateRaw(this, &FLGUIEditorModule::CreateUIPostProcessSubMenu)
		);
		if (!IsSceneOutlineMenu)
		{
			MenuBuilder.AddSubMenu(
				LOCTEXT("BasicSetup", "Basic Setup"),
				LOCTEXT("BasicSetup", "Basic Setup"),
				FNewMenuDelegate::CreateRaw(this, &FLGUIEditorModule::BasicSetupSubMenu)
			);
		}
		MenuBuilder.AddSubMenu(
			LOCTEXT("ReplaceUIElement", "Replace this by..."),
			LOCTEXT("ReplaceUIElement_Tooltip", "Replace UI Element with..."),
			FNewMenuDelegate::CreateRaw(this, &FLGUIEditorModule::ReplaceUIElementSubMenu),
			FUIAction(FExecuteAction(), FCanExecuteAction::CreateLambda([] {return GEditor->GetSelectedActorCount() > 0; })),
			NAME_None,
			EUserInterfaceActionType::Button
		);
		MenuBuilder.AddSubMenu(
			LOCTEXT("Layout", "Attach Layout"),
			LOCTEXT("Layout_Tooltip", "Attach Layout to selected UI Element"),
			FNewMenuDelegate::CreateRaw(this, &FLGUIEditorModule::AttachLayout),
			FUIAction(FExecuteAction(), FCanExecuteAction::CreateStatic(&LGUIEditorTools::IsSelectUIActor)),
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
		MenuBuilder.AddMenuEntry(commandList.DestroyActor);
		MenuBuilder.AddSubMenu(
			LOCTEXT("ChangeCollisionChannelSubMenu", "Change Trace Channel"),
			LOCTEXT("ChangeCollisionChannelSubMenu_Tooltip", "Change a UI element's trace channel to selected channel, with hierarchy"),
			FNewMenuDelegate::CreateRaw(this, &FLGUIEditorModule::ChangeTraceChannelSubMenu),
			FUIAction(FExecuteAction(), FCanExecuteAction::CreateStatic(&LGUIEditorTools::IsSelectUIActor)),
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

		MenuBuilder.BeginSection("OpenWindow", LOCTEXT("OpenWindow", "Open Window"));
		{
			MenuBuilder.AddMenuEntry(
				LOCTEXT("OpenAtlasViewer", "Open LGUI atlas viewer"),
				LOCTEXT("OpenAtlasViewer_Tooltip", "Open LGUI atlas viewer"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&LGUIEditorTools::OpenAtlasViewer_Impl))
			);
			//MenuBuilder.AddMenuEntry(FLGUIEditorCommands::Get().OpenScreenSpaceUIViewer);
		}
		MenuBuilder.EndSection();

		MenuBuilder.BeginSection("PreviewScreenSpaceUISelector", LOCTEXT("PreviewScreenSpaceUISelector", "Preview Viewport"));
		{
			MenuBuilder.AddMenuEntry(commandList.ActiveViewportAsLGUIPreview);
		}
		MenuBuilder.EndSection();

		MenuBuilder.BeginSection("EditorCamera", LOCTEXT("EditorCameraControl", "EditorCameraControl"));
		{
			MenuBuilder.AddMenuEntry(commandList.FocusToScreenSpaceUI);
			MenuBuilder.AddMenuEntry(commandList.FocusToSelectedUI);
		}
		MenuBuilder.EndSection();

		MenuBuilder.BeginSection("Others", LOCTEXT("Others", "Others"));
		{
			MenuBuilder.AddMenuEntry(commandList.ToggleLGUIInfoColume);
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
				FUIAction(FExecuteAction::CreateStatic(&LGUIEditorTools::CreateUIItemActor, InClass))
			);
		}
		static void CreateUIControlMenuEntry(FMenuBuilder& InBuilder, const FString& InControlName, const FString& InPrefabPath, FText InTooltip = FText())
		{
			if (InTooltip.IsEmpty())
			{
				InTooltip = FText::FromString(FString::Printf(TEXT("Create %s"), *InControlName));
			}
			InBuilder.AddMenuEntry(
				FText::FromString(InControlName),
				InTooltip,
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&LGUIEditorTools::CreateUIControls, InPrefabPath))
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
		FunctionContainer::CreateUIControlMenuEntry(MenuBuilder, TEXT("Dropdown"), "/LGUI/Prefabs/DefaultDropdown");
		FunctionContainer::CreateUIControlMenuEntry(MenuBuilder, TEXT("TextInput"), "/LGUI/Prefabs/DefaultTextInput");
		FunctionContainer::CreateUIControlMenuEntry(MenuBuilder, TEXT("TextInputMultiline"), "/LGUI/Prefabs/DefaultTextInputMultiline");
		FunctionContainer::CreateUIControlMenuEntry(MenuBuilder, TEXT("ScrollView"), "/LGUI/Prefabs/DefaultScrollView");
	}
	MenuBuilder.EndSection();
}

void FLGUIEditorModule::UseActiveViewportAsPreview()
{
	if (auto viewport = GEditor->GetActiveViewport())
	{
		if (auto viewportClient = viewport->GetClient())
		{
			if (auto editorViewportClient = (FEditorViewportClient*)(viewportClient))
			{
				ULGUIEditorSettings::SetLGUIPreview_EditorViewIndex(editorViewportClient->ViewIndex);
			}
		}
	}
}
void FLGUIEditorModule::ClearViewportPreview()
{
	ULGUIEditorSettings::SetLGUIPreview_EditorViewIndex(-1);
}
void FLGUIEditorModule::ToggleActiveViewportAsPreview()
{
	bActiveViewportAsPreview = !bActiveViewportAsPreview;
	if (bActiveViewportAsPreview)
	{
		UseActiveViewportAsPreview();
	}
	else
	{
		ClearViewportPreview();
	}
}

void FLGUIEditorModule::ToggleLGUIColumnInfo()
{
	auto LGUIEditorSettings = GetMutableDefault<ULGUIEditorSettings>();
	LGUIEditorSettings->ShowLGUIColumnInSceneOutliner = !LGUIEditorSettings->ShowLGUIColumnInSceneOutliner;
	LGUIEditorSettings->SaveConfig();

	ApplyLGUIColumnInfo(LGUIEditorSettings->ShowLGUIColumnInSceneOutliner, true);
}
bool FLGUIEditorModule::LGUIColumnInfoChecked()
{
	return GetDefault<ULGUIEditorSettings>()->ShowLGUIColumnInSceneOutliner;
}
void FLGUIEditorModule::ApplyLGUIColumnInfo(bool value, bool refreshSceneOutliner)
{
	FSceneOutlinerModule& SceneOutlinerModule = FModuleManager::LoadModuleChecked< FSceneOutlinerModule >("SceneOutliner");
	if (value)
	{
		FSceneOutlinerColumnInfo ColumnInfo(ESceneOutlinerColumnVisibility::Visible, 15, FCreateSceneOutlinerColumn::CreateStatic(&LGUISceneOutliner::FLGUISceneOutlinerInfoColumn::MakeInstance));
		SceneOutlinerModule.RegisterDefaultColumnType<LGUISceneOutliner::FLGUISceneOutlinerInfoColumn>(ColumnInfo);
	}
	else
	{
		SceneOutlinerModule.UnRegisterColumnType<LGUISceneOutliner::FLGUISceneOutlinerInfoColumn>();
	}

	//refresh scene outliner
	if (refreshSceneOutliner)
	{
		FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

		TSharedPtr<FTabManager> LevelEditorTabManager = LevelEditorModule.GetLevelEditorTabManager();
		if (LevelEditorTabManager->FindExistingLiveTab(FName("LevelEditorSceneOutliner")).IsValid())
		{
			if (LevelEditorTabManager.IsValid() && LevelEditorTabManager.Get())
			{
				if (LevelEditorTabManager->GetOwnerTab().IsValid())
				{
					LevelEditorTabManager->TryInvokeTab(FName("LevelEditorSceneOutliner"))->RequestCloseTab();
				}
			}

			if (LevelEditorTabManager.IsValid() && LevelEditorTabManager.Get())
			{
				if (LevelEditorTabManager->GetOwnerTab().IsValid())
				{
					LevelEditorTabManager->TryInvokeTab(FName("LevelEditorSceneOutliner"));
				}
			}
		}
	}
}

void FLGUIEditorModule::CreateUIPostProcessSubMenu(FMenuBuilder& MenuBuilder)
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
				FUIAction(FExecuteAction::CreateStatic(&LGUIEditorTools::CreateUIItemActor, InClass))
			);
		}
	};

	MenuBuilder.BeginSection("UIPostProcessRenderable");
	{
		for (TObjectIterator<UClass> ClassItr; ClassItr; ++ClassItr)
		{
			if (ClassItr->IsChildOf(AUIPostProcessBaseActor::StaticClass()))
			{
				if (
					   !(ClassItr->HasAnyClassFlags(CLASS_Transient))
					&& !(ClassItr->HasAnyClassFlags(CLASS_Abstract))
					&& !(ClassItr->HasAnyClassFlags(CLASS_Deprecated))
					)
				{
					bool isBlueprint = ClassItr->HasAnyClassFlags(CLASS_CompiledFromBlueprint);
					if (isBlueprint)
					{
						if (ClassItr->GetName().StartsWith(TEXT("SKEL_")))
						{
							continue;
						}
					}
					FunctionContainer::CreateUIBaseElementMenuEntry(MenuBuilder, *ClassItr);
				}
			}
		}
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
				FUIAction(FExecuteAction::CreateStatic(&LGUIEditorTools::CreateUIItemActor, InClass))
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
					&& !(*ClassItr)->IsChildOf(AUIPostProcessBaseActor::StaticClass())
					&& !(ClassItr->HasAnyClassFlags(CLASS_Transient))
					&& !(ClassItr->HasAnyClassFlags(CLASS_Abstract))
					&& !(ClassItr->HasAnyClassFlags(CLASS_Deprecated))
					)
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
			FUIAction(FExecuteAction::CreateStatic(&LGUIEditorTools::CreateScreenSpaceUI_BasicSetup))
		);
		MenuBuilder.AddMenuEntry(
			FText::FromString(TEXT("World Space UI - UE Renderer")),
			FText::FromString(FString::Printf(TEXT("Render in world space by UE default render pipeline.\n This mode use engine's default render pieple, so post process will affect ui."))),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&LGUIEditorTools::CreateWorldSpaceUIUERenderer_BasicSetup))
		);
		MenuBuilder.AddMenuEntry(
			FText::FromString(TEXT("World Space UI - LGUI Renderer")),
			FText::FromString(FString::Printf(TEXT("Render in world space by LGUI's custom render pipeline.\n This mode use LGUI's custom render pipeline, will not be affected by post process."))),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&LGUIEditorTools::CreateWorldSpaceUILGUIRenderer_BasicSetup))
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
					FUIAction(FExecuteAction::CreateStatic(&LGUIEditorTools::ChangeTraceChannel_Impl, (ETraceTypeQuery)i))
				);
			}
		}
	}
	MenuBuilder.EndSection();
}

void FLGUIEditorModule::AttachLayout(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.BeginSection("Layout");
	{
		MenuBuilder.AddMenuEntry(
			FText::FromString(TEXT("Horizontal Layout")),
			FText::FromString(FString::Printf(TEXT("Layout child elements side by side horizontally"))),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&LGUIEditorTools::AttachComponentToSelectedActor, TSubclassOf<UActorComponent>(UUIHorizontalLayout::StaticClass())))
		);
		MenuBuilder.AddMenuEntry(
			FText::FromString(TEXT("Vertical Layout")),
			FText::FromString(FString::Printf(TEXT("Layout child elements side by side vertically"))),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&LGUIEditorTools::AttachComponentToSelectedActor, TSubclassOf<UActorComponent>(UUIVerticalLayout::StaticClass())))
		);
		MenuBuilder.AddMenuEntry(
			FText::FromString(TEXT("Grid Layout")),
			FText::FromString(FString::Printf(TEXT("Layout child elements in grid"))),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&LGUIEditorTools::AttachComponentToSelectedActor, TSubclassOf<UActorComponent>(UUIGridLayout::StaticClass())))
		);
		MenuBuilder.AddMenuEntry(
			FText::FromString(TEXT("Rounded Layout")),
			FText::FromString(FString::Printf(TEXT("Rounded layout, only affect children's position and angle, not affect size"))),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&LGUIEditorTools::AttachComponentToSelectedActor, TSubclassOf<UActorComponent>(UUIRoundedLayout::StaticClass())))
		);
		MenuBuilder.AddMenuEntry(
			FText::FromString(TEXT("Layout Element")),
			FText::FromString(FString::Printf(TEXT("Attach to layout's child, make is specific or ignore layout"))),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&LGUIEditorTools::AttachComponentToSelectedActor, TSubclassOf<UActorComponent>(UUILayoutElement::StaticClass())))
		);
		MenuBuilder.AddMenuEntry(
			FText::FromString(TEXT("Size Control by Aspect Ratio")),
			FText::FromString(FString::Printf(TEXT("Use aspect ratio to control with and height"))),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&LGUIEditorTools::AttachComponentToSelectedActor, TSubclassOf<UActorComponent>(UUISizeControlByAspectRatio::StaticClass())))
		);
		MenuBuilder.AddMenuEntry(
			FText::FromString(TEXT("Size Control by Other")),
			FText::FromString(FString::Printf(TEXT("Use other UI element to control the size of this one"))),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&LGUIEditorTools::AttachComponentToSelectedActor, TSubclassOf<UActorComponent>(UUISizeControlByOther::StaticClass())))
		);
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
				FUIAction(FExecuteAction::CreateStatic(&LGUIEditorTools::ReplaceUIElementWith, InClass))
			);
		}
	};

	MenuBuilder.BeginSection("Replace");
	{
		FunctionContainer::ReplaceUIElement(MenuBuilder, AUIContainerActor::StaticClass());
		FunctionContainer::ReplaceUIElement(MenuBuilder, AUISpriteActor::StaticClass());
		FunctionContainer::ReplaceUIElement(MenuBuilder, AUITextActor::StaticClass());
		FunctionContainer::ReplaceUIElement(MenuBuilder, AUITextureActor::StaticClass());

		for (TObjectIterator<UClass> ClassItr; ClassItr; ++ClassItr)
		{
			if (ClassItr->IsChildOf(AUIBaseActor::StaticClass()))
			{
				if (*ClassItr != AUIContainerActor::StaticClass()
					&& *ClassItr != AUISpriteActor::StaticClass()
					&& *ClassItr != AUITextActor::StaticClass()
					&& *ClassItr != AUITextureActor::StaticClass()
					&& !(ClassItr->HasAnyClassFlags(CLASS_Transient))
					&& !(ClassItr->HasAnyClassFlags(CLASS_Abstract))
					&& !(ClassItr->HasAnyClassFlags(CLASS_Deprecated))
					)
				{
					bool isBlueprint = ClassItr->HasAnyClassFlags(CLASS_CompiledFromBlueprint);
					if (isBlueprint)
					{
						if (ClassItr->GetName().StartsWith(TEXT("SKEL_")))
						{
							continue;
						}
					}
					FunctionContainer::ReplaceUIElement(MenuBuilder, *ClassItr);
				}
			}
		}
	}
	MenuBuilder.EndSection();
}

IMPLEMENT_MODULE(FLGUIEditorModule, LGUIEditor)

#undef LOCTEXT_NAMESPACE