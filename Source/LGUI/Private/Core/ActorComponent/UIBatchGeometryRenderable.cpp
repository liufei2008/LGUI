// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UIBatchGeometryRenderable.h"
#include "LGUI.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Utils/LGUIUtils.h"
#include "GeometryModifier/UIGeometryModifierBase.h"
#include "Core/LGUIMesh/LGUIMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Core/UIDrawcall.h"
#include "Core/Actor/LGUIManagerActor.h"

DECLARE_CYCLE_STAT(TEXT("UIBatchGeometryRenderable ApplyModifier"), STAT_ApplyModifier, STATGROUP_LGUI);

UUIBatchGeometryRenderable::UUIBatchGeometryRenderable(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	uiRenderableType = EUIRenderableType::UIBatchGeometryRenderable;
	geometry = TSharedPtr<UIGeometry>(new UIGeometry);

	bLocalVertexPositionChanged = true;
	bUVChanged = true;
	bTriangleChanged = true;
}

void UUIBatchGeometryRenderable::BeginPlay()
{
	Super::BeginPlay();
	if (CheckRenderCanvas())
	{
		RenderCanvas->MarkCanvasUpdate();
	}

	bLocalVertexPositionChanged = true;
	bUVChanged = true;
	bTriangleChanged = true;
}

void UUIBatchGeometryRenderable::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );
}

void UUIBatchGeometryRenderable::ApplyUIActiveState()
{
	if (!GetIsUIActiveInHierarchy())
	{
		if (RenderCanvas.IsValid() && drawcall.IsValid())
		{
			RenderCanvas->RemoveUIRenderable(this);
		}
	}
	UUIItem::ApplyUIActiveState();
}
#if WITH_EDITOR
void UUIBatchGeometryRenderable::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.Property)
	{

	}
}
#endif
void UUIBatchGeometryRenderable::OnUnregister()
{
	Super::OnUnregister();
	if (RenderCanvas.IsValid() && drawcall.IsValid())
	{
		RenderCanvas->RemoveUIRenderable(this);
	}
}

void UUIBatchGeometryRenderable::OnRenderCanvasChanged(ULGUICanvas* OldCanvas, ULGUICanvas* NewCanvas)
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
		if (IsValid(OldCanvas))
		{
			if (OldCanvas->IsRenderByLGUIRendererOrUERenderer() != NewCanvas->IsRenderByLGUIRendererOrUERenderer())//is render to screen or world changed, then uimesh need to be recreate
			{
				geometry->Clear();
			}
		}
		NewCanvas->MarkCanvasUpdate();
	}
}

void UUIBatchGeometryRenderable::WidthChanged()
{
	MarkVertexPositionDirty();
}
void UUIBatchGeometryRenderable::HeightChanged()
{
	MarkVertexPositionDirty();
}
void UUIBatchGeometryRenderable::PivotChanged()
{
	MarkVertexPositionDirty();
}

void UUIBatchGeometryRenderable::MarkVertexPositionDirty()
{
	bLocalVertexPositionChanged = true;
	MarkCanvasUpdate();
}
void UUIBatchGeometryRenderable::MarkUVDirty()
{
	bUVChanged = true;
	MarkCanvasUpdate();
}
void UUIBatchGeometryRenderable::MarkTriangleDirty()
{
	bTriangleChanged = true;
	MarkCanvasUpdate();
}
void UUIBatchGeometryRenderable::MarkTextureDirty()
{
	if (CheckRenderCanvas())
	{
		if (drawcall.IsValid())
		{
			if (NeedTextureToCreateGeometry())
			{
				if (!IsValid(GetTextureToCreateGeometry()))//need texture, but texture is not valid
				{
					UE_LOG(LGUI, Error, TEXT("[UUIGeometryRenderable::CreateGeometry]Need texture to create geometry, but texture is no valid!"));
					RenderCanvas->RemoveUIRenderable(this);
				}
				else
				{
					geometry->texture = GetTextureToCreateGeometry();
					//Remove from old drawcall, then add to new drawcall.
					RenderCanvas->RemoveUIRenderable(this);
					RenderCanvas->AddUIRenderable(this);
				}
			}
			else
			{
				//Remove from old drawcall, then add to new drawcall.
				RenderCanvas->RemoveUIRenderable(this);
				RenderCanvas->AddUIRenderable(this);
			}
		}
	}
	MarkCanvasUpdate();
}
void UUIBatchGeometryRenderable::MarkMaterialDirty()
{
	if (CheckRenderCanvas())
	{
		if (drawcall.IsValid())
		{
			//Remove from old drawcall, then add to new drawcall.
			RenderCanvas->RemoveUIRenderable(this);
			geometry->material = CustomUIMaterial;
			RenderCanvas->AddUIRenderable(this);
		}
	}
	MarkCanvasUpdate();
}

