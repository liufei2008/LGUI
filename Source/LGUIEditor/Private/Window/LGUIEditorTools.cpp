// Copyright 2019 LexLiu. All Rights Reserved.

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
void ULGUIEditorToolsAgentObject::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (UIContainer.IsClicked())CreateUIItemActor<AUIContainerActor>();
	if (UISprite.IsClicked())CreateUIItemActor<AUISpriteActor>();
	if (UIText.IsClicked())CreateUIItemActor<AUITextActor>();
	if (UITexture.IsClicked())CreateUIItemActor<AUITextureActor>();

	if (Button.IsClicked())CreateUIControls(TEXT("/LGUI/Prefabs/DefaultButton"));
	if (Toggle.IsClicked())CreateUIControls(TEXT("/LGUI/Prefabs/DefaultToggle"));
	if (HorizontalSlider.IsClicked())CreateUIControls(TEXT("/LGUI/Prefabs/DefaultHorizontalSlider"));
	if (VerticalSlider.IsClicked())CreateUIControls(TEXT("/LGUI/Prefabs/DefaultVerticalSlider"));
	if (HorizontalScrollbar.IsClicked())CreateUIControls(TEXT("/LGUI/Prefabs/DefaultHorizontalScrollbar"));
	if (VerticalScrollbar.IsClicked())CreateUIControls(TEXT("/LGUI/Prefabs/DefaultVerticalScrollbar"));
	if (FlyoutMenuButton.IsClicked())CreateUIControls(TEXT("/LGUI/Prefabs/DefaultFlyoutMenuButton"));
	if (ComboBoxButton.IsClicked())CreateUIControls(TEXT("/LGUI/Prefabs/DefaultComboBoxButton"));
	if (TextInput.IsClicked())CreateUIControls(TEXT("/LGUI/Prefabs/DefaultTextInput"));
	if (ScrollView.IsClicked())CreateUIControls(TEXT("/LGUI/Prefabs/DefaultScrollView"));

	if (CopySelectedActors.IsClicked())CopySelectedActors_Impl();
	if (PasteSelectedActors.IsClicked())PasteSelectedActors_Impl();
	if (DuplicateSelectedActors.IsClicked())DuplicateSelectedActors_Impl();
	if (DeleteSelectedActors.IsClicked())DeleteSelectedActors_Impl();

	if (CopyComponentValues.IsClicked())CopyComponentValues_Impl();
	if (PasteComponentValues.IsClicked())PasteComponentValues_Impl();

	if (AtlasViewer.IsClicked())OpenAtlasViewer_Impl();

	GEditor->RedrawAllViewports();
}

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
		auto actor = ULGUIBPLibrary::LoadPrefab(GetWorldFromSelection(), prefab
			, selectedActor == nullptr ? nullptr : selectedActor->GetRootComponent(), true);
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
		if (auto parentActor = selectedActors[0]->GetAttachParentActor())
		{
			parentComp = parentActor->GetRootComponent();
		}
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
		ULGUIBPLibrary::DeleteActor(item);
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
	if (IsValid(GetInstance()->copiedComponent))
	{
		GEditor->BeginTransaction(LOCTEXT("PasteComponentValues", "LGUI Paste Component Proeprties"));
		for (UActorComponent* item : selectedComponents)
		{
			ActorCopier::CopyComponentValue(GetInstance()->copiedComponent, item);
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
		auto actor = ULGUIBPLibrary::LoadPrefab(GetWorldFromSelection(), prefab, nullptr, true);
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
		auto actor = ULGUIBPLibrary::LoadPrefab(GetWorldFromSelection(), prefab, nullptr, true);
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
	return IsValid(GetInstance()->copiedComponent);
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
			auto selectedFilePath = OutFileNames[0];
			if (selectedFilePath.StartsWith(FPaths::ProjectContentDir()))
			{
				selectedFilePath.RemoveFromStart(FPaths::ProjectContentDir(), ESearchCase::CaseSensitive);
				FString packageName = TEXT("/Game/") + selectedFilePath;
				UPackage* package = CreatePackage(NULL, *packageName);
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
				UE_LOG(LGUIEditor, Error, TEXT("Prefab is a uasset, should only save inside Content folder!"));
			}
		}
	}
}
void ULGUIEditorToolsAgentObject::ApplyPrefab()
{
	auto selectedActor = GetFirstSelectedActor();
	auto prefabActor = ULGUIEditorToolsAgentObject::GetPrefabActor_WhichManageThisActor(selectedActor);
	if (prefabActor != nullptr)
	{
		prefabActor->GetPrefabComponent()->SavePrefab();
	}
}
void ULGUIEditorToolsAgentObject::RevertPrefab()
{
	auto selectedActor = GetFirstSelectedActor();
	auto prefabActor = ULGUIEditorToolsAgentObject::GetPrefabActor_WhichManageThisActor(selectedActor);
	if (prefabActor != nullptr)
	{
		prefabActor->GetPrefabComponent()->RevertPrefab();
	}
}
void ULGUIEditorToolsAgentObject::DeletePrefab()
{
	auto selectedActor = GetFirstSelectedActor();
	auto prefabActor = ULGUIEditorToolsAgentObject::GetPrefabActor_WhichManageThisActor(selectedActor);
	if (prefabActor != nullptr)
	{
		prefabActor->GetPrefabComponent()->DeleteThisInstance();
	}
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