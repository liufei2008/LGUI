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

bool UUIBaseRenderable::LineTraceUIGeometry(TSharedPtr<UIGeometry> InGeo, FHitResult& OutHit, const FVector& Start, const FVector& End)
{
	if (bRaycastComplex)
	{
		if (!bRaycastTarget)return false;
		if (!GetIsUIActiveInHierarchy())return false;
		if (!RenderCanvas.IsValid())return false;
		if (!GetOwner())return false;
		if (GetOwner()->GetRootComponent() != this)return false;//only root component can do line trace hit
		auto inverseTf = GetComponentTransform().Inverse();
		auto localSpaceRayOrigin = inverseTf.TransformPosition(Start);
		auto localSpaceRayEnd = inverseTf.TransformPosition(End);

		//DrawDebugLine(this->GetWorld(), Start, End, FColor::Red, false, 5.0f);//just for test
		//check Line-Plane intersection first, then check Line-Triangle
		//start and end point must be different side of z plane
		if (FMath::Sign(localSpaceRayOrigin.Z) != FMath::Sign(localSpaceRayEnd.Z))
		{
			auto result = FMath::LinePlaneIntersection(localSpaceRayOrigin, localSpaceRayEnd, FVector::ZeroVector, FVector(0, 0, 1));
			//hit point inside rect area
			if (result.X > GetLocalSpaceLeft() && result.X < GetLocalSpaceRight() && result.Y > GetLocalSpaceBottom() && result.Y < GetLocalSpaceTop())
			{
				//triangle hit test
				auto& vertices = InGeo->originPositions;
				auto& triangleIndices = InGeo->triangles;
				int triangleCount = triangleIndices.Num() / 3;
				int index = 0;
				for (int i = 0; i < triangleCount; i++)
				{
					auto point0 = (vertices[triangleIndices[index++]]);
					auto point1 = (vertices[triangleIndices[index++]]);
					auto point2 = (vertices[triangleIndices[index++]]);
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
