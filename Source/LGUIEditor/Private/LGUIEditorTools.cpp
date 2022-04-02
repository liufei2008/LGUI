// Copyright 2019-2022 LexLiu. All Rights Reserved.

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
#include "PrefabSystem/ActorCopier.h"
#include "PrefabSystem/ActorReplaceTool.h"
#include "PrefabSystem/LGUIPrefabHelperActor.h"
#include "PrefabSystem/LGUIPrefabHelperObject.h"
#include "PrefabSystem/ActorSerializer5.h"
#include "LGUIEditorModule.h"
#include "PrefabEditor/LGUIPrefabEditor.h"
#include "LGUIHeaders.h"

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
	auto selectedActor = GetFirstSelectedActor();
	if (selectedActor == nullptr)return;
	GEditor->BeginTransaction(FText::FromString(TEXT("LGUI Create Actor")));
	MakeCurrentLevel(selectedActor);
	AActor* newActor = GetWorldFromSelection()->SpawnActor<AActor>(ActorClass, FTransform::Identity, FActorSpawnParameters());
	if (IsValid(newActor))
	{
		if (selectedActor != nullptr)
		{
			auto SelectedRootComp = selectedActor->GetRootComponent();
			auto NewRootComp = newActor->GetRootComponent();
			if (SelectedRootComp && NewRootComp)
			{
				NewRootComp->SetMobility(SelectedRootComp->Mobility);
				newActor->AttachToActor(selectedActor, FAttachmentTransformRules::KeepRelativeTransform);
			}
			GEditor->SelectActor(selectedActor, false, true);
		}
		GEditor->SelectActor(newActor, true, true);

		SetTraceChannelToParent(newActor);
	}
	GEditor->EndTransaction();
}
void LGUIEditorTools::CreateEmptyActor()
{
	auto selectedActor = GetFirstSelectedActor();
	if (selectedActor == nullptr)return;
	GEditor->BeginTransaction(FText::FromString(TEXT("LGUI Create UI Element")));
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

TArray<AActor*> LGUIEditorTools::GetSelectedActors()
{
	return LGUIEditorToolsHelperFunctionHolder::ConvertSelectionToActors(GEditor->GetSelectedActors());
}

void LGUIEditorTools::CreateUIControls(FString InPrefabPath)
{
	auto selectedActor = GetFirstSelectedActor();
	if (selectedActor == nullptr)return;
	GEditor->BeginTransaction(LOCTEXT("CreateUIControl", "LGUI Create UI Control"));
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
	for (auto Actor : RootActorList)
	{
		MakeCurrentLevel(Actor);
		Actor->GetLevel()->Modify();
		auto copiedActorLabel = LGUIEditorToolsHelperFunctionHolder::GetCopiedActorLabel(Actor);
		AActor* copiedActor;
		USceneComponent* Parent = nullptr;
		if (Actor->GetAttachParentActor())
		{
			Parent = Actor->GetAttachParentActor()->GetRootComponent();
		}
		TMap<AActor*, FLGUISubPrefabData> DuplicatedSubPrefabMap;
		TMap<FGuid, UObject*> OutMapGuidToObject;
		TMap<UObject*, FGuid> InMapObjectToGuid;
		if (auto PrefabHelperObject = LGUIEditorTools::GetPrefabHelperObject_WhichManageThisActor(Actor))
		{
			if (PrefabHelperObject->CleanupInvalidSubPrefab())//do cleanup before everything else
			{
				PrefabHelperObject->Modify();
			}
			PrefabHelperObject->SetCanNotifyAttachment(false);
			struct LOCAL {
				static void CollectSubPrefabActors(AActor* InActor, const TMap<AActor*, FLGUISubPrefabData>& InSubPrefabMap, TArray<AActor*>& OutSubPrefabRootActors)
				{
					if (InSubPrefabMap.Contains(InActor))
					{
						OutSubPrefabRootActors.Add(InActor);
					}
					else
					{
						TArray<AActor*> ChildrenActors;
						InActor->GetAttachedActors(ChildrenActors);
						for (auto& ChildActor : ChildrenActors)
						{
							CollectSubPrefabActors(ChildActor, InSubPrefabMap, OutSubPrefabRootActors);
						}
					}
				}
			};
			TArray<AActor*> SubPrefabRootActors;
			LOCAL::CollectSubPrefabActors(Actor, PrefabHelperObject->SubPrefabMap, SubPrefabRootActors);//collect sub prefabs that is attached to this Actor
			for (auto& SubPrefabKeyValue : PrefabHelperObject->SubPrefabMap)//generate MapObjectToGuid
			{
				auto SubPrefabRootActor = SubPrefabKeyValue.Key;
				if (SubPrefabRootActors.Contains(SubPrefabRootActor))
				{
					auto& SubPrefabData = SubPrefabKeyValue.Value;
					PrefabHelperObject->RefreshOnSubPrefabDirty(SubPrefabData.PrefabAsset, SubPrefabRootActor);//need to update subprefab to latest before duplicate
					auto FindObjectGuidInParentPrefab = [&](FGuid InGuidInSubPrefab) {
						for (auto& KeyValue : SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab)
						{
							if (KeyValue.Value == InGuidInSubPrefab)
							{
								return KeyValue.Key;
							}
						}
						check(0);
						return FGuid::NewGuid();
					};
					for (auto& MapGuidToObjectKeyValue : SubPrefabData.MapGuidToObject)
					{
						InMapObjectToGuid.Add(MapGuidToObjectKeyValue.Value, FindObjectGuidInParentPrefab(MapGuidToObjectKeyValue.Key));
					}
				}
			}
			copiedActor = LGUIPrefabSystem5::ActorSerializer::DuplicateActorForEditor(Actor, Parent, PrefabHelperObject->SubPrefabMap, InMapObjectToGuid, DuplicatedSubPrefabMap, OutMapGuidToObject);
			for (auto& KeyValue : DuplicatedSubPrefabMap)
			{
				TMap<FGuid, UObject*> SubMapGuidToObject;
				for (auto& MapGuidItem : KeyValue.Value.MapObjectGuidFromParentPrefabToSubPrefab)
				{
					SubMapGuidToObject.Add(MapGuidItem.Value, OutMapGuidToObject[MapGuidItem.Key]);
				}
				PrefabHelperObject->MakePrefabAsSubPrefab(KeyValue.Value.PrefabAsset, KeyValue.Key, SubMapGuidToObject, KeyValue.Value.ObjectOverrideParameterArray);
			}
			PrefabHelperObject->SetCanNotifyAttachment(true);
		}
		else 
		{
			copiedActor = LGUIPrefabSystem5::ActorSerializer::DuplicateActorForEditor(Actor, Parent, {}, InMapObjectToGuid, DuplicatedSubPrefabMap, OutMapGuidToObject);
		}
		copiedActor->SetActorLabel(copiedActorLabel);
		GEditor->SelectActor(Actor, false, true);
		GEditor->SelectActor(copiedActor, true, true);
	}
	GEditor->EndTransaction();
	ULGUIEditorManagerObject::RefreshAllUI();
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
	auto CopyActorList = LGUIEditorTools::GetRootActorListFromSelection(selectedActors);
	copiedActorPrefabList.Reset();
	for (auto Actor : CopyActorList)
	{
		auto prefab = NewObject<ULGUIPrefab>();
		prefab->AddToRoot();
		TMap<UObject*, FGuid> InOutMapObjectToGuid;
		TMap<AActor*, FLGUISubPrefabData> InSubPrefabMap;
		if (auto PrefabHelperObject = LGUIEditorTools::GetPrefabHelperObject_WhichManageThisActor(Actor))
		{
			InSubPrefabMap = PrefabHelperObject->SubPrefabMap;

			if (PrefabHelperObject->CleanupInvalidSubPrefab())//do cleanup before everything else
			{
				PrefabHelperObject->Modify();
			}
			struct LOCAL {
				static void CollectSubPrefabActors(AActor* InActor, const TMap<AActor*, FLGUISubPrefabData>& InSubPrefabMap, TArray<AActor*>& OutSubPrefabRootActors)
				{
					if (InSubPrefabMap.Contains(InActor))
					{
						OutSubPrefabRootActors.Add(InActor);
					}
					else
					{
						TArray<AActor*> ChildrenActors;
						InActor->GetAttachedActors(ChildrenActors);
						for (auto& ChildActor : ChildrenActors)
						{
							CollectSubPrefabActors(ChildActor, InSubPrefabMap, OutSubPrefabRootActors);
						}
					}
				}
			};
			TArray<AActor*> SubPrefabRootActors;
			LOCAL::CollectSubPrefabActors(Actor, PrefabHelperObject->SubPrefabMap, SubPrefabRootActors);//collect sub prefabs that is attached to this Actor
			for (auto& SubPrefabKeyValue : PrefabHelperObject->SubPrefabMap)//generate MapObjectToGuid
			{
				auto SubPrefabRootActor = SubPrefabKeyValue.Key;
				if (SubPrefabRootActors.Contains(SubPrefabRootActor))
				{
					auto& SubPrefabData = SubPrefabKeyValue.Value;
					PrefabHelperObject->RefreshOnSubPrefabDirty(SubPrefabData.PrefabAsset, SubPrefabRootActor);//need to update subprefab to latest before duplicate
					auto FindObjectGuidInParentPrefab = [&](FGuid InGuidInSubPrefab) {
						for (auto& KeyValue : SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab)
						{
							if (KeyValue.Value == InGuidInSubPrefab)
							{
								return KeyValue.Key;
							}
						}
						check(0);
						return FGuid::NewGuid();
					};
					for (auto& MapGuidToObjectKeyValue : SubPrefabData.MapGuidToObject)
					{
						InOutMapObjectToGuid.Add(MapGuidToObjectKeyValue.Value, FindObjectGuidInParentPrefab(MapGuidToObjectKeyValue.Key));
					}
				}
			}
		}
		prefab->SavePrefab(Actor, InOutMapObjectToGuid, InSubPrefabMap);
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
	ULGUIPrefabHelperObject* PrefabHelperObject = nullptr;
	if (parentComp)
	{
		PrefabHelperObject = LGUIEditorTools::GetPrefabHelperObject_WhichManageThisActor(parentComp->GetOwner());
	}
	if (!PrefabHelperObject)
	{
		UWorld* World = nullptr;
		if (parentComp != nullptr)
		{
			World = parentComp->GetWorld();
		}
		else
		{
			World = GWorld;
		}
		if (World)
		{
			if (auto Level = World->GetCurrentLevel())
			{
				if (auto ManagerActor = ALGUIPrefabManagerActor::GetPrefabManagerActor(Level))
				{
					PrefabHelperObject = ManagerActor->PrefabHelperObject;
				}
			}
		}
	}
	if (PrefabHelperObject == nullptr)return;

	PrefabHelperObject->SetCanNotifyAttachment(false);
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
			TMap<FGuid, UObject*> OutMapGuidToObject;
			TMap<AActor*, FLGUISubPrefabData> LoadedSubPrefabMap;
			auto copiedActor = prefab->LoadPrefabInEditor(GetWorldFromSelection(), parentComp, LoadedSubPrefabMap, OutMapGuidToObject, false);
			for (auto& KeyValue : LoadedSubPrefabMap)
			{
				TMap<FGuid, UObject*> SubMapGuidToObject;
				for (auto& MapGuidItem : KeyValue.Value.MapObjectGuidFromParentPrefabToSubPrefab)
				{
					SubMapGuidToObject.Add(MapGuidItem.Value, OutMapGuidToObject[MapGuidItem.Key]);
				}
				PrefabHelperObject->MakePrefabAsSubPrefab(KeyValue.Value.PrefabAsset, KeyValue.Key, SubMapGuidToObject, KeyValue.Value.ObjectOverrideParameterArray);
			}
			auto copiedActorLabel = LGUIEditorToolsHelperFunctionHolder::GetCopiedActorLabel(copiedActor);
			copiedActor->SetActorLabel(copiedActorLabel);
			GEditor->SelectActor(copiedActor, true, true);
		}
		else
		{
			UE_LOG(LGUIEditor, Error, TEXT("Source copied actor is missing!"));
		}
	}
	PrefabHelperObject->SetCanNotifyAttachment(true);
	GEditor->EndTransaction();
	ULGUIEditorManagerObject::RefreshAllUI();
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
			PrefabHelperObject->RemoveSubPrefab(Actor);
			LGUIUtils::DestroyActorWithHierarchy(Actor);
		}
		else//common actor
		{
			LGUIUtils::DestroyActorWithHierarchy(Actor);
		}
	}
	GEditor->EndTransaction();
	CleanupPrefabsInWorld(RootActorList[0]->GetWorld());
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
		ULGUIEditorManagerObject::RefreshAllUI();
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
	FString prefabPath(TEXT("/LGUI/Prefabs/ScreenSpaceUI"));
	auto prefab = LoadObject<ULGUIPrefab>(NULL, *prefabPath);
	if (prefab)
	{
		GEditor->BeginTransaction(FText::FromString(TEXT("LGUI Create Screen Space UI")));
		auto actor = prefab->LoadPrefabInEditor(GetWorldFromSelection(), nullptr, true);
		actor->GetRootComponent()->SetRelativeScale3D(FVector::OneVector);
		actor->GetRootComponent()->SetRelativeLocation(FVector(0, 0, 250));
		if (auto selectedActor = GetFirstSelectedActor())
		{
			GEditor->SelectActor(selectedActor, false, true);
		}
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
		GEditor->EndTransaction();
		ULGUIEditorManagerObject::RefreshAllUI();
	}
	else
	{
		UE_LOG(LGUIEditor, Error, TEXT("[LGUIEditorToolsAgentObject::CreateScreenSpaceUI_BasicSetup]Load control prefab error! Path:%s. Missing some content of LGUI plugin, reinstall this plugin may fix the issure."), *prefabPath);
	}
}
void LGUIEditorTools::CreateWorldSpaceUIUERenderer_BasicSetup()
{
	FString prefabPath(TEXT("/LGUI/Prefabs/WorldSpaceUI_UERenderer"));
	auto prefab = LoadObject<ULGUIPrefab>(NULL, *prefabPath);
	if (prefab)
	{
		GEditor->BeginTransaction(FText::FromString(TEXT("LGUI Create World Space UI - UE Renderer")));
		auto actor = prefab->LoadPrefabInEditor(GetWorldFromSelection(), nullptr, true);
		actor->GetRootComponent()->SetRelativeLocation(FVector(0, 0, 250));
		actor->GetRootComponent()->SetWorldScale3D(FVector::OneVector);
		if (auto selectedActor = GetFirstSelectedActor())
		{
			GEditor->SelectActor(selectedActor, false, true);
		}
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
		GEditor->EndTransaction();
		ULGUIEditorManagerObject::RefreshAllUI();
	}
	else
	{
		UE_LOG(LGUIEditor, Error, TEXT("[LGUIEditorToolsAgentObject::CreateWorldSpaceUIUERenderer_BasicSetup]Load control prefab error! Path:%s. Missing some content of LGUI plugin, reinstall this plugin may fix the issure."), *prefabPath);
	}
}
void LGUIEditorTools::CreateWorldSpaceUILGUIRenderer_BasicSetup()
{
	FString prefabPath(TEXT("/LGUI/Prefabs/WorldSpaceUI_LGUIRenderer"));
	auto prefab = LoadObject<ULGUIPrefab>(NULL, *prefabPath);
	if (prefab)
	{
		GEditor->BeginTransaction(FText::FromString(TEXT("LGUI Create World Space UI - LGUI Renderer")));
		auto actor = prefab->LoadPrefabInEditor(GetWorldFromSelection(), nullptr, true);
		actor->GetRootComponent()->SetRelativeLocation(FVector(0, 0, 250));
		actor->GetRootComponent()->SetWorldScale3D(FVector::OneVector);
		if (auto selectedActor = GetFirstSelectedActor())
		{
			GEditor->SelectActor(selectedActor, false, true);
		}
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
		GEditor->EndTransaction();
		ULGUIEditorManagerObject::RefreshAllUI();
	}
	else
	{
		UE_LOG(LGUIEditor, Error, TEXT("[LGUIEditorToolsAgentObject::CreateWorldSpaceUILGUIRenderer_BasicSetup]Load control prefab error! Path:%s. Missing some content of LGUI plugin, reinstall this plugin may fix the issure."), *prefabPath);
	}
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
void LGUIEditorTools::CreatePrefabAsset()//@todo: make some referenced parameter as override parameter(eg: Actor parameter reference other actor that is not belongs to prefab hierarchy)
{
	auto selectedActor = GetFirstSelectedActor();
	if (selectedActor == nullptr)return;
	if (Cast<ALGUIPrefabHelperActor>(selectedActor) != nullptr || Cast<ALGUIPrefabManagerActor>(selectedActor) != nullptr)
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

				auto PrefabHelperObjectWhichManageThisActor = LGUIEditorTools::GetPrefabHelperObject_WhichManageThisActor(selectedActor);
				if (PrefabHelperObjectWhichManageThisActor == nullptr)//not exist, means in level editor and not create PrefabManagerActor yet, so create it
				{
					auto ManagerActor = ALGUIPrefabManagerActor::GetPrefabManagerActor(selectedActor->GetLevel());
					if (ManagerActor != nullptr)
					{
						PrefabHelperObjectWhichManageThisActor = ManagerActor->PrefabHelperObject;
					}
				}
				check(PrefabHelperObjectWhichManageThisActor != nullptr)
				{
					struct LOCAL
					{
						static auto Make_MapGuidFromParentToSub(const TMap<UObject*, FGuid>& InNewParentMapObjectToGuid, ULGUIPrefabHelperObject* InPrefabHelperObject, const FLGUISubPrefabData& InOriginSubPrefabData)
						{
							TMap<FGuid, FGuid> Result;
							for (auto& KeyValue : InOriginSubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab)
							{
								auto Object = InPrefabHelperObject->MapGuidToObject[KeyValue.Key];
								auto Guid = InNewParentMapObjectToGuid[Object];
								if (!Result.Contains(Guid))
								{
									Result.Add(Guid, KeyValue.Value);
								}
							}
							return Result;
						}
						static void CollectSubPrefab(AActor* InActor, TMap<AActor*, FLGUISubPrefabData>& InOutSubPrefabMap, ULGUIPrefabHelperObject* InPrefabHelperObject, const TMap<UObject*, FGuid>& InMapObjectToGuid)
						{
							if (InPrefabHelperObject->IsActorBelongsToSubPrefab(InActor))
							{
								auto OriginSubPrefabData = InPrefabHelperObject->GetSubPrefabData(InActor);
								FLGUISubPrefabData SubPrefabData;
								SubPrefabData.PrefabAsset = OriginSubPrefabData.PrefabAsset;
								SubPrefabData.ObjectOverrideParameterArray = OriginSubPrefabData.ObjectOverrideParameterArray;
								SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab = Make_MapGuidFromParentToSub(InMapObjectToGuid, InPrefabHelperObject, OriginSubPrefabData);
								InOutSubPrefabMap.Add(InActor, SubPrefabData);
								return;
							}
							TArray<AActor*> ChildrenActors;
							InActor->GetAttachedActors(ChildrenActors);
							for (auto ChildActor : ChildrenActors)
							{
								CollectSubPrefab(ChildActor, InOutSubPrefabMap, InPrefabHelperObject, InMapObjectToGuid);//collect all actor, include subprefab's actor
							}
						}
					};
					TMap<AActor*, FLGUISubPrefabData> SubPrefabMap;
					TMap<UObject*, FGuid> MapObjectToGuid;
					OutPrefab->SavePrefab(selectedActor, MapObjectToGuid, SubPrefabMap);//save prefab first step, just collect guid and sub prefab
					LOCAL::CollectSubPrefab(selectedActor, SubPrefabMap, PrefabHelperObjectWhichManageThisActor, MapObjectToGuid);
					for (auto& KeyValue : SubPrefabMap)
					{
						PrefabHelperObjectWhichManageThisActor->RemoveSubPrefab(KeyValue.Key);//remove prefab from origin PrefabHelperObject
					}
					OutPrefab->SavePrefab(selectedActor, MapObjectToGuid, SubPrefabMap);//save prefab second step, store sub prefab data
					OutPrefab->RefreshAgentObjectsInPreviewWorld();

					//make it as subprefab
					TMap<FGuid, UObject*> MapGuidToObject;
					for (auto KeyValue : MapObjectToGuid)
					{
						MapGuidToObject.Add(KeyValue.Value, KeyValue.Key);
					}
					PrefabHelperObjectWhichManageThisActor->MakePrefabAsSubPrefab(OutPrefab, selectedActor, MapGuidToObject, {});
					if (auto PrefabManagerActor = ALGUIPrefabManagerActor::GetPrefabManagerActorByPrefabHelperObject(PrefabHelperObjectWhichManageThisActor))
					{
						PrefabManagerActor->MarkPackageDirty();
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

void LGUIEditorTools::RefreshLevelLoadedPrefab(ULGUIPrefab* InPrefab)
{
	for (TObjectIterator<ULGUIPrefabHelperObject> Itr; Itr; ++Itr)
	{
		if (Itr->GetIsManagerObject())
		{
			if (!Itr->IsInsidePrefabEditor())
			{
				Itr->CheckPrefabVersion();
			}
		}
	}
}

void LGUIEditorTools::RefreshOpenedPrefabEditor(ULGUIPrefab* InPrefab)
{
	if (auto PrefabEditor = FLGUIPrefabEditor::GetEditorForPrefabIfValid(InPrefab))//refresh opened prefab
	{
		if (PrefabEditor->GetAnythingDirty())
		{
			auto Msg = LOCTEXT("PrefabEditorChangedDataWillLose", "Prefab editor will automaticallly refresh changed prefab, but detect some data changed in prefab editor, refresh the prefab editor will lose these data, do you want to continue?");
			auto Result = FMessageDialog::Open(EAppMsgType::YesNo, Msg);
			if (Result == EAppReturnType::Yes)
			{
				//reopen this prefab editor
				PrefabEditor->CloseWithoutCheckDataDirty();
				UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
				AssetEditorSubsystem->OpenEditorForAsset(InPrefab);
			}
		}
		else
		{
			//reopen this prefab editor
			PrefabEditor->CloseWithoutCheckDataDirty();
			UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
			AssetEditorSubsystem->OpenEditorForAsset(InPrefab);
		}
	}
}

void LGUIEditorTools::RefreshOnSubPrefabChange(ULGUIPrefab* InSubPrefab)
{
	auto AllPrefabs = GetAllPrefabArray();

	struct Local
	{
	public:
		static void RefreshAllPrefabsOnSubPrefabChange(const TArray<ULGUIPrefab*>& InPrefabs, ULGUIPrefab* InSubPrefab)
		{
			for (auto& Prefab : InPrefabs)
			{
				if (Prefab->IsPrefabBelongsToThisSubPrefab(InSubPrefab, false))
				{
					//check if is opened by prefab editor
					if (auto PrefabEditor = FLGUIPrefabEditor::GetEditorForPrefabIfValid(Prefab))//refresh opened prefab
					{
						PrefabEditor->RefreshOnSubPrefabDirty(InSubPrefab);
					}
					else
					{
						//Why comment this? Because we don't need to refresh un-opened prefab, because prefab will reload all sub prefab when open
						//if (Prefab->RefreshOnSubPrefabDirty(InSubPrefab))
						//{
						//	RefreshAllPrefabsOnSubPrefabChange(InPrefabs, Prefab);
						//}
					}
					RefreshAllPrefabsOnSubPrefabChange(InPrefabs, Prefab);
				}
			}
		}
	};

	Local::RefreshAllPrefabsOnSubPrefabChange(AllPrefabs, InSubPrefab);
}

TArray<ULGUIPrefab*> LGUIEditorTools::GetAllPrefabArray()
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

	TArray<ULGUIPrefab*> AllPrefabs;
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
				AllPrefabs.Add(Prefab);
			}
		}
	}
	//collect prefabs that are not saved to disc yet
	for (TObjectIterator<ULGUIPrefab> Itr; Itr; ++Itr)
	{
		if (!AllPrefabs.Contains(*Itr))
		{
			AllPrefabs.Add(*Itr);
		}
	}
	return AllPrefabs;
}

