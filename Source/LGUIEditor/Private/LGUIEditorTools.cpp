// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "LGUIEditorTools.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Misc/FileHelper.h"
#include "Misc/MessageDialog.h"
#include "LGUIEditorPCH.h"
#include "PropertyCustomizationHelpers.h"
#include "DesktopPlatformModule.h"
#include "AssetRegistryModule.h"
#include "Toolkits/AssetEditorManager.h"
#include "Engine/EngineTypes.h"
#include "Kismet2/ComponentEditorUtils.h"
#include "Widgets/SViewport.h"
#include "Layout/LGUICanvasScaler.h"
#include "EditorViewportClient.h"

#define LOCTEXT_NAMESPACE "LGUIEditorTools"

struct LGUIEditorToolsHelperFunctionHolder
{
public:
	static TArray<AActor*> ConvertSelectionToActors(USelection* InSelection)
	{
		TArray<AActor*> result;
		auto count = InSelection->Num();
		for (int i = 0; i < count; i++)
		{
			auto obj = (AActor*)(InSelection->GetSelectedObject(i));
			if (obj != nullptr)
			{
				result.Add(obj);
			}
		}
		return result;
	}
	static TArray<AActor*> GetRootActorListFromSelection(TArray<AActor*> selectedActors)
	{
		TArray<AActor*> rootActorList;
		auto count = selectedActors.Num();
		//search upward find parent and put into list, only root actor can add to list
		for (int i = 0; i < count; i++)
		{
			auto obj = selectedActors[i];
			auto parent = obj->GetAttachParentActor();
			bool isRootActor = false;
			while (true)
			{
				if (parent == nullptr)//top level
				{
					isRootActor = true;
					break;
				}
				if (selectedActors.Contains(parent))//if parent is already in list, skip it
				{
					isRootActor = false;
					break;
				}
				else//if not in list, keep search upward
				{
					parent = parent->GetAttachParentActor();
					continue;
				}
			}
			if (isRootActor)
			{
				rootActorList.Add(obj);
			}
		}
		return rootActorList;
	}
private:
	static FString GetActorLabelPrefixForCopy(const FString& srcActorLabel, FString& outNumetricSuffix)
	{
		int rightCount = 1;
		while (rightCount <= srcActorLabel.Len() && srcActorLabel.Right(rightCount).IsNumeric())
		{
			rightCount++;
		}
		rightCount--;
		outNumetricSuffix = srcActorLabel.Right(rightCount);
		return srcActorLabel.Left(srcActorLabel.Len() - rightCount);
	}
public:
	static FString GetCopiedActorLabel(AActor* srcActor)
	{
		TArray<AActor*> sameLevelActorList;
		auto parentActor = srcActor->GetAttachParentActor();
		for (TActorIterator<AActor> ActorItr(srcActor->GetWorld()); ActorItr; ++ActorItr)
		{
			if (AActor* itemActor = *ActorItr)
			{
				if (IsValid(itemActor))
				{
					if (IsValid(parentActor))
					{
						if (itemActor->GetAttachParentActor() == parentActor)
						{
							sameLevelActorList.Add(itemActor);
						}
					}
					else
					{
						if (itemActor->GetAttachParentActor() == nullptr)
						{
							sameLevelActorList.Add(itemActor);
						}
					}
				}
			}
		}
		
		auto srcActorLabel = srcActor->GetActorLabel();

		FString maxNumetricSuffixStr = TEXT("");
		srcActorLabel = GetActorLabelPrefixForCopy(srcActorLabel, maxNumetricSuffixStr);
		int maxNumetricSuffixStrLength = maxNumetricSuffixStr.Len();
		int count = sameLevelActorList.Num();
		for (int i = 0; i < count; i ++)//search from same level actors, and get the right suffix
		{
			auto item = sameLevelActorList[i];
			auto itemActorLabel = item->GetActorLabel();
			if (srcActorLabel.Len() == 0 || itemActorLabel.StartsWith(srcActorLabel))
			{
				auto itemRightStr = itemActorLabel.Right(itemActorLabel.Len() - srcActorLabel.Len());
				if (!itemRightStr.IsNumeric())//if rest is not numetric
				{
					continue;
				}
				FString itemNumetrixSuffixStr = itemRightStr;
				int itemNumetrix = FCString::Atoi(*itemNumetrixSuffixStr);
				int maxNumetrixSuffix = FCString::Atoi(*maxNumetricSuffixStr);
				if (itemNumetrix > maxNumetrixSuffix)
				{
					maxNumetrixSuffix = itemNumetrix;
					maxNumetricSuffixStr = FString::Printf(TEXT("%d"), maxNumetrixSuffix);
				}
			}
		}
		FString copiedActorLabel = srcActorLabel;
		int maxNumtrixSuffix = FCString::Atoi(*maxNumetricSuffixStr);
		maxNumtrixSuffix++;
		FString numetrixSuffixStr = FString::Printf(TEXT("%d"), maxNumtrixSuffix);
		while (numetrixSuffixStr.Len() < maxNumetricSuffixStrLength)
		{
			numetrixSuffixStr = TEXT("0") + numetrixSuffixStr;
		}
		copiedActorLabel += numetrixSuffixStr;
		return copiedActorLabel;
	}
	
public:
	static TArray<UActorComponent*> ConvertSelectionToComponents(USelection* InSelection)
	{
		TArray<UActorComponent*> result;
		auto count = InSelection->Num();
		for (int i = 0; i < count; i++)
		{
			auto obj = (UActorComponent*)(InSelection->GetSelectedObject(i));
			if (obj != nullptr)
			{
				result.Add(obj);
			}
		}
		return result;
	}
};

TArray<TWeakObjectPtr<class ULGUIPrefab>> LGUIEditorTools::copiedActorPrefabList;
TWeakObjectPtr<class UActorComponent> LGUIEditorTools::copiedComponent;

UWorld* LGUIEditorTools::GetWorldFromSelection()
{
	if (auto selectedActor = GetFirstSelectedActor())
	{
		return selectedActor->GetWorld();
	}
	return GWorld;
}
void LGUIEditorTools::CreateUIItemActor(UClass* ActorClass)
{
	ULGUIEditorManagerObject::CanExecuteSelectionConvert = false;
	GEditor->BeginTransaction(FText::FromString(TEXT("LGUI Create UI Element")));
	auto selectedActor = GetFirstSelectedActor();
	MakeCurrentLevel(selectedActor);
	AActor* newActor = GetWorldFromSelection()->SpawnActor<AActor>(ActorClass, FTransform::Identity, FActorSpawnParameters());
	if (IsValid(newActor))
	{
		if (selectedActor != nullptr)
		{
			newActor->AttachToActor(selectedActor, FAttachmentTransformRules::KeepRelativeTransform);
			GEditor->SelectActor(selectedActor, false, true);
		}
		GEditor->SelectActor(newActor, true, true);
	}
	GEditor->EndTransaction();
	ULGUIEditorManagerObject::CanExecuteSelectionConvert = true;
}
void LGUIEditorTools::CreateEmptyActor()
{
	ULGUIEditorManagerObject::CanExecuteSelectionConvert = false;
	GEditor->BeginTransaction(FText::FromString(TEXT("LGUI Create UI Element")));
	auto selectedActor = GetFirstSelectedActor();
	MakeCurrentLevel(selectedActor);
	AActor* newActor = GetWorldFromSelection()->SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, FActorSpawnParameters());
	if (IsValid(newActor))
	{
		//create SceneComponent
		{
			USceneComponent* RootComponent = NewObject<USceneComponent>(newActor, USceneComponent::GetDefaultSceneRootVariableName(), RF_Transactional);
			RootComponent->Mobility = EComponentMobility::Movable;
			RootComponent->bVisualizeComponent = false;

			newActor->SetRootComponent(RootComponent);
			RootComponent->RegisterComponent();
			newActor->AddInstanceComponent(RootComponent);
		}
		if (selectedActor != nullptr)
		{
			newActor->AttachToActor(selectedActor, FAttachmentTransformRules::KeepRelativeTransform);
			GEditor->SelectActor(selectedActor, false, true);
		}
		GEditor->SelectActor(newActor, true, true);
	}
	GEditor->EndTransaction();
	ULGUIEditorManagerObject::CanExecuteSelectionConvert = true;
}

