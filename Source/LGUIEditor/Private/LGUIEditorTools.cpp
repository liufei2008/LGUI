// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "LGUIEditorTools.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Misc/FileHelper.h"
#include "Misc/MessageDialog.h"
#include "PropertyCustomizationHelpers.h"
#include "DesktopPlatformModule.h"
#include "AssetRegistryModule.h"
#include "Engine/EngineTypes.h"
#include "Kismet2/ComponentEditorUtils.h"
#include "Widgets/SViewport.h"
#include "Layout/LGUICanvasScaler.h"
#include "EditorViewportClient.h"
#include "Engine/Selection.h"
#include "EngineUtils.h"
#include "PrefabSystem/2/ActorCopier.h"
#include "PrefabSystem/2/ActorReplaceTool.h"
#include "PrefabSystem/LGUIPrefabHelperActor.h"
#include "PrefabSystem/LGUIPrefabHelperObject.h"
#include "LGUIEditorModule.h"
#include "PrefabEditor/LGUIPrefabEditor.h"

#define LOCTEXT_NAMESPACE "LGUIEditorTools"

PRAGMA_DISABLE_OPTIMIZATION
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
	static FString GetLabelPrefixForCopy(const FString& srcActorLabel, FString& outNumetricSuffix)
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
		srcActorLabel = GetLabelPrefixForCopy(srcActorLabel, maxNumetricSuffixStr);
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

FString LGUIEditorTools::LGUIPresetPrefabPath = TEXT("/LGUI/Prefabs/");

FString LGUIEditorTools::GetUniqueNumetricName(const FString& InPrefix, const TArray<FString>& InExistNames)
{
	auto ExtractNumetric = [](const FString& InString, int32& Num) {
		int NumetricStringIndex = -1;
		FString SubNumetricString;
		int NumetricStringCharCount = 0;
		for (int i = InString.Len() - 1; i >= 0; i--)
		{
			auto SubChar = InString[i];
			if (SubChar >= '0' && SubChar <= '9')
			{
				NumetricStringIndex = i;

				NumetricStringCharCount++;
				if (NumetricStringCharCount >= 4)
				{
					break;
				}
			}
			else
			{
				break;
			}
		}
		if (NumetricStringIndex != -1)
		{
			auto NumetricSubString = InString.Right(InString.Len() - NumetricStringIndex);
			Num = FCString::Atoi(*NumetricSubString);
			return true;
		}
		else
		{
			return false;
		}
	};
	int MaxNumSuffix = 0;
	for (int i = 0; i < InExistNames.Num(); i++)//search from same level actors, and get the right suffix
	{
		auto& Item = InExistNames[i];
		if (Item.Len() == 0)continue;
		int Num;
		if (ExtractNumetric(Item, Num))
		{
			if (Num > MaxNumSuffix)
			{
				MaxNumSuffix = Num;
			}
		}
	}
	return FString::Printf(TEXT("%s_%d"), *InPrefix, MaxNumSuffix + 1);
}

TArray<AActor*> LGUIEditorTools::GetRootActorListFromSelection(const TArray<AActor*>& selectedActors)
{
	TArray<AActor*> RootActorList;
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
			RootActorList.Add(obj);
		}
	}
	return RootActorList;
}
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

		SetTraceChannelToParent(newActor);
	}
	GEditor->EndTransaction();
}
void LGUIEditorTools::CreateEmptyActor()
{
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
	GEditor->BeginTransaction(LOCTEXT("CreateUIControl", "LGUI Create UI Control"));
	auto selectedActor = GetFirstSelectedActor();
	MakeCurrentLevel(selectedActor);
	auto prefab = LoadObject<ULGUIPrefab>(NULL, *InPrefabPath);
	if (prefab)
	{
		auto actor = prefab->LoadPrefabInEditor(GetWorldFromSelection()
			, selectedActor == nullptr ? nullptr : selectedActor->GetRootComponent());
		GEditor->SelectActor(selectedActor, false, true);
		GEditor->SelectActor(actor, true, true);

		SetTraceChannelToParent_Recursive(actor);
	}
	else
	{
		UE_LOG(LGUIEditor, Error, TEXT("[LGUIEditorToolsAgentObject::CreateUIControls]Load control prefab error! Path:%s. Missing some content of LGUI plugin, reinstall this plugin may fix the issure."), *InPrefabPath);
	}
	GEditor->EndTransaction();
}
void LGUIEditorTools::ReplaceUIElementWith(UClass* ActorClass)
{
	auto selectedActors = LGUIEditorToolsHelperFunctionHolder::ConvertSelectionToActors(GEditor->GetSelectedActors());
	auto count = selectedActors.Num();
	if (count == 0)
	{
		UE_LOG(LGUIEditor, Error, TEXT("NothingSelected"));
		return;
	}
	auto RootActorList = LGUIEditorTools::GetRootActorListFromSelection(selectedActors);

	GEditor->BeginTransaction(LOCTEXT("ReplaceUIElement", "LGUI Replace UI Element"));
	GEditor->SelectNone(true, true);
	for (auto item : RootActorList)
	{
		MakeCurrentLevel(item);
		auto newActor = LGUIPrefabSystem::ActorReplaceTool::ReplaceActorClass(item, ActorClass);//@todo: use buildin replace tool
		GEditor->SelectActor(newActor, true, true);
	}
	GEditor->EndTransaction();
}
void LGUIEditorTools::DuplicateSelectedActors_Impl()
{
	auto selectedActors = LGUIEditorToolsHelperFunctionHolder::ConvertSelectionToActors(GEditor->GetSelectedActors());
	auto count = selectedActors.Num();
	if (count == 0)
	{
		UE_LOG(LGUIEditor, Error, TEXT("NothingSelected"));
		return;
	}
	auto RootActorList = LGUIEditorTools::GetRootActorListFromSelection(selectedActors);
	GEditor->BeginTransaction(LOCTEXT("DuplicateActor", "LGUI Duplicate Actors"));
	for (auto item : RootActorList)
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
	auto copiedActorList = LGUIEditorTools::GetRootActorListFromSelection(selectedActors);
	copiedActorPrefabList.Reset();
	for (auto copiedActor : copiedActorList)
	{
		auto prefab = NewObject<ULGUIPrefab>();
		prefab->AddToRoot();
		TMap<UObject*, FGuid> InOutMapObjectToGuid;
		TMap<AActor*, FLGUISubPrefabData> InSubPrefabMap;
		prefab->SavePrefab(copiedActor, InOutMapObjectToGuid, InSubPrefabMap);
		copiedActorPrefabList.Add(prefab);
	}
}
void LGUIEditorTools::PasteSelectedActors_Impl()
{
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
			auto copiedActor = prefab->LoadPrefabInEditor(GetWorldFromSelection(), parentComp, false);
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
}
void LGUIEditorTools::DeleteSelectedActors_Impl()
{
	auto selectedActors = LGUIEditorToolsHelperFunctionHolder::ConvertSelectionToActors(GEditor->GetSelectedActors());
	DeleteActors_Impl(selectedActors);
}

