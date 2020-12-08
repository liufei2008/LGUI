﻿// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Window/LGUIEditorTools.h"
#include "LGUI.h"
#include "Widgets/Docking/SDockTab.h"
#include "Misc/FileHelper.h"
#include "LGUIBPLibrary.h"
#include "Core/Actor/UIContainerActor.h"
#include "Core/Actor/UISpriteActor.h"
#include "Core/Actor/UITextActor.h"
#include "Core/Actor/UITextureActor.h"
#include "Extensions/UISector.h"
#include "Extensions/UIRing.h"
#include "LGUIEditorStyle.h"
#include "LGUIEditorModule.h"
#include "PropertyCustomizationHelpers.h"
#include "PrefabSystem/ActorSerializer.h"
#include "PrefabSystem/LGUIPrefab.h"
#include "PrefabSystem/ActorReplaceTool.h"
#include "DesktopPlatformModule.h"
#include "AssetRegistryModule.h"
#include "Toolkits/AssetEditorManager.h"
#include "Core/ActorComponent/UIItem.h"
#include "Core/ActorComponent/LGUICanvas.h"

#define LOCTEXT_NAMESPACE "LGUIEditorTools"

SLGUIEditorTools::SLGUIEditorTools()
{

}
struct EditorToolsHelperFunctionHolder
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
		for (TActorIterator<AActor> ActorItr(srcActor->GetWorld()); ActorItr; ++ActorItr)
		{
			if (AActor* itemActor = *ActorItr)
			{
				if (IsValid(itemActor))
				{
					sameLevelActorList.Add(itemActor);
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

#if WITH_EDITOR
ULGUIEditorToolsAgentObject* ULGUIEditorToolsAgentObject::EditorToolsAgentObject;

UWorld* ULGUIEditorToolsAgentObject::GetWorldFromSelection()
{
	if (auto selectedActor = GetFirstSelectedActor())
	{
		return selectedActor->GetWorld();
	}
	return GWorld;
}
void ULGUIEditorToolsAgentObject::CreateUIItemActor(UClass* ActorClass)
{
	GEditor->BeginTransaction(FText::FromString(TEXT("LGUI Create UI Element")));
	auto selectedActor = GetFirstSelectedActor();
	AActor* newActor = nullptr;
	newActor = GetWorldFromSelection()->SpawnActor<AActor>(ActorClass, FTransform::Identity, FActorSpawnParameters());
	if (selectedActor != nullptr)
	{
		newActor->AttachToActor(selectedActor, FAttachmentTransformRules::KeepRelativeTransform);
		GEditor->SelectActor(selectedActor, false, true);
	}
	GEditor->SelectActor(newActor, true, true);
	GEditor->EndTransaction();
}
void ULGUIEditorToolsAgentObject::CreateEmptyActor()
{
	GEditor->BeginTransaction(FText::FromString(TEXT("LGUI Create UI Element")));
	auto selectedActor = GetFirstSelectedActor();
	AActor* newActor = nullptr;
	newActor = GetWorldFromSelection()->SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, FActorSpawnParameters());
	//create SceneComponent
	{
		USceneComponent* RootComponent = NewObject<USceneComponent>(newActor, USceneComponent::GetDefaultSceneRootVariableName(), RF_Transactional);
		RootComponent->Mobility = EComponentMobility::Movable;
		RootComponent->bVisualizeComponent = false;

		newActor->SetRootComponent(RootComponent);
		newActor->AddInstanceComponent(RootComponent);

		RootComponent->RegisterComponent();
	}
	if (selectedActor != nullptr)
	{
		newActor->AttachToActor(selectedActor, FAttachmentTransformRules::KeepRelativeTransform);
		GEditor->SelectActor(selectedActor, false, true);
	}
	GEditor->SelectActor(newActor, true, true);
	GEditor->EndTransaction();
}

AActor* ULGUIEditorToolsAgentObject::GetFirstSelectedActor()
{
	auto selectedActors = EditorToolsHelperFunctionHolder::ConvertSelectionToActors(GEditor->GetSelectedActors());
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
void ULGUIEditorToolsAgentObject::CreateUIControls(FString InPrefabPath)
{
	GEditor->BeginTransaction(LOCTEXT("CreateUIControl", "LGUI Create UI Control"));
	auto selectedActor = GetFirstSelectedActor();
	auto prefab = LoadObject<ULGUIPrefab>(NULL, *InPrefabPath);
	if (prefab)
	{
		auto actor = ActorSerializer::LoadPrefabForEdit(GetWorldFromSelection(), prefab
			, selectedActor == nullptr ? nullptr : selectedActor->GetRootComponent());
		GEditor->SelectActor(selectedActor, false, true);
		GEditor->SelectActor(actor, true, true);
	}
	else
	{
		UE_LOG(LGUIEditor, Error, TEXT("[LGUIEditorToolsAgentObject::CreateUIControls]Load control prefab error! Path:%s. Missing some content of LGUI plugin, reinstall this plugin may fix the issure."), *InPrefabPath);
	}
	GEditor->EndTransaction();
}
void ULGUIEditorToolsAgentObject::ReplaceUIElementWith(UClass* ActorClass)
{
	auto selectedActors = EditorToolsHelperFunctionHolder::ConvertSelectionToActors(GEditor->GetSelectedActors());
	auto count = selectedActors.Num();
	if (count == 0)
	{
		UE_LOG(LGUIEditor, Error, TEXT("NothingSelected"));
		return;
	}
	auto rootActorList = EditorToolsHelperFunctionHolder::GetRootActorListFromSelection(selectedActors);

	GEditor->BeginTransaction(LOCTEXT("ReplaceUIElement", "LGUI Replace UI Element"));
	GEditor->SelectNone(true, true);
	for (auto item : rootActorList)
	{
		auto newActor = ActorReplaceTool::ReplaceActorClass(item, ActorClass);
		GEditor->SelectActor(newActor, true, true);
	}
	GEditor->EndTransaction();
}
void ULGUIEditorToolsAgentObject::DuplicateSelectedActors_Impl()
{
	auto selectedActors = EditorToolsHelperFunctionHolder::ConvertSelectionToActors(GEditor->GetSelectedActors());
	auto count = selectedActors.Num();
	if (count == 0)
	{
		UE_LOG(LGUIEditor, Error, TEXT("NothingSelected"));
		return;
	}
	auto rootActorList = EditorToolsHelperFunctionHolder::GetRootActorListFromSelection(selectedActors);
	GEditor->BeginTransaction(LOCTEXT("DuplicateActor", "LGUI Duplicate Actors"));
	for (auto item : rootActorList)
	{
		auto copiedActorLabel = EditorToolsHelperFunctionHolder::GetCopiedActorLabel(item);
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
}
void ULGUIEditorToolsAgentObject::CopySelectedActors_Impl()
{
	auto selectedActors = EditorToolsHelperFunctionHolder::ConvertSelectionToActors(GEditor->GetSelectedActors());
	auto count = selectedActors.Num();
	if (count == 0)
	{
		UE_LOG(LGUIEditor, Error, TEXT("NothingSelected"));
		return;
	}
	auto copiedActorList = EditorToolsHelperFunctionHolder::GetRootActorListFromSelection(selectedActors);
	GetInstance()->copiedActorPrefabList.Reset();
	for (auto copiedActor : copiedActorList)
	{
		auto prefab = NewObject<ULGUIPrefab>();
		ActorSerializer::SavePrefab(copiedActor, prefab);
		GetInstance()->copiedActorPrefabList.Add(prefab);
	}
}
void ULGUIEditorToolsAgentObject::PasteSelectedActors_Impl()
{
	auto selectedActors = EditorToolsHelperFunctionHolder::ConvertSelectionToActors(GEditor->GetSelectedActors());
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
	for (auto prefab : GetInstance()->copiedActorPrefabList)
	{
		if (IsValid(prefab))
		{
			auto copiedActor = ActorSerializer::LoadPrefabForEdit(GetWorldFromSelection(), prefab, parentComp);
			auto copiedActorLabel = EditorToolsHelperFunctionHolder::GetCopiedActorLabel(copiedActor);
			copiedActor->SetActorLabel(copiedActorLabel);
			GEditor->SelectActor(copiedActor, true, true);
		}
		else
		{
			UE_LOG(LGUIEditor, Error, TEXT("Source copied actor is missing!"));
		}
	}
	GEditor->EndTransaction();
}
void ULGUIEditorToolsAgentObject::DeleteSelectedActors_Impl()
{
	auto selectedActors = EditorToolsHelperFunctionHolder::ConvertSelectionToActors(GEditor->GetSelectedActors());
	auto count = selectedActors.Num();
	if (count == 0)
	{
		UE_LOG(LGUIEditor, Error, TEXT("NothingSelected"));
		return;
	}
	auto rootActorList = EditorToolsHelperFunctionHolder::GetRootActorListFromSelection(selectedActors);
	GEditor->BeginTransaction(LOCTEXT("DeleteActor", "LGUI Delete Actor"));
	for (auto item : rootActorList)
	{
		bool shouldDeletePrefab = false;
		auto prefabActor = ULGUIEditorToolsAgentObject::GetPrefabActor_WhichManageThisActor(item);
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
		ULGUIBPLibrary::DeleteActor(item);
		if (shouldDeletePrefab)
		{
			ULGUIBPLibrary::DeleteActor(prefabActor, false);
		}
	}
	GEditor->EndTransaction();
}
void ULGUIEditorToolsAgentObject::CopyComponentValues_Impl()
{
	auto selectedComponents = EditorToolsHelperFunctionHolder::ConvertSelectionToComponents(GEditor->GetSelectedComponents());
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
	GetInstance()->copiedComponent = selectedComponents[0];
}
void ULGUIEditorToolsAgentObject::PasteComponentValues_Impl()
{
	auto selectedComponents = EditorToolsHelperFunctionHolder::ConvertSelectionToComponents(GEditor->GetSelectedComponents());
	auto count = selectedComponents.Num();
	if (count == 0)
	{
		UE_LOG(LGUIEditor, Error, TEXT("NothingSelected"));
		return;
	}
	if (GetInstance()->copiedComponent.IsValid())
	{
		GEditor->BeginTransaction(LOCTEXT("PasteComponentValues", "LGUI Paste Component Proeprties"));
		for (UActorComponent* item : selectedComponents)
		{
			ActorCopier::CopyComponentValue(GetInstance()->copiedComponent.Get(), item);
		}
		GEditor->EndTransaction();
	}
	else
	{
		UE_LOG(LGUIEditor, Error, TEXT("Selected component is missing!"));
	}
}
void ULGUIEditorToolsAgentObject::OpenAtlasViewer_Impl()
{
	FGlobalTabmanager::Get()->InvokeTab(FLGUIEditorModule::LGUIAtlasViewerName);
}
void ULGUIEditorToolsAgentObject::ChangeTraceChannel_Impl(ETraceTypeQuery InTraceTypeQuery)
{
	auto selectedActors = EditorToolsHelperFunctionHolder::ConvertSelectionToActors(GEditor->GetSelectedActors());
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
	auto rootActorList = EditorToolsHelperFunctionHolder::GetRootActorListFromSelection(selectedActors);
	GEditor->BeginTransaction(LOCTEXT("ChangeTraceChannel", "LGUI Change Trace Channel"));
	for (auto item : rootActorList)
	{
		FunctionContainer::ChangeTraceChannel(item->GetRootComponent(), InTraceTypeQuery);
	}
	GEditor->EndTransaction();
}
void ULGUIEditorToolsAgentObject::CreateScreenSpaceUIBasicSetup()
{
	GEditor->BeginTransaction(FText::FromString(TEXT("LGUI Create Screen Space UI")));
	auto selectedActor = GetFirstSelectedActor();
	FString prefabPath(TEXT("/LGUI/Prefabs/ScreenSpaceUI"));
	auto prefab = LoadObject<ULGUIPrefab>(NULL, *prefabPath);
	if (prefab)
	{
		auto actor = ActorSerializer::LoadPrefabForEdit(GetWorldFromSelection(), prefab, nullptr, true);
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
}
void ULGUIEditorToolsAgentObject::CreateWorldSpaceUIBasicSetup()
{
	GEditor->BeginTransaction(FText::FromString(TEXT("LGUI Create World Space UI")));
	auto selectedActor = GetFirstSelectedActor();
	FString prefabPath(TEXT("/LGUI/Prefabs/WorldSpaceUI"));
	auto prefab = LoadObject<ULGUIPrefab>(NULL, *prefabPath);
	if (prefab)
	{
		auto actor = ActorSerializer::LoadPrefabForEdit(GetWorldFromSelection(), prefab, nullptr, true);
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
}
bool ULGUIEditorToolsAgentObject::HaveValidCopiedActors()
{
	if (GetInstance()->copiedActorPrefabList.Num() == 0)return false;
	for (auto item : GetInstance()->copiedActorPrefabList)
	{
		if (!IsValid(item))
		{
			return false;
		}
	}
	return true;
}
bool ULGUIEditorToolsAgentObject::HaveValidCopiedComponent()
{
	return GetInstance()->copiedComponent.IsValid();
}

ULGUIEditorToolsAgentObject* ULGUIEditorToolsAgentObject::GetInstance()
{
	if (EditorToolsAgentObject == nullptr)
	{
		EditorToolsAgentObject = NewObject<ULGUIEditorToolsAgentObject>();
		EditorToolsAgentObject->AddToRoot();
	}
	return EditorToolsAgentObject;
}
void ULGUIEditorToolsAgentObject::CreatePrefabAsset()
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
				UPackage::SavePackage(package, OutPrefab, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *packageSavePath);

				auto prefabActor = selectedActor->GetWorld()->SpawnActorDeferred<ALGUIPrefabActor>(ALGUIPrefabActor::StaticClass(), FTransform::Identity);
				prefabActor->SetActorLabel(fileName);
				auto prefabComp = prefabActor->GetPrefabComponent();
				prefabComp->PrefabAsset = OutPrefab;
				prefabComp->LoadedRootActor = selectedActor;
				LGUIUtils::CollectChildrenActors(selectedActor, prefabComp->AllLoadedActorArray);
				prefabActor->FinishSpawning(FTransform::Identity, true);
				prefabComp->SavePrefab();
			}
			else
			{
				FMessageDialog::Open(EAppMsgType::Ok
					, FText::FromString(FString::Printf(TEXT("Prefab should only save inside Content folder!"))));
			}
		}
	}
}
void ULGUIEditorToolsAgentObject::ApplyPrefab()
{
	GEditor->BeginTransaction(FText::FromString(TEXT("LGUI ApplyPrefab")));
	auto selectedActor = GetFirstSelectedActor();
	auto prefabActor = ULGUIEditorToolsAgentObject::GetPrefabActor_WhichManageThisActor(selectedActor);
	if (prefabActor != nullptr)
	{
		prefabActor->GetPrefabComponent()->SavePrefab();
	}
	GEditor->EndTransaction();
}
void ULGUIEditorToolsAgentObject::RevertPrefab()
{
	GEditor->BeginTransaction(FText::FromString(TEXT("LGUI RevertPrefab")));
	auto selectedActor = GetFirstSelectedActor();
	auto prefabActor = ULGUIEditorToolsAgentObject::GetPrefabActor_WhichManageThisActor(selectedActor);
	if (prefabActor != nullptr)
	{
		prefabActor->GetPrefabComponent()->RevertPrefab();
	}
	GEditor->EndTransaction();
}
void ULGUIEditorToolsAgentObject::DeletePrefab()
{
	GEditor->BeginTransaction(FText::FromString(TEXT("LGUI DeletePrefab")));
	auto selectedActor = GetFirstSelectedActor();
	auto prefabActor = ULGUIEditorToolsAgentObject::GetPrefabActor_WhichManageThisActor(selectedActor);
	if (prefabActor != nullptr)
	{
		prefabActor->GetPrefabComponent()->DeleteThisInstance();
	}
	GEditor->EndTransaction();
}
void ULGUIEditorToolsAgentObject::SelectPrefabAsset()
{
	GEditor->BeginTransaction(FText::FromString(TEXT("LGUI SelectPrefabAsset")));
	auto selectedActor = GetFirstSelectedActor();
	auto prefabActor = ULGUIEditorToolsAgentObject::GetPrefabActor_WhichManageThisActor(selectedActor);
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
ALGUIPrefabActor* ULGUIEditorToolsAgentObject::GetPrefabActor_WhichManageThisActor(AActor* InActor)
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
void ULGUIEditorToolsAgentObject::SaveAsset(UObject* InObject, UPackage* InPackage)
{
	FAssetRegistryModule::AssetCreated(InObject);
	FString packageSavePath = FString::Printf(TEXT("/Game/%s%s"), *(InObject->GetPathName()), *FPackageName::GetAssetPackageExtension());
	UPackage::SavePackage(InPackage, InObject, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *packageSavePath);
	InPackage->MarkPackageDirty();
}
bool ULGUIEditorToolsAgentObject::IsCanvasActor(AActor* InActor)
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
int ULGUIEditorToolsAgentObject::GetCanvasDrawcallCount(AActor* InActor)
{
	if (auto rootComp = InActor->GetRootComponent())
	{
		if (auto rootUIItem = Cast<UUIItem>(rootComp))
		{
			if (rootUIItem->IsCanvasUIItem())
			{
				return rootUIItem->GetRenderCanvas()->GetDrawcallCount();
			}
		}
	}
	return 0;
}
FString ULGUIEditorToolsAgentObject::PrintObjectFlags(UObject* Target)
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
#endif

void SLGUIEditorTools::Construct(const FArguments& InArgs, TSharedPtr<SDockTab> InOwnerTab)
{
	FPropertyEditorModule& EditModule = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	{
		DetailsViewArgs.bAllowSearch = false;
		DetailsViewArgs.bShowOptions = false;
		DetailsViewArgs.bAllowMultipleTopLevelObjects = false;
		DetailsViewArgs.bAllowFavoriteSystem = false;
		DetailsViewArgs.bShowActorLabel = false;
		DetailsViewArgs.bHideSelectionTip = true;
	}
	TSharedPtr<IDetailsView> DescriptorDetailView = EditModule.CreateDetailView(DetailsViewArgs);
	DescriptorDetailView->SetObject(ULGUIEditorToolsAgentObject::GetInstance());

	ChildSlot
		[
			DescriptorDetailView.ToSharedRef()
		];
}
#undef LOCTEXT_NAMESPACE