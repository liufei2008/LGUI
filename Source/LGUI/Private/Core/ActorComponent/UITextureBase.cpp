// Copyright 2019 LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UITextureBase.h"
#include "LGUI.h"
#include "Core/UIGeometry.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Materials/MaterialInterface.h"


UUITextureBase::UUITextureBase()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUITextureBase::BeginPlay()
{
	Super::BeginPlay();
}
#if WITH_EDITOR
void UUITextureBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
void UUITextureBase::EditorForceUpdateImmediately()
{
	Super::EditorForceUpdateImmediately();
}
void UUITextureBase::CheckTexture()
{
	if (!IsValid(texture))
	{
		static auto defaultWhiteSolid = LoadObject<UTexture2D>(NULL, TEXT("/LGUI/Textures/LGUIPreset_WhiteSolid"));
		if (defaultWhiteSolid != nullptr)
		{
			texture = (UTexture*)defaultWhiteSolid;
		}
		else
		{
			UE_LOG(LGUI, Error, TEXT("[UUITextureBase::CheckTexture]Load default texture error! Missing some content of LGUI plugin, reinstall this plugin may fix the issure."));
		}
	}
}
#endif

void UUITextureBase::UpdateGeometry(const bool& parentTransformChanged)
{
	if (IsUIActiveInHierarchy() == false)return;
	if (!IsValid(texture))
	{
		//UE_LOG(LGUI, Log, TEXT("texture is null"));
		if (cacheForThisUpdate_TextureChanged)
		{
			if (geometry->vertices.Num() != 0)//have geometry data, clear it
			{
				geometry->Clear();
				RenderCanvas->MarkRebuildSpecificDrawcall(geometry->drawcallIndex);
			}
		}
		return;
	}
	if (!CheckRenderCanvas())return;

	if (geometry->vertices.Num() == 0)//if geometry not created yet
	{
		CreateGeometry();
		RenderCanvas->MarkRebuildAllDrawcall();
	}
	else//if geometry is created, update data
	{
		if (cacheForThisUpdate_TextureChanged || cacheForThisUpdate_MaterialChanged)//texture change or material change, need to recreate drawcall
		{
			if (!IsValid(texture))//texture is cleared
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
					UIGeometry::CheckAndApplyAdditionalChannel(geometry);
					RenderCanvas->MarkRebuildSpecificDrawcall(geometry->drawcallIndex);
				}
				else
				{
					RenderCanvas->MarkUpdateSpecificDrawcallVertex(geometry->drawcallIndex, cacheForThisUpdate_VertexPositionChanged);
				}
				if (cacheForThisUpdate_VertexPositionChanged)
				{
					UIGeometry::TransformVertices(RenderCanvas, this, geometry, RenderCanvas->GetRequireNormal(), RenderCanvas->GetRequireTangent());
				}
			}
		}
	}
COMPLETE:
	;
	if (!geometry->CheckDataValid())
	{
		UE_LOG(LGUI, Error, TEXT("[UUITextureBase::UpdateGeometry]UITextureBase:%s geometry is not valid!"), *(this->GetFullName()));
	}
}

void UUITextureBase::CreateGeometry()
{
	geometry->Clear();
	geometry->texture = texture;
	geometry->material = CustomUIMaterial;
	geometry->depth = widget.depth;
	OnCreateGeometry();
	ApplyGeometryModifier();
	UIGeometry::CheckAndApplyAdditionalChannel(geometry);
	UIGeometry::TransformVertices(RenderCanvas, this, geometry, RenderCanvas->GetRequireNormal(), RenderCanvas->GetRequireTangent());
}

void UUITextureBase::OnCreateGeometry()
{
	UE_LOG(LGUI, Error, TEXT("[UUITextureBase::OnCreateGeometry]This function must be override!"));
}
void UUITextureBase::OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	UE_LOG(LGUI, Error, TEXT("[UUITextureBase::OnCreateGeometry]This function must be override!"));
}

void UUITextureBase::SetTexture(UTexture* newTexture)
{
	if (texture != newTexture)
	{
		texture = newTexture;
		MarkTextureDirty();
	}
}
void UUITextureBase::SetSizeFromTexture()
{
	if (IsValid(texture))
	{
		SetWidth(texture->GetSurfaceWidth());
		SetHeight(texture->GetSurfaceHeight());
	}
	else
	{
		UE_LOG(LGUI, Error, TEXT("[UUITextureBase::SetSizeFromTexture]texture is null!"));
	}
}