void LGUIEditorTools::DeleteActors_Impl(const TArray<AActor*>& InActors)
{
	auto count = InActors.Num();
	if (count == 0)
	{
		UE_LOG(LGUIEditor, Error, TEXT("NothingSelected"));
		return;
	}
	auto confirmMsg = FString::Printf(TEXT("Destroy selected actors? This will also destroy the children attached to selected actors."));
	auto confirmResult = FMessageDialog::Open(EAppMsgType::YesNo, FText::FromString(confirmMsg));
	if (confirmResult == EAppReturnType::No)return;

	auto RootActorList = LGUIEditorTools::GetRootActorListFromSelection(InActors);
	GEditor->BeginTransaction(LOCTEXT("DestroyActor", "LGUI Destroy Actor"));
	GEditor->GetSelectedActors()->DeselectAll();
	for (auto Actor : RootActorList)
	{
		bool shouldDeletePrefab = false;
		auto PrefabHelperObject = LGUIEditorTools::GetPrefabHelperObject_WhichManageThisActor(Actor);
		if (PrefabHelperObject != nullptr)
		{
			if (!PrefabHelperObject->bIsInsidePrefabEditor)//prefab editor should handle delete by itself
			{
				if (PrefabHelperObject->LoadedRootActor == Actor)
				{
					auto OwnerActor = Cast<ALGUIPrefabHelperActor>(PrefabHelperObject->GetOuter());
					check(OwnerActor != nullptr);
					LGUIUtils::DestroyActorWithHierarchy(OwnerActor);
				}
			}
		}
		LGUIUtils::DestroyActorWithHierarchy(Actor);
	}
	CleanupPrefabsInWorld(RootActorList[0]->GetWorld());
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
			LGUIPrefabSystem::ActorCopier::CopyComponentValue(copiedComponent.Get(), item);//@todo: use buildin copy tool
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
	FGlobalTabmanager::Get()->TryInvokeTab(FLGUIEditorModule::LGUIAtlasViewerName);
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
				auto& children = InSceneComp->GetAttachChildren();
				for (auto itemComp : children)
				{
					ChangeTraceChannel(itemComp, InChannel);
				}
			}
		}
	};
	auto RootActorList = LGUIEditorTools::GetRootActorListFromSelection(selectedActors);
	GEditor->BeginTransaction(LOCTEXT("ChangeTraceChannel", "LGUI Change Trace Channel"));
	for (auto item : RootActorList)
	{
		FunctionContainer::ChangeTraceChannel(item->GetRootComponent(), InTraceTypeQuery);
	}
	GEditor->EndTransaction();
}
void LGUIEditorTools::CreateScreenSpaceUI_BasicSetup()
{
	GEditor->BeginTransaction(FText::FromString(TEXT("LGUI Create Screen Space UI")));
	auto selectedActor = GetFirstSelectedActor();
	FString prefabPath(TEXT("/LGUI/Prefabs/ScreenSpaceUI"));
	auto prefab = LoadObject<ULGUIPrefab>(NULL, *prefabPath);
	if (prefab)
	{
		auto actor = prefab->LoadPrefabInEditor(GetWorldFromSelection(), nullptr, true);
		actor->GetRootComponent()->SetRelativeScale3D(FVector::OneVector);
		actor->GetRootComponent()->SetRelativeLocation(FVector(0, 0, 250));
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
				UE_LOG(LGUIEditor, Error, TEXT("[ULGUIEditorToolsAgentObject::CreateScreenSpaceUI_BasicSetup]Load PresetEventSystemActor error! Missing some content of LGUI plugin, reinstall this plugin may fix the issure."));
			}
		}
	}
	else
	{
		UE_LOG(LGUIEditor, Error, TEXT("[LGUIEditorToolsAgentObject::CreateScreenSpaceUI_BasicSetup]Load control prefab error! Path:%s. Missing some content of LGUI plugin, reinstall this plugin may fix the issure."), *prefabPath);
	}
	GEditor->EndTransaction();
}
void LGUIEditorTools::CreateWorldSpaceUIUERenderer_BasicSetup()
{
	GEditor->BeginTransaction(FText::FromString(TEXT("LGUI Create World Space UI - UE Renderer")));
	auto selectedActor = GetFirstSelectedActor();
	FString prefabPath(TEXT("/LGUI/Prefabs/WorldSpaceUI_UERenderer"));
	auto prefab = LoadObject<ULGUIPrefab>(NULL, *prefabPath);
	if (prefab)
	{
		auto actor = prefab->LoadPrefabInEditor(GetWorldFromSelection(), nullptr, true);
		actor->GetRootComponent()->SetRelativeLocation(FVector(0, 0, 250));
		actor->GetRootComponent()->SetWorldScale3D(FVector::OneVector);
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
				UE_LOG(LGUIEditor, Error, TEXT("[ULGUIEditorToolsAgentObject::CreateWorldSpaceUIUERenderer_BasicSetup]Load PresetEventSystemActor error! Missing some content of LGUI plugin, reinstall this plugin may fix the issure."));
			}
		}
	}
	else
	{
		UE_LOG(LGUIEditor, Error, TEXT("[LGUIEditorToolsAgentObject::CreateWorldSpaceUIUERenderer_BasicSetup]Load control prefab error! Path:%s. Missing some content of LGUI plugin, reinstall this plugin may fix the issure."), *prefabPath);
	}
	GEditor->EndTransaction();
}
void LGUIEditorTools::CreateWorldSpaceUILGUIRenderer_BasicSetup()
{
	GEditor->BeginTransaction(FText::FromString(TEXT("LGUI Create World Space UI - LGUI Renderer")));
	auto selectedActor = GetFirstSelectedActor();
	FString prefabPath(TEXT("/LGUI/Prefabs/WorldSpaceUI_LGUIRenderer"));
	auto prefab = LoadObject<ULGUIPrefab>(NULL, *prefabPath);
	if (prefab)
	{
		auto actor = prefab->LoadPrefabInEditor(GetWorldFromSelection(), nullptr, true);
		actor->GetRootComponent()->SetRelativeLocation(FVector(0, 0, 250));
		actor->GetRootComponent()->SetWorldScale3D(FVector::OneVector);
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
				UE_LOG(LGUIEditor, Error, TEXT("[ULGUIEditorToolsAgentObject::CreateWorldSpaceUILGUIRenderer_BasicSetup]Load PresetEventSystemActor error! Missing some content of LGUI plugin, reinstall this plugin may fix the issure."));
			}
		}
	}
	else
	{
		UE_LOG(LGUIEditor, Error, TEXT("[LGUIEditorToolsAgentObject::CreateWorldSpaceUILGUIRenderer_BasicSetup]Load control prefab error! Path:%s. Missing some content of LGUI plugin, reinstall this plugin may fix the issure."), *prefabPath);
	}
	GEditor->EndTransaction();
}
void LGUIEditorTools::AttachComponentToSelectedActor(TSubclassOf<UActorComponent> InComponentClass)
{
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

FString LGUIEditorTools::PrevSavePrafabFolder = TEXT("");
void LGUIEditorTools::CreatePrefabAsset()
{
	auto selectedActor = GetFirstSelectedActor();
	if (Cast<ALGUIPrefabHelperActor>(selectedActor) != nullptr)
	{
		FMessageDialog::Open(EAppMsgType::Ok
			, FText::FromString(FString::Printf(TEXT("Cannot create prefab on a PrefabActor!"))));
		return;
	}
	auto oldPrefabActor = GetPrefabHelperObject_WhichManageThisActor(selectedActor);
	if (IsValid(oldPrefabActor) && oldPrefabActor->LoadedRootActor == selectedActor)//If create prefab from an existing prefab's root actor, this is not allowed
	{
		FMessageDialog::Open(EAppMsgType::Ok
			, FText::FromString(FString::Printf(TEXT("This actor is a root actor of another prefab, this is not allowed! Instead you can duplicate the prefab asset."))));
		return;
	}
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform)
	{
		TArray<FString> OutFileNames;
		DesktopPlatform->SaveFileDialog(
			FSlateApplication::Get().FindBestParentWindowHandleForDialogs(FSlateApplication::Get().GetGameViewport()),
			TEXT("Choose a path to save prefab asset, must inside Content folder"),
			PrevSavePrafabFolder.IsEmpty() ? FPaths::ProjectContentDir() : PrevSavePrafabFolder,
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
				PrevSavePrafabFolder = FPaths::GetPath(selectedFilePath);
				if (FPaths::FileExists(selectedFilePath + TEXT(".uasset")))
				{
					auto returnValue = FMessageDialog::Open(EAppMsgType::YesNo, FText::FromString(FString::Printf(TEXT("Asset already exist at path: \"%s\" !\nReplace it?"), *(selectedFilePath))));
					if (returnValue == EAppReturnType::No)
					{
						return;
					}
				}
				selectedFilePath.RemoveFromStart(FPaths::ProjectContentDir(), ESearchCase::CaseSensitive);
				FString packageName = TEXT("/Game/") + selectedFilePath;
				UPackage* package = CreatePackage(*packageName);
				if (package == nullptr)
				{
					FMessageDialog::Open(EAppMsgType::Ok
						, FText::FromString(FString::Printf(TEXT("Selected path not valid, please choose another path to save prefab."))));
					return;
				}
				package->FullyLoad();
				FString fileName = FPaths::GetBaseFilename(selectedFilePath);
				auto OutPrefab = NewObject<ULGUIPrefab>(package, ULGUIPrefab::StaticClass(), *fileName, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone);
				FAssetRegistryModule::AssetCreated(OutPrefab);

				auto otherPrefabObjectWhichHaveThisActor = GetPrefabHelperObject_WhichManageThisActor(selectedActor);//@todo: could be multiple other prefabs
				if (otherPrefabObjectWhichHaveThisActor != nullptr && otherPrefabObjectWhichHaveThisActor->bIsInsidePrefabEditor)//create sub prefab in prefab editor
				{
					auto PrefabHelperObject = NewObject<ULGUIPrefabHelperObject>();
					PrefabHelperObject->PrefabAsset = OutPrefab;
					PrefabHelperObject->LoadedRootActor = selectedActor;
					PrefabHelperObject->SavePrefab();
					OutPrefab->RefreshAgentObjectsInPreviewWorld();
					//remove actors from other prefab.
					if (otherPrefabObjectWhichHaveThisActor != nullptr)
					{
						for (auto actorItem : PrefabHelperObject->AllLoadedActorArray)
						{
							auto foundIndex = otherPrefabObjectWhichHaveThisActor->AllLoadedActorArray.Find(actorItem);
							if (foundIndex != INDEX_NONE)
							{
								otherPrefabObjectWhichHaveThisActor->AllLoadedActorArray.RemoveAt(foundIndex);
							}
						}
					}
					//make it as subprefab
					if (auto PrefabEditor = FLGUIPrefabEditor::GetEditorForPrefabIfValid(otherPrefabObjectWhichHaveThisActor->PrefabAsset))
					{
						PrefabEditor->MakePrefabAsSubPrefab(OutPrefab, selectedActor, PrefabHelperObject->MapGuidToObject);
					}

					PrefabHelperObject->PrefabAsset = nullptr;
					PrefabHelperObject->LoadedRootActor = nullptr;
					PrefabHelperObject->ConditionalBeginDestroy();
				}
				else//create prefab in level editor
				{
					auto prefabActor = selectedActor->GetWorld()->SpawnActorDeferred<ALGUIPrefabHelperActor>(ALGUIPrefabHelperActor::StaticClass(), FTransform::Identity);
					if (IsValid(prefabActor))
					{
						prefabActor->FinishSpawning(FTransform::Identity, true);
						prefabActor->SetActorLabel(fileName);
						auto PrefabHelperObject = prefabActor->PrefabHelperObject;
						PrefabHelperObject->PrefabAsset = OutPrefab;
						PrefabHelperObject->LoadedRootActor = selectedActor;
						PrefabHelperObject->SavePrefab();
						OutPrefab->RefreshAgentObjectsInPreviewWorld();
						prefabActor->MoveActorToPrefabFolder();
						//remove actors from other prefab. @todo: should be multiple other prefabs
						if (otherPrefabObjectWhichHaveThisActor != nullptr)
						{
							for (auto actorItem : PrefabHelperObject->AllLoadedActorArray)
							{
								auto foundIndex = otherPrefabObjectWhichHaveThisActor->AllLoadedActorArray.Find(actorItem);
								if (foundIndex != INDEX_NONE)
								{
									otherPrefabObjectWhichHaveThisActor->AllLoadedActorArray.RemoveAt(foundIndex);
								}
							}
						}
					}
					else
					{
						FMessageDialog::Open(EAppMsgType::Ok
							, FText::FromString(FString::Printf(TEXT("Spawn helper actor failed!"))));
					}
				}
				CleanupPrefabsInWorld(selectedActor->GetWorld());
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
	auto PrefabHelperObject = LGUIEditorTools::GetPrefabHelperObject_WhichManageThisActor(selectedActor);
	if (PrefabHelperObject != nullptr)
	{
		check(!PrefabHelperObject->bIsInsidePrefabEditor);//should already handled by button
		CreateOrApplyPrefab(PrefabHelperObject);
	}
	CleanupPrefabsInWorld(selectedActor->GetWorld());
	GEditor->EndTransaction();
}
bool LGUIEditorTools::CreateOrApplyPrefab(ULGUIPrefabHelperObject* InPrefabHelperObject)
{
	if (InPrefabHelperObject->LoadedRootActor)
	{
		if (auto PrefabAsset = InPrefabHelperObject->PrefabAsset)
		{
			GEditor->BeginTransaction(FText::FromString(TEXT("LGUI ApplyPrefab")));
			InPrefabHelperObject->SavePrefab();
			GEditor->EndTransaction();

			PrefabAsset->RefreshAgentObjectsInPreviewWorld();

			RefreshOnSubPrefabChange(InPrefabHelperObject->PrefabAsset);
			return true;
		}
	}
	return false;
}

void LGUIEditorTools::RefreshOnSubPrefabChange(ULGUIPrefab* InSubPrefab)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// Need to do this if running in the editor with -game to make sure that the assets in the following path are available
	TArray<FString> PathsToScan;
	PathsToScan.Add(TEXT("/Game/"));
	AssetRegistry.ScanPathsSynchronous(PathsToScan);

	// Get asset in path
	TArray<FAssetData> ScriptAssetList;
	AssetRegistry.GetAssetsByPath(FName("/Game/"), ScriptAssetList, /*bRecursive=*/true);

	// Ensure all assets are loaded
	for (const FAssetData& Asset : ScriptAssetList)
	{
		// Gets the loaded asset, loads it if necessary
		if (Asset.AssetClass == TEXT("LGUIPrefab"))
		{
			auto AssetObject = Asset.GetAsset();
			if (auto Prefab = Cast<ULGUIPrefab>(AssetObject))
			{
				Prefab->MakeAgentObjectsInPreviewWorld();
				//check if is opened by prefab editor
				if (auto PrefabEditor = FLGUIPrefabEditor::GetEditorForPrefabIfValid(Prefab))
				{
					PrefabEditor->RefreshOnSubPrefabDirty(InSubPrefab);
				}
				else
				{
					Prefab->RefreshOnSubPrefabDirty(InSubPrefab);
				}
			}
		}
	}
}

void LGUIEditorTools::RevertPrefab()
{
	GEditor->BeginTransaction(FText::FromString(TEXT("LGUI RevertPrefab")));
	auto selectedActor = GetFirstSelectedActor();
	auto prefabActor = LGUIEditorTools::GetPrefabHelperObject_WhichManageThisActor(selectedActor);
	if (prefabActor != nullptr)
	{
		prefabActor->RevertPrefab();
	}
	CleanupPrefabsInWorld(selectedActor->GetWorld());
	GEditor->EndTransaction();
}
void LGUIEditorTools::DeletePrefab()
{
	auto confirmMsg = FString::Printf(TEXT("Delete selected prefab instance?"));
	auto confirmResult = FMessageDialog::Open(EAppMsgType::YesNo, FText::FromString(confirmMsg));
	if (confirmResult == EAppReturnType::No)return;

	GEditor->BeginTransaction(FText::FromString(TEXT("LGUI DeletePrefabInstance")));
	auto selectedActor = GetFirstSelectedActor();
	auto PrefabHelperObject = LGUIEditorTools::GetPrefabHelperObject_WhichManageThisActor(selectedActor);
	if (PrefabHelperObject != nullptr)
	{
		check(!PrefabHelperObject->bIsInsidePrefabEditor);

		LGUIUtils::DestroyActorWithHierarchy(PrefabHelperObject->LoadedRootActor);
		auto OwnerActor = Cast<ALGUIPrefabHelperActor>(PrefabHelperObject->GetOuter());
		check(OwnerActor != nullptr);
		LGUIUtils::DestroyActorWithHierarchy(OwnerActor);
	}
	CleanupPrefabsInWorld(selectedActor->GetWorld());
	GEditor->EndTransaction();
}

void LGUIEditorTools::UnlinkPrefab()
{
	GEditor->BeginTransaction(FText::FromString(TEXT("LGUI UnlinkPrefab")));
	auto SelectedActor = GetFirstSelectedActor();
	auto PrefabHelperObject = LGUIEditorTools::GetPrefabHelperObject_WhichManageThisActor(SelectedActor);
	if (PrefabHelperObject != nullptr)
	{
		if (PrefabHelperObject->bIsInsidePrefabEditor)//prefab editor only allow operate on sub prefab
		{
			check(PrefabHelperObject->SubPrefabMap.Contains(SelectedActor));//should have being check in Unlink button
			PrefabHelperObject->UnlinkSubPrefab(SelectedActor);
		}
		else
		{
			auto OwnerActor = Cast<ALGUIPrefabHelperActor>(PrefabHelperObject->GetOuter());
			check(OwnerActor != nullptr);
			PrefabHelperObject->UnlinkPrefab(SelectedActor);
			LGUIUtils::DestroyActorWithHierarchy(OwnerActor);
		}
	}
	GEditor->EndTransaction();
}

void LGUIEditorTools::SelectPrefabAsset()
{
	GEditor->BeginTransaction(FText::FromString(TEXT("LGUI SelectPrefabAsset")));
	auto SelectedActor = GetFirstSelectedActor();
	auto PrefabHelperObject = LGUIEditorTools::GetPrefabHelperObject_WhichManageThisActor(SelectedActor);
	if (PrefabHelperObject != nullptr)
	{
		ULGUIPrefab* PrefabAsset = nullptr;
		if (PrefabHelperObject->bIsInsidePrefabEditor)//prefab editor only allow operate on sub prefab
		{
			check(PrefabHelperObject->SubPrefabMap.Contains(SelectedActor));//should have being check in Browse button
			PrefabAsset = PrefabHelperObject->GetSubPrefabAsset(SelectedActor);
		}
		else
		{
			PrefabAsset = PrefabHelperObject->PrefabAsset;
		}
		if (IsValid(PrefabAsset))
		{
			TArray<UObject*> ObjectsToSync;
			ObjectsToSync.Add(PrefabAsset);
			GEditor->SyncBrowserToObjects(ObjectsToSync);
		}
	}
	GEditor->EndTransaction();
}

ULGUIPrefabHelperObject* LGUIEditorTools::GetPrefabHelperObject_WhichManageThisActor(AActor* InActor)
{
	if (!IsValid(InActor))return nullptr;
	for (TObjectIterator<ULGUIPrefabHelperObject> Itr; Itr; ++Itr)
	{
		if (Itr->IsActorBelongsToThis(InActor, true))
		{
			return *Itr;
		}
	}
	return nullptr;
}

bool LGUIEditorTools::IsPrefabActor(AActor* InActor)
{
	for (TObjectIterator<ULGUIPrefabHelperObject> Itr; Itr; ++Itr)
	{
		if (Itr->AllLoadedActorArray.Contains(InActor))
		{
			if (auto LoadedRootActor = Itr->LoadedRootActor)
			{
				if (InActor->IsAttachedTo(LoadedRootActor) || InActor == LoadedRootActor)
				{
					return true;
				}
			}
		}
	}
	return false;
}

void LGUIEditorTools::CleanupPrefabsInWorld(UWorld* World)
{
	for (TActorIterator<ALGUIPrefabHelperActor> ActorItr(World); ActorItr; ++ActorItr)
	{
		auto prefabActor = *ActorItr;
		if (IsValid(prefabActor))
		{
			if (!IsValid(prefabActor->PrefabHelperObject->LoadedRootActor))
			{
				LGUIUtils::DestroyActorWithHierarchy(prefabActor, false);
			}
		}
	}
}

void LGUIEditorTools::ClearInvalidPrefabActor(UWorld* World)
{
	for (TActorIterator<ALGUIPrefabHelperActor> ActorItr(World); ActorItr; ++ActorItr)
	{
		auto prefabActor = *ActorItr;
		if (IsValid(prefabActor))
		{
			if (!IsValid(prefabActor->PrefabHelperObject->LoadedRootActor))
			{
				LGUIUtils::DestroyActorWithHierarchy(prefabActor, false);
			}
		}
	}
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
void LGUIEditorTools::SetTraceChannelToParent(AActor* InActor)
{
	//change trace channel to same as parent
	if (auto parentActor = InActor->GetAttachParentActor())
	{
		if (auto parentComp = parentActor->GetRootComponent())
		{
			if (auto parentUIComp = Cast<UUIItem>(parentComp))
			{
				TArray<UUIItem*> components;
				InActor->GetComponents<UUIItem>(components);
				for (auto compItem : components)
				{
					compItem->SetTraceChannel(parentUIComp->GetTraceChannel());
				}
			}
		}
	}
}
void LGUIEditorTools::SetTraceChannelToParent_Recursive(AActor* InActor)
{
	SetTraceChannelToParent(InActor);
	TArray<AActor*> childrenActors;
	InActor->GetAttachedActors(childrenActors);
	for (auto itemActor : childrenActors)
	{
		SetTraceChannelToParent_Recursive(itemActor);
	}
}

void LGUIEditorTools::RefreshSceneOutliner()
{
	GEngine->BroadcastLevelActorListChanged();
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
						auto viewRotation = canvas->GetViewRotator().Quaternion() * FQuat::MakeFromEuler(FVector(90, 90, 0));
						editorViewportClient->SetViewLocation(canvas->GetUIItem()->GetComponentLocation() - viewRotation.GetForwardVector() * editorViewDistance);
						editorViewportClient->SetViewRotation(viewRotation.Rotator());
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
							auto viewRotation = canvas->GetViewRotator().Quaternion() * FQuat::MakeFromEuler(FVector(90, 90, 0));
							editorViewportClient->SetViewRotation(viewRotation.Rotator());
							editorViewportClient->SetLookAtLocation(canvas->GetUIItem()->GetComponentLocation());
						}
					}
				}
			}
		}
	}
}

void LGUIEditorTools::ForceGC()
{
	GEngine->ForceGarbageCollection();
}

#include "Core/UIWidget.h"
void LGUIEditorTools::UpgradeLevelToLGUI3()
{
	auto confirmMsg = FString::Printf(TEXT("This upgrade operation cannot do every modification, so some work should do manually."));
	if (FMessageDialog::Open(EAppMsgType::YesNo, FText::FromString(confirmMsg)) == EAppReturnType::No)return;

	confirmMsg = FString(TEXT("Remember to backup your work. Continue?"));
	if (FMessageDialog::Open(EAppMsgType::YesNo, FText::FromString(confirmMsg)) == EAppReturnType::No)return;

	if (GetDefault<ULGUIEditorSettings>()->AnchorControlPosition)
	{
		confirmMsg = FString(TEXT("LGUIEditorSetting->AnchorControlPosition must set to false before this upgrade operation! You can set it back to true after upgrade is done."));
		if (FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(confirmMsg)) == EAppReturnType::Ok)return;
	}

	if (!GWorld)return;
	TArray<AActor*> ActorArray;
	for (TActorIterator<AActor> ActorItr(GWorld); ActorItr; ++ActorItr)
	{
		auto Actor = *ActorItr;
		ActorArray.Add(Actor);
	}
	if (ActorArray.Num() > 0) ActorArray[0]->MarkPackageDirty();
	UpgradeActorArray(ActorArray, false);
}

