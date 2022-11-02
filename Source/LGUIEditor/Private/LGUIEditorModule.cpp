﻿// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "LGUIEditorModule.h"
#include "LevelEditor.h"
#include "Modules/ModuleManager.h"

#include "LGUIHeaders.h"
#include "Core/LGUISettings.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "PrefabSystem/LGUIPrefabHelperObject.h"

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

#include "LGUIEditorStyle.h"
#include "LGUIEditorCommands.h"
#include "LGUIEditorTools.h"

#include "Thumbnail/LGUIPrefabThumbnailRenderer.h"
#include "Thumbnail/LGUISpriteThumbnailRenderer.h"
#include "Thumbnail/LGUISpriteDataBaseObjectThumbnailRenderer.h"
#include "ContentBrowserExtensions/LGUIContentBrowserExtensions.h"
#include "LevelEditorMenuExtensions/LGUILevelEditorExtensions.h"
#include "Window/LGUIAtlasViewer.h"

#include "AssetTypeActions/AssetTypeActions_LGUISpriteData.h"
#include "AssetTypeActions/AssetTypeActions_LGUIFontData.h"
#include "AssetTypeActions/AssetTypeActions_LGUIPrefab.h"
#include "AssetTypeActions/AssetTypeActions_LGUIStaticMeshCache.h"

#include "DetailCustomization/UIItemCustomization.h"
#include "DetailCustomization/UISpriteBaseCustomization.h"
#include "DetailCustomization/UISpriteCustomization.h"
#include "DetailCustomization/UITextureCustomization.h"
#include "DetailCustomization/LGUICanvasCustomization.h"
#include "DetailCustomization/UITextCustomization.h"
#include "DetailCustomization/UITextureBaseCustomization.h"
#include "DetailCustomization/LGUISpriteDataCustomization.h"
#include "DetailCustomization/LGUIFreeTypeRenderFontDataCustomization.h"
#include "DetailCustomization/UISelectableCustomization.h"
#include "DetailCustomization/UIToggleCustomization.h"
#include "DetailCustomization/UITextInputCustomization.h"
#include "DetailCustomization/UILayoutBaseCustomization.h"
#include "DetailCustomization/UIVerticalLayoutCustomization.h"
#include "DetailCustomization/UIHorizontalLayoutCustomization.h"
#include "DetailCustomization/UIGridLayoutCustomization.h"
#include "DetailCustomization/UIFlexibleGridLayoutCustomization.h"
#include "DetailCustomization/UILayoutElementCustomization.h"
#include "DetailCustomization/UICanvasScalerCustomization.h"
#include "DetailCustomization/LGUIPrefabCustomization.h"
#include "DetailCustomization/LGUIEventDelegateCustomization.h"
#include "DetailCustomization/LGUIEventDelegatePresetParamCustomization.h"
#include "DetailCustomization/LGUIComponentReferenceCustomization.h"
#include "DetailCustomization/UIEffectTextAnimationCustomization.h"
#include "DetailCustomization/UIScrollViewWithScrollBarCustomization.h"
#include "DetailCustomization/UISpriteSequencePlayerCustomization.h"
#include "DetailCustomization/UISpriteSheetTexturePlayerCustomization.h"
#include "DetailCustomization/UIPostProcessRenderableCustomization.h"

#include "PrefabEditor/LGUIPrefabOverrideDataViewer.h"
#include "PrefabSystem/LGUIPrefabHelperActor.h"
#include "Engine/Selection.h"

#include "PrefabAnimation/LGUIPrefabSequenceComponentCustomization.h"
#include "PrefabAnimation/MovieSceneSequenceEditor_LGUIPrefabSequence.h"
#include "BlueprintEditorModule.h"
#include "BlueprintEditorTabs.h"
#include "Framework/Docking/LayoutExtender.h"
#include "WorkflowOrientedApp/WorkflowTabManager.h"
#include "SequencerSettings.h"
#include "ISequencerModule.h"
#include "PrefabAnimation/LGUIPrefabSequenceEditor.h"
#include "MovieSceneToolsProjectSettings.h"

#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#include "AssetRegistryModule.h"

const FName FLGUIEditorModule::LGUIAtlasViewerName(TEXT("LGUIAtlasViewerName"));
const FName FLGUIEditorModule::LGUIPrefabSequenceTabName(TEXT("LGUIPrefabSequenceTabName"));

#define LOCTEXT_NAMESPACE "FLGUIEditorModule"
DEFINE_LOG_CATEGORY(LGUIEditor);

void FLGUIEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FLGUIEditorStyle::Initialize();
	FLGUIEditorStyle::ReloadTextures();

	OnInitializeSequenceHandle = ULGUIPrefabSequence::OnInitializeSequence().AddStatic(FLGUIEditorModule::OnInitializeSequence);

	ISequencerModule& SequencerModule = FModuleManager::Get().LoadModuleChecked<ISequencerModule>("Sequencer");
	SequenceEditorHandle = SequencerModule.RegisterSequenceEditor(ULGUIPrefabSequence::StaticClass(), MakeUnique<FMovieSceneSequenceEditor_LGUIPrefabSequence>());

	//ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

	//if (SettingsModule != nullptr)
	//{
	//	Settings = USequencerSettingsContainer::GetOrCreate<USequencerSettings>(TEXT("EmbeddedLGUIPrefabSequenceEditor"));

	//	SettingsModule->RegisterSettings("Editor", "ContentEditors", "EmbeddedLGUIPrefabSequenceEditor",
	//		LOCTEXT("EmbeddedLGUIPrefabSequenceEditorSettingsName", "Embedded Actor Sequence Editor"),
	//		LOCTEXT("EmbeddedLGUIPrefabSequenceEditorSettingsDescription", "Configure the look and feel of the Embedded Actor Sequence Editor."),
	//		Settings);
	//}

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
			FCanExecuteAction::CreateStatic(&LGUIEditorTools::CanCopyActor),
			FGetActionCheckState(),
			FIsActionButtonVisible::CreateStatic(&LGUIEditorTools::CanCopyActor)
		);
		PluginCommands->MapAction(
			editorCommand.PasteActor,
			FExecuteAction::CreateStatic(&LGUIEditorTools::CutSelectedActors_Impl),
			FCanExecuteAction::CreateStatic(&LGUIEditorTools::CanCutActor),
			FGetActionCheckState(),
			FIsActionButtonVisible::CreateStatic(&LGUIEditorTools::CanCutActor)
		);
		PluginCommands->MapAction(
			editorCommand.PasteActor,
			FExecuteAction::CreateStatic(&LGUIEditorTools::PasteSelectedActors_Impl),
			FCanExecuteAction::CreateStatic(&LGUIEditorTools::CanPasteActor),
			FGetActionCheckState(),
			FIsActionButtonVisible::CreateStatic(&LGUIEditorTools::CanPasteActor)
		);
		PluginCommands->MapAction(
			editorCommand.DuplicateActor,
			FExecuteAction::CreateStatic(&LGUIEditorTools::DuplicateSelectedActors_Impl),
			FCanExecuteAction::CreateStatic(&LGUIEditorTools::CanDuplicateActor),
			FGetActionCheckState(),
			FIsActionButtonVisible::CreateStatic(&LGUIEditorTools::CanDuplicateActor)
		);
		PluginCommands->MapAction(
			editorCommand.DestroyActor,
			FExecuteAction::CreateStatic(&LGUIEditorTools::DeleteSelectedActors_Impl),
			FCanExecuteAction::CreateStatic(&LGUIEditorTools::CanDeleteActor),
			FGetActionCheckState(),
			FIsActionButtonVisible::CreateStatic(&LGUIEditorTools::CanDeleteActor)
		);

		//component action
		PluginCommands->MapAction(
			editorCommand.CopyComponentValues,
			FExecuteAction::CreateStatic(&LGUIEditorTools::CopyComponentValues_Impl),
			FCanExecuteAction::CreateLambda([] {return GEditor->GetSelectedComponentCount() > 0; }),
			FGetActionCheckState(),
			FIsActionButtonVisible::CreateLambda([] {return GEditor->GetSelectedComponentCount() > 0; })
		);
		PluginCommands->MapAction(
			editorCommand.PasteComponentValues,
			FExecuteAction::CreateStatic(&LGUIEditorTools::PasteComponentValues_Impl),
			FCanExecuteAction::CreateLambda([] {return LGUIEditorTools::HaveValidCopiedComponent(); }),
			FGetActionCheckState(),
			FIsActionButtonVisible::CreateLambda([] {return LGUIEditorTools::HaveValidCopiedComponent(); })
		);
		//view
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
		//settings
		PluginCommands->MapAction(
			editorCommand.ToggleLGUIInfoColume,
			FExecuteAction::CreateRaw(this, &FLGUIEditorModule::ToggleLGUIColumnInfo),
			FCanExecuteAction(),
			FIsActionChecked::CreateRaw(this, &FLGUIEditorModule::LGUIColumnInfoChecked)
		);
		PluginCommands->MapAction(
			editorCommand.ToggleDrawHelperFrame,
			FExecuteAction::CreateRaw(this, &FLGUIEditorModule::ToggleDrawHelperFrame),
			FCanExecuteAction(),
			FIsActionChecked::CreateRaw(this, &FLGUIEditorModule::GetDrawHelperFrameChecked)
		);
		//gc
		PluginCommands->MapAction(
			editorCommand.ForceGC,
			FExecuteAction::CreateStatic(&LGUIEditorTools::ForceGC)
		);

		TSharedPtr<FExtender> toolbarExtender = MakeShareable(new FExtender);
		toolbarExtender->AddToolBarExtension("Game", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FLGUIEditorModule::AddEditorToolsToToolbarExtension));
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
		FGlobalTabmanager::Get()->RegisterNomadTabSpawner(LGUIPrefabSequenceTabName, FOnSpawnTab::CreateRaw(this, &FLGUIEditorModule::HandleSpawnLGUIPrefabSequenceTab))
			.SetDisplayName(LOCTEXT("LGUIPrefabSequenceTabName", "LGUI Prefab Sequence"))
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
		PropertyModule.RegisterCustomClassLayout(UUIPostProcessRenderable::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUIPostProcessRenderableCustomization::MakeInstance));

		PropertyModule.RegisterCustomClassLayout(ULGUISpriteData::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FLGUISpriteDataCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(ULGUIFreeTypeRenderFontData::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FLGUIFreeTypeRenderFontDataCustomization::MakeInstance));
		
		PropertyModule.RegisterCustomClassLayout(UUISelectableComponent::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUISelectableCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(UUIToggleComponent::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUIToggleCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(UUITextInputComponent::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUITextInputCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(UUIScrollViewWithScrollbarComponent::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUIScrollViewWithScrollBarCustomization::MakeInstance));

		PropertyModule.RegisterCustomClassLayout(UUILayoutBase::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUILayoutBaseCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(UUIVerticalLayout::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUIVerticalLayoutCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(UUIHorizontalLayout::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUIHorizontalLayoutCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(UUIGridLayout::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUIGridLayoutCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(UUIFlexibleGridLayout::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUIFlexibleGridLayoutCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(UUILayoutElement::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUILayoutElementCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(ULGUICanvasScaler::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUICanvasScalerCustomization::MakeInstance));

		PropertyModule.RegisterCustomClassLayout(ULGUIPrefab::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FLGUIPrefabCustomization::MakeInstance));

		PropertyModule.RegisterCustomClassLayout(UUIEffectTextAnimation::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUIEffectTextAnimationCustomization::MakeInstance));

		PropertyModule.RegisterCustomClassLayout(UUISpriteSequencePlayer::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUISpriteSequencePlayerCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(UUISpriteSheetTexturePlayer::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUISpriteSheetTexturePlayerCustomization::MakeInstance));

		PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIEventDelegate::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FLGUIEventDelegateCustomization::MakeInstance));
		//PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIEventDelegateTwoParam::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FLGUIEventDelegateTwoParamCustomization::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIEventDelegate_Empty::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIEventDelegatePresetParamCustomization::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIEventDelegate_Bool::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIEventDelegatePresetParamCustomization::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIEventDelegate_Float::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIEventDelegatePresetParamCustomization::MakeInstance));
		//PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIEventDelegate_Double::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIEventDelegatePresetParamCustomization::MakeInstance));
		//PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIEventDelegate_Int8::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIEventDelegatePresetParamCustomization::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIEventDelegate_UInt8::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIEventDelegatePresetParamCustomization::MakeInstance));
		//PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIEventDelegate_Int16::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIEventDelegatePresetParamCustomization::MakeInstance));
		//PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIEventDelegate_UInt16::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIEventDelegatePresetParamCustomization::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIEventDelegate_Int32::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIEventDelegatePresetParamCustomization::MakeInstance));
		//PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIEventDelegate_UInt32::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIEventDelegatePresetParamCustomization::MakeInstance));
		//PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIEventDelegate_Int64::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIEventDelegatePresetParamCustomization::MakeInstance));
		//PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIEventDelegate_UInt64::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIEventDelegatePresetParamCustomization::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIEventDelegate_Vector2::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIEventDelegatePresetParamCustomization::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIEventDelegate_Vector3::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIEventDelegatePresetParamCustomization::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIEventDelegate_Vector4::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIEventDelegatePresetParamCustomization::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIEventDelegate_Color::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIEventDelegatePresetParamCustomization::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIEventDelegate_LinearColor::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIEventDelegatePresetParamCustomization::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIEventDelegate_Quaternion::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIEventDelegatePresetParamCustomization::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIEventDelegate_String::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIEventDelegatePresetParamCustomization::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIEventDelegate_Object::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIEventDelegatePresetParamCustomization::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIEventDelegate_Actor::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIEventDelegatePresetParamCustomization::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIEventDelegate_PointerEvent::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIEventDelegatePresetParamCustomization::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIEventDelegate_Class::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIEventDelegatePresetParamCustomization::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIEventDelegate_Rotator::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIEventDelegatePresetParamCustomization::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIEventDelegate_Text::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIEventDelegatePresetParamCustomization::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIEventDelegate_Name::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&LGUIEventDelegatePresetParamCustomization::MakeInstance));

		PropertyModule.RegisterCustomPropertyTypeLayout(FLGUIComponentReference::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FLGUIComponentReferenceCustomization::MakeInstance));

		PropertyModule.RegisterCustomClassLayout(ULGUIPrefabSequenceComponent::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FLGUIPrefabSequenceComponentCustomization::MakeInstance));
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

		TSharedPtr<FAssetTypeActions_Base> SpriteDataAction = MakeShareable(new FAssetTypeActions_LGUISpriteData(LGUIAssetCategoryBit));
		TSharedPtr<FAssetTypeActions_Base> FontDataAction = MakeShareable(new FAssetTypeActions_LGUIFontData(LGUIAssetCategoryBit));
		TSharedPtr<FAssetTypeActions_Base> PrefabDataAction = MakeShareable(new FAssetTypeActions_LGUIPrefab(LGUIAssetCategoryBit));
		TSharedPtr<FAssetTypeActions_Base> UIStaticMeshCacheDataAction = MakeShareable(new FAssetTypeActions_LGUIStaticMeshCache(LGUIAssetCategoryBit));
		AssetTools.RegisterAssetTypeActions(SpriteDataAction.ToSharedRef());
		AssetTools.RegisterAssetTypeActions(FontDataAction.ToSharedRef());
		AssetTools.RegisterAssetTypeActions(PrefabDataAction.ToSharedRef());
		AssetTools.RegisterAssetTypeActions(UIStaticMeshCacheDataAction.ToSharedRef());
		AssetTypeActionsArray.Add(SpriteDataAction);
		AssetTypeActionsArray.Add(FontDataAction);
		AssetTypeActionsArray.Add(PrefabDataAction);
		AssetTypeActionsArray.Add(UIStaticMeshCacheDataAction);
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
		FKismetEditorUtilities::RegisterAutoGeneratedDefaultEvent(this, ULGUILifeCycleBehaviour::StaticClass(), TEXT("ReceiveAwake"));
		FKismetEditorUtilities::RegisterAutoGeneratedDefaultEvent(this, ULGUILifeCycleBehaviour::StaticClass(), TEXT("ReceiveStart"));
		FKismetEditorUtilities::RegisterAutoGeneratedDefaultEvent(this, ULGUILifeCycleBehaviour::StaticClass(), TEXT("ReceiveUpdate"));

		FKismetEditorUtilities::RegisterAutoGeneratedDefaultEvent(this, UUISelectableTransitionComponent::StaticClass(), TEXT("ReceiveOnNormal"));
		FKismetEditorUtilities::RegisterAutoGeneratedDefaultEvent(this, UUISelectableTransitionComponent::StaticClass(), TEXT("ReceiveOnHighlighted"));
		FKismetEditorUtilities::RegisterAutoGeneratedDefaultEvent(this, UUISelectableTransitionComponent::StaticClass(), TEXT("ReceiveOnPressed"));
		FKismetEditorUtilities::RegisterAutoGeneratedDefaultEvent(this, UUISelectableTransitionComponent::StaticClass(), TEXT("ReceiveOnDisabled"));
		FKismetEditorUtilities::RegisterAutoGeneratedDefaultEvent(this, UUISelectableTransitionComponent::StaticClass(), TEXT("ReceiveOnStartCustomTransition"));

		FKismetEditorUtilities::RegisterAutoGeneratedDefaultEvent(this, UUIRenderableCustomRaycast::StaticClass(), TEXT("ReceiveRaycast"));
		FKismetEditorUtilities::RegisterAutoGeneratedDefaultEvent(this, UUIRenderableCustomRaycast::StaticClass(), TEXT("ReceiveInit"));

		FKismetEditorUtilities::RegisterAutoGeneratedDefaultEvent(this, ULGUIWorldSpaceRaycasterSource::StaticClass(), TEXT("ReceiveInit"));
		FKismetEditorUtilities::RegisterAutoGeneratedDefaultEvent(this, ULGUIWorldSpaceRaycasterSource::StaticClass(), TEXT("ReceiveGenerateRay"));
		FKismetEditorUtilities::RegisterAutoGeneratedDefaultEvent(this, ULGUIWorldSpaceRaycasterSource::StaticClass(), TEXT("ReceiveShouldStartDrag"));

		FKismetEditorUtilities::RegisterAutoGeneratedDefaultEvent(this, UUIBatchGeometryRenderable::StaticClass(), TEXT("ReceiveOnBeforeCreateOrUpdateGeometry"));
		FKismetEditorUtilities::RegisterAutoGeneratedDefaultEvent(this, UUIBatchGeometryRenderable::StaticClass(), TEXT("ReceiveGetTextureToCreateGeometry"));
		FKismetEditorUtilities::RegisterAutoGeneratedDefaultEvent(this, UUIBatchGeometryRenderable::StaticClass(), TEXT("ReceiveOnUpdateGeometry"));

		FKismetEditorUtilities::RegisterAutoGeneratedDefaultEvent(this, UUISpriteBase::StaticClass(), TEXT("ReceiveOnUpdateGeometry"));

		FKismetEditorUtilities::RegisterAutoGeneratedDefaultEvent(this, UUITextureBase::StaticClass(), TEXT("ReceiveOnUpdateGeometry"));

		FKismetEditorUtilities::RegisterAutoGeneratedDefaultEvent(this, UUIGeometryModifierBase::StaticClass(), TEXT("ReceiveModifyUIGeometry"));
	}

	CheckPrefabOverrideDataViewerEntry();
}

void FLGUIEditorModule::OnInitializeSequence(ULGUIPrefabSequence* Sequence)
{
	auto* ProjectSettings = GetDefault<UMovieSceneToolsProjectSettings>();
	UMovieScene* MovieScene = Sequence->GetMovieScene();

	FFrameNumber StartFrame = (ProjectSettings->DefaultStartTime * MovieScene->GetTickResolution()).RoundToFrame();
	int32        Duration = (ProjectSettings->DefaultDuration * MovieScene->GetTickResolution()).RoundToFrame().Value;

	MovieScene->SetPlaybackRange(StartFrame, Duration);
}

void FLGUIEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FLGUIEditorStyle::Shutdown();

	FLGUIEditorCommands::Unregister();

	ULGUIPrefabSequence::OnInitializeSequence().Remove(OnInitializeSequenceHandle);
	ISequencerModule* SequencerModule = FModuleManager::Get().GetModulePtr<ISequencerModule>("Sequencer");
	if (SequencerModule)
	{
		SequencerModule->UnregisterSequenceEditor(SequenceEditorHandle);
	}
	//ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	//if (SettingsModule != nullptr)
	//{
	//	SettingsModule->UnregisterSettings("Editor", "ContentEditors", "EmbeddedLGUIPrefabSequenceEditor");
	//}

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
		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(LGUIPrefabSequenceTabName);
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
		PropertyModule.UnregisterCustomClassLayout(UUIPostProcessRenderable::StaticClass()->GetFName());

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
		PropertyModule.UnregisterCustomClassLayout(UUIFlexibleGridLayout::StaticClass()->GetFName());
		PropertyModule.UnregisterCustomClassLayout(UUILayoutElement::StaticClass()->GetFName());

		PropertyModule.UnregisterCustomClassLayout(ULGUIPrefab::StaticClass()->GetFName());

		PropertyModule.UnregisterCustomClassLayout(UUIEffectTextAnimation_Property::StaticClass()->GetFName());
		PropertyModule.UnregisterCustomClassLayout(UUIEffectTextAnimation::StaticClass()->GetFName());

		PropertyModule.UnregisterCustomClassLayout(UUISpriteSequencePlayer::StaticClass()->GetFName());
		PropertyModule.UnregisterCustomClassLayout(UUISpriteSheetTexturePlayer::StaticClass()->GetFName());

		PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIEventDelegate::StaticStruct()->GetFName());
		PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIEventDelegate::StaticStruct()->GetFName());
		//PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIEventDelegateTwoParam::StaticStruct()->GetFName());
		PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIEventDelegate_Empty::StaticStruct()->GetFName());
		PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIEventDelegate_Bool::StaticStruct()->GetFName());
		PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIEventDelegate_Float::StaticStruct()->GetFName());
		//PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIEventDelegate_Double::StaticStruct()->GetFName());
		//PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIEventDelegate_Int8::StaticStruct()->GetFName());
		PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIEventDelegate_UInt8::StaticStruct()->GetFName());
		//PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIEventDelegate_Int16::StaticStruct()->GetFName());
		//PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIEventDelegate_UInt16::StaticStruct()->GetFName());
		PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIEventDelegate_Int32::StaticStruct()->GetFName());
		//PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIEventDelegate_UInt32::StaticStruct()->GetFName());
		//PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIEventDelegate_Int64::StaticStruct()->GetFName());
		//PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIEventDelegate_UInt64::StaticStruct()->GetFName());
		PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIEventDelegate_Vector2::StaticStruct()->GetFName());
		PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIEventDelegate_Vector3::StaticStruct()->GetFName());
		PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIEventDelegate_Vector4::StaticStruct()->GetFName());
		PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIEventDelegate_Color::StaticStruct()->GetFName());
		PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIEventDelegate_LinearColor::StaticStruct()->GetFName());
		PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIEventDelegate_Quaternion::StaticStruct()->GetFName());
		PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIEventDelegate_String::StaticStruct()->GetFName());
		PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIEventDelegate_Object::StaticStruct()->GetFName());
		PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIEventDelegate_Actor::StaticStruct()->GetFName());
		PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIEventDelegate_PointerEvent::StaticStruct()->GetFName());
		PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIEventDelegate_Class::StaticStruct()->GetFName());
		PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIEventDelegate_Rotator::StaticStruct()->GetFName());

		PropertyModule.UnregisterCustomPropertyTypeLayout(FLGUIComponentReference::StaticStruct()->GetFName());

		PropertyModule.RegisterCustomClassLayout(ULGUIPrefabSequenceComponent::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FLGUIPrefabSequenceComponentCustomization::MakeInstance));
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

	USelection::SelectionChangedEvent.RemoveAll(this);
}

FLGUIEditorModule& FLGUIEditorModule::Get()
{
	return FModuleManager::Get().GetModuleChecked<FLGUIEditorModule>(TEXT("LGUIEditor"));
}

void FLGUIEditorModule::CheckPrefabOverrideDataViewerEntry()
{
	if (PrefabOverrideDataViewer != nullptr && PrefabOverrideDataViewer.IsValid())return;
	PrefabOverrideDataViewer = 
	SNew(SLGUIPrefabOverrideDataViewer, nullptr)
	.AfterRevertPrefab_Lambda([=](ULGUIPrefab* PrefabAsset) {
		OnOutlinerSelectionChange();//force refresh
		})
	.AfterApplyPrefab_Lambda([=](ULGUIPrefab* PrefabAsset) {
		OnOutlinerSelectionChange();//force refresh
		LGUIEditorTools::RefreshLevelLoadedPrefab(PrefabAsset);
		LGUIEditorTools::RefreshOnSubPrefabChange(PrefabAsset);
		LGUIEditorTools::RefreshOpenedPrefabEditor(PrefabAsset);
		})
	;
}

TSharedRef<SDockTab> FLGUIEditorModule::HandleSpawnAtlasViewerTab(const FSpawnTabArgs& SpawnTabArgs)
{
	auto ResultTab = SNew(SDockTab).TabRole(ETabRole::NomadTab);
	auto TabContentWidget = SNew(SLGUIAtlasViewer, ResultTab);
	ResultTab->SetContent(TabContentWidget);
	return ResultTab;
}

