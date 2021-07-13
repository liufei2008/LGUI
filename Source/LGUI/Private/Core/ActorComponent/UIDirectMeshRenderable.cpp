// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UIDirectMeshRenderable.h"
#include "LGUI.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Core/LGUIMesh/UIDrawcallMesh.h"

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
	if (UIDrawcallMesh.IsValid())
	{
		UIDrawcallMesh->SetUIMeshVisibility(this->IsUIActiveInHierarchy());
	}
	Super::ApplyUIActiveState();
}

void UUIDirectMeshRenderable::OnRenderCanvasChanged(ULGUICanvas* OldCanvas, ULGUICanvas* NewCanvas)
{
	if (IsValid(OldCanvas))
	{
		OldCanvas->RemoveUIRenderable(this);
		OldCanvas->MarkRebuildAllDrawcall();
	}
	if (IsValid(NewCanvas))
	{
		NewCanvas->AddUIRenderable(this);
		NewCanvas->MarkRebuildAllDrawcall();
	}
}



void UUIDirectMeshRenderable::UpdateGeometry(const bool& parentLayoutChanged)
{
	
}


UUIDrawcallMesh* UUIDirectMeshRenderable::GetDrawcallMesh()const 
{
	return UIDrawcallMesh.Get();
}
void UUIDirectMeshRenderable::ClearDrawcallMesh()
{
	UIDrawcallMesh.Reset(); 
}
void UUIDirectMeshRenderable::SetDrawcallMesh(UUIDrawcallMesh* InUIDrawcallMesh)
{
	UIDrawcallMesh = InUIDrawcallMesh;
	if (UIDrawcallMesh.IsValid())
	{
		UIDrawcallMesh->SetUIMeshVisibility(this->IsUIActiveInHierarchy());
	}
	if (UIDrawcallMesh.IsValid() && Material.IsValid())
	{
		UIDrawcallMesh->SetMaterial(0, Material.Get());
	}
}
void UUIDirectMeshRenderable::SetMaterial(UMaterialInterface* InMaterial)
{
	Material = InMaterial;
	if (UIDrawcallMesh.IsValid() && Material.IsValid())
	{
		UIDrawcallMesh->SetMaterial(0, Material.Get());
	}
}