AActor* LGUIEditorTools::GetFirstSelectedActor()
{
	auto selectedActors = LGUIEditorToolsHelperFunctionHolder::ConvertSelectionToActors(GEditor->GetSelectedActors());
	auto count = selectedActors.Num();
	if (count == 0)
	{
		//UE_LOG(LGUIEditor, Error, TEXT("NothingSelected"));
		return nullptr;
	}
	else if (count > 1)
	{
		//UE_LOG(LGUIEditor, Error, TEXT("Only support one component"));
		return nullptr;
	}
	return selectedActors[0];
}
void LGUIEditorTools::CreateUIControls(FString InPrefabPath)
{
	ULGUIEditorManagerObject::CanExecuteSelectionConvert = false;
	GEditor->BeginTransaction(LOCTEXT("CreateUIControl", "LGUI Create UI Control"));
	auto selectedActor = GetFirstSelectedActor();
	MakeCurrentLevel(selectedActor);
	auto prefab = LoadObject<ULGUIPrefab>(NULL, *InPrefabPath);
	if (prefab)
	{
		auto actor = LGUIPrefabSystem::ActorSerializer::LoadPrefabForEdit(GetWorldFromSelection(), prefab
			, selectedActor == nullptr ? nullptr : selectedActor->GetRootComponent());
		GEditor->SelectActor(selectedActor, false, true);
		GEditor->SelectActor(actor, true, true);
	}
	else
	{
		UE_LOG(LGUIEditor, Error, TEXT("[LGUIEditorToolsAgentObject::CreateUIControls]Load control prefab error! Path:%s. Missing some content of LGUI plugin, reinstall this plugin may fix the issure."), *InPrefabPath);
	}
	GEditor->EndTransaction();
	ULGUIEditorManagerObject::CanExecuteSelectionConvert = true;
}
void LGUIEditorTools::ReplaceUIElementWith(UClass* ActorClass)
{
	ULGUIEditorManagerObject::CanExecuteSelectionConvert = false;
	auto selectedActors = LGUIEditorToolsHelperFunctionHolder::ConvertSelectionToActors(GEditor->GetSelectedActors());
	auto count = selectedActors.Num();
	if (count == 0)
	{
		UE_LOG(LGUIEditor, Error, TEXT("NothingSelected"));
		ULGUIEditorManagerObject::CanExecuteSelectionConvert = true;
		return;
	}
	auto rootActorList = LGUIEditorToolsHelperFunctionHolder::GetRootActorListFromSelection(selectedActors);

	GEditor->BeginTransaction(LOCTEXT("ReplaceUIElement", "LGUI Replace UI Element"));
	GEditor->SelectNone(true, true);
	for (auto item : rootActorList)
	{
		MakeCurrentLevel(item);
		auto newActor = LGUIPrefabSystem::ActorReplaceTool::ReplaceActorClass(item, ActorClass);
		GEditor->SelectActor(newActor, true, true);
	}
	GEditor->EndTransaction();
	ULGUIEditorManagerObject::CanExecuteSelectionConvert = true;
}
void LGUIEditorTools::DuplicateSelectedActors_Impl()
{
	ULGUIEditorManagerObject::CanExecuteSelectionConvert = false;
	auto selectedActors = LGUIEditorToolsHelperFunctionHolder::ConvertSelectionToActors(GEditor->GetSelectedActors());
	auto count = selectedActors.Num();
	if (count == 0)
	{
		UE_LOG(LGUIEditor, Error, TEXT("NothingSelected"));
		ULGUIEditorManagerObject::CanExecuteSelectionConvert = true;
		return;
	}
	auto rootActorList = LGUIEditorToolsHelperFunctionHolder::GetRootActorListFromSelection(selectedActors);
	GEditor->BeginTransaction(LOCTEXT("DuplicateActor", "LGUI Duplicate Actors"));
	for (auto item : rootActorList)
	{
		MakeCurrentLevel(item);
		auto copiedActorLabel = LGUIEditorToolsHelperFunctionHolder::GetCopiedActorLabel(item);
		AActor* copiedActor;
		if (item->GetAttachParentActor())
		{
			copiedActor = ULGUIBPLibrary::DuplicateActor(item, item->GetAttachParentActor()->GetRootComponent());
		}
		else
		{
			copiedActor = ULGUIBPLibrary::DuplicateActor(item, nullptr);
		}
		copiedActor->SetActorLabel(copiedActorLabel);
		GEditor->SelectActor(item, false, true);
		GEditor->SelectActor(copiedActor, true, true);
	}
	GEditor->EndTransaction();
	ULGUIEditorManagerObject::CanExecuteSelectionConvert = true;
}
void LGUIEditorTools::CopySelectedActors_Impl()
{
	auto selectedActors = LGUIEditorToolsHelperFunctionHolder::ConvertSelectionToActors(GEditor->GetSelectedActors());
	auto count = selectedActors.Num();
	if (count == 0)
	{
		UE_LOG(LGUIEditor, Error, TEXT("NothingSelected"));
		return;
	}
	for (auto prevCopiedActorPrefab : copiedActorPrefabList)
	{
		prevCopiedActorPrefab->RemoveFromRoot();
		prevCopiedActorPrefab->ConditionalBeginDestroy();
	}
	auto copiedActorList = LGUIEditorToolsHelperFunctionHolder::GetRootActorListFromSelection(selectedActors);
	copiedActorPrefabList.Reset();
	for (auto copiedActor : copiedActorList)
	{
		auto prefab = NewObject<ULGUIPrefab>();
		prefab->AddToRoot();
		LGUIPrefabSystem::ActorSerializer::SavePrefab(copiedActor, prefab);
		copiedActorPrefabList.Add(prefab);
	}
}
void LGUIEditorTools::PasteSelectedActors_Impl()
{
	ULGUIEditorManagerObject::CanExecuteSelectionConvert = false;
	auto selectedActors = LGUIEditorToolsHelperFunctionHolder::ConvertSelectionToActors(GEditor->GetSelectedActors());
	USceneComponent* parentComp = nullptr;
	if (selectedActors.Num() > 0)
	{
		parentComp = selectedActors[0]->GetRootComponent();
	}
	GEditor->BeginTransaction(LOCTEXT("PasteActor", "LGUI Paste Actors"));
	for (auto item : selectedActors)
	{
		GEditor->SelectActor(item, false, true);
	}
	if (IsValid(parentComp))
	{
		MakeCurrentLevel(parentComp->GetOwner());
	}
	for (auto prefab : copiedActorPrefabList)
	{
		if (prefab.IsValid())
		{
			auto copiedActor = LGUIPrefabSystem::ActorSerializer::LoadPrefabForEdit(GetWorldFromSelection(), prefab.Get(), parentComp, false);
			auto copiedActorLabel = LGUIEditorToolsHelperFunctionHolder::GetCopiedActorLabel(copiedActor);
			copiedActor->SetActorLabel(copiedActorLabel);
			GEditor->SelectActor(copiedActor, true, true);
		}
		else
		{
			UE_LOG(LGUIEditor, Error, TEXT("Source copied actor is missing!"));
		}
	}
	GEditor->EndTransaction();
	ULGUIEditorManagerObject::CanExecuteSelectionConvert = true;
}
void LGUIEditorTools::DeleteSelectedActors_Impl()
{
	auto selectedActors = LGUIEditorToolsHelperFunctionHolder::ConvertSelectionToActors(GEditor->GetSelectedActors());
	auto count = selectedActors.Num();
	if (count == 0)
	{
		UE_LOG(LGUIEditor, Error, TEXT("NothingSelected"));
		return;
	}
	auto confirmMsg = FString::Printf(TEXT("Destroy selected actors? This will also destroy the children attached to selected actors."));
	auto confirmResult = FMessageDialog::Open(EAppMsgType::YesNo, FText::FromString(confirmMsg));
	if (confirmResult == EAppReturnType::No)return;

	auto rootActorList = LGUIEditorToolsHelperFunctionHolder::GetRootActorListFromSelection(selectedActors);
	GEditor->BeginTransaction(LOCTEXT("DestroyActor", "LGUI Destroy Actor"));
	GEditor->GetSelectedActors()->DeselectAll();
	for (auto item : rootActorList)
	{
		bool shouldDeletePrefab = false;
		auto prefabActor = LGUIEditorTools::GetPrefabActor_WhichManageThisActor(item);
		if (prefabActor != nullptr)
		{
			if (auto prefabComp = prefabActor->GetPrefabComponent())
			{
				if (auto loadedRootActor = prefabComp->LoadedRootActor)
				{
					if (loadedRootActor == item)
					{
						shouldDeletePrefab = true;
					}
				}
			}
		}
		ULGUIBPLibrary::DestroyActorWithHierarchy(item);
		if (shouldDeletePrefab)
		{
			ULGUIBPLibrary::DestroyActorWithHierarchy(prefabActor, false);
		}
	}
	GEditor->EndTransaction();
}
void LGUIEditorTools::CopyComponentValues_Impl()
{
	auto selectedComponents = LGUIEditorToolsHelperFunctionHolder::ConvertSelectionToComponents(GEditor->GetSelectedComponents());
	auto count = selectedComponents.Num();
	if (count == 0)
	{
		UE_LOG(LGUIEditor, Error, TEXT("NothingSelected"));
		return;
	}
	else if (count > 1)
	{
		UE_LOG(LGUIEditor, Error, TEXT("Only support one component"));
		return;
	}
	copiedComponent = selectedComponents[0];
}
void LGUIEditorTools::PasteComponentValues_Impl()
{
	auto selectedComponents = LGUIEditorToolsHelperFunctionHolder::ConvertSelectionToComponents(GEditor->GetSelectedComponents());
	auto count = selectedComponents.Num();
	if (count == 0)
	{
		UE_LOG(LGUIEditor, Error, TEXT("NothingSelected"));
		return;
	}
	if (copiedComponent.IsValid())
	{
		GEditor->BeginTransaction(LOCTEXT("PasteComponentValues", "LGUI Paste Component Proeprties"));
		for (UActorComponent* item : selectedComponents)
		{
			LGUIPrefabSystem::ActorCopier::CopyComponentValue(copiedComponent.Get(), item);
		}
		GEditor->EndTransaction();
	}
	else
	{
		UE_LOG(LGUIEditor, Error, TEXT("Selected component is missing!"));
	}
}
void LGUIEditorTools::OpenAtlasViewer_Impl()
{
	FGlobalTabmanager::Get()->InvokeTab(FLGUIEditorModule::LGUIAtlasViewerName);
}
void LGUIEditorTools::ChangeTraceChannel_Impl(ETraceTypeQuery InTraceTypeQuery)
{
	auto selectedActors = LGUIEditorToolsHelperFunctionHolder::ConvertSelectionToActors(GEditor->GetSelectedActors());
	auto count = selectedActors.Num();
	if (count == 0)
	{
		UE_LOG(LGUIEditor, Error, TEXT("NothingSelected"));
		return;
	}
	struct FunctionContainer
	{
		static void ChangeTraceChannel(USceneComponent* InSceneComp, ETraceTypeQuery InChannel)
		{
			if (IsValid(InSceneComp))
			{
				if (auto uiItemComp = Cast<UUIItem>(InSceneComp))
				{
					uiItemComp->SetTraceChannel(InChannel);
				}
				auto children = InSceneComp->GetAttachChildren();
				for (auto itemComp : children)
				{
					ChangeTraceChannel(itemComp, InChannel);
				}
			}
		}
	};
	auto rootActorList = LGUIEditorToolsHelperFunctionHolder::GetRootActorListFromSelection(selectedActors);
	GEditor->BeginTransaction(LOCTEXT("ChangeTraceChannel", "LGUI Change Trace Channel"));
	for (auto item : rootActorList)
	{
		FunctionContainer::ChangeTraceChannel(item->GetRootComponent(), InTraceTypeQuery);
	}
	GEditor->EndTransaction();
}
void LGUIEditorTools::CreateScreenSpaceUIBasicSetup()
{
	ULGUIEditorManagerObject::CanExecuteSelectionConvert = false;
	GEditor->BeginTransaction(FText::FromString(TEXT("LGUI Create Screen Space UI")));
	auto selectedActor = GetFirstSelectedActor();
	FString prefabPath(TEXT("/LGUI/Prefabs/ScreenSpaceUI"));
	auto prefab = LoadObject<ULGUIPrefab>(NULL, *prefabPath);
	if (prefab)
	{
		auto actor = LGUIPrefabSystem::ActorSerializer::LoadPrefabForEdit(GetWorldFromSelection(), prefab, nullptr, true);
		actor->GetRootComponent()->SetRelativeScale3D(FVector::OneVector);
		actor->GetRootComponent()->SetRelativeLocation(FVector(0, 0, 250));
		actor->GetRootComponent()->SetRelativeRotationExact(FRotator::MakeFromEuler(FVector(0, 180, 0)));
		if (selectedActor)GEditor->SelectActor(selectedActor, false, true);
		GEditor->SelectActor(actor, true, true);

		bool haveEventSystem = false;
		for (TActorIterator<ALGUIEventSystemActor> eventSysActorItr(GetWorldFromSelection()); eventSysActorItr; ++eventSysActorItr)
		{
			haveEventSystem = true;
			break;
		}
		if (!haveEventSystem)
		{
			if (auto presetEventSystemActorClass = LoadObject<UClass>(NULL, TEXT("/LGUI/Blueprints/PresetEventSystemActor.PresetEventSystemActor_C")))
			{
				GetWorldFromSelection()->SpawnActor<AActor>(presetEventSystemActorClass);
			}
			else
			{
				UE_LOG(LGUIEditor, Error, TEXT("[ULGUIEditorToolsAgentObject::CreateScreenSpaceUIBasicSetup]Load PresetEventSystemActor error! Missing some content of LGUI plugin, reinstall this plugin may fix the issure."));
			}
		}
	}
	else
	{
		UE_LOG(LGUIEditor, Error, TEXT("[LGUIEditorToolsAgentObject::CreateScreenSpaceUIBasicSetup]Load control prefab error! Path:%s. Missing some content of LGUI plugin, reinstall this plugin may fix the issure."), *prefabPath);
	}
	GEditor->EndTransaction();
	ULGUIEditorManagerObject::CanExecuteSelectionConvert = true;
}
void LGUIEditorTools::CreateWorldSpaceUIBasicSetup()
{
	ULGUIEditorManagerObject::CanExecuteSelectionConvert = false;
	GEditor->BeginTransaction(FText::FromString(TEXT("LGUI Create World Space UI")));
	auto selectedActor = GetFirstSelectedActor();
	FString prefabPath(TEXT("/LGUI/Prefabs/WorldSpaceUI"));
	auto prefab = LoadObject<ULGUIPrefab>(NULL, *prefabPath);
	if (prefab)
	{
		auto actor = LGUIPrefabSystem::ActorSerializer::LoadPrefabForEdit(GetWorldFromSelection(), prefab, nullptr, true);
		actor->GetRootComponent()->SetRelativeLocation(FVector(0, 0, 250));
		actor->GetRootComponent()->SetRelativeRotationExact(FRotator::MakeFromEuler(FVector(-90, 0, 0)));
		actor->GetRootComponent()->SetWorldScale3D(FVector::OneVector * 0.3f);
		if (selectedActor)GEditor->SelectActor(selectedActor, false, true);
		GEditor->SelectActor(actor, true, true);
		
		bool haveEventSystem = false;
		for (TActorIterator<ALGUIEventSystemActor> eventSysActorItr(GetWorldFromSelection()); eventSysActorItr; ++eventSysActorItr)
		{
			haveEventSystem = true;
			break;
		}
		if (!haveEventSystem)
		{
			if (auto presetEventSystemActorClass = LoadObject<UClass>(NULL, TEXT("/LGUI/Blueprints/PresetEventSystemActor.PresetEventSystemActor_C")))
			{
				GetWorldFromSelection()->SpawnActor<AActor>(presetEventSystemActorClass);
			}
			else
			{
				UE_LOG(LGUIEditor, Error, TEXT("[ULGUIEditorToolsAgentObject::CreateWorldSpaceUIBasicSetup]Load PresetEventSystemActor error! Missing some content of LGUI plugin, reinstall this plugin may fix the issure."));
			}
		}
	}
	else
	{
		UE_LOG(LGUIEditor, Error, TEXT("[LGUIEditorToolsAgentObject::CreateWorldSpaceUIBasicSetup]Load control prefab error! Path:%s. Missing some content of LGUI plugin, reinstall this plugin may fix the issure."), *prefabPath);
	}
	GEditor->EndTransaction();
	ULGUIEditorManagerObject::CanExecuteSelectionConvert = true;
}
void LGUIEditorTools::AttachComponentToSelectedActor(TSubclassOf<UActorComponent> InComponentClass)
{
	ULGUIEditorManagerObject::CanExecuteSelectionConvert = false;
	GEditor->BeginTransaction(FText::FromString(TEXT("LGUI Attach Component to Actor")));

	auto selectedActors = LGUIEditorToolsHelperFunctionHolder::ConvertSelectionToActors(GEditor->GetSelectedActors());
	auto count = selectedActors.Num();
	if (count == 0)
	{
		UE_LOG(LGUIEditor, Error, TEXT("NothingSelected"));
		return;
	}
	UActorComponent* lastCreatedComponent = nullptr;
	for (auto actor : selectedActors)
	{
		if (IsValid(actor))
		{
			auto comp = NewObject<UActorComponent>(actor, InComponentClass, *FComponentEditorUtils::GenerateValidVariableName(InComponentClass, actor), RF_Transactional);
			actor->AddInstanceComponent(comp);
			comp->OnComponentCreated();
			comp->RegisterComponent();
			lastCreatedComponent = comp;
		}
	}

	GEditor->EndTransaction();
	ULGUIEditorManagerObject::CanExecuteSelectionConvert = true;

	if (selectedActors.Num() == 1)
	{
		GEditor->SelectNone(true, true);
		GEditor->SelectActor(lastCreatedComponent->GetOwner(), true, true, false, true);
		GEditor->SelectComponent(lastCreatedComponent, true, true, false);
	}
}
bool LGUIEditorTools::HaveValidCopiedActors()
{
	if (copiedActorPrefabList.Num() == 0)return false;
	for (auto item : copiedActorPrefabList)
	{
		if (!item.IsValid())
		{
			return false;
		}
	}
	return true;
}
bool LGUIEditorTools::HaveValidCopiedComponent()
{
	return copiedComponent.IsValid();
}

