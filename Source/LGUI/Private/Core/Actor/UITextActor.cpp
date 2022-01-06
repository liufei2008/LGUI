// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Core/Actor/UITextActor.h"
#include "LGUI.h"



AUITextActor::AUITextActor()
{
	PrimaryActorTick.bCanEverTick = false;

	UIText = CreateDefaultSubobject<UUIText>(TEXT("UITextComponent"));
	RootComponent = UIText;
}
