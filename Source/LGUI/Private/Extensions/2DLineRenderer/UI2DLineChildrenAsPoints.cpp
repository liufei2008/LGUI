// Copyright 2019-2022 LexLiu. All Rights Reserved.

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

    RebuildChildrenList();
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
#if WITH_EDITOR
    if (!GetWorld()->IsGameWorld())
    {
        RebuildChildrenList();
    }
#endif
}

void UUI2DLineChildrenAsPoints::OnUIChildHierarchyIndexChanged(UUIItem *child)
{
    Super::OnUIChildHierarchyIndexChanged(child);
    //sort order
    SortedItemArray.Sort([](const UUIItem &A, const UUIItem &B) {
        if (A.GetHierarchyIndex() < B.GetHierarchyIndex())
            return true;
        return false;
    });

    MarkCanvasUpdate(false, false, false, true);
}

void UUI2DLineChildrenAsPoints::OnUIChildAttachmentChanged(UUIItem *child, bool attachOrDetach)
{
    Super::OnUIChildAttachmentChanged(child, attachOrDetach);

    if (attachOrDetach)
    {
        int32 index;
        if (!SortedItemArray.Find(child, index))
        {
            SortedItemArray.Add(child);

            SortedItemArray.Sort([](const UUIItem &A, const UUIItem &B) {
                if (A.GetHierarchyIndex() < B.GetHierarchyIndex())
                    return true;
                return false;
            });

            MarkVerticesDirty(true, true, true, true);
        }
    }
    else
    {
        int32 index;
        if (SortedItemArray.Find(child, index))
        {
            SortedItemArray.RemoveAt(index);
            MarkVerticesDirty(true, true, true, true);
        }
    }
}
void UUI2DLineChildrenAsPoints::OnUIChildDimensionsChanged(UUIItem *child, bool positionChanged, bool sizeChanged)
{
    Super::OnUIChildDimensionsChanged(child, positionChanged, sizeChanged);
    if (positionChanged)
    {
        OnChildPositionChanged();
    }
}

void UUI2DLineChildrenAsPoints::RebuildChildrenList()
{
    SortedItemArray.Reset();
    const auto &childrenArray = this->GetAttachChildren();
    for (int i = 0, count = childrenArray.Num(); i < count; i++)
    {
        if (auto uiItem = Cast<UUIItem>(childrenArray[i]))
        {
            SortedItemArray.Add(uiItem);
        }
    }
    SortedItemArray.Sort([](const UUIItem &A, const UUIItem &B) {
        if (A.GetHierarchyIndex() < B.GetHierarchyIndex())
            return true;
        return false;
    });
}

void UUI2DLineChildrenAsPoints::CalculatePoints()
{
    int pointCount = SortedItemArray.Num();
    CurrentPointArray.Reset(pointCount);
    for (int i = 0; i < pointCount; i++)
    {
        CurrentPointArray.Add(FVector2D(SortedItemArray[i]->GetRelativeLocation()));
    }
}

void UUI2DLineChildrenAsPoints::OnChildPositionChanged()
{
    MarkVertexPositionDirty();
}