// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "PrefabSystem/LGUIPrefab.h"
#include "LGUI.h"
#include "PrefabSystem/2/ActorSerializer.h"
#include "PrefabSystem/ActorSerializer3.h"
#include "PrefabSystem/ActorSerializer4.h"
#include "Utils/LGUIUtils.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "PrefabSystem/LGUIPrefabHelperObject.h"
#include "Core/Actor/UIBaseActor.h"
#include "Core/Actor/UIContainerActor.h"

#define LOCTEXT_NAMESPACE "LGUIPrefab"


void FLGUISubPrefabData::AddMemberProperty(UObject* InObject, FName InPropertyName)
{
	auto Index = ObjectOverrideParameterArray.IndexOfByPredicate([=](const FLGUIPrefabOverrideParameterData& Item) {
		return Item.Object == InObject;
		});
	if (Index == INDEX_NONE)
	{
		FLGUIPrefabOverrideParameterData DataItem;
		DataItem.Object = InObject;
		DataItem.MemberPropertyName.Add(InPropertyName);
		ObjectOverrideParameterArray.Add(DataItem);
	}
	else
	{
		auto& DataItem = ObjectOverrideParameterArray[Index];
		if (!DataItem.MemberPropertyName.Contains(InPropertyName))
		{
			DataItem.MemberPropertyName.Add(InPropertyName);
		}
	}
}

void FLGUISubPrefabData::AddMemberProperty(UObject* InObject, const TSet<FName>& InPropertyNameSet)
{
	auto Index = ObjectOverrideParameterArray.IndexOfByPredicate([=](const FLGUIPrefabOverrideParameterData& Item) {
		return Item.Object == InObject;
		});
	if (Index == INDEX_NONE)
	{
		FLGUIPrefabOverrideParameterData DataItem;
		DataItem.Object = InObject;
		DataItem.MemberPropertyName = InPropertyNameSet;
		ObjectOverrideParameterArray.Add(DataItem);
	}
	else
	{
		auto& DataItem = ObjectOverrideParameterArray[Index];
		for (auto& NameItem : InPropertyNameSet)
		{
			if (!DataItem.MemberPropertyName.Contains(NameItem))
			{
				DataItem.MemberPropertyName.Add(NameItem);
			}
		}
	}
}

void FLGUISubPrefabData::RemoveMemberProperty(UObject* InObject, FName InPropertyName)
{
	auto Index = ObjectOverrideParameterArray.IndexOfByPredicate([=](const FLGUIPrefabOverrideParameterData& Item) {
		return Item.Object == InObject;
		});
	if (Index != INDEX_NONE)
	{
		auto& DataItem = ObjectOverrideParameterArray[Index];
		if (DataItem.MemberPropertyName.Contains(InPropertyName))
		{
			DataItem.MemberPropertyName.Remove(InPropertyName);
		}
		if (DataItem.MemberPropertyName.Num() <= 0)
		{
			ObjectOverrideParameterArray.RemoveAt(Index);
		}
	}
}

void FLGUISubPrefabData::RemoveMemberProperty(UObject* InObject)
{
	auto Index = ObjectOverrideParameterArray.IndexOfByPredicate([=](const FLGUIPrefabOverrideParameterData& Item) {
		return Item.Object == InObject;
		});
	if (Index != INDEX_NONE)
	{
		ObjectOverrideParameterArray.RemoveAt(Index);
	}
}

bool FLGUISubPrefabData::CheckParameters()
{
	bool AnythingChanged = false;
	TSet<int> ObjectsNeedToRemove;
	for (int i = 0; i < ObjectOverrideParameterArray.Num(); i++)
	{
		auto DataItem = ObjectOverrideParameterArray[i];
		if (!DataItem.Object.IsValid())
		{
			ObjectsNeedToRemove.Add(i);
		}
		else
		{
			TSet<FName> PropertyNamesToRemove;
			auto Object = DataItem.Object;
			for (auto PropertyName : DataItem.MemberPropertyName)
			{
				auto Property = FindFProperty<FProperty>(Object->GetClass(), PropertyName);
				if (Property == nullptr)
				{
					PropertyNamesToRemove.Add(PropertyName);
				}
			}
			for (auto PropertyName : PropertyNamesToRemove)
			{
				DataItem.MemberPropertyName.Remove(PropertyName);
				AnythingChanged = true;
			}
		}
	}
	for (auto Index : ObjectsNeedToRemove)
	{
		ObjectOverrideParameterArray.RemoveAt(Index);
		AnythingChanged = true;
	}
	return AnythingChanged;
}