void LGUIEditorTools::UnpackPrefab()
{
	GEditor->BeginTransaction(FText::FromString(TEXT("LGUI UnpackPrefab")));
	auto SelectedActor = GetFirstSelectedActor();
	if (SelectedActor == nullptr)return;
	auto PrefabHelperObject = LGUIEditorTools::GetPrefabHelperObject_WhichManageThisActor(SelectedActor);
	if (PrefabHelperObject != nullptr)
	{
		check(PrefabHelperObject->SubPrefabMap.Contains(SelectedActor));//should have being checked in Unpack button
		PrefabHelperObject->RemoveSubPrefab(SelectedActor);
	}
	GEditor->EndTransaction();
	CleanupPrefabsInWorld(SelectedActor->GetWorld());
}

void LGUIEditorTools::SelectPrefabAsset()
{
	GEditor->BeginTransaction(FText::FromString(TEXT("LGUI SelectPrefabAsset")));
	auto SelectedActor = GetFirstSelectedActor();
	if (SelectedActor == nullptr)return;
	auto PrefabHelperObject = LGUIEditorTools::GetPrefabHelperObject_WhichManageThisActor(SelectedActor);
	if (PrefabHelperObject != nullptr)
	{
		check(PrefabHelperObject->SubPrefabMap.Contains(SelectedActor));//should have being checked in Browse button
		auto PrefabAsset = PrefabHelperObject->GetSubPrefabAsset(SelectedActor);
		if (IsValid(PrefabAsset))
		{
			TArray<UObject*> ObjectsToSync;
			ObjectsToSync.Add(PrefabAsset);
			GEditor->SyncBrowserToObjects(ObjectsToSync);
		}
	}
	GEditor->EndTransaction();
}

