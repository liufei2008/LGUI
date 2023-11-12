// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UIDirectMeshRenderable.h"
#include "LGUI.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Core/LGUIMesh/LGUIMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Core/UIDrawcall.h"

UUIDirectMeshRenderable::UUIDirectMeshRenderable(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	bLocalVertexPositionChanged = true;
	UIRenderableType = EUIRenderableType::UIDirectMeshRenderable;
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

void UUIDirectMeshRenderable::MarkAllDirty()
{
	bLocalVertexPositionChanged = true;
	Super::MarkAllDirty();
}

void UUIDirectMeshRenderable::OnAnchorChange(bool InPivotChange, bool InWidthChange, bool InHeightChange, bool InDiscardCache)
{
    Super::OnAnchorChange(InPivotChange, InWidthChange, InHeightChange, InDiscardCache);
    if (InPivotChange || InWidthChange || InHeightChange)
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


TWeakPtr<FLGUIRenderSection> UUIDirectMeshRenderable::GetMeshSection()const
{
	if (drawcall.IsValid())
	{
		return drawcall->DrawcallRenderSection;
	}
	return nullptr;
}
TWeakObjectPtr<ULGUIMeshComponent> UUIDirectMeshRenderable::GetUIMesh()const
{
	if (drawcall.IsValid())
	{
		return drawcall->DrawcallMesh;
	}
	return nullptr;
}
void UUIDirectMeshRenderable::ClearMeshData()
{
	MarkCanvasUpdate(false, false, false, true);
}
void UUIDirectMeshRenderable::OnMeshDataReady()
{

}

bool UUIDirectMeshRenderable::LineTraceUI(FHitResult& OutHit, const FVector& Start, const FVector& End)
{
	if (RaycastType == EUIRenderableRaycastType::Rect)
	{
		return Super::LineTraceUI(OutHit, Start, End);
	}
	else if (RaycastType == EUIRenderableRaycastType::Mesh)
	{
		if (!drawcall.IsValid())return false;
		if (!drawcall->DrawcallRenderSection.IsValid())return false;

		auto inverseTf = GetComponentTransform().Inverse();
		auto localSpaceRayOrigin = inverseTf.TransformPosition(Start);
		auto localSpaceRayEnd = inverseTf.TransformPosition(End);

		//DrawDebugLine(this->GetWorld(), Start, End, FColor::Red, false, 5.0f);//just for test
		//check Line-Plane intersection first, then check Line-Triangle
		//start and end point must be different side of X plane
		if (FMath::Sign(localSpaceRayOrigin.X) != FMath::Sign(localSpaceRayEnd.X))
		{
			auto IntersectionPoint = FMath::LinePlaneIntersection(localSpaceRayOrigin, localSpaceRayEnd, FVector::ZeroVector, FVector(1, 0, 0));
			//hit point inside rect area
			if (IntersectionPoint.Y > GetLocalSpaceLeft() && IntersectionPoint.Y < GetLocalSpaceRight() && IntersectionPoint.Z > GetLocalSpaceBottom() && IntersectionPoint.Z < GetLocalSpaceTop())
			{
				//triangle hit test
				auto MeshSection = (FLGUIMeshSection*)drawcall->DrawcallRenderSection.Pin().Get();
				auto& vertices = MeshSection->vertices;
				auto& triangleIndices = MeshSection->triangles;
				int triangleCount = triangleIndices.Num() / 3;
				int index = 0;
				for (int i = 0; i < triangleCount; i++)
				{
					auto point0 = (FVector)(vertices[triangleIndices[index++]].Position);
					auto point1 = (FVector)(vertices[triangleIndices[index++]].Position);
					auto point2 = (FVector)(vertices[triangleIndices[index++]].Position);
					FVector HitPoint, HitNormal;
					if (FMath::SegmentTriangleIntersection(localSpaceRayOrigin, localSpaceRayEnd, point0, point1, point2, HitPoint, HitNormal))
					{
						OutHit.TraceStart = Start;
						OutHit.TraceEnd = End;
						OutHit.Component = (UPrimitiveComponent*)this;//acturally this convert is incorrect, but I need this pointer
						OutHit.Location = GetComponentTransform().TransformPosition(HitPoint);
						OutHit.Normal = GetComponentTransform().TransformVector(HitNormal);
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
		return LineTraceUICustom(OutHit, Start, End);
	}
}
