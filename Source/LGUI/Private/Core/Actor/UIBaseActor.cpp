// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Core/Actor/UIBaseActor.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIItem.h"


AUIBaseActor::AUIBaseActor()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
}

AUIPostProcessBaseActor::AUIPostProcessBaseActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
}