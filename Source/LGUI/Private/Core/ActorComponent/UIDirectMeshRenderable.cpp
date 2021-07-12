// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UIDirectMeshRenderable.h"
#include "LGUI.h"
#include "Core/ActorComponent/LGUICanvas.h"

UUIDirectMeshRenderable::UUIDirectMeshRenderable(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	uiRenderableType = EUIRenderableType::UIDirectMeshRenderable;
}

void UUIDirectMeshRenderable::BeginPlay()
{
	Super::BeginPlay();
	if (CheckRenderCanvas())
	{
		RenderCanvas->MarkRebuildAllDrawcall();
		RenderCanvas->MarkCanvasUpdate();
	}
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



void UUIDirectMeshRenderable::UpdateGeometry(const bool& parentLayoutChanged)
{
	
}

