// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/Actor/UIBackgroundBlurActor.h"
#include "LGUI.h"



AUIBackgroundBlurActor::AUIBackgroundBlurActor()
{
	PrimaryActorTick.bCanEverTick = false;

	UIBackgroundBlur = CreateDefaultSubobject<UUIBackgroundBlur>(TEXT("UIBackgroundBlurComponent"));
	RootComponent = UIBackgroundBlur;
}