void UUIBatchGeometryRenderable::AddGeometryModifier(class UUIGeometryModifierBase* InModifier)
{
	int count = GeometryModifierComponentArray.Num();
	if (count > 0)
	{
		int32 index;
		if (!GeometryModifierComponentArray.Find(InModifier, index))
		{
			GeometryModifierComponentArray.Add(InModifier);
			GeometryModifierComponentArray.Sort([](const UUIGeometryModifierBase& A, const UUIGeometryModifierBase& B) {
				return A.GetExecuteOrder() < B.GetExecuteOrder();
			});
		}
	}
	else
	{
		GeometryModifierComponentArray.Add(InModifier);
	}
}
void UUIBatchGeometryRenderable::RemoveGeometryModifier(class UUIGeometryModifierBase* InModifier)
{
	GeometryModifierComponentArray.Remove(InModifier);
}

void UUIBatchGeometryRenderable::UpdateCachedData()
{
	cacheForThisUpdate_LocalVertexPositionChanged = bLocalVertexPositionChanged;
	cacheForThisUpdate_UVChanged = bUVChanged;
	cacheForThisUpdate_TriangleChanged = bTriangleChanged;
	Super::UpdateCachedData();
}
void UUIBatchGeometryRenderable::UpdateCachedDataBeforeGeometry()
{
	if (bLocalVertexPositionChanged)cacheForThisUpdate_LocalVertexPositionChanged = true;
	if (bUVChanged)cacheForThisUpdate_UVChanged = true;
	if (bTriangleChanged)cacheForThisUpdate_TriangleChanged = true;
	Super::UpdateCachedDataBeforeGeometry();
}
void UUIBatchGeometryRenderable::UpdateBasePrevData()
{
	bLocalVertexPositionChanged = false;
	bUVChanged = false;
	bTriangleChanged = false;
	Super::UpdateBasePrevData();
}
void UUIBatchGeometryRenderable::MarkAllDirtyRecursive()
{
	bLocalVertexPositionChanged = true;
	bUVChanged = true;
	bTriangleChanged = true;
	Super::MarkAllDirtyRecursive();
}
void UUIBatchGeometryRenderable::SetCustomUIMaterial(UMaterialInterface* inMat)
{
	if (CustomUIMaterial != inMat)
	{
		CustomUIMaterial = inMat;
		MarkMaterialDirty();
	}
}

UMaterialInstanceDynamic* UUIBatchGeometryRenderable::GetMaterialInstanceDynamic()const
{
	if (drawcall.IsValid())
	{
		return drawcall->materialInstanceDynamic.Get();
	}
	return nullptr;
}
bool UUIBatchGeometryRenderable::HaveGeometryModifier()
{
#if WITH_EDITOR
	if (GetWorld()->WorldType == EWorldType::Editor || GetWorld()->WorldType == EWorldType::EditorPreview)
	{
		if (auto modifierComp = GetOwner()->FindComponentByClass<UUIGeometryModifierBase>())
		{
			return true;
		}
	}
	else
#endif
	{
		return GeometryModifierComponentArray.Num() > 0;
	}
	return false;
}
bool UUIBatchGeometryRenderable::ApplyGeometryModifier(bool uvChanged, bool colorChanged, bool vertexPositionChanged, bool layoutChanged)
{
	SCOPE_CYCLE_COUNTER(STAT_ApplyModifier);
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())
	{
		GetOwner()->GetComponents(GeometryModifierComponentArray, false);
		GeometryModifierComponentArray.Sort([](const UUIGeometryModifierBase& A, const UUIGeometryModifierBase& B) {
			return A.GetExecuteOrder() < B.GetExecuteOrder();
		});
	}
