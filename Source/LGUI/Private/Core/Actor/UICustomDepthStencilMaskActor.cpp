// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Core/Actor/UICustomDepthStencilMaskActor.h"
#include "LGUI.h"



AUICustomDepthStencilMaskActor::AUICustomDepthStencilMaskActor()
{
	PrimaryActorTick.bCanEverTick = false;

	UICustomDepthStencilMask = CreateDefaultSubobject<UUICustomDepthStencilMask>(TEXT("UICustomDepthStencilMaskComponent"));
	RootComponent = UICustomDepthStencilMask;
}
