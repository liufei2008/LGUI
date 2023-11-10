// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/Actor/UIFrameCaptureActor.h"
#include "LGUI.h"



AUIFrameCaptureActor::AUIFrameCaptureActor()
{
	PrimaryActorTick.bCanEverTick = false;

	UIFrameCapture = CreateDefaultSubobject<UUIFrameCapture>(TEXT("UIFrameCapture"));
	RootComponent = UIFrameCapture;
}