ULGUIPrefab::ULGUIPrefab()
{

}
#if WITH_EDITOR
AActor* ULGUIPrefab::GetContainerActor()
{
	if (ContainerActor.IsValid())return ContainerActor.Get();
	auto World = ULGUIEditorManagerObject::GetPreviewWorldForPrefabPackage();
	if (ReferenceClassList.Num() > 0 && ReferenceClassList[0]->IsChildOf(AUIBaseActor::StaticClass()))
	{
		auto RootUICanvasActor = (AUIContainerActor*)(World->SpawnActor<AActor>(AUIContainerActor::StaticClass(), FTransform::Identity));
		RootUICanvasActor->GetRootComponent()->SetWorldLocationAndRotationNoPhysics(FVector::ZeroVector, FRotator(0, 0, 0));

		RootUICanvasActor->GetUIItem()->SetWidth(1920);
		RootUICanvasActor->GetUIItem()->SetHeight(1080);
		ContainerActor = RootUICanvasActor;
	}
	else
	{
		auto RootUICanvasActor = World->SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
		ContainerActor = RootUICanvasActor;
	}
	return ContainerActor.Get();
}
void ULGUIPrefab::RefreshAgentObjectsInPreviewWorld()
{
	ClearAgentObjectsInPreviewWorld();
	MakeAgentObjectsInPreviewWorld();
}
void ULGUIPrefab::MakeAgentObjectsInPreviewWorld()
{
	if (PrefabVersion >= LGUI_PREFAB_VERSION_BuildinFArchive)
	{
		if (PrefabHelperObject == nullptr)
		{
			PrefabHelperObject = NewObject<ULGUIPrefabHelperObject>(this, "PrefabHelper");
			PrefabHelperObject->PrefabAsset = this;
		}
		if (!IsValid(PrefabHelperObject->LoadedRootActor))
		{
			auto World = ULGUIEditorManagerObject::GetPreviewWorldForPrefabPackage();
			PrefabHelperObject->LoadPrefab(World, GetContainerActor()->GetRootComponent());
		}
	}
}
void ULGUIPrefab::ClearAgentObjectsInPreviewWorld()
{
	if (PrefabHelperObject != nullptr)
	{
		PrefabHelperObject->ClearLoadedPrefab();
	}
}

ULGUIPrefabHelperObject* ULGUIPrefab::GetPrefabHelperObject()
{
	if (PrefabHelperObject == nullptr)
	{
		PrefabHelperObject = NewObject<ULGUIPrefabHelperObject>(this, "PrefabHelper");
		PrefabHelperObject->PrefabAsset = this;
	}
	if (!IsValid(PrefabHelperObject->LoadedRootActor))
	{
		auto World = ULGUIEditorManagerObject::GetPreviewWorldForPrefabPackage();
		PrefabHelperObject->LoadPrefab(World, nullptr);
	}
	return PrefabHelperObject;
}