TSharedRef<SDockTab> FLGUIEditorModule::HandleSpawnLGUIPrefabSequenceTab(const FSpawnTabArgs& SpawnTabArgs)
{
	auto ResultTab = SNew(SDockTab).TabRole(ETabRole::NomadTab);
	auto TabContentWidget = SNew(SLGUIPrefabSequenceEditor);
	ResultTab->SetContent(TabContentWidget);
	return ResultTab;
}

bool FLGUIEditorModule::CanUnpackActorForPrefab()
{
	auto SelectedActor = LGUIEditorTools::GetFirstSelectedActor();
	if (SelectedActor == nullptr)return false;
	if (auto PrefabHelperObject = LGUIEditorTools::GetPrefabHelperObject_WhichManageThisActor(SelectedActor))
	{
		if (PrefabHelperObject->SubPrefabMap.Contains(SelectedActor))
		{
			return true;
		}
		return false;
	}
	else
	{
		return false;
	}
}
bool FLGUIEditorModule::CanBrowsePrefab()
{
	auto SelectedActor = LGUIEditorTools::GetFirstSelectedActor();
	if (SelectedActor == nullptr)return false;
	if (auto PrefabHelperObject = LGUIEditorTools::GetPrefabHelperObject_WhichManageThisActor(SelectedActor))
	{
		if (PrefabHelperObject->SubPrefabMap.Contains(SelectedActor))
		{
			return true;
		}
		return false;
	}
	else
	{
		return false;
	}
}

bool FLGUIEditorModule::CanUpdateLevelPrefab()
{
	auto SelectedActor = LGUIEditorTools::GetFirstSelectedActor();
	if (SelectedActor == nullptr)return false;
	if (auto PrefabHelperObject = LGUIEditorTools::GetPrefabHelperObject_WhichManageThisActor(SelectedActor))
	{
		if (PrefabHelperObject->SubPrefabMap.Contains(SelectedActor) && !PrefabHelperObject->IsInsidePrefabEditor())//Can only update prefab in level editor
		{
			return true;
		}
		return false;
	}
	else
	{
		return false;
	}
}

bool FLGUIEditorModule::CanCreateActor()
{
	auto SelectedActor = LGUIEditorTools::GetFirstSelectedActor();
	if (SelectedActor == nullptr)return false;
	if (auto PrefabHelperObject = LGUIEditorTools::GetPrefabHelperObject_WhichManageThisActor(SelectedActor))
	{
		if (PrefabHelperObject->IsActorBelongsToSubPrefab(SelectedActor))//not allowd to create actor on sub prefab's actor
		{
			return false;
		}
	}
	return true;
}

bool FLGUIEditorModule::CanCheckPrefabOverrideParameter()const
{
	auto SelectedActor = LGUIEditorTools::GetFirstSelectedActor();
	if (SelectedActor == nullptr)return false;
	if (auto PrefabHelperObject = LGUIEditorTools::GetPrefabHelperObject_WhichManageThisActor(SelectedActor))
	{
		for (auto& KeyValue : PrefabHelperObject->SubPrefabMap)
		{
			if (KeyValue.Key == SelectedActor || SelectedActor->IsAttachedTo(KeyValue.Key))
			{
				return true;
			}
		}
		return false;
	}
	else
	{
		return false;
	}
}

bool FLGUIEditorModule::CanReplaceActor()
{
	auto SelectedActor = LGUIEditorTools::GetFirstSelectedActor();
	if (SelectedActor == nullptr)return false;
	if (auto PrefabHelperObject = LGUIEditorTools::GetPrefabHelperObject_WhichManageThisActor(SelectedActor))
	{
		if (PrefabHelperObject->IsActorBelongsToSubPrefab(SelectedActor))//sub prefab's actor not allow replace
		{
			return false;
		}
	}
	return true;
}

bool FLGUIEditorModule::CanAttachLayout()
{
	if (LGUIEditorTools::IsSelectUIActor())
	{
		auto SelectedActor = LGUIEditorTools::GetFirstSelectedActor();
		if (SelectedActor == nullptr)return false;
		if (auto PrefabHelperObject = LGUIEditorTools::GetPrefabHelperObject_WhichManageThisActor(SelectedActor))
		{
			if (PrefabHelperObject->IsActorBelongsToSubPrefab(SelectedActor))//sub prefab's actor not allowed
			{
				return false;
			}
		}
		auto LayoutComponents = SelectedActor->GetComponentsByInterface(ULGUILayoutInterface::StaticClass());
		return LayoutComponents.Num() == 0;
	}
	return false;
}

bool FLGUIEditorModule::CanCreatePrefab()
{
	auto SelectedActor = LGUIEditorTools::GetFirstSelectedActor();
	if (SelectedActor == nullptr)return false;
	if (auto PrefabHelperObject = LGUIEditorTools::GetPrefabHelperObject_WhichManageThisActor(SelectedActor))
	{
		if (PrefabHelperObject->LoadedRootActor == SelectedActor)
		{
			return false;
		}
		if (PrefabHelperObject->IsActorBelongsToSubPrefab(SelectedActor))
		{
			return false;
		}
	}
	return true;
}

void FLGUIEditorModule::OnOutlinerSelectionChange()
{
	auto SelectedActor = LGUIEditorTools::GetFirstSelectedActor();
	if (SelectedActor == nullptr)return;
	auto NewPrefabHelperObject = LGUIEditorTools::GetPrefabHelperObject_WhichManageThisActor(SelectedActor);
	if (CurrentPrefabHelperObject != NewPrefabHelperObject)
	{
		CurrentPrefabHelperObject = NewPrefabHelperObject;
		if (CurrentPrefabHelperObject != nullptr)
		{
			PrefabOverrideDataViewer->SetPrefabHelperObject(CurrentPrefabHelperObject.Get());
		}
	}
	if (CurrentPrefabHelperObject != nullptr)
	{
		bool bIsSubPrefabRoot = false;
		for (auto& KeyValue : CurrentPrefabHelperObject->SubPrefabMap)
		{
			if (KeyValue.Key == SelectedActor)
			{
				bIsSubPrefabRoot = true;
				break;
			}
		}
		PrefabOverrideDataViewer->RefreshDataContent(CurrentPrefabHelperObject->GetSubPrefabData(SelectedActor).ObjectOverrideParameterArray, bIsSubPrefabRoot ? nullptr : SelectedActor);
	}
}

void FLGUIEditorModule::AddEditorToolsToToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.BeginSection("LGUI");
	{
		Builder.AddComboButton(
			FUIAction(),
			FOnGetContent::CreateRaw(this, &FLGUIEditorModule::MakeEditorToolsMenu, true, true, true, true, true, true, true),
			LOCTEXT("LGUITools", "LGUI Tools"),
			LOCTEXT("LGUIEditorTools", "LGUI Editor Tools"),
			FSlateIcon(FLGUIEditorStyle::GetStyleSetName(), "LGUIEditor.EditorTools")
		);
	}
	Builder.EndSection();
}

