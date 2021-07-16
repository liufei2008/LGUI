// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UIBatchGeometryRenderable.h"
#include "LGUI.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Utils/LGUIUtils.h"
#include "GeometryModifier/UIGeometryModifierBase.h"
#include "Core/LGUIMesh/UIDrawcallMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Core/UIDrawcall.h"

DECLARE_CYCLE_STAT(TEXT("UIBatchGeometryRenderable ApplyModifier"), STAT_ApplyModifier, STATGROUP_LGUI);

UUIBatchGeometryRenderable::UUIBatchGeometryRenderable(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	uiRenderableType = EUIRenderableType::UIBatchGeometryRenderable;
	geometry = TSharedPtr<UIGeometry>(new UIGeometry);

	bLocalVertexPositionChanged = true;
	bUVChanged = true;
	bTriangleChanged = true;
	bTextureChanged = true;
	bMaterialChanged = true;
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
	bTextureChanged = true;
	bMaterialChanged = true;
}

void UUIBatchGeometryRenderable::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );
}

void UUIBatchGeometryRenderable::ApplyUIActiveState()
{
	bUVChanged = true;
	bTriangleChanged = true;
	bTextureChanged = true;
	bMaterialChanged = true;
	if (!IsUIActiveInHierarchy())
	{
		if (bIsSelfRender)
		{
			UpdateSelfRenderDrawcall();
		}
		else
		{
			if (RenderCanvas.IsValid() && drawcall.IsValid())
			{
				RenderCanvas->RemoveUIRenderable(this);
			}
		}
	}
	UUIItem::ApplyUIActiveState();
}
#if WITH_EDITOR
void UUIBatchGeometryRenderable::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	bUVChanged = true;
	bTriangleChanged = true;
	bTextureChanged = true;
	bMaterialChanged = true;
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.Property)
	{
		if (Property->GetName() == TEXT("bIsSelfRender"))
		{
			if (bIsSelfRender)//prev is not self renderer, then remove this from canvas
			{
				if (RenderCanvas.IsValid() && drawcall.IsValid())
				{
					RenderCanvas->RemoveUIRenderable(this);
				}
			}
			else//prev is self renderer, then ui mesh
			{
				if (IsValid(uiMesh))//delete ui mesh when not self render
				{
					uiMesh->DestroyComponent();
					uiMesh = nullptr;
				}
			}
		}
	}
}
#endif
void UUIBatchGeometryRenderable::OnUnregister()
{
	Super::OnUnregister();
	if (IsValid(uiMesh))//delete ui mesh when this component is delete
	{
		uiMesh->DestroyComponent();
		uiMesh = nullptr;
	}
}