void ULGUIPrefab::BeginCacheForCookedPlatformData(const ITargetPlatform* TargetPlatform)
{
	BinaryDataForBuild.Empty();
	if (!IsValid(PrefabHelperObject) || !IsValid(PrefabHelperObject->LoadedRootActor))
	{
		UE_LOG(LGUI, Warning, TEXT("AgentObjects not valid, recreate it! prefab: '%s'"), *(this->GetPathName()));
		MakeAgentObjectsInPreviewWorld();
	}
	//serialize to runtime data
	{
		//check override parameter. although parameter is refreshed when sub prefab change, but what if sub prefab is changed outside of editor?
		bool AnythingChange = false;
		for (auto& KeyValue : PrefabHelperObject->SubPrefabMap)
		{
			if (KeyValue.Value.CheckParameters())
			{
				AnythingChange = true;
			}
		}
		if (AnythingChange)
		{
			UE_LOG(LGUI, Log, TEXT("[ULGUIPrefab::BeginCacheForCookedPlatformData]Something changed in sub prefab override parameter, refresh it. prefab: '%s'."), *(this->GetPathName()));
		}

		TMap<UObject*, FGuid> MapObjectToGuid;
		for (auto& KeyValue : PrefabHelperObject->MapGuidToObject)
		{
			if (IsValid(KeyValue.Value))
			{
				MapObjectToGuid.Add(KeyValue.Value, KeyValue.Key);
			}
		}
		this->SavePrefab(PrefabHelperObject->LoadedRootActor
			, MapObjectToGuid, PrefabHelperObject->SubPrefabMap
			, false
		);
		PrefabHelperObject->MapGuidToObject.Empty();
		for (auto KeyValue : MapObjectToGuid)
		{
			PrefabHelperObject->MapGuidToObject.Add(KeyValue.Value, KeyValue.Key);
		}
	}
}
void ULGUIPrefab::WillNeverCacheCookedPlatformDataAgain()
{
	if (PrefabVersion >= LGUI_PREFAB_VERSION_BuildinFArchive)
	{
		BinaryDataForBuild.Empty();
		ReferenceAssetListForBuild.Empty();
		ReferenceClassListForBuild.Empty();
		ReferenceNameListForBuild.Empty();
	}
}
void ULGUIPrefab::ClearCachedCookedPlatformData(const ITargetPlatform* TargetPlatform)
{
	if (PrefabVersion >= LGUI_PREFAB_VERSION_BuildinFArchive)
	{
		BinaryDataForBuild.Empty();
		ReferenceAssetListForBuild.Empty();
		ReferenceClassListForBuild.Empty();
		ReferenceNameListForBuild.Empty();
	}
}

void ULGUIPrefab::PostInitProperties()
{
	Super::PostInitProperties();
}
void ULGUIPrefab::PostCDOContruct()
{
	Super::PostCDOContruct();
}

void ULGUIPrefab::PostRename(UObject* OldOuter, const FName OldName)
{
	Super::PostRename(OldOuter, OldName);
}
void ULGUIPrefab::PreDuplicate(FObjectDuplicationParameters& DupParams)
{
	Super::PreDuplicate(DupParams);
}

void ULGUIPrefab::PostDuplicate(bool bDuplicateForPIE)
{
	Super::PostDuplicate(bDuplicateForPIE);
	if (PrefabVersion >= LGUI_PREFAB_VERSION_BuildinFArchive)
	{

	}
	else
	{
		//generate new guid for actor data
		LGUIPrefabSystem::ActorSerializer::RenewActorGuidForDuplicate(this);
	}
}

void ULGUIPrefab::PostLoad()
{
	Super::PostLoad();
}

void ULGUIPrefab::BeginDestroy()
{
#if WITH_EDITOR
	if (IsValid(PrefabHelperObject))
	{
		ClearAgentObjectsInPreviewWorld();
		PrefabHelperObject->ConditionalBeginDestroy();
	}
#endif
	Super::BeginDestroy();
}

void ULGUIPrefab::FinishDestroy()
{
	Super::FinishDestroy();
}

void ULGUIPrefab::PostEditUndo()
{
	Super::PostEditUndo();
	RefreshAgentObjectsInPreviewWorld();
}

#endif

AActor* ULGUIPrefab::LoadPrefab(UWorld* InWorld, USceneComponent* InParent, bool SetRelativeTransformToIdentity)
{
	AActor* LoadedRootActor = nullptr;
	if (InWorld)
	{
#if WITH_EDITOR
		if (PrefabVersion >= LGUI_PREFAB_VERSION_NestedDefaultSubObject)
		{
			LoadedRootActor = LGUIPrefabSystem4::ActorSerializer::LoadPrefab(InWorld, this, InParent, SetRelativeTransformToIdentity);
		}
		else if (PrefabVersion >= LGUI_PREFAB_VERSION_BuildinFArchive)
		{
			LoadedRootActor = LGUIPrefabSystem3::ActorSerializer::LoadPrefab(InWorld, this, InParent, SetRelativeTransformToIdentity);
		}
		else
		{
			LoadedRootActor = LGUIPrefabSystem::ActorSerializer::LoadPrefab(InWorld, this, InParent, SetRelativeTransformToIdentity);
		}
#else
		LoadedRootActor = LGUIPrefabSystem4::ActorSerializer::LoadPrefab(InWorld, this, InParent, SetRelativeTransformToIdentity);
#endif
	}
	return LoadedRootActor;
}

