// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Core/Actor/UICustomDepthMaskActor.h"
#include "LGUI.h"



AUICustomDepthMaskActor::AUICustomDepthMaskActor()
{
	PrimaryActorTick.bCanEverTick = false;

	UICustomDepthMask = CreateDefaultSubobject<UUICustomDepthMask>(TEXT("UICustomDepthMaskComponent"));
	RootComponent = UICustomDepthMask;
}
