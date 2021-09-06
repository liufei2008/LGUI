// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "PrefabSystem/LGUIPrefab.h"
#include "LGUI.h"
#include "PrefabSystem/ActorSerializer.h"

#if WITH_EDITOR
void ULGUIPrefab::BeginCacheForCookedPlatformData(const ITargetPlatform* TargetPlatform)
{
	BinaryDataForBuild.Empty();
	LGUIPrefabSystem::ActorSerializer::ConvertForBuildData(this);
}
void ULGUIPrefab::WillNeverCacheCookedPlatformDataAgain()
{
	BinaryDataForBuild.Empty();
}
void ULGUIPrefab::ClearCachedCookedPlatformData(const ITargetPlatform* TargetPlatform)
{
	BinaryDataForBuild.Empty();
}
void ULGUIPrefab::PreSave(const ITargetPlatform* TargetPlatform)
{
	Super::PreSave(TargetPlatform);
}
void ULGUIPrefab::PostDuplicate(bool bDuplicateForPIE)
{
	//generate new guid for actor data
	//@todo: support nested prefab
	LGUIPrefabSystem::ActorSerializer::RenewActorGuidForDuplicate(this);
}

bool ULGUIPrefab::ContainsSubPrefab(ULGUIPrefab* InPrefab, bool InRecursive, int32& OutDepth)
{
	OutDepth++;
	for (auto SubPrefabKeyValue : SubPrefabs)
	{
		if (SubPrefabKeyValue.Value.Prefab == InPrefab)
		{
			return true;
		}
	}
	if (InRecursive)
	{
		for (auto SubPrefabKeyValue : SubPrefabs)
		{
			if (SubPrefabKeyValue.Value.Prefab->ContainsSubPrefab(InPrefab, InRecursive, OutDepth))
			{
				return true;
			}
		}
	}
	OutDepth--;
	return false;
}
#endif