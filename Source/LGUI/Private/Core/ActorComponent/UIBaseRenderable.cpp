// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UIBaseRenderable.h"
#include "LGUI.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Utils/LGUIUtils.h"
#include "GeometryModifier/UIGeometryModifierBase.h"


UUIBaseRenderable::UUIBaseRenderable(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	itemType = UIItemType::UIBatchGeometryRenderable;
	uiRenderableType = EUIRenderableType::None;
}

void UUIBaseRenderable::BeginPlay()
{
	Super::BeginPlay();
}

void UUIBaseRenderable::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );
}

#if WITH_EDITOR
void UUIBaseRenderable::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

void UUIBaseRenderable::ApplyUIActiveState()
{
	if (!GetIsUIActiveInHierarchy())
	{
		if (RenderCanvas.IsValid() && drawcall.IsValid())
		{
			RenderCanvas->RemoveUIRenderable(this);
		}
	}
	Super::ApplyUIActiveState();
}
void UUIBaseRenderable::OnRenderCanvasChanged(ULGUICanvas* OldCanvas, ULGUICanvas* NewCanvas)
{
	if (IsValid(OldCanvas))
	{
		if (drawcall.IsValid())
		{
			OldCanvas->RemoveUIRenderable(this);
		}
		OldCanvas->MarkCanvasUpdate();
	}
	if (IsValid(NewCanvas))
	{
		NewCanvas->MarkCanvasUpdate();
	}
}

void UUIBaseRenderable::DepthChanged()
{
	if (CheckRenderCanvas())
	{
		if (drawcall.IsValid())
		{
			//Remove from old drawcall, then add to new drawcall.
			RenderCanvas->RemoveUIRenderable(this);
			RenderCanvas->AddUIRenderable(this);
		}
	}
}
