// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Core/Actor/UIBaseActor.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIItem.h"
#include "Core/ActorComponent/UIBaseRenderable.h"
#include "Core/ActorComponent/UIPostProcessRenderable.h"


AUIBaseActor::AUIBaseActor()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
}

#if WITH_EDITOR
void AUIBaseActor::SetIsTemporarilyHiddenInEditor(bool bIsHidden)
{
	Super::SetIsTemporarilyHiddenInEditor(bIsHidden);

	TArray<UUIItem*> UIItems;
	GetComponents<UUIItem>(UIItems);

	for (UUIItem* Item : UIItems)
	{
		// notify UIItem to refresh render state
		Item->ActorTemporaryHiddenChanged();
	}
}
#endif

AUIBaseRenderableActor::AUIBaseRenderableActor()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
}
UUIItem* AUIBaseRenderableActor::GetUIItem()const 
{
	return GetUIRenderable(); 
}

AUIBasePostProcessActor::AUIBasePostProcessActor()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
}
UUIBaseRenderable* AUIBasePostProcessActor::GetUIRenderable()const 
{
	return GetUIPostProcessRenderable(); 
}

