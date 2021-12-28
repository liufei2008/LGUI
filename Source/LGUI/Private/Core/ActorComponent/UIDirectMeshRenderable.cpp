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

void UUIDirectMeshRenderable::MarkAllDirtyRecursive()
{
	bLocalVertexPositionChanged = true;
	Super::MarkAllDirtyRecursive();
}

void UUIDirectMeshRenderable::OnAnchorChange(bool InPivotChange, bool InSizeChange, bool InDiscardCache)
{
    Super::OnAnchorChange(InPivotChange, InSizeChange, InDiscardCache);
    if (InPivotChange || InSizeChange)
    {
        MarkVertexPositionDirty();
    }
}

void UUIDirectMeshRenderable::MarkVertexPositionDirty()
{
	bLocalVertexPositionChanged = true;
	MarkCanvasUpdate(false, false, false);//since DirectMeshRenderable will always take a drawcall, we don't need to rebuild drawcall on it
}
void UUIDirectMeshRenderable::UpdateGeometry()
{
	if (GetIsUIActiveInHierarchy() == false)return;
	if (!RenderCanvas.IsValid())return;

	Super::UpdateGeometry();
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

bool UUIDirectMeshRenderable::LineTraceUI(FHitResult& OutHit, const FVector& Start, const FVector& End)
{
	if (bRaycastComplex)
	{
		if (!MeshSection.IsValid())return false;

		auto inverseTf = GetComponentTransform().Inverse();
		auto localSpaceRayOrigin = inverseTf.TransformPosition(Start);
		auto localSpaceRayEnd = inverseTf.TransformPosition(End);

		//DrawDebugLine(this->GetWorld(), Start, End, FColor::Red, false, 5.0f);//just for test
		//check Line-Plane intersection first, then check Line-Triangle
		//start and end point must be different side of X plane
		if (FMath::Sign(localSpaceRayOrigin.X) != FMath::Sign(localSpaceRayEnd.X))
		{
			auto result = FMath::LinePlaneIntersection(localSpaceRayOrigin, localSpaceRayEnd, FVector::ZeroVector, FVector(1, 0, 0));
			//hit point inside rect area
			if (result.Y > GetLocalSpaceLeft() && result.Y < GetLocalSpaceRight() && result.Z > GetLocalSpaceBottom() && result.Z < GetLocalSpaceTop())
			{
				//triangle hit test
				auto& vertices = MeshSection.Pin()->vertices;
				auto& triangleIndices = MeshSection.Pin()->triangles;
				int triangleCount = triangleIndices.Num() / 3;
				int index = 0;
				for (int i = 0; i < triangleCount; i++)
				{
					auto point0 = (vertices[triangleIndices[index++]].Position);
					auto point1 = (vertices[triangleIndices[index++]].Position);
					auto point2 = (vertices[triangleIndices[index++]].Position);
					FVector hitPoint, hitNormal;
					if (FMath::SegmentTriangleIntersection(localSpaceRayOrigin, localSpaceRayEnd, point0, point1, point2, hitPoint, hitNormal))
					{
						OutHit.TraceStart = Start;
						OutHit.TraceEnd = End;
						OutHit.Actor = GetOwner();
						OutHit.Component = (UPrimitiveComponent*)this;//acturally this convert is incorrect, but I need this pointer
						OutHit.Location = GetComponentTransform().TransformPosition(hitPoint);
						OutHit.Normal = GetComponentTransform().TransformVector(hitNormal);
						OutHit.Normal.Normalize();
						OutHit.Distance = FVector::Distance(Start, OutHit.Location);
						OutHit.ImpactPoint = OutHit.Location;
						OutHit.ImpactNormal = OutHit.Normal;
						return true;
					}
				}
			}
		}
		return false;
	}
	else
	{
		return Super::LineTraceUI(OutHit, Start, End);
	}
}