AActor* ULGUIPrefab::LoadPrefab(UObject* WorldContextObject, USceneComponent* InParent, bool SetRelativeTransformToIdentity)
{
	auto World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (World)
	{
		return LoadPrefab(World, InParent, SetRelativeTransformToIdentity);
	}
	return nullptr;
}
AActor* ULGUIPrefab::LoadPrefabWithTransform(UObject* WorldContextObject, USceneComponent* InParent, FVector Location, FRotator Rotation, FVector Scale)
{
	AActor* LoadedRootActor = nullptr;
	auto World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (World)
	{
#if WITH_EDITOR
		if (PrefabVersion >= LGUI_PREFAB_VERSION_NestedDefaultSubObject)
		{
			LoadedRootActor = LGUIPrefabSystem4::ActorSerializer::LoadPrefab(World, this, InParent, Location, Rotation.Quaternion(), Scale);
		}
		else if (PrefabVersion >= LGUI_PREFAB_VERSION_BuildinFArchive)
		{
			LoadedRootActor = LGUIPrefabSystem3::ActorSerializer::LoadPrefab(World, this, InParent, Location, Rotation.Quaternion(), Scale);
		}
		else
		{
			LoadedRootActor = LGUIPrefabSystem::ActorSerializer::LoadPrefab(World, this, InParent, Location, Rotation.Quaternion(), Scale);
		}
#else
		LoadedRootActor = LGUIPrefabSystem4::ActorSerializer::LoadPrefab(World, this, InParent, Location, Rotation.Quaternion(), Scale);
#endif
	}
	return LoadedRootActor;
}
AActor* ULGUIPrefab::LoadPrefabWithTransform(UObject* WorldContextObject, USceneComponent* InParent, FVector Location, FQuat Rotation, FVector Scale)
{
	AActor* LoadedRootActor = nullptr;
	auto World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (World)
	{
#if WITH_EDITOR
		if (PrefabVersion >= LGUI_PREFAB_VERSION_NestedDefaultSubObject)
		{
			LoadedRootActor = LGUIPrefabSystem4::ActorSerializer::LoadPrefab(World, this, InParent, Location, Rotation, Scale);
		}
		else if (PrefabVersion >= LGUI_PREFAB_VERSION_BuildinFArchive)
		{
			LoadedRootActor = LGUIPrefabSystem3::ActorSerializer::LoadPrefab(World, this, InParent, Location, Rotation, Scale);
		}
		else
		{
			LoadedRootActor = LGUIPrefabSystem::ActorSerializer::LoadPrefab(World, this, InParent, Location, Rotation, Scale);
		}
#else
		LoadedRootActor = LGUIPrefabSystem4::ActorSerializer::LoadPrefab(World, this, InParent, Location, Rotation, Scale);
#endif
	}
	return LoadedRootActor;
}

#if WITH_EDITOR
AActor* ULGUIPrefab::LoadPrefabWithExistingObjects(UWorld* InWorld, USceneComponent* InParent
	, TMap<FGuid, UObject*>& InOutMapGuidToObject, TMap<AActor*, FLGUISubPrefabData>& OutSubPrefabMap
	, bool InSetHierarchyIndexForRootComponent
)
{
	AActor* LoadedRootActor = nullptr;
	if (PrefabVersion >= LGUI_PREFAB_VERSION_NestedDefaultSubObject)
	{
		LoadedRootActor = LGUIPrefabSystem4::ActorSerializer::LoadPrefabWithExistingObjects(InWorld, this, InParent
			, InOutMapGuidToObject, OutSubPrefabMap
			, InSetHierarchyIndexForRootComponent
		);
	}
	else if (PrefabVersion >= LGUI_PREFAB_VERSION_BuildinFArchive)
	{
		LoadedRootActor = LGUIPrefabSystem3::ActorSerializer::LoadPrefabWithExistingObjects(InWorld, this, InParent
			, InOutMapGuidToObject, OutSubPrefabMap
			, InSetHierarchyIndexForRootComponent
		);
	}
	else
	{
		TArray<AActor*> OutCreatedActors;
		TArray<FGuid> OutCreatedActorsGuid;
		LoadedRootActor = LGUIPrefabSystem::ActorSerializer::LoadPrefabForEdit(InWorld, this, InParent
			, nullptr, nullptr, OutCreatedActors, OutCreatedActorsGuid);
		for (int i = 0; i < OutCreatedActors.Num(); i++)
		{
			InOutMapGuidToObject.Add(OutCreatedActorsGuid[i], OutCreatedActors[i]);
		}
	}
	return LoadedRootActor;
}

