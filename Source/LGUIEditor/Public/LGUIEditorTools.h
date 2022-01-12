// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Widgets/SCompoundWidget.h"
#include "Widgets/Docking/SDockTab.h"
#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"

#pragma once
class ALGUIPrefabHelperActor;
class ULGUIPrefabHelperObject;
class ULGUIPrefab;

class LGUIEDITOR_API LGUIEditorTools
{
private:
	static FString PrevSavePrafabFolder;
public:
	static FString LGUIPresetPrefabPath;

	static AActor * GetFirstSelectedActor();
	template<class T>
	static void CreateUIItemActor()
	{
		static_assert(TPointerIsConvertibleFromTo<T, const AActor>::Value, "'T' template parameter to CreateUIItemActor must be derived from AActor");
		auto selectedActor = GetFirstSelectedActor();
		AActor* newActor = nullptr;
		GEditor->BeginTransaction(FText::FromString(TEXT("LGUI Create UI Element")));
		newActor = GetWorldFromSelection()->SpawnActor<T>(FVector::ZeroVector, FQuat::Identity.Rotator(), FActorSpawnParameters());
		if (selectedActor != nullptr)
		{
			newActor->AttachToActor(selectedActor, FAttachmentTransformRules::KeepRelativeTransform);
			GEditor->SelectActor(selectedActor, false, true);
			GEditor->SelectActor(newActor, true, true);
		}
		GEditor->EndTransaction();
	}
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
	static void DeleteActors_Impl(const TArray<AActor*>& InActors);
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
	static void ApplyPrefabInLevelEditor();
	static void RefreshLevelLoadedPrefab(ULGUIPrefab* InPrefab);
	static void RefreshOpenPrefabEditor(ULGUIPrefab* InPrefab);
	static void RefreshOnSubPrefabChange(ULGUIPrefab* InSubPrefab);
	static TArray<ULGUIPrefab*> GetAllPrefabArray();
	static void RevertPrefabInLevelEditor();
	static void DeletePrefabInLevelEditor();
	static void UnlinkPrefab();
	static void SelectPrefabAsset();
	static void OpenPrefabAsset();
	static void CleanupPrefabsInWorld(UWorld* World);
	static ULGUIPrefabHelperObject* GetPrefabHelperObject_WhichManageThisActor(AActor* InActor);
	static ALGUIPrefabHelperActor* GetPrefabHelperActor_WhichManageThisActor(AActor* InActor);
	static bool IsPrefabActor(AActor* InActor);
	static void SaveAsset(UObject* InObject, UPackage* InPackage);
	static void ClearInvalidPrefabActor(UWorld* World);
	static bool IsSelectUIActor();
	static bool IsCanvasActor(AActor* InActor);
	static int GetDrawcallCount(AActor* InActor);
	static void FocusToScreenSpaceUI();
	static void FocusToSelectedUI();

	static TArray<TWeakObjectPtr<class ULGUIPrefab>> copiedActorPrefabList;
	static TWeakObjectPtr<class UActorComponent> copiedComponent;
	static bool HaveValidCopiedActors();
	static bool HaveValidCopiedComponent();

	static void RefreshSceneOutliner();

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
