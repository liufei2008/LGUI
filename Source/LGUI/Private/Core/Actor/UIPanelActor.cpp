// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Core/Actor/UIPanelActor.h"
#include "LGUI.h"



AUIPanelActor::AUIPanelActor()
{
	PrimaryActorTick.bCanEverTick = false;

	UIPanel = CreateDefaultSubobject<UUIPanel>(TEXT("UIPanelComponent"));
	RootComponent = UIPanel;
}
