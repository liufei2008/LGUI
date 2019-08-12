// Copyright 2019 LexLiu. All Rights Reserved.

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
protected:
	void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent);

	UPROPERTY(EditAnywhere, Category = "UI Create Base Element")FLGUIEditHelperButton UIPanel;
	UPROPERTY(EditAnywhere, Category = "UI Create Base Element")FLGUIEditHelperButton UIContainer;
	UPROPERTY(EditAnywhere, Category = "UI Create Base Element")FLGUIEditHelperButton UISprite;
	UPROPERTY(EditAnywhere, Category = "UI Create Base Element")FLGUIEditHelperButton UIText;
	UPROPERTY(EditAnywhere, Category = "UI Create Base Element")FLGUIEditHelperButton UITexture;

	UPROPERTY(EditAnywhere, Category = "UI Create Control")FLGUIEditHelperButton Button;
	UPROPERTY(EditAnywhere, Category = "UI Create Control")FLGUIEditHelperButton Toggle;
	UPROPERTY(EditAnywhere, Category = "UI Create Control")FLGUIEditHelperButton HorizontalSlider;
	UPROPERTY(EditAnywhere, Category = "UI Create Control")FLGUIEditHelperButton VerticalSlider;
	UPROPERTY(EditAnywhere, Category = "UI Create Control")FLGUIEditHelperButton HorizontalScrollbar;
	UPROPERTY(EditAnywhere, Category = "UI Create Control")FLGUIEditHelperButton VerticalScrollbar;
	UPROPERTY(EditAnywhere, Category = "UI Create Control")FLGUIEditHelperButton FlyoutMenuButton;
	UPROPERTY(EditAnywhere, Category = "UI Create Control")FLGUIEditHelperButton ComboBoxButton;
	UPROPERTY(EditAnywhere, Category = "UI Create Control")FLGUIEditHelperButton TextInput;
	UPROPERTY(EditAnywhere, Category = "UI Create Control")FLGUIEditHelperButton ScrollView;

	UPROPERTY(EditAnywhere, Category = "Actor Action(With Hierarchy)")FLGUIEditHelperButton CopySelectedActors;
	UPROPERTY(EditAnywhere, Category = "Actor Action(With Hierarchy)")FLGUIEditHelperButton PasteSelectedActors;
	UPROPERTY(EditAnywhere, Category = "Actor Action(With Hierarchy)")FLGUIEditHelperButton DuplicateSelectedActors;
	UPROPERTY(EditAnywhere, Category = "Actor Action(With Hierarchy)")FLGUIEditHelperButton DeleteSelectedActors;

	UPROPERTY(EditAnywhere, Category = "Component Action")FLGUIEditHelperButton CopyComponentValues;
	UPROPERTY(EditAnywhere, Category = "Component Action")FLGUIEditHelperButton PasteComponentValues;

	UPROPERTY(EditAnywhere, Category = "UI Atlas Viewer")FLGUIEditHelperButton AtlasViewer;

public:
	static AActor * GetFirstSelectedActor();
	template<class T>
	static void CreateUIItemActor()
	{
		static_assert(TPointerIsConvertibleFromTo<T, const AActor>::Value, "'T' template parameter to CreateUIItemActor must be derived from AActor");
		auto selectedActor = GetFirstSelectedActor();
		AActor* newActor = nullptr;
		GEditor->BeginTransaction(FText::FromString(TEXT("LGUI Create UI Element")));
		newActor = GWorld->SpawnActor<T>(FVector::ZeroVector, FQuat::Identity.Rotator(), FActorSpawnParameters());
		if (selectedActor != nullptr)
		{
			newActor->AttachToActor(selectedActor, FAttachmentTransformRules::KeepRelativeTransform);
			GEditor->SelectActor(selectedActor, false, true);
			GEditor->SelectActor(newActor, true, true);
		}
		GEditor->EndTransaction();
	}
	static void CreateUIItemActor(UClass* ActorClass);
	static void CreateUIControls(FString InPrefabPath);
	static void ReplaceUIElementWith(UClass* ActorClass);
	static void DuplicateSelectedActors_Impl();
	static void CopySelectedActors_Impl();
	static void PasteSelectedActors_Impl();
	static void DeleteSelectedActors_Impl();
	static void CopyComponentValues_Impl();
	static void PasteComponentValues_Impl();
	static void OpenAtlasViewer_Impl();
	static void OpenScreenSpaceUIViewer_Impl();
	static void ChangeTraceChannel_Impl(ETraceTypeQuery InTraceTypeQuery);
	static void CreateScreenSpaceUIBasicSetup();
	static void CreateWorldSpaceUIBasicSetup();
	static UWorld* GetWorldFromSelection();
	static void CreatePrefabAsset();

	UPROPERTY(Transient) TArray<class ULGUIPrefab*> copiedActorPrefabList;
	UPROPERTY(Transient) UActorComponent* copiedComponent;
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
