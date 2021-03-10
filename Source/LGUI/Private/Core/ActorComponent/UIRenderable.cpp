// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UIRenderable.h"
#include "LGUI.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Utils/LGUIUtils.h"
#include "GeometryModifier/UIGeometryModifierBase.h"
#include "Core/LGUIMesh/UIDrawcallMesh.h"

DECLARE_CYCLE_STAT(TEXT("UIRenderable ApplyModifier"), STAT_ApplyModifier, STATGROUP_LGUI);

UUIRenderable::UUIRenderable(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	uiRenderableType = EUIRenderableType::UIGeometryRenderable;
	geometry = TSharedPtr<UIGeometry>(new UIGeometry);

	bLocalVertexPositionChanged = true;
	bUVChanged = true;
	bTriangleChanged = true;
	bTextureChanged = true;
	bMaterialChanged = true;
}

void UUIRenderable::BeginPlay()
{
	Super::BeginPlay();
	if (CheckRenderCanvas())
	{
		if (!bIsSelfRender)
		{
			RenderCanvas->MarkRebuildAllDrawcall();
		}
		RenderCanvas->MarkCanvasUpdate();
	}

	bLocalVertexPositionChanged = true;
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
				if (bIsSelfRender)
				{
					UpdateSelfRenderDrawcall();
				}
				else
				{
					RenderCanvas->MarkRebuildSpecificDrawcall(geometry->drawcallIndex);
				}
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
	if (auto Property = PropertyChangedEvent.Property)
	{
		if (Property->GetName() == TEXT("bIsSelfRender"))
		{
			if (!bIsSelfRender)
			{
				if (uiMesh.IsValid())//delete ui mesh when not self render
				{
					uiMesh->DestroyComponent();
					uiMesh.Reset();
				}
			}
		}
	}
}
#endif
void UUIRenderable::OnUnregister()
{
	Super::OnUnregister();
	if (uiMesh.IsValid())//delete ui mesh when this component is delete
	{
		uiMesh->DestroyComponent();
		uiMesh.Reset();
	}
}

void UUIRenderable::OnRenderCanvasChanged(ULGUICanvas* OldCanvas, ULGUICanvas* NewCanvas)
{
	if (IsValid(OldCanvas))
	{
		if (!bIsSelfRender)
		{
			OldCanvas->RemoveUIRenderable(this);
		}
		OldCanvas->MarkCanvasUpdate();
	}
	if (IsValid(NewCanvas))
	{
		if (!bIsSelfRender)
		{
			NewCanvas->AddUIRenderable(this);
			if (IsValid(OldCanvas))
			{
				if (OldCanvas->IsRenderToScreenSpaceOrRenderTarget() != NewCanvas->IsRenderToScreenSpaceOrRenderTarget())//is render to screen or world changed, then uimesh need to be recreate
				{
					geometry->Clear();
					if (uiMesh.IsValid())
					{
						uiMesh->DestroyComponent();
						uiMesh.Reset();
					}
				}
			}
		}
		NewCanvas->MarkCanvasUpdate();
	}
}

void UUIRenderable::WidthChanged()
{
	MarkVertexPositionDirty();
}
void UUIRenderable::HeightChanged()
{
	MarkVertexPositionDirty();
}
void UUIRenderable::PivotChanged()
{
	MarkVertexPositionDirty();
}

