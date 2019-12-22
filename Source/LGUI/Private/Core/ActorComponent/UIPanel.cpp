// Copyright 2019 LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UIPanel.h"
#include "LGUI.h"
#include "CoreGlobals.h"

UUIPanel::UUIPanel(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UUIPanel::BeginPlay()
{
	Super::BeginPlay();
}
void UUIPanel::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void UUIPanel::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