void LGUIEditorTools::OpenPrefabAsset()
{
	auto SelectedActor = GetFirstSelectedActor();
	if (SelectedActor == nullptr)return;
	auto PrefabHelperObject = LGUIEditorTools::GetPrefabHelperObject_WhichManageThisActor(SelectedActor);
	if (PrefabHelperObject != nullptr)
	{
		check(PrefabHelperObject->SubPrefabMap.Contains(SelectedActor));//should have being check in menu
		auto PrefabAsset = PrefabHelperObject->GetSubPrefabAsset(SelectedActor);
		if (IsValid(PrefabAsset))
		{
			UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
			AssetEditorSubsystem->OpenEditorForAsset(PrefabAsset);
		}
	}
}

void LGUIEditorTools::UpdateLevelPrefab()
{
	auto SelectedActor = GetFirstSelectedActor();
	if (SelectedActor == nullptr)return;
	if (auto PrefabHelperObject = LGUIEditorTools::GetPrefabHelperObject_WhichManageThisActor(SelectedActor))
	{
		if (auto SubPrefabDataPtr = PrefabHelperObject->SubPrefabMap.Find(SelectedActor))
		{
			PrefabHelperObject->RefreshOnSubPrefabDirty(SubPrefabDataPtr->PrefabAsset, SelectedActor);
		}
	}
}

