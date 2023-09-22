// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Extensions/2DLineRenderer/UI2DLineChildrenAsPoints.h"
#include "LGUI.h"
#include "Core/UIGeometry.h"
#include "Core/ActorComponent/LGUICanvas.h"

UUI2DLineChildrenAsPoints::UUI2DLineChildrenAsPoints(const FObjectInitializer &ObjectInitializer) : Super(ObjectInitializer)
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UUI2DLineChildrenAsPoints::BeginPlay()
{
    Super::BeginPlay();
}

AUI2DLineChildrenAsPointsActor::AUI2DLineChildrenAsPointsActor()
{
    PrimaryActorTick.bCanEverTick = false;

    UIElement = CreateDefaultSubobject<UUI2DLineChildrenAsPoints>(TEXT("UIElement"));
    RootComponent = UIElement;
}

void UUI2DLineChildrenAsPoints::OnRegister()
{
    Super::OnRegister();
}

void UUI2DLineChildrenAsPoints::CalculatePoints()
{
    auto& SortedItemArray = this->GetAttachUIChildren();
    int pointCount = SortedItemArray.Num();
    CurrentPointArray.Reset(pointCount);
    for (int i = 0; i < pointCount; i++)
    {
        auto Location3D = SortedItemArray[i]->GetRelativeLocation();
        CurrentPointArray.Add(FVector2D(Location3D.Y, Location3D.Z));
    }
}

void UUI2DLineChildrenAsPoints::OnChildPositionChanged()
{
    MarkVertexPositionDirty();
}