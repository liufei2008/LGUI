// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Core/Actor/UIBaseActor.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIItem.h"


AUIBaseActor::AUIBaseActor()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
}

#if WITH_EDITOR
void AUIBaseActor::SetIsTemporarilyHiddenInEditor(bool bIsHidden)
{
	//if (IsTemporarilyHiddenInEditor() != bIsHidden)
	//{
	//	TArray<UUIItem*> UIItems;
	//	GetComponents<UUIItem>(UIItems);

	//	for (UUIItem* Item : UIItems)
	//	{
	//		if (bIsHidden)
	//		{
	//			Item->SetIsUIActive(false);
	//		}
	//		else
	//		{
	//			Item->SetIsUIActive(true);
	//		}
	//	}
	//}

	Super::SetIsTemporarilyHiddenInEditor(bIsHidden);
}
#endif

AUIBaseRenderableActor::AUIBaseRenderableActor()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
}

AUIBasePostProcessActor::AUIBasePostProcessActor()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
}