bool ULGUIPrefab::IsPrefabBelongsToThisSubPrefab(ULGUIPrefab* InPrefab, bool InRecursive)
{
	MakeAgentObjectsInPreviewWorld();
	if (!PrefabHelperObject)return false;
	if (this == InPrefab)return false;
	for (auto& KeyValue : PrefabHelperObject->SubPrefabMap)
	{
		if (KeyValue.Value.PrefabAsset == InPrefab)
		{
			return true;
		}
	}
	if (InRecursive)
	{
		for (auto& KeyValue : PrefabHelperObject->SubPrefabMap)
		{
			if (KeyValue.Value.PrefabAsset->IsPrefabBelongsToThisSubPrefab(InPrefab, InRecursive))
			{
				return true;
			}
		}
	}
	return false;
}

void ULGUIPrefab::SavePrefab(AActor* RootActor
	, TMap<UObject*, FGuid>& InOutMapObjectToGuid, TMap<AActor*, FLGUISubPrefabData>& InSubPrefabMap
	, bool InForEditorOrRuntimeUse
)
{
	LGUIPrefabSystem4::ActorSerializer::SavePrefab(RootActor, this
		, InOutMapObjectToGuid, InSubPrefabMap
		, InForEditorOrRuntimeUse
	);
}

AActor* ULGUIPrefab::LoadPrefabInEditor(UWorld* InWorld, USceneComponent* InParent, bool SetRelativeTransformToIdentity)
{
	AActor* LoadedRootActor = nullptr;
	if (PrefabVersion >= LGUI_PREFAB_VERSION_NestedDefaultSubObject)
	{
		TMap<FGuid, UObject*> MapGuidToObject;
		TMap<AActor*, FLGUISubPrefabData> SubPrefabMap;
		LoadedRootActor = LGUIPrefabSystem4::ActorSerializer::LoadPrefabWithExistingObjects(InWorld, this
			, InParent, MapGuidToObject, SubPrefabMap
		);
	}
	else if (PrefabVersion >= LGUI_PREFAB_VERSION_BuildinFArchive)
	{
		TMap<FGuid, UObject*> MapGuidToObject;
		TMap<AActor*, FLGUISubPrefabData> SubPrefabMap;
		LoadedRootActor = LGUIPrefabSystem3::ActorSerializer::LoadPrefabWithExistingObjects(InWorld, this
			, InParent, MapGuidToObject, SubPrefabMap
		);
	}
	else
	{
		LoadedRootActor = LGUIPrefabSystem::ActorSerializer::LoadPrefabInEditor(InWorld, this
			, InParent);
	}
	return LoadedRootActor;
}

AActor* ULGUIPrefab::LoadPrefabInEditor(UWorld* InWorld, USceneComponent* InParent, TMap<AActor*, FLGUISubPrefabData>& OutSubPrefabMap, TMap<FGuid, UObject*>& OutMapGuidToObject, bool SetRelativeTransformToIdentity)
{
	AActor* LoadedRootActor = nullptr;
	if (PrefabVersion >= LGUI_PREFAB_VERSION_NestedDefaultSubObject)
	{
		LoadedRootActor = LGUIPrefabSystem4::ActorSerializer::LoadPrefabWithExistingObjects(InWorld, this
			, InParent, OutMapGuidToObject, OutSubPrefabMap
		);
	}
	else if (PrefabVersion >= LGUI_PREFAB_VERSION_BuildinFArchive)
	{
		LoadedRootActor = LGUIPrefabSystem3::ActorSerializer::LoadPrefabWithExistingObjects(InWorld, this
			, InParent, OutMapGuidToObject, OutSubPrefabMap
		);
	}
	else
	{
		LoadedRootActor = LGUIPrefabSystem::ActorSerializer::LoadPrefabInEditor(InWorld, this
			, InParent);
	}
	return LoadedRootActor;
}

#endif

#undef LOCTEXT_NAMESPACE