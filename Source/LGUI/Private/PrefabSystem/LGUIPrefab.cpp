// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "PrefabSystem/LGUIPrefab.h"
#include "LGUI.h"
#include "PrefabSystem/2/ActorSerializer.h"
#include "PrefabSystem/ActorSerializer3.h"
#include "Utils/LGUIUtils.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "PrefabSystem/LGUIPrefabOverrideParameter.h"
#include "PrefabSystem/LGUIPrefabHelperObject.h"

#define LOCTEXT_NAMESPACE "LGUIPrefab"

ULGUIPrefab::ULGUIPrefab()
{
#if WITH_EDITORONLY_DATA
	if (this != GetDefault<ULGUIPrefab>())
	{
		PrefabHelperObject = CreateDefaultSubobject<ULGUIPrefabHelperObject>("PrefabHelper");
		PrefabHelperObject->bIsInsidePrefabEditor = false;
		PrefabHelperObject->PrefabAsset = this;
	}
#endif
}
#if WITH_EDITOR
void ULGUIPrefab::RefreshAgentObjectsInPreviewWorld()
{
	ClearAgentObjectsInPreviewWorld();
	MakeAgentObjectsInPreviewWorld();
}
void ULGUIPrefab::MakeAgentObjectsInPreviewWorld()
{
	if (PrefabVersion >= LGUI_PREFAB_VERSION_BuildinFArchive)
	{
		if (!PrefabHelperObject->LoadedRootActor.IsValid())
		{
			auto World = ULGUIEditorManagerObject::GetPreviewWorldForPrefabPackage();
			PrefabHelperObject->LoadPrefab(World, nullptr);
		}
	}
}
void ULGUIPrefab::ClearAgentObjectsInPreviewWorld()
{
	PrefabHelperObject->ClearLoadedPrefab();
}

void ULGUIPrefab::RefreshOnSubPrefabDirty(ULGUIPrefab* InSubPrefab)
{
	if (PrefabVersion >= LGUI_PREFAB_VERSION_BuildinFArchive)
	{
		bool AnythingChange = false;
		for (auto& KeyValue : PrefabHelperObject->SubPrefabMap)
		{
			if (KeyValue.Value.PrefabAsset == InSubPrefab)
			{
				if (KeyValue.Value.OverrideParameterObject->RefreshParameterOnTemplate(InSubPrefab->PrefabHelperObject->PrefabOverrideParameterObject))
				{
					AnythingChange = true;
				}
			}
		}
		if (AnythingChange)
		{
			TMap<TWeakObjectPtr<UObject>, FGuid> MapObjectToGuid;
			for (auto& KeyValue : PrefabHelperObject->MapGuidToObject)
			{
				if (KeyValue.Value.IsValid())
				{
					MapObjectToGuid.Add(KeyValue.Value.Get(), KeyValue.Key);
				}
			}
			TMap<TWeakObjectPtr<AActor>, FLGUISubPrefabData> TempAgentSubPrefabMap;
			for (auto& KeyValue : PrefabHelperObject->SubPrefabMap)
			{
				if (KeyValue.Key.IsValid())
				{
					TempAgentSubPrefabMap.Add(KeyValue.Key.Get(), KeyValue.Value);
				}
			}

			this->SavePrefab(PrefabHelperObject->LoadedRootActor.Get()
				, MapObjectToGuid, TempAgentSubPrefabMap
				, PrefabHelperObject->PrefabOverrideParameterObject, this->OverrideParameterData
			);

			PrefabHelperObject->MapGuidToObject.Empty();
			for (auto KeyValue : MapObjectToGuid)
			{
				PrefabHelperObject->MapGuidToObject.Add(KeyValue.Value, KeyValue.Key);
			}
			PrefabHelperObject->SubPrefabMap.Empty();
			for (auto& KeyValue : TempAgentSubPrefabMap)
			{
				PrefabHelperObject->SubPrefabMap.Add(KeyValue.Key, KeyValue.Value);
			}

			this->MarkPackageDirty();
		}
	}
}

void ULGUIPrefab::BeginCacheForCookedPlatformData(const ITargetPlatform* TargetPlatform)
{
	BinaryDataForBuild.Empty();
	if (PrefabVersion >= LGUI_PREFAB_VERSION_BuildinFArchive)
	{
		if (!PrefabHelperObject->LoadedRootActor.IsValid())
		{
			UE_LOG(LGUI, Error, TEXT("AgentObjects not valid! prefab:%s"), *(this->GetPathName()));
		}
		else
		{
			//check override parameter. although parameter is refreshed when sub prefab change, but what if sub prefab is changed outside of editor?
			bool AnythingChange = false;
			for (auto& KeyValue : PrefabHelperObject->SubPrefabMap)
			{
				if (KeyValue.Value.OverrideParameterObject->RefreshParameterOnTemplate(KeyValue.Value.PrefabAsset->PrefabHelperObject->PrefabOverrideParameterObject))
				{
					AnythingChange = true;
				}
			}
			if (AnythingChange)
			{
				UE_LOG(LGUI, Log, TEXT("[ULGUIPrefab::BeginCacheForCookedPlatformData]Something changed in sub prefab override parameter, refresh it."));
			}

			TMap<TWeakObjectPtr<UObject>, FGuid> MapObjectToGuid;
			for (auto& KeyValue : PrefabHelperObject->MapGuidToObject)
			{
				if (KeyValue.Value.IsValid())
				{
					MapObjectToGuid.Add(KeyValue.Value.Get(), KeyValue.Key);
				}
			}
			this->SavePrefab(PrefabHelperObject->LoadedRootActor.Get()
				, MapObjectToGuid, PrefabHelperObject->SubPrefabMap
				, PrefabHelperObject->PrefabOverrideParameterObject, this->OverrideParameterData
				, false
			);
			PrefabHelperObject->MapGuidToObject.Empty();
			for (auto KeyValue : MapObjectToGuid)
			{
				PrefabHelperObject->MapGuidToObject.Add(KeyValue.Value, KeyValue.Key);
			}
		}
	}
	else
	{
		LGUIPrefabSystem::ActorSerializer::ConvertForBuildData(this);
	}
}
void ULGUIPrefab::WillNeverCacheCookedPlatformDataAgain()
{
	BinaryDataForBuild.Empty();
}
void ULGUIPrefab::ClearCachedCookedPlatformData(const ITargetPlatform* TargetPlatform)
{
	BinaryDataForBuild.Empty();
}

void ULGUIPrefab::PostInitProperties()
{
	Super::PostInitProperties();
#if WITH_EDITOR
	if (this != GetDefault<ULGUIPrefab>())
	{
		if (ULGUIEditorManagerObject::Instance == nullptr)//if LGUIEditorManager is not valid, means engine could be not valid too, so add to a temporary list, wait for it valid and do the stuff
		{
			ULGUIEditorManagerObject::AddPrefabForGenerateAgent(this);
		}
		else//LGUIEditorManager is valid, means everything is good to make the agent objects
		{
			MakeAgentObjectsInPreviewWorld();
		}
	}
#endif
}
void ULGUIPrefab::PostCDOContruct()
{
	Super::PostCDOContruct();
}
bool ULGUIPrefab::PreSaveRoot(const TCHAR* Filename)
{
	return Super::PreSaveRoot(Filename);
}
void ULGUIPrefab::PostSaveRoot(bool bCleanupIsRequired)
{
	Super::PostSaveRoot(bCleanupIsRequired);
}
void ULGUIPrefab::PreSave(const class ITargetPlatform* TargetPlatform)
{
	Super::PreSave(TargetPlatform);
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
		//(should generate new guid for all object inside prefab)
		//previoursly I write the line upper, but after a while I realize: two prefab won't share same MapObjectToGuid, even for nested prefab( sub prefab only get root actor in parent's guid-map, and not the same guid inside sub prefab), so we don't need do the upper line.
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
	if (IsValid(PrefabHelperObject))
	{
		ClearAgentObjectsInPreviewWorld();
		PrefabHelperObject->ConditionalBeginDestroy();
	}
	Super::BeginDestroy();
}

void ULGUIPrefab::FinishDestroy()
{
	Super::FinishDestroy();
}

#endif

AActor* ULGUIPrefab::LoadPrefab(UWorld* InWorld, USceneComponent* InParent, bool SetRelativeTransformToIdentity)
{
	AActor* LoadedRootActor = nullptr;
	if (PrefabVersion >= LGUI_PREFAB_VERSION_BuildinFArchive)
	{
		LoadedRootActor = LGUIPrefabSystem3::ActorSerializer3::LoadPrefab(InWorld, this, InParent, SetRelativeTransformToIdentity);
	}
	else
	{
		LoadedRootActor = LGUIPrefabSystem::ActorSerializer::LoadPrefab(InWorld, this, InParent, SetRelativeTransformToIdentity);
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
		if (PrefabVersion >= LGUI_PREFAB_VERSION_BuildinFArchive)
		{
			LoadedRootActor = LGUIPrefabSystem3::ActorSerializer3::LoadPrefab(World, this, InParent, Location, Rotation.Quaternion(), Scale);
		}
		else
		{
			LoadedRootActor = LGUIPrefabSystem::ActorSerializer::LoadPrefab(World, this, InParent, Location, Rotation.Quaternion(), Scale);
		}
	}
	return LoadedRootActor;
}
AActor* ULGUIPrefab::LoadPrefabWithTransform(UObject* WorldContextObject, USceneComponent* InParent, FVector Location, FQuat Rotation, FVector Scale)
{
	AActor* LoadedRootActor = nullptr;
	auto World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (PrefabVersion >= LGUI_PREFAB_VERSION_BuildinFArchive)
	{
		LoadedRootActor = LGUIPrefabSystem3::ActorSerializer3::LoadPrefab(World, this, InParent, Location, Rotation, Scale);
	}
	else
	{
		LoadedRootActor = LGUIPrefabSystem::ActorSerializer::LoadPrefab(World, this, InParent, Location, Rotation, Scale);
	}
	return LoadedRootActor;
}

#if WITH_EDITOR
AActor* ULGUIPrefab::LoadPrefabForEdit(UWorld* InWorld, USceneComponent* InParent
	, TMap<FGuid, TWeakObjectPtr<UObject>>& InOutMapGuidToObject, TMap<TWeakObjectPtr<AActor>, FLGUISubPrefabData>& OutSubPrefabMap
	, const TArray<uint8>& InOverrideParameterData, ULGUIPrefabOverrideParameterObject*& OutOverrideParameterObject
)
{
	AActor* LoadedRootActor = nullptr;
	if (PrefabVersion >= LGUI_PREFAB_VERSION_BuildinFArchive)
	{
		LoadedRootActor = LGUIPrefabSystem3::ActorSerializer3::LoadPrefabForEdit(InWorld, this, InParent
			, InOutMapGuidToObject, OutSubPrefabMap
			, InOverrideParameterData, OutOverrideParameterObject
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
			UE_LOG(LGUI, Warning, TEXT("Add actor:%s"), *(OutCreatedActors[i]->GetActorLabel()));
			InOutMapGuidToObject.Add(OutCreatedActorsGuid[i], OutCreatedActors[i]);
		}
	}
	return LoadedRootActor;
}

void ULGUIPrefab::SavePrefab(AActor* RootActor
	, TMap<TWeakObjectPtr<UObject>, FGuid>& InOutMapObjectToGuid, TMap<TWeakObjectPtr<AActor>, FLGUISubPrefabData>& InSubPrefabMap
	, ULGUIPrefabOverrideParameterObject* InOverrideParameterObject, TArray<uint8>& OutOverrideParameterData
	, bool InForEditorOrRuntimeUse
)
{
	LGUIPrefabSystem3::ActorSerializer3::SavePrefab(RootActor, this
		, InOutMapObjectToGuid, InSubPrefabMap
		, InOverrideParameterObject, OutOverrideParameterData
		, InForEditorOrRuntimeUse
	);
}

AActor* ULGUIPrefab::LoadPrefabInEditor(UWorld* InWorld, USceneComponent* InParent, bool SetRelativeTransformToIdentity)
{
	AActor* LoadedRootActor = nullptr;
	if (PrefabVersion >= LGUI_PREFAB_VERSION_BuildinFArchive)
	{
		TMap<FGuid, TWeakObjectPtr<UObject>> MapGuidToObject;
		TMap<TWeakObjectPtr<AActor>, FLGUISubPrefabData> SubPrefabMap;
		TArray<uint8> TempOverrideParameterData;
		ULGUIPrefabOverrideParameterObject* OverrideParameterObject = nullptr;
		LoadedRootActor = LGUIPrefabSystem3::ActorSerializer3::LoadPrefabForEdit(InWorld, this
			, InParent, MapGuidToObject, SubPrefabMap
			, TempOverrideParameterData, OverrideParameterObject
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