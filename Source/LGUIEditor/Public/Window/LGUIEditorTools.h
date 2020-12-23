// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Widgets/SCompoundWidget.h"
#include "Widgets/Docking/SDockTab.h"
#include "CoreMinimal.h"
#include "LGUIEditHelper.h"
#include "LGUIEditorTools.generated.h"

#pragma once


UCLASS()
class LGUIEDITOR_API ULGUIEditorToolsAgentObject :public UObject
{
	GENERATED_BODY()
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
	static void CreateScreenSpaceUIBasicSetup();
	static void CreateWorldSpaceUIBasicSetup();
	static UWorld* GetWorldFromSelection();
	static void CreatePrefabAsset();
	static void ApplyPrefab();
	static void RevertPrefab();
	static void DeletePrefab();
	static void SelectPrefabAsset();
	static class ALGUIPrefabActor* GetPrefabActor_WhichManageThisActor(AActor* InActor);
	static void SaveAsset(UObject* InObject, UPackage* InPackage);
	static bool IsCanvasActor(AActor* InActor);
	static int GetCanvasDrawcallCount(AActor* InActor);
	static FString PrintObjectFlags(UObject* Target);

	UPROPERTY(Transient) TArray<class ULGUIPrefab*> copiedActorPrefabList;
	UPROPERTY(Transient) TWeakObjectPtr<UActorComponent> copiedComponent;
	static bool HaveValidCopiedActors();
	static bool HaveValidCopiedComponent();

	static ULGUIEditorToolsAgentObject* GetInstance();
private:
	static ULGUIEditorToolsAgentObject* EditorToolsAgentObject;
};
/**
 * 
 */
class SLGUIEditorTools : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLGUIEditorTools)

	{}

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TSharedPtr<SDockTab> InOwnerTab);

	SLGUIEditorTools();
};
