// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Core/Actor/UIContainerActor.h"
#include "LGUI.h"


AUIContainerActor::AUIContainerActor()
{
	PrimaryActorTick.bCanEverTick = false;

	UIItem = CreateDefaultSubobject<UUIItem>(TEXT("UIItemComponent"));
	RootComponent = UIItem;
}