TSharedRef<SWidget> FLGUIEditorModule::MakeEditorToolsMenu(bool InitialSetup, bool ComponentAction, bool OpenWindow, bool PreviewInViewport, bool EditorCameraControl, bool Others, bool UpgradeToLGUI3)
{
	FMenuBuilder MenuBuilder(true, PluginCommands);
	auto commandList = FLGUIEditorCommands::Get();

	//prefab
	{
		MenuBuilder.BeginSection("Prefab", LOCTEXT("Prefab", "Prefab"));
		{
			MenuBuilder.AddMenuEntry(
				LOCTEXT("CreatePrefab", "Create Prefab"),
				LOCTEXT("Create_Tooltip", "Use selected actor to create a new prefab"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&LGUIEditorTools::CreatePrefabAsset)
					, FCanExecuteAction::CreateRaw(this, &FLGUIEditorModule::CanCreatePrefab)
					, FGetActionCheckState()
					, FIsActionButtonVisible::CreateRaw(this, &FLGUIEditorModule::CanCreatePrefab))
			);
			MenuBuilder.AddMenuEntry(
				LOCTEXT("UnpackPrefab", "Unpack this Prefab"),
				LOCTEXT("UnpackPrefab_Tooltip", "Unpack the actor from related prefab asset"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&LGUIEditorTools::UnpackPrefab)
					, FCanExecuteAction::CreateRaw(this, &FLGUIEditorModule::CanUnpackActorForPrefab)
					, FGetActionCheckState()
					, FIsActionButtonVisible::CreateRaw(this, &FLGUIEditorModule::CanUnpackActorForPrefab))
			);
			//MenuBuilder.AddMenuEntry(
			//	LOCTEXT("SelectPrefabAsset", "Browse to Prefab asset"),
			//	LOCTEXT("SelectPrefabAsset_Tooltip", "Browse to Prefab asset in Content Browser"),
			//	FSlateIcon(),
			//	FUIAction(FExecuteAction::CreateStatic(&LGUIEditorTools::SelectPrefabAsset)
			//		, FCanExecuteAction::CreateRaw(this, &FLGUIEditorModule::CanBrowsePrefab)
			//		, FGetActionCheckState()
			//		, FIsActionButtonVisible::CreateRaw(this, &FLGUIEditorModule::CanBrowsePrefab))
			//);
			MenuBuilder.AddMenuEntry(
				LOCTEXT("OpenPrefabAsset", "Open Prefab asset"),
				LOCTEXT("OpenPrefabAsset_Tooltip", "Open Prefab asset in PrefabEditor"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&LGUIEditorTools::OpenPrefabAsset)
					, FCanExecuteAction::CreateRaw(this, &FLGUIEditorModule::CanBrowsePrefab)
					, FGetActionCheckState()
					, FIsActionButtonVisible::CreateRaw(this, &FLGUIEditorModule::CanBrowsePrefab))
			);
			MenuBuilder.AddMenuEntry(
				LOCTEXT("UpdateLevelPrefab", "Update Prefab"),
				LOCTEXT("UpdateLevelPrefab_Tooltip", "Update this prefab to latest version"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&LGUIEditorTools::UpdateLevelPrefab)
					, FCanExecuteAction::CreateRaw(this, &FLGUIEditorModule::CanUpdateLevelPrefab)
					, FGetActionCheckState()
					, FIsActionButtonVisible::CreateRaw(this, &FLGUIEditorModule::CanUpdateLevelPrefab))
			);
			CheckPrefabOverrideDataViewerEntry();
			MenuBuilder.AddMenuEntry(
				FUIAction(FExecuteAction()
					, FCanExecuteAction::CreateRaw(this, &FLGUIEditorModule::CanCheckPrefabOverrideParameter)
					, FGetActionCheckState()
					, FIsActionButtonVisible::CreateRaw(this, &FLGUIEditorModule::CanCheckPrefabOverrideParameter))
				, 
				SNew(SComboButton)
				.HasDownArrow(true)
				.ToolTipText(LOCTEXT("PrefabOverride", "Edit override parameters for this prefab"))
				.ButtonContent()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("OverrideButton", "Prefab Override Properties"))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
				.MenuContent()
				[
					SNew(SBox)
					.Padding(FMargin(4, 4))
					[
						SNew(SHorizontalBox)
						+SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SVerticalBox)
							+SVerticalBox::Slot()
							.AutoHeight()
							[
								SNew(SHorizontalBox)
								+SHorizontalBox::Slot()
								.AutoWidth()
								[
									PrefabOverrideDataViewer.ToSharedRef()
								]
							]
						]
					]
				]
			);
		}
		MenuBuilder.EndSection();
	}

	MenuBuilder.BeginSection("Create", LOCTEXT("Create", "Create"));
	{
		MenuBuilder.AddSubMenu(
			LOCTEXT("CreateUIElementSubMenu", "Create UI Element"),
			LOCTEXT("CreateUIElementSubMenu_Tooltip", "Create UI Element"),
			FNewMenuDelegate::CreateRaw(this, &FLGUIEditorModule::CreateUIElementSubMenu),
			FUIAction(FExecuteAction()
				, FCanExecuteAction()
				, FGetActionCheckState()
				, FIsActionButtonVisible::CreateRaw(this, &FLGUIEditorModule::CanCreateActor)),
			NAME_None, EUserInterfaceActionType::None
		);
		MenuBuilder.AddSubMenu(
			LOCTEXT("CreateUIExtensionSubMenu", "Create UI Extension Element"),
			LOCTEXT("CreateUIExtensionSubMenu_Tooltip", "Create UI Extension Element"),
			FNewMenuDelegate::CreateRaw(this, &FLGUIEditorModule::CreateUIExtensionSubMenu),
			FUIAction(FExecuteAction()
				, FCanExecuteAction()
				, FGetActionCheckState()
				, FIsActionButtonVisible::CreateRaw(this, &FLGUIEditorModule::CanCreateActor)),
			NAME_None, EUserInterfaceActionType::None
		);
		MenuBuilder.AddSubMenu(
			LOCTEXT("CreateUIPostProcessSubMenu", "Create UI Post Process"),
			LOCTEXT("CreateUIPostProcessSubMenu_Tooltip", "Create UI Post Process"),
			FNewMenuDelegate::CreateRaw(this, &FLGUIEditorModule::CreateUIPostProcessSubMenu),
			FUIAction(FExecuteAction()
				, FCanExecuteAction()
				, FGetActionCheckState()
				, FIsActionButtonVisible::CreateRaw(this, &FLGUIEditorModule::CanCreateActor)),
			NAME_None, EUserInterfaceActionType::None
		);
		MenuBuilder.AddSubMenu(
			LOCTEXT("CreateCommonActorSubMenu", "Create Actor"),
			LOCTEXT("CreateCommonActorSubMenu_Tooltip", "Create Actor"),
			FNewMenuDelegate::CreateRaw(this, &FLGUIEditorModule::CreateCommonActorSubMenu),
			FUIAction(FExecuteAction()
				, FCanExecuteAction()
				, FGetActionCheckState()
				, FIsActionButtonVisible::CreateRaw(this, &FLGUIEditorModule::CanCreateActor)),
			NAME_None, EUserInterfaceActionType::None
		);
		CreateExtraPrefabsSubMenu(MenuBuilder);
		if (InitialSetup)
		{
			MenuBuilder.AddSubMenu(
				LOCTEXT("BasicSetup", "Basic Setup"),
				LOCTEXT("BasicSetup", "Basic Setup"),
				FNewMenuDelegate::CreateRaw(this, &FLGUIEditorModule::BasicSetupSubMenu)
			);
		}
		MenuBuilder.AddSubMenu(
			LOCTEXT("ReplaceActorMenu", "Replace this by..."),
			LOCTEXT("ReplaceActorMenu_Tooltip", "Replace this actor with..."),
			FNewMenuDelegate::CreateRaw(this, &FLGUIEditorModule::ReplaceActorSubMenu),
			FUIAction(FExecuteAction()
				, FCanExecuteAction::CreateRaw(this, &FLGUIEditorModule::CanReplaceActor)
				, FGetActionCheckState()
				, FIsActionButtonVisible::CreateRaw(this, &FLGUIEditorModule::CanReplaceActor)),
			NAME_None,
			EUserInterfaceActionType::None
		);
		MenuBuilder.AddSubMenu(
			LOCTEXT("Layout", "Attach Layout"),
			LOCTEXT("Layout_Tooltip", "Attach Layout to selected UI Element"),
			FNewMenuDelegate::CreateRaw(this, &FLGUIEditorModule::AttachLayout),
			FUIAction(FExecuteAction()
				, FCanExecuteAction()
				, FGetActionCheckState()
				, FIsActionButtonVisible::CreateRaw(this, &FLGUIEditorModule::CanAttachLayout)),
			NAME_None, EUserInterfaceActionType::None
		);
	}
	MenuBuilder.EndSection();

	MenuBuilder.BeginSection("ActorAction", LOCTEXT("ActorAction", "Edit Actor With Hierarchy"));
	{
		MenuBuilder.AddMenuEntry(commandList.CopyActor);
		MenuBuilder.AddMenuEntry(commandList.PasteActor);
		MenuBuilder.AddMenuEntry(commandList.CutActor);
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

	if (ComponentAction)
	{
		MenuBuilder.BeginSection("ComponentAction", LOCTEXT("ComponentAction", "Edit Component"));
		{
			MenuBuilder.AddMenuEntry(commandList.CopyComponentValues);
			MenuBuilder.AddMenuEntry(commandList.PasteComponentValues);
		}
		MenuBuilder.EndSection();
	}

	if (OpenWindow)
	{

	}

	if (EditorCameraControl)
	{
		MenuBuilder.BeginSection("EditorCamera", LOCTEXT("EditorCameraControl", "EditorCameraControl"));
		{
			MenuBuilder.AddMenuEntry(commandList.FocusToScreenSpaceUI);
			MenuBuilder.AddMenuEntry(commandList.FocusToSelectedUI);
		}
		MenuBuilder.EndSection();
	}

	if (Others)
	{
		MenuBuilder.BeginSection("Others", LOCTEXT("Others", "Others"));
		{
			MenuBuilder.AddMenuEntry(commandList.ActiveViewportAsLGUIPreview);
			MenuBuilder.AddMenuEntry(commandList.ToggleLGUIInfoColume);
			MenuBuilder.AddMenuEntry(commandList.ToggleDrawHelperFrame);
			MenuBuilder.AddMenuEntry(commandList.ForceGC);
		}
		MenuBuilder.EndSection();
	}

	if (UpgradeToLGUI3)
	{
		MenuBuilder.BeginSection("LGUI3", LOCTEXT("UpgradeToLGUI3", "Upgrade To LGUI3"));
		MenuBuilder.AddMenuEntry(
			LOCTEXT("Upgrade_Level_to_LGUI3", "Upgrade current Level to LGUI3"),
			LOCTEXT("Upgrade_Level_to_LGUI3_Tooltip", "Upgrade current Level to LGUI3"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&LGUIEditorTools::UpgradeLevelToLGUI3))
		);
		MenuBuilder.AddMenuEntry(
			LOCTEXT("Upgrade_SelectedPrefabs_to_LGUI3", "Upgrade selected LGUI prefabs to LGUI3"),
			LOCTEXT("Upgrade_SelectedPrefabs_to_LGUI3_Tooltip", "Upgrade selected LGUI prefabs to LGUI3"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&LGUIEditorTools::UpgradeSelectedPrefabToLGUI3))
		);
		MenuBuilder.AddMenuEntry(
			LOCTEXT("Upgrade_AllPrefabs_to_LGUI3", "Upgrade all LGUI prefab to LGUI3"),
			LOCTEXT("Upgrade_AllPrefabs_to_LGUI3_Tooltip", "Upgrade all LGUI prefab to LGUI3"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&LGUIEditorTools::UpgradeAllPrefabToLGUI3))
		);
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
			auto ShotName = FString(*UIItemName);
			ShotName.RemoveFromEnd(TEXT("Actor"));
			InBuilder.AddMenuEntry(
				FText::FromString(ShotName),
				FText::Format(LOCTEXT("CreateUIElementTitle", "Create {0}"), FText::FromString(UIItemName)),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&LGUIEditorTools::CreateUIItemActor, InClass))
			);
		}
		static void CreateUIControlMenuEntry(FMenuBuilder& InBuilder, const FString& InControlName, FText InTooltip = FText())
		{
			if (InTooltip.IsEmpty())
			{
				InTooltip = FText::Format(LOCTEXT("CreateUIElementTitle", "Create {0}"), FText::FromString(InControlName));
			}
			InBuilder.AddMenuEntry(
				FText::FromString(InControlName),
				InTooltip,
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&LGUIEditorTools::CreateUIControls, LGUIEditorTools::LGUIPresetPrefabPath + InControlName))
			);
		}
	};

	MenuBuilder.BeginSection("UIElement");
	{
		FunctionContainer::CreateUIBaseElementMenuEntry(MenuBuilder, AUIContainerActor::StaticClass());
		FunctionContainer::CreateUIBaseElementMenuEntry(MenuBuilder, AUISpriteActor::StaticClass());
		FunctionContainer::CreateUIBaseElementMenuEntry(MenuBuilder, AUITextActor::StaticClass());
		FunctionContainer::CreateUIBaseElementMenuEntry(MenuBuilder, AUITextureActor::StaticClass());

		FunctionContainer::CreateUIControlMenuEntry(MenuBuilder, TEXT("Button"));
		FunctionContainer::CreateUIControlMenuEntry(MenuBuilder, TEXT("Toggle"));
		FunctionContainer::CreateUIControlMenuEntry(MenuBuilder, TEXT("ToggleGroup"));
		FunctionContainer::CreateUIControlMenuEntry(MenuBuilder, TEXT("HorizontalSlider"));
		FunctionContainer::CreateUIControlMenuEntry(MenuBuilder, TEXT("VerticalSlider"));
		FunctionContainer::CreateUIControlMenuEntry(MenuBuilder, TEXT("HorizontalScrollbar"));
		FunctionContainer::CreateUIControlMenuEntry(MenuBuilder, TEXT("VerticalScrollbar"));
		FunctionContainer::CreateUIControlMenuEntry(MenuBuilder, TEXT("Dropdown"));
		FunctionContainer::CreateUIControlMenuEntry(MenuBuilder, TEXT("TextInput"));
		FunctionContainer::CreateUIControlMenuEntry(MenuBuilder, TEXT("TextInputMultiline"));
		FunctionContainer::CreateUIControlMenuEntry(MenuBuilder, TEXT("ScrollView"));
		FunctionContainer::CreateUIControlMenuEntry(MenuBuilder, TEXT("RecyclableScrollViewHorizontal"));
		FunctionContainer::CreateUIControlMenuEntry(MenuBuilder, TEXT("RecyclableScrollViewVertical"));
	}
	MenuBuilder.EndSection();
}

