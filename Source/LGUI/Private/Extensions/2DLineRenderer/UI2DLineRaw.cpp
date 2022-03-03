// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Extensions/2DLineRenderer/UI2DLineRaw.h"
#include "LGUI.h"
#include "Core/UIGeometry.h"
#include "Core/ActorComponent/LGUICanvas.h"

UUI2DLineRaw::UUI2DLineRaw(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUI2DLineRaw::BeginPlay()
{
	Super::BeginPlay();
}

#if WITH_EDITOR
void UUI2DLineRaw::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

AUI2DLineActor::AUI2DLineActor()
{
	PrimaryActorTick.bCanEverTick = false;

	UIElement = CreateDefaultSubobject<UUI2DLineRaw>(TEXT("UIElement"));
	RootComponent = UIElement;
}

void UUI2DLineRaw::SetPoints(const TArray<FVector2D>& InPoints)
{
	if (InPoints.Num() != PointArray.Num())
	{
		PointArray = InPoints;
		MarkVerticesDirty(true, true, true, true);
	}
	else
	{
		PointArray = InPoints;
		MarkVertexPositionDirty();
	}
}
