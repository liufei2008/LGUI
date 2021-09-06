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
	LGUIPrefabSystem::ActorSerializer::RenewActorGuidForDuplicate(this);
}

#endif