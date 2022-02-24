// Copyright 2019-2022 Tencent All Rights Reserved.

#include "Core/ActorComponent/UIAnimationComp.h"
#include "LGUI.h"


#define LOCTEXT_NAMESPACE "LGUIBPEditor"

UUIAnimationComp::UUIAnimationComp()
{
}


void UUIAnimationComp::BeginPlay()
{
	Super::BeginPlay();
}

void UUIAnimationComp::OnAnimationStartedPlaying(ULGUIAnimationPlayer& Player)
{
	UE_LOG(LGUI, Warning, TEXT("OnAnimationStartedPlaying, not finished!"));
}

void UUIAnimationComp::OnAnimationFinishedPlaying(ULGUIAnimationPlayer& Player)
{
	UE_LOG(LGUI, Warning, TEXT("OnAnimationFinishedPlaying, not finished!"));
}


#undef LOCTEXT_NAMESPACE