#endif
	int count = GeometryModifierComponentArray.Num();
	if (count > 0)
	{
		int32 originVerticesCount = geometry->originVerticesCount;
		int32 originTriangleIndicesCount = geometry->originTriangleCount;
		bool modifierAffectTriangleCount = false;
		for (int i = 0; i < count; i++)
		{
			auto modifierComp = GeometryModifierComponentArray[i];
			bool thisModifierAffectTriangleCount = false;
			modifierComp->ModifyUIGeometry(geometry, originVerticesCount, originTriangleIndicesCount, thisModifierAffectTriangleCount,
				uvChanged, colorChanged, vertexPositionChanged, layoutChanged);
			if (thisModifierAffectTriangleCount)modifierAffectTriangleCount = true;
		}
		return modifierAffectTriangleCount;
	}
	return false;
}

void UUIBatchGeometryRenderable::UpdateLayout(bool& parentLayoutChanged, bool shouldUpdateLayout)
{
	Super::UpdateLayout(parentLayoutChanged, shouldUpdateLayout);
	if (!cacheForThisUpdate_LocalVertexPositionChanged)
	{
		if (parentLayoutChanged
			&& RenderCanvas->GetActualPixelPerfect()//@todo: review this line, is this necessary? (old commit: fix pixel perfect update.)
			)
		{
			cacheForThisUpdate_LocalVertexPositionChanged = true;
		}
	}
}

DECLARE_CYCLE_STAT(TEXT("UIGeometryRenderable UpdateRenderable"), STAT_UIGeometryRenderableUpdate, STATGROUP_LGUI);
void UUIBatchGeometryRenderable::UpdateGeometry()
{
	SCOPE_CYCLE_COUNTER(STAT_UIGeometryRenderableUpdate);
	if (GetIsUIActiveInHierarchy() == false)return;
	if (!CheckRenderCanvas())return;

	Super::UpdateGeometry();
	UpdateGeometry_Implement();
}

void UUIBatchGeometryRenderable::UpdateGeometry_Implement()
{
	OnBeforeCreateOrUpdateGeometry();
	if (!drawcall.IsValid()//not add to render yet
		)
	{
		if (CreateGeometry())
		{
			RenderCanvas->AddUIRenderable(this);
		}
		goto COMPLETE;
	}
	else//if geometry is created, update data
	{
		if (cacheForThisUpdate_TriangleChanged)//triangle change, need to recreate geometry
		{
			if (CreateGeometry())
			{
				drawcall->needToRebuildMesh = true;
			}
			else
			{
				RenderCanvas->RemoveUIRenderable(this);
			}
			goto COMPLETE;
		}
		else//update geometry
		{
			OnUpdateGeometry(cacheForThisUpdate_LocalVertexPositionChanged || cacheForThisUpdate_LayoutChanged, cacheForThisUpdate_UVChanged, cacheForThisUpdate_ColorChanged);

			if (ApplyGeometryModifier(cacheForThisUpdate_UVChanged, cacheForThisUpdate_ColorChanged, cacheForThisUpdate_LocalVertexPositionChanged, cacheForThisUpdate_LayoutChanged))//vertex data change, need to update geometry's vertex
			{
				drawcall->needToRebuildMesh = true;
				drawcall->needToUpdateVertex = true;
			}
			else
			{
				drawcall->needToUpdateVertex = true;
			}
			if (cacheForThisUpdate_LocalVertexPositionChanged || cacheForThisUpdate_LayoutChanged)
			{
				UIGeometry::TransformVertices(RenderCanvas.Get(), this, geometry);
			}
		}
	}
COMPLETE:
	;
}

bool UUIBatchGeometryRenderable::CreateGeometry()
{
	geometry->Clear();
	if (HaveDataToCreateGeometry())
	{
		if (NeedTextureToCreateGeometry() && !IsValid(GetTextureToCreateGeometry()))
		{
			UE_LOG(LGUI, Error, TEXT("[UUIGeometryRenderable::CreateGeometry]Need texture to create geometry, but texture is no valid!"));
			return false;
		}
		geometry->texture = GetTextureToCreateGeometry();
		geometry->material = CustomUIMaterial;
		OnCreateGeometry();
		ApplyGeometryModifier(true, true, true, true);
		UIGeometry::TransformVertices(RenderCanvas.Get(), this, geometry);
		return true;
	}
	else
	{
		return false;
	}
}

bool UUIBatchGeometryRenderable::LineTraceUI(FHitResult& OutHit, const FVector& Start, const FVector& End)
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
				auto& vertices = geometry->originPositions;
				auto& triangleIndices = geometry->triangles;
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