bool FLGUIEditorModule::IsValidClassName(const FString& InName)
{
	return 
		!InName.StartsWith(TEXT("SKEL_"))
		&& !InName.StartsWith(TEXT("REINST_"))
		;
}

void FLGUIEditorModule::CreateCommonActorSubMenu(FMenuBuilder& MenuBuilder)
{
	struct FunctionContainer
	{
		static void CreateCommonActorMenuEntry(FMenuBuilder& InBuilder, UClass* InClass)
		{
			auto ActorName = InClass->GetName();
			auto ShotName = FString(*ActorName);
			ShotName.RemoveFromEnd(TEXT("Actor"));
			InBuilder.AddMenuEntry(
				FText::FromString(ShotName),
				FText::FromString(ActorName),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&LGUIEditorTools::CreateUIItemActor, InClass))
			);
		}
	};

	struct GroupDataContainer
	{
		FString GroupName;
		TArray<UClass*> ActorClassArray;
		TArray<GroupDataContainer> SubGroupData;

		static int PushItemToContainer(const FString& GroupName, UClass* InClass, TArray<GroupDataContainer>& InOutGroupData)
		{
			auto FoundIndex = InOutGroupData.IndexOfByPredicate([GroupName](const GroupDataContainer& DataItem) {
				return GroupName == DataItem.GroupName;
				});
			if (FoundIndex == INDEX_NONE)
			{
				GroupDataContainer Container;
				Container.GroupName = GroupName;
				FoundIndex = InOutGroupData.Add(Container);
			}
			if (InClass != nullptr)
			{
				auto& Container = InOutGroupData[FoundIndex];
				Container.ActorClassArray.Add(InClass);
			}
			return FoundIndex;
		}
		static void PushToContainer(TArray<FString> InGroupNameArray, UClass* InClass, TArray<GroupDataContainer>& InOutGroupData)
		{
			auto FirstGroup = InGroupNameArray[0];
			InGroupNameArray.RemoveAt(0);
			if (InGroupNameArray.Num() == 0)
			{
				PushItemToContainer(FirstGroup, InClass, InOutGroupData);
			}
			else
			{
				auto FoundIndex = PushItemToContainer(FirstGroup, nullptr, InOutGroupData);
				auto& GroupData = InOutGroupData[FoundIndex];
				PushToContainer(InGroupNameArray, InClass, GroupData.SubGroupData);
			}
		}

		static void MakeMenu(FMenuBuilder& MenuBuilder, const TArray<GroupDataContainer>& InGroupData, FLGUIEditorModule* EditorModulePtr)
		{
			for (auto& GroupDataItem : InGroupData)
			{
				MenuBuilder.AddSubMenu(
					FText::FromString(GroupDataItem.GroupName),
					FText(),
					FNewMenuDelegate::CreateLambda([=](FMenuBuilder& MenuBuilder) {
						MenuBuilder.BeginSection(FName(*GroupDataItem.GroupName));
						{
							MenuBuilder.AddSearchWidget();
							for (UClass* ActorClassItem : GroupDataItem.ActorClassArray)
							{
								FunctionContainer::CreateCommonActorMenuEntry(MenuBuilder, ActorClassItem);
							}
						}
						MenuBuilder.EndSection();
						}),
					FUIAction(FExecuteAction()
						, FCanExecuteAction()
						, FGetActionCheckState()
						, FIsActionButtonVisible::CreateRaw(EditorModulePtr, &FLGUIEditorModule::CanCreateActor)),
					NAME_None, EUserInterfaceActionType::None
				);

				if (GroupDataItem.SubGroupData.Num() > 0)
				{
					MakeMenu(MenuBuilder, GroupDataItem.SubGroupData, EditorModulePtr);
				}
			}
		}
	};

	const FString AllActorGroupName = TEXT("All Actors");
	TArray<GroupDataContainer> GroupData;
	TArray<UClass*> AllValidActorArray;
	TArray<FName> SkipArray =//these actors don't have RootComponent, which is not good for LGUI's prefab system, so ignore them
	{
		TEXT("Brush"),
		TEXT("BrushShape"),
		TEXT("AbstractNavData"),
		TEXT("NavModifierVolume"),
		TEXT("ARActor"),
		TEXT("AROriginActor"),
		TEXT("NavSystemConfigOverride"),
		TEXT("SequenceRecorderGroup"),
	};
	for (TObjectIterator<UClass> ClassItr; ClassItr; ++ClassItr)
	{
		if (ClassItr->IsChildOf(AActor::StaticClass())
			&& (*ClassItr) != AActor::StaticClass()
			&& !SkipArray.Contains(ClassItr->GetFName())
			)
		{
			if (
				!(ClassItr->HasAnyClassFlags(CLASS_Transient))
				&& !(ClassItr->HasAnyClassFlags(CLASS_Abstract))
				&& !(ClassItr->HasAnyClassFlags(CLASS_Deprecated))
				&& !(ClassItr->HasAnyClassFlags(CLASS_NotPlaceable))
				)
			{
				if (!IsValidClassName(ClassItr->GetName()))
				{
					continue;
				}

				TArray<FString> GroupNames;
				ClassItr->GetClassGroupNames(GroupNames);
				if (GroupNames.Num() != 0)
				{
					GroupDataContainer::PushToContainer(GroupNames, *ClassItr, GroupData);
				}
				AllValidActorArray.Add(*ClassItr);
			}
		}
	}

	MenuBuilder.AddSubMenu(
		FText::FromString(AllActorGroupName),
		FText(),
		FNewMenuDelegate::CreateLambda([=](FMenuBuilder& MenuBuilder) {
			MenuBuilder.BeginSection(FName(*AllActorGroupName));
			{
				MenuBuilder.AddSearchWidget();
				MenuBuilder.AddMenuEntry(
					LOCTEXT("EmptyActor", "Empty Actor"),
					LOCTEXT("EmptyActor_Tooltip", "Create an empty actor"),
					FSlateIcon(),
					FUIAction(FExecuteAction::CreateStatic(&LGUIEditorTools::CreateEmptyActor))
				);
				for (UClass* ActorClassItem : AllValidActorArray)
				{
					FunctionContainer::CreateCommonActorMenuEntry(MenuBuilder, ActorClassItem);
				}
			}
			MenuBuilder.EndSection();
			}),
		FUIAction(FExecuteAction()
			, FCanExecuteAction()
			, FGetActionCheckState()
			, FIsActionButtonVisible::CreateRaw(this, &FLGUIEditorModule::CanCreateActor)),
				NAME_None, EUserInterfaceActionType::None
				);

	GroupDataContainer::MakeMenu(MenuBuilder, GroupData, this);
}