FVector ConvertPositionFromLGUI2ToLGUI3(FVector InValue)
{
	return FVector(InValue.Z, InValue.X, InValue.Y);
}
FRotator ConvertRotatorFromLGUI2ToLGUI3(FRotator InValue)
{
	return FRotator(InValue.Roll, -InValue.Pitch, InValue.Yaw);

	//return FRotator(InValue.Yaw, InValue.Roll, InValue.Pitch);

	//return FRotator(InValue.Pitch, InValue.Yaw, InValue.Roll);
}

void LGUIEditorTools::UpgradeActorArray(const TArray<AActor*>& InActorArray, bool InIsPrefabOrWorld)
{
	for (auto& Actor : InActorArray)
	{
		if (auto UIItem = Cast<UUIItem>(Actor->GetRootComponent()))
		{
			auto ActorLabel = Actor->GetActorLabel();
			if (ActorLabel.StartsWith(TEXT("//")))
			{
				ActorLabel = ActorLabel.Right(ActorLabel.Len() - 2);
			}
			Actor->SetActorLabel(ActorLabel);

			auto UIParent = UIItem->GetParentUIItem();
			//color
			auto UIRenderable = Cast<UUIBaseRenderable>(UIItem);
			if (UIRenderable)
			{
				UIRenderable->SetColor(UIItem->widget.color);
			}
			//coordinate
			if (UIParent != nullptr)//get parent
			{
				auto RelativeLocation = UIItem->GetRelativeLocation();
				UIItem->SetRelativeLocation(ConvertPositionFromLGUI2ToLGUI3(RelativeLocation));
				auto RelativeRotation = UIItem->GetRelativeRotation();
				UIItem->SetRelativeRotation(ConvertRotatorFromLGUI2ToLGUI3(RelativeRotation));
			}
			else//no parent
			{
				if (InIsPrefabOrWorld)//prefab mostly be child of other UIItem, so consider it have parent, so set relative location
				{
					auto RelativeLocation = UIItem->GetRelativeLocation();
					UIItem->SetRelativeLocation(ConvertPositionFromLGUI2ToLGUI3(RelativeLocation));
					auto RelativeRotation = UIItem->GetRelativeRotation();
					UIItem->SetRelativeRotation(ConvertRotatorFromLGUI2ToLGUI3(RelativeRotation));
				}
				else
				{
					UIItem->SetRelativeRotation(UIItem->GetRelativeRotation().Add(0, -90, 90));
				}
			}
			//anchor
			auto widget = UIItem->widget;
			FUIAnchorData AnchorData;
			switch (widget.anchorHAlign)
			{
			case UIAnchorHorizontalAlign::Left:
			{
				AnchorData.AnchorMin.X = AnchorData.AnchorMax.X = 0;
				AnchorData.AnchoredPosition.X = widget.anchorOffsetX;
				AnchorData.SizeDelta.X = widget.width;
			}
			break;
			case UIAnchorHorizontalAlign::Center:
			{
				AnchorData.AnchorMin.X = AnchorData.AnchorMax.X = 0.5f;
				AnchorData.AnchoredPosition.X = widget.anchorOffsetX;
				AnchorData.SizeDelta.X = widget.width;
			}
			break;
			case UIAnchorHorizontalAlign::Right:
			{
				AnchorData.AnchorMin.X = AnchorData.AnchorMax.X = 1.0f;
				AnchorData.AnchoredPosition.X = widget.anchorOffsetX;
				AnchorData.SizeDelta.X = widget.width;
			}
			break;
			case UIAnchorHorizontalAlign::Stretch:
			{
				AnchorData.AnchorMin.X = 0; AnchorData.AnchorMax.X = 1;
				AnchorData.SizeDelta.X = -(widget.stretchRight + widget.stretchLeft);
				AnchorData.AnchoredPosition.X = FMath::Lerp(widget.stretchLeft, -widget.stretchRight, widget.pivot.X);
			}
			break;
			}
			switch (widget.anchorVAlign)
			{
			case UIAnchorVerticalAlign::Bottom:
			{
				AnchorData.AnchorMin.Y = AnchorData.AnchorMax.Y = 0;
				AnchorData.AnchoredPosition.Y = widget.anchorOffsetY;
				AnchorData.SizeDelta.Y = widget.height;
			}
			break;
			case UIAnchorVerticalAlign::Middle:
			{
				AnchorData.AnchorMin.Y = AnchorData.AnchorMax.Y = 0.5f;
				AnchorData.AnchoredPosition.Y = widget.anchorOffsetY;
				AnchorData.SizeDelta.Y = widget.height;
			}
			break;
			case UIAnchorVerticalAlign::Top:
			{
				AnchorData.AnchorMin.Y = AnchorData.AnchorMax.Y = 1.0f;
				AnchorData.AnchoredPosition.Y = widget.anchorOffsetY;
				AnchorData.SizeDelta.Y = widget.height;
			}
			break;
			case UIAnchorVerticalAlign::Stretch:
			{
				AnchorData.AnchorMin.Y = 0; AnchorData.AnchorMax.Y = 1;
				AnchorData.SizeDelta.Y = -(widget.stretchTop + widget.stretchBottom);
				AnchorData.AnchoredPosition.Y = FMath::Lerp(widget.stretchBottom, -widget.stretchTop, widget.pivot.Y);
			}
			break;
			}
			if (UIParent == nullptr && !InIsPrefabOrWorld)
			{
				AnchorData.AnchoredPosition.X = UIItem->GetRelativeLocation().Y;
				AnchorData.AnchoredPosition.Y = UIItem->GetRelativeLocation().Z;
				AnchorData.SizeDelta.X = widget.width;
				AnchorData.SizeDelta.Y = widget.height;
			}
			AnchorData.Pivot = widget.pivot;
			UIItem->SetAnchorData(AnchorData);
		}
		//UIEffectLongShadow
		TArray<UUIEffectLongShadow*> LongShadowArray;
		Actor->GetComponents(LongShadowArray);
		for (auto& Item : LongShadowArray)
		{
			auto ShadowSize = Item->GetShadowSize();
			Item->SetShadowSize(FVector(ShadowSize.Z, ShadowSize.X, ShadowSize.Y));
		}
		//Canvas
		TArray<ULGUICanvas*> CanvasArray;
		Actor->GetComponents(CanvasArray);
		for (auto& Item : CanvasArray)
		{
			if (Item->GetSortOrder() != 0)
			{
				Item->SetOverrideSorting(true);
			}
		}
		//
		UpgradeObjectProperty(Actor);
		auto& Comps = Actor->GetComponents();
		for (auto& Comp : Comps)
		{
			UpgradeObjectProperty(Comp);
		}
	}

	for (auto& Actor : InActorArray)
	{
		//Slider
		{
			TArray<UUISliderComponent*> SliderArray;
			Actor->GetComponents(SliderArray);
			for (auto& Item : SliderArray)
			{
				Item->ForUpgrade2to3_ApplyValueToUI();
				if (auto Fill = Item->GetFillActor())
				{
					auto FillUIItem = Fill->GetUIItem();
					switch (Item->GetDirectionType())
					{
					case UISliderDirectionType::LeftToRight:
					case UISliderDirectionType::RightToLeft:
					{
						FillUIItem->SetAnchorLeft(0);
						FillUIItem->SetAnchorRight(0);
						FillUIItem->SetHorizontalAnchoredPosition(0);
						auto SizeDelta = FillUIItem->GetSizeDelta();
						SizeDelta.X = 0;
						FillUIItem->SetSizeDelta(SizeDelta);
					}
					break;
					case UISliderDirectionType::BottomToTop:
					case UISliderDirectionType::TopToBottom:
					{
						FillUIItem->SetAnchorBottom(0);
						FillUIItem->SetAnchorTop(0);
						FillUIItem->SetVerticalAnchoredPosition(0);
						auto SizeDelta = FillUIItem->GetSizeDelta();
						SizeDelta.Y = 0;
						FillUIItem->SetSizeDelta(SizeDelta);
					}
					break;
					}
				}
				if (auto Handle = Item->GetHandleActor())
				{
					auto HandleUIItem = Handle->GetUIItem();
					switch (Item->GetDirectionType())
					{
					case UISliderDirectionType::LeftToRight:
					case UISliderDirectionType::RightToLeft:
					{
						HandleUIItem->SetHorizontalAnchoredPosition(0);
					}
					break;
					case UISliderDirectionType::BottomToTop:
					case UISliderDirectionType::TopToBottom:
					{
						HandleUIItem->SetVerticalAnchoredPosition(0);
					}
					break;
					}
				}
			}
		}

		//Scrollbar
		{
			TArray<UUIScrollbarComponent*> ScrollbarArray;
			Actor->GetComponents(ScrollbarArray);
			for (auto& Item : ScrollbarArray)
			{
				Item->ForUpgrade2to3_ApplyValueToUI();
				if (auto Handle = Item->GetHandleActor())
				{
					auto HandleUIItem = Handle->GetUIItem();
					switch (Item->GetDirectionType())
					{
					case UIScrollbarDirectionType::LeftToRight:
					case UIScrollbarDirectionType::RightToLeft:
					{
						HandleUIItem->SetAnchorLeft(0);
						HandleUIItem->SetAnchorRight(0);
						HandleUIItem->SetHorizontalAnchoredPosition(0);
						auto SizeDelta = HandleUIItem->GetSizeDelta();
						SizeDelta.X = 0;
						HandleUIItem->SetSizeDelta(SizeDelta);
					}
					break;
					case UIScrollbarDirectionType::BottomToTop:
					case UIScrollbarDirectionType::TopToBottom:
					{
						HandleUIItem->SetAnchorBottom(0);
						HandleUIItem->SetAnchorTop(0);
						HandleUIItem->SetVerticalAnchoredPosition(0);
						auto SizeDelta = HandleUIItem->GetSizeDelta();
						SizeDelta.Y = 0;
						HandleUIItem->SetSizeDelta(SizeDelta);
					}
					break;
					}
				}
			}
		}

		//UIEffectTextAnimation
		{
			TArray<UUIEffectTextAnimation*> TextAnimationArray;
			Actor->GetComponents(TextAnimationArray);
			for (auto& Item : TextAnimationArray)
			{
				auto& Properties = Item->GetProperties();
				for (auto& PropertyItem : Properties)
				{
					if (auto PositionProperty = Cast<UUIEffectTextAnimation_PositionProperty>(PropertyItem))
					{
						auto Position = PositionProperty->GetPosition();
						PositionProperty->SetPosition(ConvertPositionFromLGUI2ToLGUI3(Position));
					}
					else if (auto PositionRandomProperty = Cast<UUIEffectTextAnimation_PositionRandomProperty>(PropertyItem))
					{
						auto Min = PositionRandomProperty->GetMin();
						auto Max = PositionRandomProperty->GetMax();
						PositionRandomProperty->SetMin(ConvertPositionFromLGUI2ToLGUI3(Min));
						PositionRandomProperty->SetMax(ConvertPositionFromLGUI2ToLGUI3(Max));
					}
					else if (auto PositionWaveProperty = Cast<UUIEffectTextAnimation_PositionWaveProperty>(PropertyItem))
					{
						auto Value = PositionWaveProperty->GetPosition();
						PositionWaveProperty->SetPosition(ConvertPositionFromLGUI2ToLGUI3(Value));
					}

					else if (auto RotationProperty = Cast<UUIEffectTextAnimation_RotationProperty>(PropertyItem))
					{
						auto Rotator = RotationProperty->GetRotator();
						RotationProperty->SetRotator(ConvertRotatorFromLGUI2ToLGUI3(Rotator));
					}
					else if (auto RotationRandomProperty = Cast <UUIEffectTextAnimation_RotationRandomProperty>(PropertyItem))
					{
						auto Min = RotationRandomProperty->GetMin();
						auto Max = RotationRandomProperty->GetMax();
						RotationRandomProperty->SetMin(ConvertRotatorFromLGUI2ToLGUI3(Min));
						RotationRandomProperty->SetMax(ConvertRotatorFromLGUI2ToLGUI3(Max));
					}
					else if (auto RotationWaveProperty = Cast<UUIEffectTextAnimation_RotationWaveProperty>(PropertyItem))
					{
						auto Value = RotationWaveProperty->GetRotator();
						RotationWaveProperty->SetRotator(ConvertRotatorFromLGUI2ToLGUI3(Value));
					}

					else if (auto ScaleProperty = Cast<UUIEffectTextAnimation_ScaleProperty>(PropertyItem))
					{
						auto Value = ScaleProperty->GetScale();
						ScaleProperty->SetScale(ConvertPositionFromLGUI2ToLGUI3(Value));
					}
					else if (auto ScaleRandomProperty = Cast<UUIEffectTextAnimation_ScaleRandomProperty>(PropertyItem))
					{
						auto Min = ScaleRandomProperty->GetMin();
						auto Max = ScaleRandomProperty->GetMax();
						ScaleRandomProperty->SetMin(ConvertPositionFromLGUI2ToLGUI3(Min));
						ScaleRandomProperty->SetMax(ConvertPositionFromLGUI2ToLGUI3(Max));
					}
					else if (auto ScaleWaveProperty = Cast<UUIEffectTextAnimation_ScaleWaveProperty>(PropertyItem))
					{
						auto Value = ScaleWaveProperty->GetScale();
						ScaleWaveProperty->SetScale(ConvertPositionFromLGUI2ToLGUI3(Value));
					}
				}
			}
		}
	}
}