void UUIBatchGeometryRenderable::OnRenderCanvasChanged(ULGUICanvas* OldCanvas, ULGUICanvas* NewCanvas)
{
	if (IsValid(OldCanvas))
	{
		if (!bIsSelfRender)
		{
			if (drawcall.IsValid())
			{
				OldCanvas->RemoveUIRenderable(this);
			}
		}
		OldCanvas->MarkCanvasUpdate();
	}
	if (IsValid(NewCanvas))
	{
		if (!bIsSelfRender)
		{
			if (IsValid(OldCanvas))
			{
				if (OldCanvas->IsRenderToScreenSpaceOrRenderTarget() != NewCanvas->IsRenderToScreenSpaceOrRenderTarget())//is render to screen or world changed, then uimesh need to be recreate
				{
					geometry->Clear();
					if (IsValid(uiMesh))
					{
						uiMesh->DestroyComponent();
						uiMesh = nullptr;
					}
				}
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
	bTextureChanged = true;
	MarkCanvasUpdate();
}
void UUIBatchGeometryRenderable::MarkMaterialDirty()
{
	bMaterialChanged = true;
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
	cacheForThisUpdate_TextureChanged = bTextureChanged ;
	cacheForThisUpdate_MaterialChanged = bMaterialChanged;
	Super::UpdateCachedData();
}
void UUIBatchGeometryRenderable::UpdateCachedDataBeforeGeometry()
{
	if (bLocalVertexPositionChanged)cacheForThisUpdate_LocalVertexPositionChanged = true;
	if (bUVChanged)cacheForThisUpdate_UVChanged = true;
	if (bTriangleChanged)cacheForThisUpdate_TriangleChanged = true;
	if (bTextureChanged)cacheForThisUpdate_TextureChanged = true;
	if (bMaterialChanged)cacheForThisUpdate_MaterialChanged = true;
	Super::UpdateCachedDataBeforeGeometry();
}
void UUIBatchGeometryRenderable::UpdateBasePrevData()
{
	bLocalVertexPositionChanged = false;
	bUVChanged = false;
	bTriangleChanged = false;
	bTextureChanged = false;
	bMaterialChanged = false;
	Super::UpdateBasePrevData();
}
void UUIBatchGeometryRenderable::MarkAllDirtyRecursive()
{
	bLocalVertexPositionChanged = true;
	bUVChanged = true;
	bTriangleChanged = true;
	bTextureChanged = true;
	bMaterialChanged = true;
	Super::MarkAllDirtyRecursive();
}
void UUIBatchGeometryRenderable::SetCustomUIMaterial(UMaterialInterface* inMat)
{
	if (CustomUIMaterial != inMat)
	{
		CustomUIMaterial = inMat;
		bMaterialChanged = true;
	}
}
void UUIBatchGeometryRenderable::SetIsSelfRender(bool value)
{
	if (bIsSelfRender != value)
	{
		bIsSelfRender = value;
		if (bIsSelfRender)
		{
			if (RenderCanvas.IsValid() && drawcall.IsValid())
			{
				RenderCanvas->RemoveUIRenderable(this);
			}
		}
		else
		{
			//destroy mesh if not selfrender, because canvas will render it
			if (IsValid(uiMesh))
			{
				uiMesh->DestroyComponent();
				uiMesh = nullptr;
			}
		}
		MarkCanvasUpdate();
	}
}
UMaterialInstanceDynamic* UUIBatchGeometryRenderable::GetMaterialInstanceDynamic()const
{
	if (bIsSelfRender)
	{
		return uiMaterial;
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

DECLARE_CYCLE_STAT(TEXT("UIGeometryRenderable UpdateRenderable"), STAT_UIGeometryRenderableUpdate, STATGROUP_LGUI);
void UUIBatchGeometryRenderable::UpdateGeometry(const bool& parentLayoutChanged)
{
	SCOPE_CYCLE_COUNTER(STAT_UIGeometryRenderableUpdate);
	if (IsUIActiveInHierarchy() == false)return;
	if (!CheckRenderCanvas())return;

	if (bIsSelfRender)
	{
		UpdateGeometry_ImplementForSelfRender(parentLayoutChanged);
	}
	else
	{
		if (RenderCanvas->GetAutoManageDepth())
		{
			UpdateGeometry_ImplementForAutoManageDepth(parentLayoutChanged);
		}
		else
		{
			UpdateGeometry_Implement(parentLayoutChanged);
		}
	}
}

void UUIBatchGeometryRenderable::UpdateGeometry_Implement(const bool& parentLayoutChanged)
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
		if (cacheForThisUpdate_TextureChanged || cacheForThisUpdate_MaterialChanged)//texture change or material change, need to recreate drawcall
		{
			if (NeedTextureToCreateGeometry() && !IsValid(GetTextureToCreateGeometry()))//need texture, but texture is not valid
			{
				UE_LOG(LGUI, Error, TEXT("[UUIGeometryRenderable::CreateGeometry]Need texture to create geometry, but texture is no valid!"));
				RenderCanvas->RemoveUIRenderable(this);
				goto COMPLETE;
			}
			drawcall->textureChanged = cacheForThisUpdate_TextureChanged;
			drawcall->materialChanged = cacheForThisUpdate_MaterialChanged;
		}
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
			bool pixelPerfect = RenderCanvas->GetActualPixelPerfect();
			OnUpdateGeometry(cacheForThisUpdate_LocalVertexPositionChanged || (parentLayoutChanged && pixelPerfect), cacheForThisUpdate_UVChanged, cacheForThisUpdate_ColorChanged);

			if (ApplyGeometryModifier(cacheForThisUpdate_UVChanged, cacheForThisUpdate_ColorChanged, cacheForThisUpdate_LocalVertexPositionChanged, parentLayoutChanged))//vertex data change, need to update geometry's vertex
			{
				drawcall->needToUpdateVertex = true;
			}
			else
			{
				drawcall->needToRebuildMesh = true;
			}
			if (cacheForThisUpdate_LocalVertexPositionChanged || parentLayoutChanged)
			{
				UIGeometry::TransformVertices(RenderCanvas.Get(), this, geometry);
			}
		}
	}
COMPLETE:
	;
}
void UUIBatchGeometryRenderable::UpdateGeometry_ImplementForAutoManageDepth(const bool& parentLayoutChanged)
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
	else//already add to render, update data
	{
		if (cacheForThisUpdate_TextureChanged || cacheForThisUpdate_MaterialChanged)//texture change or material change, need to recreate drawcall
		{
			if (NeedTextureToCreateGeometry() && !IsValid(GetTextureToCreateGeometry()))//need texture, but texture is not valid
			{
				UE_LOG(LGUI, Error, TEXT("[UUIGeometryRenderable::CreateGeometry]Need texture to create geometry, but texture is no valid!"));
				goto COMPLETE;
			}
			drawcall->textureChanged = cacheForThisUpdate_TextureChanged;
			drawcall->materialChanged = cacheForThisUpdate_MaterialChanged;
		}
		if (cacheForThisUpdate_TriangleChanged)//triangle change, need to clear geometry then recreate it, and mark update the specific drawcall
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
			bool pixelPerfect = RenderCanvas->GetActualPixelPerfect();
			OnUpdateGeometry(cacheForThisUpdate_LocalVertexPositionChanged || (parentLayoutChanged && pixelPerfect), cacheForThisUpdate_UVChanged, cacheForThisUpdate_ColorChanged);

			if (ApplyGeometryModifier(cacheForThisUpdate_UVChanged, cacheForThisUpdate_ColorChanged, cacheForThisUpdate_LocalVertexPositionChanged, parentLayoutChanged))//vertex data change, need to update geometry's vertex
			{
				drawcall->needToUpdateVertex = true;
			}
			else
			{
				drawcall->needToRebuildMesh = true;
			}
			if (cacheForThisUpdate_LocalVertexPositionChanged || parentLayoutChanged)
			{
				UIGeometry::TransformVertices(RenderCanvas.Get(), this, geometry);
			}
		}
	}
COMPLETE:
	;
}
void UUIBatchGeometryRenderable::UpdateGeometry_ImplementForSelfRender(const bool& parentLayoutChanged)
{
	OnBeforeCreateOrUpdateGeometry();
	if (geometry->vertices.Num() == 0//if geometry not created yet
		)
	{
		CreateGeometry();
		UpdateSelfRenderDrawcall();
		UpdateSelfRenderMaterial(true, true);
		goto COMPLETE;
	}
	else//if geometry is created, update data
	{
		if (cacheForThisUpdate_TextureChanged || cacheForThisUpdate_MaterialChanged)//texture change or material change, need to recreate drawcall
		{
			if (NeedTextureToCreateGeometry() && !IsValid(GetTextureToCreateGeometry()))//need texture, but texture is not valid
			{
				UE_LOG(LGUI, Error, TEXT("[UUIGeometryRenderable::CreateGeometry]Need texture to create geometry, but texture is no valid!"));
				geometry->Clear();
				UpdateSelfRenderDrawcall();
				ClearSelfRenderMaterial();
				goto COMPLETE;
			}
			CreateGeometry();
			UpdateSelfRenderDrawcall();
			UpdateSelfRenderMaterial(cacheForThisUpdate_TextureChanged, cacheForThisUpdate_MaterialChanged);
			goto COMPLETE;
		}
		if (cacheForThisUpdate_TriangleChanged)//triangle change, need to clear geometry then recreate the specific drawcall
		{
			CreateGeometry();
			UpdateSelfRenderDrawcall();
			goto COMPLETE;
		}
		else//update geometry
		{
			bool pixelPerfect = RenderCanvas->GetActualPixelPerfect();
			OnUpdateGeometry(cacheForThisUpdate_LocalVertexPositionChanged || (parentLayoutChanged && pixelPerfect), cacheForThisUpdate_UVChanged, cacheForThisUpdate_ColorChanged);

			if (ApplyGeometryModifier(cacheForThisUpdate_UVChanged, cacheForThisUpdate_ColorChanged, cacheForThisUpdate_LocalVertexPositionChanged, parentLayoutChanged))//vertex data change, need to update geometry's vertex
			{
			}
			else
			{
			}
			if (cacheForThisUpdate_LocalVertexPositionChanged || parentLayoutChanged)
			{
				UIGeometry::TransformVerticesForSelfRender(RenderCanvas.Get(), geometry);
				UpdateSelfRenderDrawcall();
			}
		}
	}
COMPLETE:
	;
}

