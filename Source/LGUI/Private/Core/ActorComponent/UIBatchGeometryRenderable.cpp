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
	MarkCanvasUpdate();

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
void UUIBatchGeometryRenderable::OnRegister()
{
	Super::OnRegister();
}
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
				}
				else
				{
					geometry->texture = GetTextureToCreateGeometry();
				}
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
			geometry->material = CustomUIMaterial;
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
	bLocalVertexPositionChanged = false;
	bUVChanged = false;
	bTriangleChanged = false;
	Super::UpdateCachedData();
}
void UUIBatchGeometryRenderable::UpdateCachedDataBeforeGeometry()
{
	if (bLocalVertexPositionChanged)cacheForThisUpdate_LocalVertexPositionChanged = true;
	if (bUVChanged)cacheForThisUpdate_UVChanged = true;
	if (bTriangleChanged)cacheForThisUpdate_TriangleChanged = true;
	Super::UpdateCachedDataBeforeGeometry();
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
	if (IsValid(CustomUIMaterial))
	{
		if (CustomUIMaterial->IsA(UMaterialInstanceDynamic::StaticClass()))
		{
			return (UMaterialInstanceDynamic*)CustomUIMaterial;//if CustomUIMaterial is a MaterialInstanceDynamic then just return it directly
		}
	}
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
			&& (RenderCanvas.IsValid() && RenderCanvas->GetActualPixelPerfect())//@todo: review this line, is this necessary? (old commit: fix pixel perfect update.)
			)
		{
			cacheForThisUpdate_LocalVertexPositionChanged = true;
		}
	}
}

void UUIBatchGeometryRenderable::UpdateGeometry()
{
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
	return UUIBaseRenderable::LineTraceUIGeometry(geometry, OutHit, Start, End);
}