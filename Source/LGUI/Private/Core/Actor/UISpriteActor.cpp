// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Core/Actor/UISpriteActor.h"
#include "LGUI.h"



AUISpriteActor::AUISpriteActor()
{
	PrimaryActorTick.bCanEverTick = false;

	UISprite = CreateDefaultSubobject<UUISprite>(TEXT("UISpriteComponent"));
	RootComponent = UISprite;
}