void LGUIEditorTools::UpgradeSelectedPrefabToLGUI3()
{
	auto Selection = GEditor->GetSelectedObjects();
	TArray<ULGUIPrefab*> SelectedPrefabArray;
	Selection->GetSelectedObjects(SelectedPrefabArray);
	for (auto& Prefab : SelectedPrefabArray)
	{
		if (IsValid(Prefab))
		{
			auto World = ULGUIEditorManagerObject::GetPreviewWorldForPrefabPackage();
			TMap<FGuid, UObject*> MapGuidToObject;
			TMap<AActor*, FLGUISubPrefabData> SubPrefabMap;
			auto RootActor = Prefab->LoadPrefabForEdit(World, nullptr
				, MapGuidToObject, SubPrefabMap
			);
			TArray<AActor*> AllChildrenActorArray;
			LGUIUtils::CollectChildrenActors(RootActor, AllChildrenActorArray, true);
			UpgradeActorArray(AllChildrenActorArray, true);
			TMap<UObject*, FGuid> MapObjectToGuid;
			for (auto KeyValue : MapGuidToObject)
			{
				MapObjectToGuid.Add(KeyValue.Value, KeyValue.Key);
			}
			Prefab->SavePrefab(RootActor, MapObjectToGuid, SubPrefabMap);
			Prefab->MakeAgentObjectsInPreviewWorld();

			LGUIUtils::DestroyActorWithHierarchy(RootActor, true);
		}
	}
}
void LGUIEditorTools::UpgradeAllPrefabToLGUI3()
{
	auto confirmMsg = FString::Printf(TEXT("This upgrade operation cannot do every modification, so some work should do manually."));
	if (FMessageDialog::Open(EAppMsgType::YesNo, FText::FromString(confirmMsg)) == EAppReturnType::No)return;

	confirmMsg = FString(TEXT("Remember to backup your work. Continue?"));
	if (FMessageDialog::Open(EAppMsgType::YesNo, FText::FromString(confirmMsg)) == EAppReturnType::No)return;

	if (GetDefault<ULGUIEditorSettings>()->AnchorControlPosition)
	{
		confirmMsg = FString(TEXT("LGUIEditorSetting->AnchorControlPosition must set to false!"));
		if (FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(confirmMsg)) == EAppReturnType::Ok)return;
	}


	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// Need to do this if running in the editor with -game to make sure that the assets in the following path are available
	TArray<FString> PathsToScan;
	PathsToScan.Add(TEXT("/Game/"));
	AssetRegistry.ScanPathsSynchronous(PathsToScan);

	// Get asset in path
	TArray<FAssetData> ScriptAssetList;
	AssetRegistry.GetAssetsByPath(FName("/Game/"), ScriptAssetList, /*bRecursive=*/true);

	// Ensure all assets are loaded
	for (const FAssetData& Asset : ScriptAssetList)
	{
		// Gets the loaded asset, loads it if necessary
		if (Asset.AssetClass == TEXT("LGUIPrefab"))
		{
			auto AssetObject = Asset.GetAsset();
			if (auto Prefab = Cast<ULGUIPrefab>(AssetObject))
			{
				auto World = ULGUIEditorManagerObject::GetPreviewWorldForPrefabPackage();
				TMap<FGuid, UObject*> MapGuidToObject;
				TMap<AActor*, FLGUISubPrefabData> SubPrefabMap;
				auto RootActor = Prefab->LoadPrefabForEdit(World, nullptr
					, MapGuidToObject, SubPrefabMap
				);
				TArray<AActor*> AllChildrenActorArray;
				LGUIUtils::CollectChildrenActors(RootActor, AllChildrenActorArray, true);
				UpgradeActorArray(AllChildrenActorArray, true);
				TMap<UObject*, FGuid> MapObjectToGuid;
				for (auto KeyValue : MapGuidToObject)
				{
					MapObjectToGuid.Add(KeyValue.Value, KeyValue.Key);
				}
				Prefab->SavePrefab(RootActor, MapObjectToGuid, SubPrefabMap);
				Prefab->MakeAgentObjectsInPreviewWorld();

				LGUIUtils::DestroyActorWithHierarchy(RootActor, true);
			}
		}
	}
}
#include "GameFramework/Actor.h"
#include "Serialization/BufferArchive.h"
void LGUIEditorTools::UpgradeObjectProperty(UObject* InObject)
{
	auto PropertyField = TFieldRange<FProperty>(InObject->GetClass());
	for (const auto PropertyItem : PropertyField)
	{
		UpgradeCommonProperty(PropertyItem, (uint8*)InObject);
	}
}
void LGUIEditorTools::UpgradeCommonProperty(FProperty* PropertyItem, uint8* InContainerPtr)
{
	if (auto StructProperty = CastField<FStructProperty>(PropertyItem))
	{
		auto StructName = StructProperty->Struct->GetFName();
		if (StructName == FLGUIComponentReference::StaticStruct()->GetFName())
		{
			auto StructPtr = StructProperty->ContainerPtrToValuePtr<uint8>(InContainerPtr);

			auto targetActorProp = FindFProperty<FObjectPropertyBase>(StructProperty->Struct, TEXT("targetActor"));
			auto HelperActorProp = FindFProperty<FObjectPropertyBase>(StructProperty->Struct, TEXT("HelperActor"));
			auto targetActorObject = targetActorProp->GetObjectPropertyValue_InContainer(StructPtr);
			HelperActorProp->SetObjectPropertyValue_InContainer(StructPtr, targetActorObject);
			auto targetComponentClassProp = FindFProperty<FObjectPropertyBase>(StructProperty->Struct, TEXT("targetComponentClass"));
			auto targetComponentClassObject = targetComponentClassProp->GetObjectPropertyValue_InContainer(StructPtr);
			auto targetComonentNameProp = FindFProperty<FNameProperty>(StructProperty->Struct, TEXT("targetComonentName"));
			auto targetComonentName = targetComonentNameProp->GetPropertyValue_InContainer(StructPtr);
			auto targetActor = Cast<AActor>(targetActorObject);
			auto targetComponentClass = Cast<UClass>(targetComponentClassObject);

			auto TargetCompProp = FindFProperty<FObjectPropertyBase>(StructProperty->Struct, TEXT("TargetComp"));
			auto HelperClassProp = FindFProperty<FObjectPropertyBase>(StructProperty->Struct, TEXT("HelperClass"));
			auto HelperComponentNameProp = FindFProperty<FNameProperty>(StructProperty->Struct, TEXT("HelperComponentName"));
			auto HelperClassObj = HelperClassProp->GetObjectPropertyValue_InContainer(StructPtr);
			if (targetComponentClass != nullptr)
			{
				HelperClassProp->SetObjectPropertyValue_InContainer(StructPtr, targetComponentClass);
			}
			else
			{
				targetComponentClassObject = HelperClassProp->GetObjectPropertyValue_InContainer(StructPtr);
				targetComponentClass = Cast<UClass>(targetComponentClassObject);
			}
			if (!targetComonentName.IsNone())
			{
				HelperComponentNameProp->SetPropertyValue_InContainer(StructPtr, targetComonentName);
			}
			else
			{
				targetComonentName = HelperComponentNameProp->GetPropertyValue_InContainer(StructPtr);
			}

			UActorComponent* ResultComp = nullptr;
			TArray<UActorComponent*> Components;
			if (targetActor != nullptr && targetComponentClass != nullptr)
			{
				targetActor->GetComponents(targetComponentClass, Components);
				if (Components.Num() == 1)
				{
					ResultComp = Components[0];
				}
				else if (Components.Num() > 1)
				{
					for (auto Comp : Components)
					{
						if (Comp->GetFName() == targetComonentName)
						{
							ResultComp = Comp;
						}
					}
				}
				if (ResultComp != nullptr)
				{
					TargetCompProp->SetObjectPropertyValue_InContainer(StructPtr, ResultComp);
				}
			}
		}
		else if (StructName == FLGUIEventDelegateData::StaticStruct()->GetFName())
		{
			auto StructPtr = StructProperty->ContainerPtrToValuePtr<uint8>(InContainerPtr);

			auto targetActorProp = FindFProperty<FObjectPropertyBase>(StructProperty->Struct, TEXT("targetActor"));
			auto HelperActorProp = FindFProperty<FObjectPropertyBase>(StructProperty->Struct, TEXT("HelperActor"));
			auto targetActorObject = targetActorProp->GetObjectPropertyValue_InContainer(StructPtr);
			HelperActorProp->SetObjectPropertyValue_InContainer(StructPtr, targetActorObject);

			auto componentClassProp = FindFProperty<FObjectPropertyBase>(StructProperty->Struct, TEXT("componentClass"));
			auto componentClassObject = componentClassProp->GetObjectPropertyValue_InContainer(StructPtr);
			auto targetComonentNameProp = FindFProperty<FNameProperty>(StructProperty->Struct, TEXT("componentName"));
			auto comonentName = targetComonentNameProp->GetPropertyValue_InContainer(StructPtr);
			auto targetActor = Cast<AActor>(targetActorObject);
			auto componentClass = Cast<UClass>(componentClassObject);

			auto TargetObjectProp = FindFProperty<FObjectPropertyBase>(StructProperty->Struct, TEXT("TargetObject"));
			auto HelperClassProp = FindFProperty<FObjectPropertyBase>(StructProperty->Struct, TEXT("HelperClass"));
			auto HelperComponentNameProp = FindFProperty<FNameProperty>(StructProperty->Struct, TEXT("HelperComponentName"));
			if (componentClass != nullptr)
			{
				HelperClassProp->SetObjectPropertyValue_InContainer(StructPtr, componentClass);
			}
			else
			{
				componentClassObject = HelperClassProp->GetObjectPropertyValue_InContainer(StructPtr);
				componentClass = Cast<UClass>(componentClassObject);
			}
			if (!comonentName.IsNone())
			{
				HelperComponentNameProp->SetPropertyValue_InContainer(StructPtr, comonentName);
			}
			else
			{
				comonentName = HelperComponentNameProp->GetPropertyValue_InContainer(StructPtr);
			}

			if (componentClass->IsChildOf(AActor::StaticClass()))//is actor self
			{
				TargetObjectProp->SetObjectPropertyValue_InContainer(StructPtr, targetActorObject);
			}
			else if(componentClass->IsChildOf(UActorComponent::StaticClass()))//is actor's component
			{
				UActorComponent* ResultComp = nullptr;
				TArray<UActorComponent*> Components;
				if (targetActor != nullptr && componentClass != nullptr)
				{
					targetActor->GetComponents(componentClass, Components);
					if (Components.Num() == 1)
					{
						ResultComp = Components[0];
					}
					else if (Components.Num() > 1)
					{
						for (auto Comp : Components)
						{
							if (Comp->GetFName() == comonentName)
							{
								ResultComp = Comp;
							}
						}
					}
					if (ResultComp != nullptr)
					{
						TargetObjectProp->SetObjectPropertyValue_InContainer(StructPtr, ResultComp);
					}
				}
			}

			auto ParamTypeProp = FindFProperty<FEnumProperty>(StructProperty->Struct, TEXT("ParamType"));
			auto ValuePtr = ParamTypeProp->ContainerPtrToValuePtr<uint8>(StructPtr);
			uint8 ParamTypeUint8 = ValuePtr[0];
			auto ParamType = (LGUIEventDelegateParameterType)ParamTypeUint8;
			auto ParamBufferProp = FindFProperty<FArrayProperty>(StructProperty->Struct, TEXT("ParamBuffer"));
			switch (ParamType)
			{
			case LGUIEventDelegateParameterType::Name:
			{
				auto ReferenceNameProp = FindFProperty<FNameProperty>(StructProperty->Struct, TEXT("ReferenceName"));
				auto ReferenceName = ReferenceNameProp->GetPropertyValue_InContainer(StructPtr);

				FBufferArchive ToBinary;
				ToBinary << ReferenceName;
				FScriptArrayHelper ArrayHelper(ParamBufferProp, ParamBufferProp->ContainerPtrToValuePtr<void>(StructPtr));
				ArrayHelper.Resize(ToBinary.Num());
				FMemory::Memcpy(ArrayHelper.GetRawPtr(0), ToBinary.GetData(), ToBinary.Num());
			}
			break;
			case LGUIEventDelegateParameterType::Text:
			{
				auto ReferenceNameProp = FindFProperty<FNameProperty>(StructProperty->Struct, TEXT("ReferenceName"));
				auto ReferenceName = ReferenceNameProp->GetPropertyValue_InContainer(StructPtr);

				FBufferArchive ToBinary;
				ToBinary << ReferenceName;
			}
			break;
			case LGUIEventDelegateParameterType::String:
			{
				auto ReferenceStringProp = FindFProperty<FStrProperty>(StructProperty->Struct, TEXT("ReferenceString"));
				auto ReferenceString = ReferenceStringProp->GetPropertyValue_InContainer(StructPtr);

				FBufferArchive ToBinary;
				ToBinary << ReferenceString;
			}
			break;
			case LGUIEventDelegateParameterType::Actor:
			{
				auto ReferenceActorProp = FindFProperty<FObjectPropertyBase>(StructProperty->Struct, TEXT("ReferenceActor"));
				auto ReferenceActor = ReferenceActorProp->GetObjectPropertyValue_InContainer(StructPtr);
				auto ReferenceObjectProp = FindFProperty<FObjectPropertyBase>(StructProperty->Struct, TEXT("ReferenceObject"));
				ReferenceObjectProp->SetObjectPropertyValue_InContainer(StructPtr, ReferenceActor);
			}
			break;
			case LGUIEventDelegateParameterType::Class:
			{
				auto ReferenceClassProp = FindFProperty<FObjectPropertyBase>(StructProperty->Struct, TEXT("ReferenceClass"));
				auto ReferenceClass = ReferenceClassProp->GetObjectPropertyValue_InContainer(StructPtr);
				auto ReferenceObjectProp = FindFProperty<FObjectPropertyBase>(StructProperty->Struct, TEXT("ReferenceObject"));
				ReferenceObjectProp->SetObjectPropertyValue_InContainer(StructPtr, ReferenceClass);
			}
			break;
			}
		}
		else
		{
			auto structPtr = PropertyItem->ContainerPtrToValuePtr<uint8>(InContainerPtr);
			for (TFieldIterator<FProperty> It(StructProperty->Struct); It; ++It)
			{
				UpgradeCommonProperty(*It, structPtr);
			}
		}
	}
	else if (auto arrProperty = CastField<FArrayProperty>(PropertyItem))
	{
		FScriptArrayHelper ArrayHelper(arrProperty, arrProperty->ContainerPtrToValuePtr<void>(InContainerPtr));
		auto arrayCount = ArrayHelper.Num();
		for (int i = 0; i < arrayCount; i++)
		{
			UpgradeCommonProperty(arrProperty->Inner, ArrayHelper.GetRawPtr(i));
		}
	}
	else if (auto mapProperty = CastField<FMapProperty>(PropertyItem))//map element's data stored as key/value/key/value...
	{
		FScriptMapHelper MapHelper(mapProperty, mapProperty->ContainerPtrToValuePtr<void>(InContainerPtr));
		auto count = MapHelper.Num();
		for (int i = 0; i < count; i++)
		{
			UpgradeCommonProperty(mapProperty->KeyProp, MapHelper.GetKeyPtr(i));//key
			UpgradeCommonProperty(mapProperty->ValueProp, MapHelper.GetPairPtr(i));//value
		}
	}
	else if (auto setProperty = CastField<FSetProperty>(PropertyItem))
	{
		FScriptSetHelper SetHelper(setProperty, setProperty->ContainerPtrToValuePtr<void>(InContainerPtr));
		auto count = SetHelper.Num();
		for (int i = 0; i < count; i++)
		{
			UpgradeCommonProperty(setProperty->ElementProp, SetHelper.GetElementPtr(i));
		}
	}
	else if (CastField<FClassProperty>(PropertyItem) != nullptr || CastField<FSoftClassProperty>(PropertyItem) != nullptr)//skip class property
	{
		
	}
	else if (auto objProperty = CastField<FObjectPropertyBase>(PropertyItem))
	{
		if (auto object = objProperty->GetObjectPropertyValue_InContainer(InContainerPtr))
		{
			if (object->IsAsset() || object->IsA(UClass::StaticClass()))
			{

			}
			else if (object->GetClass()->IsChildOf(AActor::StaticClass()) || object->GetClass()->IsChildOf(UActorComponent::StaticClass()))
			{

			}
			else if (objProperty->HasAnyPropertyFlags(CPF_InstancedReference))
			{
				UpgradeObjectProperty(object);
			}
		}
	}
}
PRAGMA_ENABLE_OPTIMIZATION

#undef LOCTEXT_NAMESPACE