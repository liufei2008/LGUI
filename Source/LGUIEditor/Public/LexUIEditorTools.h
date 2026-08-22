// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Widgets/SCompoundWidget.h"
#include "Widgets/Docking/SDockTab.h"
#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"

#pragma once
class ULexWidget;
class ULexUIPrefabHelperObject;
class ULexUIPrefab;

DECLARE_MULTICAST_DELEGATE_OneParam(FEditingPrefabChangedDelegate, ULexWidget*);
DECLARE_MULTICAST_DELEGATE_OneParam(FBeforeApplyPrefabDelegate, ULexUIPrefabHelperObject*);

class LGUIEDITOR_API FLexUIEditorTools
{
private:
	static FString PrevSavePrefabFolder;
public:
	static FString LexUIPresetPrefabPath;
	static FEditingPrefabChangedDelegate OnEditingPrefabChanged;
	static FBeforeApplyPrefabDelegate OnBeforeApplyPrefab;
	static TArray<ULexWidget*> GetRootWidgetListFromSelection(const TArray<ULexWidget*>& InSelectedWidgets);
	static void CreateWidget(TFunction<ULexWidget*()> GetSelectedWidgetFunction, FString Name, UClass* VisualClass, TFunction<void(class ULexWidget*)> Callback);
	static void CreateUIControls(TFunction<ULexWidget*()> GetSelectedWidgetFunction, FString InPrefabPath);
	static void DuplicateWidgets(TFunction<TArray<ULexWidget*>()> GetSelectedWidgetArrayFunction);
	static void CopyWidgets(TFunction<TArray<ULexWidget*>()> GetSelectedWidgetArrayFunction);
	static void PasteWidgets(TFunction<TArray<ULexWidget*>()> GetSelectedWidgetFunction);
	static void DeleteWidgets(TFunction<TArray<ULexWidget*>()> GetSelectedWidgetArrayFunction);
	static void CutWidgets(TFunction<TArray<ULexWidget*>()> GetSelectedWidgetArrayFunction);
	static bool CanDuplicateWidget(TFunction<TArray<ULexWidget*>()> GetSelectedWidgetArrayFunction);
	static bool CanCopyWidget(TFunction<TArray<ULexWidget*>()> GetSelectedWidgetArrayFunction);
	static bool CanPasteWidget(TFunction<ULexWidget*()> GetSelectedWidgetFunction);
	static bool CanCutWidget(TFunction<TArray<ULexWidget*>()> GetSelectedWidgetArrayFunction);
	static bool CanDeleteWidget(TFunction<TArray<ULexWidget*>()> GetSelectedWidgetArrayFunction);
	
	static bool CanCreatePrefab(TFunction<ULexWidget*()> GetSelectedWidgetFunction);
	static void CreatePrefabAsset(TFunction<ULexWidget*()> GetSelectedWidgetFunction);
	static void RefreshLevelLoadedPrefab();
	static void RefreshOpenedPrefabEditor(ULexUIPrefab* InPrefab);
	static void RefreshOnSubPrefabChange(ULexUIPrefab* InSubPrefab);
	static TArray<ULexUIPrefab*> GetAllPrefabArray();
	static bool CanUnpackWidgetForPrefab(TFunction<ULexWidget*()> GetSelectedWidgetFunction);
	static void UnpackPrefab(TFunction<ULexWidget*()> GetSelectedWidgetFunction);
	static void SelectPrefabAsset(TFunction<ULexWidget*()> GetSelectedWidgetFunction);
	static bool CanBrowsePrefabAsset(TFunction<ULexWidget*()> GetSelectedWidgetFunction);
	static void OpenPrefabAsset(TFunction<ULexWidget*()> GetSelectedWidgetFunction);
	static bool CanCheckPrefabOverrideParameter(TFunction<ULexWidget*()> GetSelectedWidgetFunction);
	static bool CanCreateWidget(TFunction<ULexWidget*()> GetSelectedWidgetFunction);
	static void CleanupPrefabs();
	static bool IsWidgetCompatibleWithLexUIToolsMenu(ULexWidget* InWidget);

	static TMap<FString, TWeakObjectPtr<ULexUIPrefab>> CopiedWidgetPrefabMap;//map ActorLabel to prefab
	static bool HaveValidCopiedWidgets();
};