void FLGUIEditorModule::CreateExtraPrefabsSubMenu(FMenuBuilder& MenuBuilder)
{
	struct LOCAL
	{
		static void CreateExtraPrefab_SubMenu(FMenuBuilder& MenuBuilder, TArray<ULGUIPrefab*> InPrefabArray)
		{
			for (auto Prefab : InPrefabArray)
			{
				MenuBuilder.AddMenuEntry(
					FText::FromString(FPaths::GetBaseFilename(Prefab->GetPathName())),
					FText::FromString(Prefab->GetPathName()),
					FSlateIcon(),
					FUIAction(FExecuteAction::CreateStatic(&LGUIEditorTools::CreateUIControls, Prefab->GetPathName()))
				);
			}
		}
	};

	auto PrefabFolders = GetDefault<ULGUIEditorSettings>()->ExtraPrefabFolders;
	for (auto PrefabFolder : PrefabFolders)
	{
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));
		IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
		// Need to do this if running in the editor with -game to make sure that the assets in the following path are available
		TArray<FString> PathsToScan;
		PathsToScan.Add(TEXT("/Game/"));
		AssetRegistry.ScanPathsSynchronous(PathsToScan);

		TArray<FAssetData> ScriptAssetList;
		AssetRegistry.GetAssetsByPath(FName(*PrefabFolder.Path), ScriptAssetList, false);
		TArray<ULGUIPrefab*> PrefabAssets;
		auto PrefabClassName = ULGUIPrefab::StaticClass()->GetFName();
		for (auto Asset : ScriptAssetList)
		{
			if (Asset.AssetClass == PrefabClassName)
			{
				auto AssetObject = Asset.GetAsset();
				if (auto Prefab = Cast<ULGUIPrefab>(AssetObject))
				{
					PrefabAssets.Add(Prefab);
				}
			}
		}

		if(PrefabAssets.Num() > 0)
		{
			MenuBuilder.AddSubMenu(
				FText::Format(LOCTEXT("CreateExtra", "CreateExtra {0}"), FText::FromString(PrefabFolder.Path)),
				FText::Format(LOCTEXT("CreateExtra_Tooltip", "CreateExtra prefab from folder {0}"), FText::FromString(PrefabFolder.Path)),
				FNewMenuDelegate::CreateStatic(&LOCAL::CreateExtraPrefab_SubMenu, PrefabAssets),
				FUIAction(FExecuteAction()
					, FCanExecuteAction()
					, FGetActionCheckState()
					, FIsActionButtonVisible::CreateRaw(this, &FLGUIEditorModule::CanCreateActor)),
				NAME_None, EUserInterfaceActionType::None
			);
		}
	}
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

void FLGUIEditorModule::ToggleDrawHelperFrame()
{
	auto LGUIEditorSettings = GetMutableDefault<ULGUIEditorSettings>();
	LGUIEditorSettings->bDrawHelperFrame = !LGUIEditorSettings->bDrawHelperFrame;
	LGUIEditorSettings->SaveConfig();
}
bool FLGUIEditorModule::GetDrawHelperFrameChecked()
{
	return GetDefault<ULGUIEditorSettings>()->bDrawHelperFrame;
}

void FLGUIEditorModule::ApplyLGUIColumnInfo(bool value, bool refreshSceneOutliner)
{
	FSceneOutlinerModule& SceneOutlinerModule = FModuleManager::LoadModuleChecked< FSceneOutlinerModule >("SceneOutliner");
	if (value)
	{
		SceneOutliner::FColumnInfo ColumnInfo(SceneOutliner::EColumnVisibility::Visible, 15, FCreateSceneOutlinerColumn::CreateStatic(&LGUISceneOutliner::FLGUISceneOutlinerInfoColumn::MakeInstance));
		SceneOutlinerModule.RegisterDefaultColumnType<LGUISceneOutliner::FLGUISceneOutlinerInfoColumn>(SceneOutliner::FDefaultColumnInfo(ColumnInfo));
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
			auto ShotName = FString(*UIItemName);
			ShotName.RemoveFromEnd(TEXT("Actor"));
			InBuilder.AddMenuEntry(
				FText::FromString(ShotName),
				FText::Format(LOCTEXT("CreateUIPoseProcessElement", "Create {0}"), FText::FromString(UIItemName)),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&LGUIEditorTools::CreateUIItemActor, InClass))
			);
		}
	};

	MenuBuilder.BeginSection("UIPostProcessRenderable");
	{
		for (TObjectIterator<UClass> ClassItr; ClassItr; ++ClassItr)
		{
			if (ClassItr->IsChildOf(AUIBasePostProcessActor::StaticClass()))
			{
				if (
					   !(ClassItr->HasAnyClassFlags(CLASS_Transient))
					&& !(ClassItr->HasAnyClassFlags(CLASS_Abstract))
					&& !(ClassItr->HasAnyClassFlags(CLASS_Deprecated))
					&& !(ClassItr->HasAnyClassFlags(CLASS_NotPlaceable))
					)
				{
					bool isBlueprint = ClassItr->HasAnyClassFlags(CLASS_CompiledFromBlueprint);
					if (isBlueprint)
					{
						if (!IsValidClassName(ClassItr->GetName()))
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
				FText::Format(LOCTEXT("CreateUIExtensionElement", "Create {0}"), FText::FromString(UIItemName)),
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
					&& !(*ClassItr)->IsChildOf(AUIBasePostProcessActor::StaticClass())
					&& !(ClassItr->HasAnyClassFlags(CLASS_Transient))
					&& !(ClassItr->HasAnyClassFlags(CLASS_Abstract))
					&& !(ClassItr->HasAnyClassFlags(CLASS_Deprecated))
					&& !(ClassItr->HasAnyClassFlags(CLASS_NotPlaceable))
					)
				{
					bool isBlueprint = ClassItr->HasAnyClassFlags(CLASS_CompiledFromBlueprint);
					if (isBlueprint)
					{
						if (!IsValidClassName(ClassItr->GetName()))
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
			LOCTEXT("BasicSetup_ScreenSpaceUI", "Screen Space UI"),
			LOCTEXT("BasicSetup_ScreenSpaceUI_Tooltip", "Create Screen Space UI"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&LGUIEditorTools::CreateScreenSpaceUI_BasicSetup))
		);
		MenuBuilder.AddMenuEntry(
			LOCTEXT("BasicSetup_WorldSpaceUERenderer", "World Space UI - UE Renderer"),
			LOCTEXT("BasicSetup_WorldSpaceUERenderer_Tooltip", "Render in world space by UE default render pipeline.\n This mode use engine's default render pieple, so post process will affect ui."),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&LGUIEditorTools::CreateWorldSpaceUIUERenderer_BasicSetup))
		);
		MenuBuilder.AddMenuEntry(
			LOCTEXT("BasicSetup_WorldSpaceLGUIRenderer", "World Space UI - LGUI Renderer"),
			LOCTEXT("BasicSetup_WorldSpaceLGUIRenderer_Tooltip", "Render in world space by LGUI's custom render pipeline.\n This mode use LGUI's custom render pipeline, will not be affected by post process."),
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
			auto channelName = CollisionProfile->ReturnChannelNameFromContainerIndex(collisionChannel);
			if (channelName != TEXT("MAX"))
			{
				MenuBuilder.AddMenuEntry(
					FText::FromName(channelName),
					FText::Format(LOCTEXT("ChangeTraceChannel_Tooltip", "Change selected UI item actor's trace channel to {0}"), FText::FromName(channelName)),
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
			LOCTEXT("AttachLayout_HorizontalLayout", "Horizontal Layout"),
			LOCTEXT("AttachLayout_HorizontalLayout_Tooltip", "Layout child elements side by side horizontally"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&LGUIEditorTools::AttachComponentToSelectedActor, TSubclassOf<UActorComponent>(UUIHorizontalLayout::StaticClass())))
		);
		MenuBuilder.AddMenuEntry(
			LOCTEXT("AttachLayout_VerticalLayout", "Vertical Layout"),
			LOCTEXT("AttachLayout_VerticalLayout_Tooltip", "Layout child elements side by side vertically"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&LGUIEditorTools::AttachComponentToSelectedActor, TSubclassOf<UActorComponent>(UUIVerticalLayout::StaticClass())))
		);
		MenuBuilder.AddMenuEntry(
			LOCTEXT("AttachLayout_GridLayout", "Grid Layout"),
			LOCTEXT("AttachLayout_GridLayout_Tooltip", "Layout child elements in grid"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&LGUIEditorTools::AttachComponentToSelectedActor, TSubclassOf<UActorComponent>(UUIGridLayout::StaticClass())))
		);
		MenuBuilder.AddMenuEntry(
			LOCTEXT("AttachLayout_RoundedLayout", "Rounded Layout"),
			LOCTEXT("AttachLayout_RoundedLayout_Tooltip", "Rounded layout, only affect children's position and angle, not affect size"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&LGUIEditorTools::AttachComponentToSelectedActor, TSubclassOf<UActorComponent>(UUIRoundedLayout::StaticClass())))
		);
		MenuBuilder.AddMenuEntry(
			LOCTEXT("AttachLayout_LayoutElement", "Layout Element"),
			LOCTEXT("AttachLayout_LayoutElement_Tooltip", "Attach to layout's child, make is specific or ignore layout"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&LGUIEditorTools::AttachComponentToSelectedActor, TSubclassOf<UActorComponent>(UUILayoutElement::StaticClass())))
		);
		MenuBuilder.AddMenuEntry(
			LOCTEXT("AttachLayout_SizeControlByAspectRatio", "Size Control by Aspect Ratio"),
			LOCTEXT("AttachLayout_SizeControlByAspectRatio_Tooltip", "Use aspect ratio to control with and height"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&LGUIEditorTools::AttachComponentToSelectedActor, TSubclassOf<UActorComponent>(UUISizeControlByAspectRatio::StaticClass())))
		);
		MenuBuilder.AddMenuEntry(
			LOCTEXT("AttachLayout_SizeControlByOther", "Size Control by Other"),
			LOCTEXT("AttachLayout_SizeControlByOther_Tooltip", "Use other UI element to control the size of this one"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&LGUIEditorTools::AttachComponentToSelectedActor, TSubclassOf<UActorComponent>(UUISizeControlByOther::StaticClass())))
		);
	}
	MenuBuilder.EndSection();
}

void FLGUIEditorModule::ReplaceActorSubMenu(FMenuBuilder& MenuBuilder)
{
	struct FunctionContainer
	{
		static void ReplaceUIElement(FMenuBuilder& InBuilder, UClass* InClass)
		{
			auto ClassName = InClass->GetName();
			auto ShotName = FString(*ClassName);
			ShotName.RemoveFromEnd(TEXT("Actor"));
			InBuilder.AddMenuEntry(
				FText::FromString(ShotName),
				FText::Format(LOCTEXT("ReplaceUIElement", "ReplaceWith {0}"), FText::FromString(ClassName)),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&LGUIEditorTools::ReplaceUIElementWith, InClass))
			);
		}
		static void CreateCommonActorMenuEntry(FMenuBuilder& InBuilder, UClass* InClass)
		{
			auto ClassName = InClass->GetName();
			auto ShotName = FString(*ClassName);
			ShotName.RemoveFromEnd(TEXT("Actor"));
			InBuilder.AddMenuEntry(
				FText::FromString(ShotName),
				FText::Format(LOCTEXT("ReplaceUIElement", "ReplaceWith {0}"), FText::FromString(ClassName)),
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
					&& !(ClassItr->HasAnyClassFlags(CLASS_NotPlaceable))
					)
				{
					bool isBlueprint = ClassItr->HasAnyClassFlags(CLASS_CompiledFromBlueprint);
					if (isBlueprint)
					{
						if (!IsValidClassName(ClassItr->GetName()))
						{
							continue;
						}
					}
					FunctionContainer::ReplaceUIElement(MenuBuilder, *ClassItr);
				}
			}
		}


		const FString AllActorGroupName = TEXT("All Actors");
		TArray<UClass*> AllValidActorArray;
		TArray<FName> SkipArray =//these actors don't have RootComponent, which is not good for LGUI's prefab system, so ignore them
		{
			TEXT("Brush"),
			TEXT("BrushShape"),
			TEXT("AbstractNavData"),
			TEXT("NavModifierVolume"),
			TEXT("ARActor"),
			TEXT("AROriginActor"),
			TEXT("NavSystemConfigOverride"),
			TEXT("SequenceRecorderGroup"),
		};
		for (TObjectIterator<UClass> ClassItr; ClassItr; ++ClassItr)
		{
			if (ClassItr->IsChildOf(AActor::StaticClass())
				&& (*ClassItr) != AActor::StaticClass()
				&& !SkipArray.Contains(ClassItr->GetFName())
				)
			{
				if (
					!(ClassItr->HasAnyClassFlags(CLASS_Transient))
					&& !(ClassItr->HasAnyClassFlags(CLASS_Abstract))
					&& !(ClassItr->HasAnyClassFlags(CLASS_Deprecated))
					&& !(ClassItr->HasAnyClassFlags(CLASS_NotPlaceable))
					)
				{
					if (!IsValidClassName(ClassItr->GetName()))
					{
						continue;
					}
					AllValidActorArray.Add(*ClassItr);
				}
			}
		}
		MenuBuilder.AddSubMenu(
			FText::FromString(AllActorGroupName),
			FText(),
			FNewMenuDelegate::CreateLambda([=](FMenuBuilder& MenuBuilder) {
				MenuBuilder.BeginSection(FName(*AllActorGroupName));
				{
					MenuBuilder.AddSearchWidget();
					for (UClass* ActorClassItem : AllValidActorArray)
					{
						FunctionContainer::CreateCommonActorMenuEntry(MenuBuilder, ActorClassItem);
					}
				}
				MenuBuilder.EndSection();
				}),
			FUIAction(),
					NAME_None, EUserInterfaceActionType::None
					);
	}
	MenuBuilder.EndSection();
}

IMPLEMENT_MODULE(FLGUIEditorModule, LGUIEditor)

#undef LOCTEXT_NAMESPACE