void UUIRenderable::MarkVertexPositionDirty()
{
	bLocalVertexPositionChanged = true;
	MarkCanvasUpdate();
}
void UUIRenderable::MarkUVDirty()
{
	bUVChanged = true;
	MarkCanvasUpdate();
}
void UUIRenderable::MarkTriangleDirty()
{
	bTriangleChanged = true;
	MarkCanvasUpdate();
}
void UUIRenderable::MarkTextureDirty()
{
	bTextureChanged = true;
	MarkCanvasUpdate();
}
void UUIRenderable::MarkMaterialDirty()
{
	bMaterialChanged = true;
	MarkCanvasUpdate();
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
	cacheForThisUpdate_LocalVertexPositionChanged = bLocalVertexPositionChanged;
	cacheForThisUpdate_UVChanged = bUVChanged;
	cacheForThisUpdate_TriangleChanged = bTriangleChanged;
	cacheForThisUpdate_TextureChanged = bTextureChanged ;
	cacheForThisUpdate_MaterialChanged = bMaterialChanged;
	Super::UpdateCachedData();
}
void UUIRenderable::UpdateCachedDataBeforeGeometry()
{
	if (bLocalVertexPositionChanged)cacheForThisUpdate_LocalVertexPositionChanged = true;
	if (bUVChanged)cacheForThisUpdate_UVChanged = true;
	if (bTriangleChanged)cacheForThisUpdate_TriangleChanged = true;
	if (bTextureChanged)cacheForThisUpdate_TextureChanged = true;
	if (bMaterialChanged)cacheForThisUpdate_MaterialChanged = true;
	Super::UpdateCachedDataBeforeGeometry();
}
void UUIRenderable::UpdateBasePrevData()
{
	bLocalVertexPositionChanged = false;
	bUVChanged = false;
	bTriangleChanged = false;
	bTextureChanged = false;
	bMaterialChanged = false;
	Super::UpdateBasePrevData();
}
void UUIRenderable::MarkAllDirtyRecursive()
{
	bLocalVertexPositionChanged = true;
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
void UUIRenderable::SetIsSelfRender(bool value)
{
	if (bIsSelfRender != value)
	{
		bIsSelfRender = value;
		if (!bIsSelfRender)
		{
			//destroy mesh if not selfrender, because canvas will render it
			if (uiMesh.IsValid())
			{
				uiMesh->DestroyComponent();
				uiMesh.Reset();
			}
		}
		MarkCanvasUpdate();
	}
}
UMaterialInstanceDynamic* UUIRenderable::GetMaterialInstanceDynamic()
{
	if (bIsSelfRender)
	{
		return uiMaterial.Get();
	}
	if (CheckRenderCanvas())
	{
		return RenderCanvas->GetMaterialInstanceDynamicForDrawcall(geometry->drawcallIndex);
	}
	return nullptr;
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
bool UUIRenderable::ApplyGeometryModifier(bool uvChanged, bool colorChanged, bool vertexPositionChanged, bool layoutChanged)
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
void UUIRenderable::UpdateGeometry(const bool& parentLayoutChanged)
{
	SCOPE_CYCLE_COUNTER(STAT_UIGeometryRenderableUpdate);
	if (IsUIActiveInHierarchy() == false)return;
	if (!CheckRenderCanvas())return;

	OnBeforeCreateOrUpdateGeometry();
	if (geometry->vertices.Num() == 0//if geometry not created yet
		|| geometry->drawcallIndex == -1//if geometry not rendered yet
		)
	{
		CreateGeometry();
		if (bIsSelfRender)
		{
			UpdateSelfRenderDrawcall();
			UpdateSelfRenderMaterial(true, true);
		}
		else
		{
			RenderCanvas->MarkRebuildAllDrawcall();
		}
		goto COMPLETE;
	}
	else//if geometry is created, update data
	{
		if (cacheForThisUpdate_TextureChanged || cacheForThisUpdate_MaterialChanged)//texture change or material change, need to recreate drawcall
		{
			if (NeedTextureToCreateGeometry() && !IsValid(GetTextureToCreateGeometry()))//need texture, but texture is not valid
			{
				geometry->Clear();
				if (bIsSelfRender)
				{
					UpdateSelfRenderDrawcall();
					ClearSelfRenderMaterial();
				}
				else
				{
					RenderCanvas->MarkRebuildSpecificDrawcall(geometry->drawcallIndex);
				}
				goto COMPLETE;
			}
			CreateGeometry();
			if (bIsSelfRender)
			{
				UpdateSelfRenderDrawcall();
				UpdateSelfRenderMaterial(cacheForThisUpdate_TextureChanged, cacheForThisUpdate_MaterialChanged);
			}
			else
			{
				RenderCanvas->MarkRebuildAllDrawcall();
			}
			goto COMPLETE;
		}
		if (cacheForThisUpdate_DepthChanged)
		{
			if (IsValid(CustomUIMaterial))
			{
				CreateGeometry();
				if (bIsSelfRender)
				{
					UpdateSelfRenderDrawcall();
				}
				else
				{
					RenderCanvas->MarkRebuildAllDrawcall();
				}
				goto COMPLETE;
			}
			else
			{
				geometry->depth = widget.depth;
				if (bIsSelfRender)
				{
					UpdateSelfRenderDrawcall();
				}
				else
				{
					RenderCanvas->OnUIElementDepthChange(this);
				}
			}
		}
		if (cacheForThisUpdate_TriangleChanged)//triangle change, need to clear geometry then recreate the specific drawcall
		{
			CreateGeometry();
			if (bIsSelfRender)
			{
				UpdateSelfRenderDrawcall();
			}
			else
			{
				RenderCanvas->MarkRebuildSpecificDrawcall(geometry->drawcallIndex);
			}
			goto COMPLETE;
		}
		else//update geometry
		{
			OnUpdateGeometry(cacheForThisUpdate_LocalVertexPositionChanged, cacheForThisUpdate_UVChanged, cacheForThisUpdate_ColorChanged);

			if (ApplyGeometryModifier(cacheForThisUpdate_UVChanged, cacheForThisUpdate_ColorChanged, cacheForThisUpdate_LocalVertexPositionChanged, parentLayoutChanged))//vertex data change, need to update geometry's vertex
			{
				if (!bIsSelfRender)
				{
					RenderCanvas->MarkRebuildSpecificDrawcall(geometry->drawcallIndex);
				}
			}
			else
			{
				if (!bIsSelfRender)
				{
					RenderCanvas->MarkUpdateSpecificDrawcallVertex(geometry->drawcallIndex, cacheForThisUpdate_LocalVertexPositionChanged || parentLayoutChanged);
				}
			}
			if (cacheForThisUpdate_LocalVertexPositionChanged || parentLayoutChanged)
			{
				if (bIsSelfRender)
				{
					UIGeometry::TransformVerticesForSelfRender(RenderCanvas, geometry);
					UpdateSelfRenderDrawcall();
				}
				else
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
	if (HaveDataToCreateGeometry())
	{
		geometry->Clear();
		if (NeedTextureToCreateGeometry())
		{
			geometry->texture = GetTextureToCreateGeometry();
			if (!geometry->texture.IsValid())
			{
				UE_LOG(LGUI, Error, TEXT("[UUIGeometryRenderable::CreateGeometry]Need texture to create geometry, but provided texture is no valid!"));
			}
		}
		geometry->material = CustomUIMaterial;
		geometry->depth = widget.depth;
		OnCreateGeometry();
		ApplyGeometryModifier(true, true, true, true);
		if (bIsSelfRender)
		{
			UIGeometry::TransformVerticesForSelfRender(RenderCanvas, geometry);
		}
		else
		{
			UIGeometry::TransformVertices(RenderCanvas, this, geometry);
		}
	}
	else
	{
		if (geometry->vertices.Num() > 0)
		{
			geometry->Clear();
			if (!bIsSelfRender)
			{
				RenderCanvas->MarkRebuildSpecificDrawcall(geometry->drawcallIndex);
			}
		}
	}
}

//set, create, update, destroy
void UUIRenderable::UpdateSelfRenderDrawcall()
{
	if (geometry->vertices.Num() == 0)
	{
		if (uiMesh.IsValid())
		{
			uiMesh->SetUIMeshVisibility(false);
		}
	}
	else
	{
		if (!uiMesh.IsValid())
		{
			uiMesh = NewObject<UUIDrawcallMesh>(this->GetOwner(), NAME_None, RF_Transient);
			this->GetOwner()->FinishAndRegisterComponent(uiMesh.Get());
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
			uiMesh->GenerateOrUpdateMesh(true, RenderCanvas->GetRootCanvas()->GetAdditionalShaderChannelFlags());
		}
	}
}
void UUIRenderable::UpdateSelfRenderMaterial(bool textureChange, bool materialChange)
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
		if (uiMaterial.IsValid())
		{
			uiMaterial->SetTextureParameterValue(FName("MainTexture"), GetTextureToCreateGeometry());
		}
	}
}
void UUIRenderable::ClearSelfRenderMaterial()
{
	if (uiMaterial.IsValid())
	{
		uiMaterial->ConditionalBeginDestroy();
		uiMaterial.Reset();
	}
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