ULGUIPrefabHelperObject* LGUIEditorTools::GetPrefabHelperObject_WhichManageThisActor(AActor* InActor)
{
	if (!IsValid(InActor))return nullptr;
	for (TObjectIterator<ULGUIPrefabHelperObject> Itr; Itr; ++Itr)
	{
		if (Itr->IsActorBelongsToThis(InActor))
		{
			return *Itr;
		}
	}
	return nullptr;
}

void LGUIEditorTools::CleanupPrefabsInWorld(UWorld* World)
{
	for (TActorIterator<ALGUIPrefabHelperActor> ActorItr(World); ActorItr; ++ActorItr)
	{
		auto PrefabActor = *ActorItr;
		if (IsValid(PrefabActor))
		{
			PrefabActor->bAutoDestroyLoadedActors = false;
			LGUIUtils::DestroyActorWithHierarchy(PrefabActor, false);
		}
	}
	for (TObjectIterator<ULGUIPrefabHelperObject> Itr; Itr; ++Itr)
	{
		Itr->CleanupInvalidSubPrefab();
	}
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
			if (rootUIItem->IsCanvasUIItem() && rootUIItem->GetRenderCanvas() != nullptr)
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
				GEditor->BeginTransaction(FText::FromString(TEXT("LGUI SelectPrefabAsset")));
				for (auto compItem : components)
				{
					compItem->Modify();
					compItem->SetTraceChannel(parentUIComp->GetTraceChannel());
					LGUIUtils::NotifyPropertyChanged(compItem, FName(TEXT("traceChannel")));
				}
				GEditor->EndTransaction();
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
	GEditor->BroadcastLevelActorListChanged();
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
						auto viewRotation = canvas->GetViewRotator().Quaternion();
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
			if (auto selectedActor = GetFirstSelectedActor())
			{
				if (auto selectedUIItem = Cast<AUIBaseActor>(selectedActor))
				{
					if (auto renderCavnas = selectedUIItem->GetUIItem()->GetRenderCanvas())
					{
						if (auto canvas = renderCavnas->GetRootCanvas())
						{
							if (canvas != nullptr)
							{
								editorViewportClient->SetViewLocation(canvas->GetViewLocation());
								auto viewRotation = canvas->GetViewRotator().Quaternion();
								editorViewportClient->SetViewRotation(viewRotation.Rotator());
								editorViewportClient->SetLookAtLocation(canvas->GetUIItem()->GetComponentLocation());
							}
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
#include "Core/ActorComponent/UIBaseRenderable.h"
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
				UIRenderable->SetColor(UIItem->widget_DEPRECATED.color);
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
			auto widget = UIItem->widget_DEPRECATED;
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
			//raycast complex
			if (UIRenderable)
			{
				if (UIRenderable->bRaycastComplex)
				{
					UIRenderable->SetRaycastType(EUIRenderableRaycastType::Geometry);
				}
				else
				{
					UIRenderable->SetRaycastType(EUIRenderableRaycastType::Rect);
				}
			}
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
			auto RootActor = Prefab->LoadPrefabWithExistingObjects(World, nullptr
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

	auto AllPrefabs = GetAllPrefabArray();
	for (auto Prefab : AllPrefabs)
	{
		auto World = ULGUIEditorManagerObject::GetPreviewWorldForPrefabPackage();
		TMap<FGuid, UObject*> MapGuidToObject;
		TMap<AActor*, FLGUISubPrefabData> SubPrefabMap;
		auto RootActor = Prefab->LoadPrefabWithExistingObjects(World, nullptr
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