bool UUIBatchGeometryRenderable::CreateGeometry()
{
	if (HaveDataToCreateGeometry())
	{
		if (NeedTextureToCreateGeometry() && !IsValid(GetTextureToCreateGeometry()))
		{
			UE_LOG(LGUI, Error, TEXT("[UUIGeometryRenderable::CreateGeometry]Need texture to create geometry, but texture is no valid!"));
			return false;
		}
		geometry->Clear();
		geometry->texture = GetTextureToCreateGeometry();
		geometry->material = CustomUIMaterial;
		OnCreateGeometry();
		ApplyGeometryModifier(true, true, true, true);
		if (bIsSelfRender)
		{
			UIGeometry::TransformVerticesForSelfRender(RenderCanvas.Get(), geometry);
		}
		else
		{
			UIGeometry::TransformVertices(RenderCanvas.Get(), this, geometry);
		}
		return true;
	}
	else
	{
		return false;
	}
}

//set, create, update, destroy
void UUIBatchGeometryRenderable::UpdateSelfRenderDrawcall()
{
	if (geometry->vertices.Num() == 0)
	{
		if (IsValid(uiMesh))
		{
			uiMesh->SetUIMeshVisibility(false);
		}
	}
	else
	{
		if (!IsValid(uiMesh))
		{
			uiMesh = NewObject<UUIDrawcallMesh>(this->GetOwner(), NAME_None, RF_Transient);
			uiMesh->RegisterComponent();
			//this->GetOwner()->AddInstanceComponent(uiMesh.Get());
			uiMesh->AttachToComponent(this->GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
			uiMesh->SetRelativeTransform(FTransform::Identity);
#if WITH_EDITOR
			if (!GetWorld()->IsGameWorld())
			{
				if (RenderCanvas->GetRootCanvas()->IsRenderToScreenSpaceOrRenderTarget())
				{
					uiMesh->SetSupportScreenSpace(true, RenderCanvas->GetRootCanvas()->GetViewExtension());
				}
				uiMesh->SetSupportWorldSpace(true);
			}
			else
#endif
			if (RenderCanvas->GetRootCanvas()->IsRenderToScreenSpaceOrRenderTarget())
			{
				uiMesh->SetSupportScreenSpace(true, RenderCanvas->GetRootCanvas()->GetViewExtension());
				uiMesh->SetSupportWorldSpace(false);
			}
		}

		//set data for mesh
		{
			auto& meshSection = uiMesh->MeshSection;
			meshSection.vertices = geometry->vertices;
			meshSection.triangles = geometry->triangles;
			uiMesh->SetUIMeshVisibility(true);
			uiMesh->GenerateOrUpdateMesh(true, RenderCanvas->GetActualAdditionalShaderChannelFlags());
			uiMesh->SetUITranslucentSortPriority(this->widget.depth);
		}
	}
}
void UUIBatchGeometryRenderable::UpdateSelfRenderMaterial(bool textureChange, bool materialChange)
{
	if (IsValid(uiMesh))
	{
		if (materialChange)
		{
			UMaterialInterface* SrcMaterial = CustomUIMaterial;
			if (!IsValid(SrcMaterial))
			{
				SrcMaterial = LoadObject<UMaterialInterface>(NULL, TEXT("/LGUI/LGUI_Standard"));
			}
			UMaterialInstanceDynamic* uiMat = nullptr;
			if (SrcMaterial->IsA(UMaterialInstanceDynamic::StaticClass()))
			{
				uiMat = (UMaterialInstanceDynamic*)SrcMaterial;
			}
			else
			{
				uiMat = UMaterialInstanceDynamic::Create(SrcMaterial, this);
				uiMat->SetFlags(RF_Transient);
			}
			uiMesh->SetMaterial(0, uiMat);
			uiMaterial = uiMat;
		}
		if (textureChange)
		{
			if (IsValid(uiMaterial))
			{
				uiMaterial->SetTextureParameterValue(FName("MainTexture"), GetTextureToCreateGeometry());
			}
		}
	}
}
void UUIBatchGeometryRenderable::ClearSelfRenderMaterial()
{
	if (IsValid(uiMaterial))
	{
		uiMaterial->ConditionalBeginDestroy();
		uiMaterial = nullptr;
	}
}

void UUIBatchGeometryRenderable::DepthChanged()
{
	if (bIsSelfRender)
	{
		UpdateSelfRenderDrawcall();
	}
	else
	{
		Super::DepthChanged();
	}
}

bool UUIBatchGeometryRenderable::LineTraceUI(FHitResult& OutHit, const FVector& Start, const FVector& End)
{
	if (bRaycastComplex)
	{
		if (!bRaycastTarget)return false;
		if (!IsUIActiveInHierarchy())return false;
		if (!RenderCanvas.IsValid())return false;
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