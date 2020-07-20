// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UIRenderable.h"
#include "LGUI.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Utils/LGUIUtils.h"
#include "GeometryModifier/UIGeometryModifierBase.h"

DECLARE_CYCLE_STAT(TEXT("UIRenderable ApplyModifier"), STAT_ApplyModifier, STATGROUP_LGUI);

UUIRenderable::UUIRenderable(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	itemType = UIItemType::UIRenderable;
	geometry = TSharedPtr<UIGeometry>(new UIGeometry);
}

void UUIRenderable::BeginPlay()
{
	Super::BeginPlay();
	if (CheckRenderCanvas())
	{
		RenderCanvas->MarkRebuildAllDrawcall();
		RenderCanvas->MarkCanvasUpdate();
	}
	bUVChanged = true;
	bTriangleChanged = true;
	bTextureChanged = true;
	bMaterialChanged = true;
}

void UUIRenderable::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );
}

void UUIRenderable::ApplyUIActiveState()
{
	bUVChanged = true;
	bTriangleChanged = true;
	bTextureChanged = true;
	bMaterialChanged = true;
	if (IsUIActiveInHierarchy() == false)
	{
		if (geometry->vertices.Num() != 0)
		{
			geometry->Clear();
			if (CheckRenderCanvas())
			{
				RenderCanvas->MarkRebuildSpecificDrawcall(geometry->drawcallIndex);
			}
		}
	}
	Super::ApplyUIActiveState();
}
#if WITH_EDITOR
void UUIRenderable::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	bUVChanged = true;
	bTriangleChanged = true;
	bTextureChanged = true;
	bMaterialChanged = true;
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

void UUIRenderable::OnRenderCanvasChanged(ULGUICanvas* OldCanvas, ULGUICanvas* NewCanvas)
{
	if (IsValid(OldCanvas))
	{
		OldCanvas->RemoveUIRenderable(this);
		OldCanvas->MarkCanvasUpdate();
	}
	if (IsValid(NewCanvas))
	{
		NewCanvas->AddUIRenderable(this);
		NewCanvas->MarkCanvasUpdate();
	}
}

void UUIRenderable::MarkUVDirty()
{
	bUVChanged = true;
	if (CheckRenderCanvas()) RenderCanvas->MarkCanvasUpdate();
}
void UUIRenderable::MarkTriangleDirty()
{
	bTriangleChanged = true;
	if (CheckRenderCanvas()) RenderCanvas->MarkCanvasUpdate();
}
void UUIRenderable::MarkTextureDirty()
{
	bTextureChanged = true;
	if (CheckRenderCanvas()) RenderCanvas->MarkCanvasUpdate();
}
void UUIRenderable::MarkMaterialDirty()
{
	bMaterialChanged = true;
	if (CheckRenderCanvas()) RenderCanvas->MarkCanvasUpdate();
}

void UUIRenderable::AddGeometryModifier(class UUIGeometryModifierBase* InModifier)
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
void UUIRenderable::RemoveGeometryModifier(class UUIGeometryModifierBase* InModifier)
{
	GeometryModifierComponentArray.Remove(InModifier);
}

void UUIRenderable::UpdateCachedData()
{
	cacheForThisUpdate_UVChanged = bUVChanged;
	cacheForThisUpdate_TriangleChanged = bTriangleChanged;
	cacheForThisUpdate_TextureChanged = bTextureChanged ;
	cacheForThisUpdate_MaterialChanged = bMaterialChanged;
	Super::UpdateCachedData();
}
void UUIRenderable::UpdateCachedDataBeforeGeometry()
{
	if (bUVChanged)cacheForThisUpdate_UVChanged = true;
	if (bTriangleChanged)cacheForThisUpdate_TriangleChanged = true;
	if (bTextureChanged)cacheForThisUpdate_TextureChanged = true;
	if (bMaterialChanged)cacheForThisUpdate_MaterialChanged = true;
	Super::UpdateCachedDataBeforeGeometry();
}
void UUIRenderable::UpdateBasePrevData()
{
	bUVChanged = false;
	bTriangleChanged = false;
	bTextureChanged = false;
	bMaterialChanged = false;
	Super::UpdateBasePrevData();
}
void UUIRenderable::MarkAllDirtyRecursive()
{
	bUVChanged = true;
	bTriangleChanged = true;
	bTextureChanged = true;
	bMaterialChanged = true;
	Super::MarkAllDirtyRecursive();
}
void UUIRenderable::SetCustomUIMaterial(UMaterialInterface* inMat)
{
	if (CustomUIMaterial != inMat)
	{
		CustomUIMaterial = inMat;
		bMaterialChanged = true;
	}
}
UMaterialInstanceDynamic* UUIRenderable::GetMaterialInstanceDynamic()
{
	if (!CheckRenderCanvas())return nullptr;
	return RenderCanvas->GetMaterialInstanceDynamicForDrawcall(geometry->drawcallIndex);
}
bool UUIRenderable::HaveGeometryModifier()
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
bool UUIRenderable::ApplyGeometryModifier()
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
			modifierComp->ModifyUIGeometry(geometry, originVerticesCount, originTriangleIndicesCount, thisModifierAffectTriangleCount);
			if (thisModifierAffectTriangleCount)modifierAffectTriangleCount = true;
		}
		return modifierAffectTriangleCount;
	}
	return false;
}

