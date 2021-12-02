// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "PrefabSystem/LGUIPrefab.h"
#include "LGUI.h"
#include "PrefabSystem/ActorSerializer.h"
#include "PrefabSystem/ActorSerializer3.h"
#include "Utils/LGUIUtils.h"
#include "Core/Actor/LGUIManagerActor.h"

#define LOCTEXT_NAMESPACE "LGUIPrefab"

#if WITH_EDITOR
void ULGUIPrefab::MakeAgentActorsInPreviewWorld()
{
	if (PrefabVersion >= 3)
	{
		if (!IsValid(AgentRootActor))
		{
			auto World = ULGUIEditorManagerObject::GetPreviewWorldForPrefabPackage();
			TMap<FGuid, UObject*> InMapGuidToObject;
			AgentRootActor = this->LoadPrefabForEdit(World, nullptr, InMapGuidToObject);
		}
	}
}
void ULGUIPrefab::ClearAgentActorsInPreviewWorld()
{
	if (IsValid(AgentRootActor))
	{
		LGUIUtils::DestroyActorWithHierarchy(AgentRootActor);
		AgentRootActor = nullptr;
	}
}

void ULGUIPrefab::BeginCacheForCookedPlatformData(const ITargetPlatform* TargetPlatform)
{
	BinaryDataForBuild.Empty();
	if (PrefabVersion >= 3)
	{
		if (!IsValid(AgentRootActor))
		{
			UE_LOG(LGUI, Error, TEXT("AgentRootActor not valid! prefab:%s"), *(this->GetPathName()));
		}
		else
		{
			this->SavePrefabForRuntime(AgentRootActor);
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
	if (PrefabVersion >= 3)
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
	if (PrefabVersion >= 3)
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
	if (PrefabVersion >= 3)
	{
		if (IsValid(AgentRootActor))
		{
			LGUIUtils::DestroyActorWithHierarchy(AgentRootActor);
		}
	}
}

#endif

AActor* ULGUIPrefab::LoadPrefab(UObject* WorldContextObject, USceneComponent* InParent, bool SetRelativeTransformToIdentity)
{
	auto world = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (PrefabVersion >= 3)
	{
		return LGUIPrefabSystem3::ActorSerializer3::LoadPrefab(world, this, InParent, SetRelativeTransformToIdentity);
	}
	else
	{
		return LGUIPrefabSystem::ActorSerializer::LoadPrefab(world, this, InParent, SetRelativeTransformToIdentity);
	}
}
AActor* ULGUIPrefab::LoadPrefabWithTransform(UObject* WorldContextObject, USceneComponent* InParent, FVector Location, FRotator Rotation, FVector Scale)
{
	auto world = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (PrefabVersion >= 3)
	{
		return LGUIPrefabSystem3::ActorSerializer3::LoadPrefab(world, this, InParent, Location, Rotation.Quaternion(), Scale);
	}
	else
	{
		return LGUIPrefabSystem::ActorSerializer::LoadPrefab(world, this, InParent, Location, Rotation.Quaternion(), Scale);
	}
}
AActor* ULGUIPrefab::LoadPrefabWithTransform(UObject* WorldContextObject, USceneComponent* InParent, FVector Location, FQuat Rotation, FVector Scale)
{
	auto world = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (PrefabVersion >= 3)
	{
		return LGUIPrefabSystem3::ActorSerializer3::LoadPrefab(world, this, InParent, Location, Rotation, Scale);
	}
	else
	{
		return LGUIPrefabSystem::ActorSerializer::LoadPrefab(world, this, InParent, Location, Rotation, Scale);
	}
}

#if WITH_EDITOR
AActor* ULGUIPrefab::LoadPrefabForEdit(UWorld* InWorld, USceneComponent* InParent
	, TMap<FGuid, UObject*>& InOutMapGuidToObject
)
{
	if (PrefabVersion >= 3)
	{
		return LGUIPrefabSystem3::ActorSerializer3::LoadPrefabForEdit(InWorld, this, InParent
			, InOutMapGuidToObject);
	}
	else
	{
		TArray<AActor*> OutCreatedActors;
		TArray<FGuid> OutCreatedActorsGuid;
		auto ResultActor = LGUIPrefabSystem::ActorSerializer::LoadPrefabForEdit(InWorld, this, InParent
			, nullptr, nullptr, OutCreatedActors, OutCreatedActorsGuid);
		for (int i = 0; i < OutCreatedActors.Num(); i++)
		{
			UE_LOG(LGUI, Warning, TEXT("Add actor:%s"), *(OutCreatedActors[i]->GetActorLabel()));
			InOutMapGuidToObject.Add(OutCreatedActorsGuid[i], OutCreatedActors[i]);
		}
		return ResultActor;
	}
}

void ULGUIPrefab::SavePrefab(AActor* RootActor
	, TMap<UObject*, FGuid>& OutMapObjectToGuid)
{
	LGUIPrefabSystem3::ActorSerializer3::SavePrefab(RootActor, this
		, OutMapObjectToGuid);
}

void ULGUIPrefab::SavePrefabForRuntime(AActor* RootActor)
{
	LGUIPrefabSystem3::ActorSerializer3::SavePrefabForRuntime(RootActor, this);
}

AActor* ULGUIPrefab::LoadPrefabInEditor(UWorld* InWorld, USceneComponent* Parent, bool SetRelativeTransformToIdentity)
{
	if (PrefabVersion >= 3)
	{
		TMap<FGuid, UObject*> MapGuidToObject;
		return LGUIPrefabSystem3::ActorSerializer3::LoadPrefabForEdit(InWorld, this
			, Parent, MapGuidToObject);
	}
	else
	{
		return LGUIPrefabSystem::ActorSerializer::LoadPrefabInEditor(InWorld, this
			, Parent);
	}
}

#endif

#undef LOCTEXT_NAMESPACE