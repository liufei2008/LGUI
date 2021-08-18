// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Widgets/SCompoundWidget.h"
#include "Widgets/Docking/SDockTab.h"
#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"

#pragma once

class LGUIEDITOR_API LGUIEditorTools
{
private:
	static FString PrevSavePrafabFolder;
public:
	static AActor * GetFirstSelectedActor();
	template<class T>
	static void CreateUIItemActor()
	{
		ULGUIEditorManagerObject::CanExecuteSelectionConvert = false;
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
		ULGUIEditorManagerObject::CanExecuteSelectionConvert = true;
	}
	static void CreateUIItemActor(UClass* ActorClass);
	static void CreateEmptyActor();
	static void CreateUIControls(FString InPrefabPath);
	static void ReplaceUIElementWith(UClass* ActorClass);
	static void DuplicateSelectedActors_Impl();
	static void CopySelectedActors_Impl();
	static void PasteSelectedActors_Impl();
	static void DeleteSelectedActors_Impl();
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
	static void ApplyPrefab();
	static void RevertPrefab();
	static void DeletePrefab();
	static void UnlinkPrefab();
	static void SelectPrefabAsset();
	static class ALGUIPrefabActor* GetPrefabActor_WhichManageThisActor(AActor* InActor);
	static void SaveAsset(UObject* InObject, UPackage* InPackage);
	static void ClearPrefabActor(UWorld* World);
	static bool IsSelectUIActor();
	static bool IsCanvasActor(AActor* InActor);
	static int GetDrawcallCount(AActor* InActor);
	static bool IsSelfRenderActor(AActor* InActor);
	static FString PrintObjectFlags(UObject* Target);
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

	static void SpawnPrefabForEdit(ULGUIPrefab* InPrefab);
};
