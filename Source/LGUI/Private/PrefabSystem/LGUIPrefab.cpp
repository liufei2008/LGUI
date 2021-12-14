// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "PrefabSystem/LGUIPrefab.h"
#include "LGUI.h"
#include "PrefabSystem/2/ActorSerializer.h"
#include "PrefabSystem/ActorSerializer3.h"
#include "Utils/LGUIUtils.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "PrefabSystem/LGUIPrefabOverrideParameter.h"

#define LOCTEXT_NAMESPACE "LGUIPrefab"

#if WITH_EDITOR
void ULGUIPrefab::RefreshAgentActorsInPreviewWorld()
{
	ClearAgentActorsInPreviewWorld();
	MakeAgentActorsInPreviewWorld();
}
void ULGUIPrefab::MakeAgentActorsInPreviewWorld()
{
	if (PrefabVersion >= LGUI_PREFAB_VERSION_BuildinFArchive)
	{
		if (!IsValid(AgentRootActor))
		{
			auto World = ULGUIEditorManagerObject::GetPreviewWorldForPrefabPackage();
			AgentRootActor = this->LoadPrefabForEdit(World, nullptr
				, AgentMapGuidToObject, AgentSubPrefabmap
				, this->OverrideParameterData, AgentOverrideParameterObject
			);
		}
	}
}
void ULGUIPrefab::ClearAgentActorsInPreviewWorld()
{
	if (IsValid(AgentRootActor))
	{
		LGUIUtils::DestroyActorWithHierarchy(AgentRootActor);
		AgentRootActor = nullptr;
		AgentSubPrefabmap.Empty();
		AgentMapGuidToObject.Empty();
		AgentOverrideParameterObject->ConditionalBeginDestroy();
		AgentOverrideParameterObject = nullptr;
	}
}

void ULGUIPrefab::RefreshOnSubPrefabDirty(ULGUIPrefab* InSubPrefab)
{
	if (PrefabVersion >= LGUI_PREFAB_VERSION_BuildinFArchive)
	{
		bool AnythingChange = false;
		if (ReferenceAssetList.Contains(InSubPrefab))
		{
			for (auto& KeyValue : AgentSubPrefabmap)
			{
				if (KeyValue.Value.PrefabAsset == InSubPrefab)
				{
					if (KeyValue.Value.OverrideParameterObject->RefreshParameterOnTemplate(InSubPrefab->AgentOverrideParameterObject))
					{
						AnythingChange = true;
					}
				}
			}
		}
		if (AnythingChange)
		{
			TMap<UObject*, FGuid> MapObjectToGuid;
			for (auto& KeyValue : AgentMapGuidToObject)
			{
				if (IsValid(KeyValue.Value))
				{
					MapObjectToGuid.Add(KeyValue.Value, KeyValue.Key);
				}
			}
			this->SavePrefab(AgentRootActor
				, MapObjectToGuid, AgentSubPrefabmap
				, AgentOverrideParameterObject, this->OverrideParameterData
			);
			AgentMapGuidToObject.Empty();
			for (auto KeyValue : MapObjectToGuid)
			{
				AgentMapGuidToObject.Add(KeyValue.Value, KeyValue.Key);
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
		if (!IsValid(AgentRootActor))
		{
			UE_LOG(LGUI, Error, TEXT("AgentRootActor not valid! prefab:%s"), *(this->GetPathName()));
		}
		else
		{
			//check override parameter. although parameter is refreshed when sub prefab change, but what if sub prefab is changed outside of editor?
			bool AnythingChange = false;
			for (auto& KeyValue : AgentSubPrefabmap)
			{
				if (KeyValue.Value.OverrideParameterObject->RefreshParameterOnTemplate(KeyValue.Value.PrefabAsset->AgentOverrideParameterObject))
				{
					AnythingChange = true;
				}
			}
			if (AnythingChange)
			{
				UE_LOG(LGUI, Log, TEXT("[ULGUIPrefab::BeginCacheForCookedPlatformData]Something changed in sub prefab override parameter, refresh it."));
			}

			TMap<UObject*, FGuid> MapObjectToGuid;
			for (auto& KeyValue : AgentMapGuidToObject)
			{
				if (IsValid(KeyValue.Value))
				{
					MapObjectToGuid.Add(KeyValue.Value, KeyValue.Key);
				}
			}
			this->SavePrefab(AgentRootActor
				, MapObjectToGuid, AgentSubPrefabmap
				, AgentOverrideParameterObject, this->OverrideParameterData
				, false
			);
			AgentMapGuidToObject.Empty();
			for (auto KeyValue : MapObjectToGuid)
			{
				AgentMapGuidToObject.Add(KeyValue.Value, KeyValue.Key);
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
void ULGUIPrefab::PostSaveRoot(bool bCleanupIsRequired)
{
	Super::PostSaveRoot(bCleanupIsRequired);
}
void ULGUIPrefab::PostDuplicate(bool bDuplicateForPIE)
{
	if (PrefabVersion >= LGUI_PREFAB_VERSION_BuildinFArchive)
	{
		//(should generate new guid for all object inside prefab)
		//previoursly I write the line upper, but after a while I realize: two prefab won't share same MapObjectToGuid, event for nested prefab( sub prefab only get root actor in parent's guid-map, and not the same guid inside sub prefab), so we don't need do the upper line.
	}
	else
	{
		//generate new guid for actor data
		LGUIPrefabSystem::ActorSerializer::RenewActorGuidForDuplicate(this);
	}
}
void ULGUIPrefab::BeginDestroy()
{
	Super::BeginDestroy();
	if (PrefabVersion >= LGUI_PREFAB_VERSION_BuildinFArchive)
	{
		if (IsValid(AgentRootActor))
		{
			LGUIUtils::DestroyActorWithHierarchy(AgentRootActor);
		}
	}
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
	, TMap<FGuid, UObject*>& InOutMapGuidToObject, TMap<AActor*, FLGUISubPrefabData>& OutSubPrefabMap
	, const TArray<uint8>& InOverrideParameterData, class ULGUIPrefabOverrideParameterObject*& OutOverrideParameterObject
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
	, TMap<UObject*, FGuid>& InOutMapObjectToGuid, TMap<AActor*, FLGUISubPrefabData>& InSubPrefabMap
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
		TMap<FGuid, UObject*> MapGuidToObject;
		TMap<AActor*, FLGUISubPrefabData> SubPrefabMap;
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