void LGUIEditorTools::CreatePrefabAsset()
{
	auto selectedActor = GetFirstSelectedActor();
	if (Cast<ALGUIPrefabActor>(selectedActor) != nullptr)
	{
		UE_LOG(LGUIEditor, Error, TEXT("[ULGUIEditorToolsAgentObject::CreatePrefabAsset]Cannot create prefab on a PrefabActor!"));
		return;
	}
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform)
	{
		TArray<FString> OutFileNames;
		DesktopPlatform->SaveFileDialog(
			FSlateApplication::Get().FindBestParentWindowHandleForDialogs(FSlateApplication::Get().GetGameViewport()),
			TEXT("Choose a path to save prefab asset, must inside Content folder"),
			FPaths::ProjectContentDir(),
			selectedActor->GetActorLabel() + TEXT("_Prefab"),
			TEXT("*.*"),
			EFileDialogFlags::None,
			OutFileNames
		);
		if (OutFileNames.Num() > 0)
		{
			FString selectedFilePath = OutFileNames[0];
			if (selectedFilePath.StartsWith(FPaths::ProjectContentDir()))
			{
				selectedFilePath.RemoveFromStart(FPaths::ProjectContentDir(), ESearchCase::CaseSensitive);
				FString packageName = TEXT("/Game/") + selectedFilePath;
				UPackage* package = CreatePackage(NULL, *packageName);
				if (package == nullptr)
				{
					FMessageDialog::Open(EAppMsgType::Ok
						, FText::FromString(FString::Printf(TEXT("Selected path not valid, please choose another path to save prefab."))));
					return;
				}
				package->FullyLoad();
				FString fileName = FPaths::GetBaseFilename(selectedFilePath);
				auto OutPrefab = NewObject<ULGUIPrefab>((UObject*)package, ULGUIPrefab::StaticClass(), *fileName, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone);
				FAssetRegistryModule::AssetCreated((UObject*)OutPrefab);
				FString packageSavePath = FString::Printf(TEXT("/Game/%s%s"), *selectedFilePath, *FPackageName::GetAssetPackageExtension());
				UPackage::SavePackage(package, OutPrefab, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *packageSavePath);//@todo: this will create a "Game" folder at D: drive, fix it!

				auto prefabActor = selectedActor->GetWorld()->SpawnActorDeferred<ALGUIPrefabActor>(ALGUIPrefabActor::StaticClass(), FTransform::Identity);
				if (IsValid(prefabActor))
				{
					prefabActor->SetActorLabel(fileName);
					auto prefabComp = prefabActor->GetPrefabComponent();
					prefabComp->PrefabAsset = OutPrefab;
					prefabComp->LoadedRootActor = selectedActor;
					LGUIUtils::CollectChildrenActors(selectedActor, prefabComp->AllLoadedActorArray);
					prefabActor->FinishSpawning(FTransform::Identity, true);
					prefabComp->SavePrefab();

					prefabComp->MoveActorToPrefabFolder();
				}
				else
				{
					FMessageDialog::Open(EAppMsgType::Ok
						, FText::FromString(FString::Printf(TEXT("Prefab spawn failed! Is there any missing class referenced by prefab?"))));
				}
			}
			else
			{
				FMessageDialog::Open(EAppMsgType::Ok
					, FText::FromString(FString::Printf(TEXT("Prefab should only save inside Content folder!"))));
			}
		}
	}
}
void LGUIEditorTools::ApplyPrefab()
{
	GEditor->BeginTransaction(FText::FromString(TEXT("LGUI ApplyPrefab")));
	auto selectedActor = GetFirstSelectedActor();
	auto prefabActor = LGUIEditorTools::GetPrefabActor_WhichManageThisActor(selectedActor);
	if (prefabActor != nullptr)
	{
		prefabActor->GetPrefabComponent()->SavePrefab();
	}
	GEditor->EndTransaction();
}
void LGUIEditorTools::RevertPrefab()
{
	GEditor->BeginTransaction(FText::FromString(TEXT("LGUI RevertPrefab")));
	auto selectedActor = GetFirstSelectedActor();
	auto prefabActor = LGUIEditorTools::GetPrefabActor_WhichManageThisActor(selectedActor);
	if (prefabActor != nullptr)
	{
		prefabActor->GetPrefabComponent()->RevertPrefab();
	}
	GEditor->EndTransaction();
}
void LGUIEditorTools::DeletePrefab()
{
	auto confirmMsg = FString::Printf(TEXT("Delete selected prefab instance?"));
	auto confirmResult = FMessageDialog::Open(EAppMsgType::YesNo, FText::FromString(confirmMsg));
	if (confirmResult == EAppReturnType::No)return;

	GEditor->BeginTransaction(FText::FromString(TEXT("LGUI DeletePrefabInstance")));
	auto selectedActor = GetFirstSelectedActor();
	auto prefabActor = LGUIEditorTools::GetPrefabActor_WhichManageThisActor(selectedActor);
	if (prefabActor != nullptr)
	{
		prefabActor->GetPrefabComponent()->DeleteThisInstance();
	}
	GEditor->EndTransaction();
}
void LGUIEditorTools::UnlinkPrefab()
{
	GEditor->BeginTransaction(FText::FromString(TEXT("LGUI UnlinkPrefab")));
	auto selectedActor = GetFirstSelectedActor();
	auto prefabActor = LGUIEditorTools::GetPrefabActor_WhichManageThisActor(selectedActor);
	if (prefabActor != nullptr)
	{
		ULGUIBPLibrary::DestroyActorWithHierarchy(prefabActor);
	}
	GEditor->EndTransaction();
}
void LGUIEditorTools::SelectPrefabAsset()
{
	GEditor->BeginTransaction(FText::FromString(TEXT("LGUI SelectPrefabAsset")));
	auto selectedActor = GetFirstSelectedActor();
	auto prefabActor = LGUIEditorTools::GetPrefabActor_WhichManageThisActor(selectedActor);
	if (prefabActor != nullptr)
	{
		auto prefabAsset = prefabActor->GetPrefabComponent()->PrefabAsset;
		if (IsValid(prefabAsset))
		{
			TArray<UObject*> objectsToSync;
			objectsToSync.Add(prefabAsset);
			GEditor->SyncBrowserToObjects(objectsToSync);
		}
	}
	GEditor->EndTransaction();
}
ALGUIPrefabActor* LGUIEditorTools::GetPrefabActor_WhichManageThisActor(AActor* InActor)
{
	for (TActorIterator<ALGUIPrefabActor> ActorItr(InActor->GetWorld()); ActorItr; ++ActorItr)
	{
		auto prefabActor = *ActorItr;
		if (prefabActor->GetPrefabComponent()->AllLoadedActorArray.Contains(InActor))
		{
			if (auto loadedRootActor = prefabActor->GetPrefabComponent()->LoadedRootActor)
			{
				if (InActor->IsAttachedTo(loadedRootActor) || InActor == loadedRootActor)
				{
					return prefabActor;
				}
			}
		}
	}
	return nullptr;
}
void LGUIEditorTools::SaveAsset(UObject* InObject, UPackage* InPackage)
{
	FAssetRegistryModule::AssetCreated(InObject);
	FString packageSavePath = FString::Printf(TEXT("/Game/%s%s"), *(InObject->GetPathName()), *FPackageName::GetAssetPackageExtension());
	UPackage::SavePackage(InPackage, InObject, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *packageSavePath);
	InPackage->MarkPackageDirty();
}
bool LGUIEditorTools::IsCanvasActor(AActor* InActor)
{
	if (auto rootComp = InActor->GetRootComponent())
	{
		if (auto rootUIItem = Cast<UUIItem>(rootComp))
		{
			if (rootUIItem->IsCanvasUIItem())
			{
				return true;
			}
		}
	}
	return false;
}
bool LGUIEditorTools::IsSelectUIActor()
{
	auto selectedActors = LGUIEditorToolsHelperFunctionHolder::ConvertSelectionToActors(GEditor->GetSelectedActors());
	if (selectedActors.Num() > 0)
	{
		bool allIsUI = true;
		for (auto actor : selectedActors)
		{
			if (IsValid(actor))
			{
				if (auto rootComp = actor->GetRootComponent())
				{
					auto uiRootComp = Cast<UUIItem>(rootComp);
					if (uiRootComp == nullptr)
					{
						allIsUI = false;
					}
				}
			}
		}
		return allIsUI;
	}
	return false;
}
bool LGUIEditorTools::IsSelfRenderActor(AActor* InActor)
{
	if (auto rootComp = InActor->GetRootComponent())
	{
		if (auto uiRenderable = Cast<UUIRenderable>(rootComp))
		{
			if (uiRenderable->GetIsSelfRender())
			{
				return true;
			}
		}
	}
	return false;
}
int LGUIEditorTools::GetDrawcallCount(AActor* InActor)
{
	if (auto rootComp = InActor->GetRootComponent())
	{
		if (auto rootUIItem = Cast<UUIItem>(rootComp))
		{
			if (rootUIItem->IsCanvasUIItem())
			{
				return rootUIItem->GetRenderCanvas()->GetDrawcallCount();
			}
			if (auto uiRenderable = Cast<UUIRenderable>(rootComp))
			{
				return (uiRenderable->GetIsSelfRender() && uiRenderable->IsUIActiveInHierarchy()) ? 1 : 0;
			}
		}
	}
	return 0;
}
void LGUIEditorTools::MakeCurrentLevel(AActor* InActor)
{
	if (IsValid(InActor) && InActor->GetWorld() && InActor->GetLevel())
	{
		if (InActor->GetWorld()->GetCurrentLevel() != InActor->GetLevel())
		{
			if (!InActor->GetWorld()->GetCurrentLevel()->bLocked)
			{
				if (!InActor->GetLevel()->IsCurrentLevel())
				{
					InActor->GetWorld()->SetCurrentLevel(InActor->GetLevel());
				}
			}
			else
			{
				LGUIUtils::EditorNotification(FText::FromString(FString::Printf(TEXT("The level of selected actor:%s is locked!"), *(InActor->GetActorLabel()))));
			}
		}
	}
}
void LGUIEditorTools::SpawnPrefabForEdit(ULGUIPrefab* InPrefab)
{
	if (!IsValid(InPrefab))return;

	ULGUIEditorManagerObject::CanExecuteSelectionConvert = false;
	GEditor->BeginTransaction(LOCTEXT("CreateUIControl", "LGUI Create UI Control"));
	auto selectedActor = GetFirstSelectedActor();
	MakeCurrentLevel(selectedActor);
	

	auto PrefabActor = GetWorldFromSelection()->SpawnActorDeferred<ALGUIPrefabActor>(ALGUIPrefabActor::StaticClass(), FTransform::Identity);
	auto prefabComp = PrefabActor->GetPrefabComponent();
	prefabComp->PrefabAsset = InPrefab;

	//prefabComp->SetRelativeRotation(FQuat::MakeFromEuler(FVector(-90, 0, 90)));
	prefabComp->ParentActorForEditor = selectedActor;
	PrefabActor->FinishSpawning(FTransform::Identity, true);

	prefabComp->LoadedRootActor = LGUIPrefabSystem::ActorSerializer::LoadPrefabForEdit(prefabComp->GetWorld(), InPrefab
		, IsValid(prefabComp->ParentActorForEditor) ? prefabComp->ParentActorForEditor->GetRootComponent()
		: nullptr, prefabComp->AllLoadedActorArray);
	prefabComp->ParentActorForEditor = nullptr;

	prefabComp->MoveActorToPrefabFolder();
	prefabComp->LoadPrefab();
	GEditor->SelectNone(true, true);
	GEditor->SelectActor(prefabComp->LoadedRootActor, true, true, false, true);


	GEditor->EndTransaction();
	ULGUIEditorManagerObject::CanExecuteSelectionConvert = true;
}
void LGUIEditorTools::RefreshSceneOutliner()
{
	GEngine->BroadcastLevelActorListChanged();
}
FString LGUIEditorTools::PrintObjectFlags(UObject* Target)
{
	return FString::Printf(TEXT("Flags:%d\
, \nRF_Public:%d\
, \nRF_Standalone:%d\
, \nRF_MarkAsNative:%d\
, \nRF_Transactional:%d\
, \nRF_ClassDefaultObject:%d\
, \nRF_ArchetypeObject:%d\
, \nRF_Transient:%d\
, \nRF_MarkAsRootSet:%d\
, \nRF_TagGarbageTemp:%d\
, \nRF_NeedInitialization:%d\
, \nRF_NeedLoad:%d\
, \nRF_KeepForCooker:%d\
, \nRF_NeedPostLoad:%d\
, \nRF_NeedPostLoadSubobjects:%d\
, \nRF_NewerVersionExists:%d\
, \nRF_BeginDestroyed:%d\
, \nRF_FinishDestroyed:%d\
, \nRF_BeingRegenerated:%d\
, \nRF_DefaultSubObject:%d\
, \nRF_WasLoaded:%d\
, \nRF_TextExportTransient:%d\
, \nRF_LoadCompleted:%d\
, \nRF_InheritableComponentTemplate:%d\
, \nRF_DuplicateTransient:%d\
, \nRF_StrongRefOnFrame:%d\
, \nRF_NonPIEDuplicateTransient:%d\
, \nRF_Dynamic:%d\
, \nRF_WillBeLoaded:%d\
"), Target->GetFlags()
, Target->HasAnyFlags(EObjectFlags::RF_Public)
, Target->HasAnyFlags(EObjectFlags::RF_Standalone)
, Target->HasAnyFlags(EObjectFlags::RF_MarkAsNative)
, Target->HasAnyFlags(EObjectFlags::RF_Transactional)
, Target->HasAnyFlags(EObjectFlags::RF_ClassDefaultObject)
, Target->HasAnyFlags(EObjectFlags::RF_ArchetypeObject)
, Target->HasAnyFlags(EObjectFlags::RF_Transient)
, Target->HasAnyFlags(EObjectFlags::RF_MarkAsRootSet)
, Target->HasAnyFlags(EObjectFlags::RF_TagGarbageTemp)
, Target->HasAnyFlags(EObjectFlags::RF_NeedInitialization)
, Target->HasAnyFlags(EObjectFlags::RF_NeedLoad)
, Target->HasAnyFlags(EObjectFlags::RF_KeepForCooker)
, Target->HasAnyFlags(EObjectFlags::RF_NeedPostLoad)
, Target->HasAnyFlags(EObjectFlags::RF_NeedPostLoadSubobjects)
, Target->HasAnyFlags(EObjectFlags::RF_NewerVersionExists)
, Target->HasAnyFlags(EObjectFlags::RF_BeginDestroyed)
, Target->HasAnyFlags(EObjectFlags::RF_FinishDestroyed)
, Target->HasAnyFlags(EObjectFlags::RF_BeingRegenerated)
, Target->HasAnyFlags(EObjectFlags::RF_DefaultSubObject)
, Target->HasAnyFlags(EObjectFlags::RF_WasLoaded)
, Target->HasAnyFlags(EObjectFlags::RF_TextExportTransient)
, Target->HasAnyFlags(EObjectFlags::RF_LoadCompleted)
, Target->HasAnyFlags(EObjectFlags::RF_InheritableComponentTemplate)
, Target->HasAnyFlags(EObjectFlags::RF_DuplicateTransient)
, Target->HasAnyFlags(EObjectFlags::RF_StrongRefOnFrame)
, Target->HasAnyFlags(EObjectFlags::RF_NonPIEDuplicateTransient)
, Target->HasAnyFlags(EObjectFlags::RF_Dynamic)
, Target->HasAnyFlags(EObjectFlags::RF_WillBeLoaded)
);
}

