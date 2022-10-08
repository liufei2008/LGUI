﻿// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Widgets/SCompoundWidget.h"
#include "Widgets/Docking/SDockTab.h"
#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"

#pragma once
class ULGUIPrefabHelperObject;
class ULGUIPrefab;

DECLARE_MULTICAST_DELEGATE_OneParam(FEditingPrefabChangedDelegate, AActor*);

class LGUIEDITOR_API LGUIEditorTools
{
private:
	static FString PrevSavePrafabFolder;
public:
	static FString LGUIPresetPrefabPath;
	static FEditingPrefabChangedDelegate EditingPrefabChangedDelegate;
	static AActor* GetFirstSelectedActor();
	static TArray<AActor*> GetSelectedActors();
	static FString GetUniqueNumetricName(const FString& InPrefix, const TArray<FString>& InExistNames);
	static TArray<AActor*> GetRootActorListFromSelection(const TArray<AActor*>& selectedActors);
	static void CreateUIItemActor(UClass* ActorClass);
	static void CreateEmptyActor();
	static void CreateUIControls(FString InPrefabPath);
	static void ReplaceUIElementWith(UClass* ActorClass);
	static void DuplicateSelectedActors_Impl();
	static void CopySelectedActors_Impl();
	static void PasteSelectedActors_Impl();
	static void DeleteSelectedActors_Impl();
	static void CutSelectedActors_Impl();
	static void DeleteActors_Impl(const TArray<AActor*>& InActors);
	static bool CanDuplicateActor();
	static bool CanCopyActor();
	static bool CanPasteActor();
	static bool CanCutActor();
	static bool CanDeleteActor();
	static void CopyComponentValues_Impl();
	static void PasteComponentValues_Impl();
	static void OpenAtlasViewer_Impl();
	static void ChangeTraceChannel_Impl(ETraceTypeQuery InTraceTypeQuery);
	static void CreateScreenSpaceUI_BasicSetup();
	static void CreateWorldSpaceUIUERenderer_BasicSetup();
	static void CreateWorldSpaceUILGUIRenderer_BasicSetup();
	static void AttachComponentToSelectedActor(TSubclassOf<UActorComponent> InComponentClass);
	static UWorld* GetWorldFromSelection();
	static void CreatePrefabAsset();
	static void RefreshLevelLoadedPrefab(ULGUIPrefab* InPrefab);
	static void RefreshOpenedPrefabEditor(ULGUIPrefab* InPrefab);
	static void RefreshOnSubPrefabChange(ULGUIPrefab* InSubPrefab);
	static TArray<ULGUIPrefab*> GetAllPrefabArray();
	static void UnpackPrefab();
	static void SelectPrefabAsset();
	static void OpenPrefabAsset();
	static void UpdateLevelPrefab();
	static void CleanupPrefabsInWorld(UWorld* World);
	static ULGUIPrefabHelperObject* GetPrefabHelperObject_WhichManageThisActor(AActor* InActor);
	static bool IsSelectUIActor();
	static bool IsCanvasActor(AActor* InActor);
	static int GetDrawcallCount(AActor* InActor);
	static void FocusToScreenSpaceUI();
	static void FocusToSelectedUI();

	static TMap<FString, TWeakObjectPtr<class ULGUIPrefab>> CopiedActorPrefabMap;//map ActorLabel to prefab
	static TWeakObjectPtr<class UActorComponent> CopiedComponent;
	static bool HaveValidCopiedActors();
	static bool HaveValidCopiedComponent();

	static void MakeCurrentLevel(AActor* InActor);
	static void SetTraceChannelToParent(AActor* InActor);
	static void SetTraceChannelToParent_Recursive(AActor* InActor);

	static void ForceGC();

	static void UpgradeLevelToLGUI3();
	static void UpgradeAllPrefabToLGUI3();
	static void UpgradeSelectedPrefabToLGUI3();
private:
	static void UpgradeActorArray(const TArray<AActor*>& InActorArray, bool InIsPrefabOrWorld);
	static void UpgradeObjectProperty(UObject* InObject);
	static void UpgradeCommonProperty(FProperty* Property, uint8* ContainerPtr);
};