DECLARE_CYCLE_STAT(TEXT("UIRenderable UpdateGeometry"), STAT_UIRenderableUpdateGeometry, STATGROUP_LGUI);
void UUIRenderable::UpdateGeometry(const bool& parentTransformChanged)
{
	SCOPE_CYCLE_COUNTER(STAT_UIRenderableUpdateGeometry);
	if (IsUIActiveInHierarchy() == false)return;
	if (!CheckRenderCanvas())return;

	OnBeforeCreateOrUpdateGeometry();
	if (geometry->vertices.Num() == 0//if geometry not created yet
		|| geometry->drawcallIndex == -1//if geometry not rendered yet
		)
	{
		if (HaveDataToCreateGeometry())
		{
			CreateGeometry();
			RenderCanvas->MarkRebuildAllDrawcall();
		}
		else
		{
			if (geometry->vertices.Num() > 0)
			{
				geometry->Clear();
			}
		}
	}
	else//if geometry is created, update data
	{
		if (cacheForThisUpdate_TextureChanged || cacheForThisUpdate_MaterialChanged)//texture change or material change, need to recreate drawcall
		{
			if (NeedTextureToCreateGeometry() && !IsValid(GetTextureToCreateGeometry()))//need texture, but texture is not valid
			{
				geometry->Clear();
				RenderCanvas->MarkRebuildSpecificDrawcall(geometry->drawcallIndex);
				goto COMPLETE;
			}
			CreateGeometry();
			RenderCanvas->MarkRebuildAllDrawcall();
			goto COMPLETE;
		}
		if (cacheForThisUpdate_DepthChanged)
		{
			if (IsValid(CustomUIMaterial))
			{
				CreateGeometry();
				RenderCanvas->MarkRebuildAllDrawcall();
			}
			else
			{
				geometry->depth = widget.depth;
				RenderCanvas->OnUIElementDepthChange(this);
			}
		}
		if (cacheForThisUpdate_TriangleChanged)//triangle change, need to clear geometry then recreate the specific drawcall
		{
			CreateGeometry();
			RenderCanvas->MarkRebuildSpecificDrawcall(geometry->drawcallIndex);
			goto COMPLETE;
		}
		else//update geometry
		{
			OnUpdateGeometry(cacheForThisUpdate_VertexPositionChanged, cacheForThisUpdate_UVChanged, cacheForThisUpdate_ColorChanged);

			if (parentTransformChanged)
			{
				cacheForThisUpdate_VertexPositionChanged = true;
			}
			if (cacheForThisUpdate_UVChanged || cacheForThisUpdate_ColorChanged || cacheForThisUpdate_VertexPositionChanged)//vertex data change, need to update geometry's vertex
			{
				if (ApplyGeometryModifier())
				{
					RenderCanvas->MarkRebuildSpecificDrawcall(geometry->drawcallIndex);
				}
				else
				{
					RenderCanvas->MarkUpdateSpecificDrawcallVertex(geometry->drawcallIndex, cacheForThisUpdate_VertexPositionChanged);
				}
				if (cacheForThisUpdate_VertexPositionChanged)
				{
					UIGeometry::TransformVertices(RenderCanvas, this, geometry);
				}
			}
		}
	}
COMPLETE:
	;
}

void UUIRenderable::CreateGeometry()
{
	geometry->Clear();
	if (NeedTextureToCreateGeometry())
	{
		geometry->texture = GetTextureToCreateGeometry();
		if (!IsValid(geometry->texture))
		{
			UE_LOG(LGUI, Error, TEXT("[UUIRenderable::CreateGeometry]Need texture to create geometry, but provided texture is no valid!"));
		}
	}
	geometry->material = CustomUIMaterial;
	geometry->depth = widget.depth;
	OnCreateGeometry();
	ApplyGeometryModifier();

	UIGeometry::TransformVertices(RenderCanvas, this, geometry);
}

void UUIRenderable::OnBeforeCreateOrUpdateGeometry()
{
	UE_LOG(LGUI, Error, TEXT("[UUIRenderable::OnBeforeCreateOrUpdateGeometry]This function must be override!"));
}
void UUIRenderable::OnCreateGeometry()
{
	UE_LOG(LGUI, Error, TEXT("[UUIRenderable::OnCreateGeometry]This function must be override!"));
}
void UUIRenderable::OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	UE_LOG(LGUI, Error, TEXT("[UUIRenderable::OnUpdateGeometry]This function must be override!"));
}


bool UUIRenderable::LineTraceUI(FHitResult& OutHit, const FVector& Start, const FVector& End)
{
	if (bRaycastComplex)
	{
		if (!bRaycastTarget)return false;
		if (!IsUIActiveInHierarchy())return false;
		if (!IsValid(RenderCanvas))return false;
		auto inverseTf = GetComponentTransform().Inverse();
		auto localSpaceRayOrigin = inverseTf.TransformPosition(Start);
		auto localSpaceRayEnd = inverseTf.TransformPosition(End);

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