void LGUIEditorTools::FocusToScreenSpaceUI()
{
	if (!GWorld)return;
	if (!GEditor)return;
	if (auto activeViewport = GEditor->GetActiveViewport())
	{
		if (auto viewportClient = activeViewport->GetClient())
		{
			auto editorViewportClient = (FEditorViewportClient*)viewportClient;
			for (TActorIterator<AUIContainerActor> ActorItr(GWorld); ActorItr; ++ActorItr)
			{
				auto canvasScaler = ActorItr->FindComponentByClass<ULGUICanvasScaler>();
				if (canvasScaler != nullptr)
				{
					auto canvas = ActorItr->FindComponentByClass<ULGUICanvas>();
					if (canvas != nullptr && canvas->IsRenderToScreenSpace())//make sure is screen space UI root
					{
						auto viewDistance = FVector::Distance(canvas->GetViewLocation(), canvas->GetUIItem()->GetComponentLocation());
						auto halfViewWidth = viewDistance * FMath::Tan(FMath::DegreesToRadians(canvasScaler->GetFovAngle() * 0.5f));
						auto editorViewDistance = halfViewWidth / FMath::Tan(FMath::DegreesToRadians(editorViewportClient->FOVAngle * 0.5f));
						editorViewportClient->SetViewLocation(canvas->GetUIItem()->GetComponentLocation() - canvas->GetViewRotator().Quaternion().GetForwardVector() * editorViewDistance);
						editorViewportClient->SetViewRotation(canvas->GetViewRotator());
						editorViewportClient->SetLookAtLocation(canvas->GetUIItem()->GetComponentLocation());
						break;
					}
				}
			}
		}
	}
}
void LGUIEditorTools::FocusToSelectedUI()
{
	if (!GEditor)return;
	if (auto activeViewport = GEditor->GetActiveViewport())
	{
		if (auto viewportClient = activeViewport->GetClient())
		{
			auto editorViewportClient = (FEditorViewportClient*)viewportClient;
			auto selectedActor = GetFirstSelectedActor();
			if (auto selectedUIItem = Cast<AUIBaseActor>(selectedActor))
			{
				if (auto renderCavnas = selectedUIItem->GetUIItem()->GetRenderCanvas())
				{
					if (auto canvas = renderCavnas->GetRootCanvas())
					{
						if (canvas != nullptr)
						{
							editorViewportClient->SetViewLocation(canvas->GetViewLocation());
							editorViewportClient->SetViewRotation(canvas->GetViewRotator());
							editorViewportClient->SetLookAtLocation(canvas->GetUIItem()->GetComponentLocation());
						}
					}
				}
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE