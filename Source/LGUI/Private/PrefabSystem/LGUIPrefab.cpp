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
			TMap<FGuid, UObject*> InMapGuidToObject;
			TArray<uint8> TempOverrideParameterData;
			ULGUIPrefabOverrideParameterObject* OverrideParameterObject = nullptr;
			AgentRootActor = this->LoadPrefabForEdit(World, nullptr
				, InMapGuidToObject, AgentSubPrefabmap
				, TempOverrideParameterData, OverrideParameterObject
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
			this->SavePrefabForRuntime(AgentRootActor, AgentSubPrefabmap);
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
	//recreate AgentRootActor, because the prefab data could change.
	if (PrefabVersion >= LGUI_PREFAB_VERSION_BuildinFArchive)
	{
		//if (IsValid(AgentRootActor))
		//{
		//	LGUIUtils::DestroyActorWithHierarchy(AgentRootActor);
		//}
		//MakeAgentActorsInPreviewWorld();
		//check(IsValid(AgentRootActor));
	}
}
void ULGUIPrefab::PostDuplicate(bool bDuplicateForPIE)
{
	if (PrefabVersion >= LGUI_PREFAB_VERSION_BuildinFArchive)
	{
		
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
)
{
	LGUIPrefabSystem3::ActorSerializer3::SavePrefab(RootActor, this
		, InOutMapObjectToGuid, InSubPrefabMap
		, InOverrideParameterObject, OutOverrideParameterData
	);
}

void ULGUIPrefab::SavePrefabForRuntime(AActor* RootActor, TMap<AActor*, FLGUISubPrefabData>& InSubPrefabMap)
{
	LGUIPrefabSystem3::ActorSerializer3::SavePrefabForRuntime(RootActor, this, InSubPrefabMap);
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