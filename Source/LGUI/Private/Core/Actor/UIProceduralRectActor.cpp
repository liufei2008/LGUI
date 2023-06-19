// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/Actor/UIProceduralRectActor.h"
#include "LGUI.h"



AUIProceduralRectActor::AUIProceduralRectActor()
{
	PrimaryActorTick.bCanEverTick = false;

	UIProceduralRect = CreateDefaultSubobject<UUIProceduralRect>(TEXT("UIProceduralRect"));
	RootComponent = UIProceduralRect;
}
