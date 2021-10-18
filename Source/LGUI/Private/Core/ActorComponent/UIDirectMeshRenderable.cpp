// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UIDirectMeshRenderable.h"
#include "LGUI.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Core/LGUIMesh/LGUIMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

UUIDirectMeshRenderable::UUIDirectMeshRenderable(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	bLocalVertexPositionChanged = true;
	uiRenderableType = EUIRenderableType::UIDirectMeshRenderable;
}

void UUIDirectMeshRenderable::BeginPlay()
{
	Super::BeginPlay();
	MarkCanvasUpdate();
	bLocalVertexPositionChanged = true;
}

void UUIDirectMeshRenderable::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );
}


#if WITH_EDITOR
void UUIDirectMeshRenderable::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

void UUIDirectMeshRenderable::OnUnregister()
{
	Super::OnUnregister();
}

void UUIDirectMeshRenderable::ApplyUIActiveState()
{
	Super::ApplyUIActiveState();
}

void UUIDirectMeshRenderable::OnRenderCanvasChanged(ULGUICanvas* OldCanvas, ULGUICanvas* NewCanvas)
{
	Super::OnRenderCanvasChanged(OldCanvas, NewCanvas);
}




void UUIDirectMeshRenderable::UpdateCachedData()
{
	cacheForThisUpdate_LocalVertexPositionChanged = bLocalVertexPositionChanged;
	Super::UpdateCachedData();
}
void UUIDirectMeshRenderable::UpdateCachedDataBeforeGeometry()
{
	if (bLocalVertexPositionChanged)cacheForThisUpdate_LocalVertexPositionChanged = true;
	Super::UpdateCachedDataBeforeGeometry();
}
void UUIDirectMeshRenderable::UpdateBasePrevData()
{
	bLocalVertexPositionChanged = false;
	Super::UpdateBasePrevData();
}
void UUIDirectMeshRenderable::MarkAllDirtyRecursive()
{
	bLocalVertexPositionChanged = true;
	Super::MarkAllDirtyRecursive();
}



void UUIDirectMeshRenderable::WidthChanged()
{
	MarkVertexPositionDirty();
}
void UUIDirectMeshRenderable::HeightChanged()
{
	MarkVertexPositionDirty();
}
void UUIDirectMeshRenderable::PivotChanged()
{
	MarkVertexPositionDirty();
}

void UUIDirectMeshRenderable::MarkVertexPositionDirty()
{
	bLocalVertexPositionChanged = true;
	MarkCanvasUpdate();
}
void UUIDirectMeshRenderable::UpdateGeometry()
{
	if (GetIsUIActiveInHierarchy() == false)return;
	if (!CheckRenderCanvas())return;

	Super::UpdateGeometry();
	if (!drawcall.IsValid()//not add to render yet
		)
	{
		RenderCanvas->AddUIRenderable(this);
	}
}


TWeakPtr<FLGUIMeshSection> UUIDirectMeshRenderable::GetMeshSection()const
{
	return MeshSection;
}
TWeakObjectPtr<ULGUIMeshComponent> UUIDirectMeshRenderable::GetUIMesh()const
{
	return UIMesh;
}
void UUIDirectMeshRenderable::ClearMeshData()
{
	UIMesh.Reset();
	MeshSection.Reset();
}
void UUIDirectMeshRenderable::SetMeshData(TWeakObjectPtr<ULGUIMeshComponent> InUIMesh, TWeakPtr<FLGUIMeshSection> InMeshSection)
{
	UIMesh = InUIMesh;
	MeshSection = InMeshSection;
}
