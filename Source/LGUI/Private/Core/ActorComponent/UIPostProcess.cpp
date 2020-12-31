// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UIPostProcess.h"
#include "LGUI.h"
#include "Core/ActorComponent/LGUICanvas.h"

UUIPostProcess::UUIPostProcess(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	itemType = UIItemType::UIRenderable;

	bIsPostProcess = true;
}

void UUIPostProcess::BeginPlay()
{
	Super::BeginPlay();
	bIsPostProcess = true;
}

void UUIPostProcess::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );
}


#if WITH_EDITOR
void UUIPostProcess::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif
