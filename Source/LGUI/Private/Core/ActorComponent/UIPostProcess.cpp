// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UIPostProcess.h"
#include "LGUI.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Core/UIGeometry.h"
#include "Core/UIPostProcessRenderProxy.h"

UUIPostProcess::UUIPostProcess(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	uiRenderableType = EUIRenderableType::UIPostProcessRenderable;
	geometry = TSharedPtr<UIGeometry>(new UIGeometry);

	bLocalVertexPositionChanged = true;
	bUVChanged = true;
}

void UUIPostProcess::BeginPlay()
{
	Super::BeginPlay();
	if (CheckRenderCanvas())
	{
		RenderCanvas->MarkRebuildAllDrawcall();
		RenderCanvas->MarkCanvasUpdate();
	}

	bLocalVertexPositionChanged = true;
	bUVChanged = true;
}

void UUIPostProcess::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );
}


#if WITH_EDITOR
void UUIPostProcess::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	bUVChanged = true;
	bLocalVertexPositionChanged = true;
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

void UUIPostProcess::OnUnregister()
{
	Super::OnUnregister();
	if (RenderProxy.IsValid())
	{
		auto tempRenderProxy = RenderProxy;
		ENQUEUE_RENDER_COMMAND(FUIPostProcess_RemoveRenderProxy)(
			[tempRenderProxy](FRHICommandListImmediate& RHICmdList)
			{
				tempRenderProxy->RemoveFromHudRenderer_RenderThread();
			});
	}
}

void UUIPostProcess::ApplyUIActiveState()
{
	bUVChanged = true;
	if (IsUIActiveInHierarchy() == false)
	{
		if (geometry->vertices.Num() != 0)
		{
			geometry->Clear();
			if (CheckRenderCanvas())
			{
				RenderCanvas->MarkRebuildAllDrawcall();//@todo: noneed to rebuild all drawcall, still have some room to optimize
			}
		}
	}
	Super::ApplyUIActiveState();
}

void UUIPostProcess::OnRenderCanvasChanged(ULGUICanvas* OldCanvas, ULGUICanvas* NewCanvas)
{
	if (IsValid(OldCanvas))
	{
		OldCanvas->RemoveUIRenderable(this);
		OldCanvas->MarkRebuildAllDrawcall();//@todo: noneed to rebuild all drawcall, still have some room to optimize
	}
	if (IsValid(NewCanvas))
	{
		NewCanvas->AddUIRenderable(this);
		NewCanvas->MarkRebuildAllDrawcall();//@todo: noneed to rebuild all drawcall, still have some room to optimize
	}
}
void UUIPostProcess::WidthChanged()
{
	MarkVertexPositionDirty();
}
void UUIPostProcess::HeightChanged()
{
	MarkVertexPositionDirty();
}
void UUIPostProcess::PivotChanged()
{
	MarkVertexPositionDirty();
}

void UUIPostProcess::MarkVertexPositionDirty()
{
	bLocalVertexPositionChanged = true;
	if (CheckRenderCanvas()) RenderCanvas->MarkCanvasUpdate();
}
void UUIPostProcess::MarkUVDirty()
{
	bUVChanged = true;
	if (CheckRenderCanvas()) RenderCanvas->MarkCanvasUpdate();
}

void UUIPostProcess::UpdateCachedData()
{
	cacheForThisUpdate_LocalVertexPositionChanged = bLocalVertexPositionChanged;
	cacheForThisUpdate_UVChanged = bUVChanged;
	Super::UpdateCachedData();
}
void UUIPostProcess::UpdateCachedDataBeforeGeometry()
{
	if (bLocalVertexPositionChanged)cacheForThisUpdate_LocalVertexPositionChanged = true;
	if (bUVChanged)cacheForThisUpdate_UVChanged = true;
	Super::UpdateCachedDataBeforeGeometry();
}
void UUIPostProcess::UpdateBasePrevData()
{
	bLocalVertexPositionChanged = false;
	bUVChanged = false;
	Super::UpdateBasePrevData();
}
void UUIPostProcess::MarkAllDirtyRecursive()
{
	bLocalVertexPositionChanged = true;
	bUVChanged = true;
	Super::MarkAllDirtyRecursive();
}

void UUIPostProcess::CreateGeometry()
{
	geometry->Clear();
	OnCreateGeometry();
	UIGeometry::TransformVertices(RenderCanvas, this, geometry);
}

DECLARE_CYCLE_STAT(TEXT("UIPostProcessRenderable UpdateRenderable"), STAT_UIPostProcessRenderableUpdate, STATGROUP_LGUI);
void UUIPostProcess::UpdateGeometry(const bool& parentLayoutChanged)
{
	SCOPE_CYCLE_COUNTER(STAT_UIPostProcessRenderableUpdate);
	if (IsUIActiveInHierarchy() == false)return;
	if (!CheckRenderCanvas())return;

	if (geometry->vertices.Num() == 0//if geometry not created yet
		)
	{
		CreateGeometry();
		RenderCanvas->MarkRebuildAllDrawcall();//@todo: noneed to rebuild all drawcall, still have some room to optimize
		goto COMPLETE;
	}
	else//if geometry is created, update data
	{
		if (cacheForThisUpdate_DepthChanged)
		{
			RenderCanvas->MarkRebuildAllDrawcall();//@todo: noneed to rebuild all drawcall, still have some room to optimize
			goto COMPLETE;
		}
		//update geometry
		{
			OnUpdateGeometry(cacheForThisUpdate_LocalVertexPositionChanged, cacheForThisUpdate_UVChanged, cacheForThisUpdate_ColorChanged);

			if (cacheForThisUpdate_LocalVertexPositionChanged || parentLayoutChanged)
			{
				UIGeometry::TransformVertices(RenderCanvas, this, geometry);
			}
		}
	}
COMPLETE:
	;
}