// Copyright 2019 LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UISpriteBase.h"
#include "LGUI.h"
#include "Core/UIGeometry.h"
#include "Core/ActorComponent/UIPanel.h"


UUISpriteBase::UUISpriteBase()
{
	PrimaryComponentTick.bCanEverTick = true;
}
UUISpriteBase::~UUISpriteBase()
{
	
}

void UUISpriteBase::BeginPlay()
{
	Super::BeginPlay();
	if (sprite != nullptr)
	{
		sprite->AddUISprite(this);
	}
}

void UUISpriteBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (sprite != nullptr)
	{
		sprite->RemoveUISprite(this);
	}
}

void UUISpriteBase::ApplyAtlasTextureScaleUp()
{
	auto& uvs = geometry->uvs;
	if (uvs.Num() != 0)
	{
		for (int i = 0; i < uvs.Num(); i++)
		{
			auto& uv = uvs[i];
			uv.X *= 0.5f;
			uv.Y *= 0.5f;
		}
		MarkUVDirty();
	}
	geometry->texture = sprite->GetAtlasTexture();
	if (CheckRenderUIPanel())
	{
		RenderUIPanel->SetDrawcallTexture(geometry->drawcallIndex, sprite->GetAtlasTexture());
	}
}

void UUISpriteBase::SetSprite(ULGUISpriteData* newSprite, bool setSize)
{
	if (sprite != newSprite)
	{
		if((sprite == nullptr || newSprite == nullptr)
			|| (sprite->GetAtlasTexture() != newSprite->GetAtlasTexture()))
		{
			MarkTextureDirty();
			//remove from old
			if (sprite != nullptr)
			{
				sprite->RemoveUISprite(this);
			}
			//add to new
			if (newSprite != nullptr)
			{
				newSprite->AddUISprite(this);
			}
		}
		sprite = newSprite;
		MarkUVDirty();
		if (setSize) SetSizeFromSpriteData();
	}
}
void UUISpriteBase::SetSizeFromSpriteData()
{
	if (sprite != nullptr)
	{
		SetWidth(sprite->GetSpriteInfo()->width);
		SetHeight(sprite->GetSpriteInfo()->height);
	}
	else
	{
		UE_LOG(LGUI, Error, TEXT("[UUISpriteBase::SetSizeFromSpriteData]sprite is null!"));
	}
}

void UUISpriteBase::OnRegister()
{
	Super::OnRegister();
#if WITH_EDITOR
	if (this->GetWorld() && this->GetWorld()->WorldType == EWorldType::Editor)
	{
		if (sprite != nullptr)
		{
			sprite->AddUISprite(this);
		}
	}
#endif
}
void UUISpriteBase::OnUnregister()
{
	Super::OnUnregister();
#if WITH_EDITOR
	if (this->GetWorld() && this->GetWorld()->WorldType == EWorldType::Editor)
	{
		if (sprite != nullptr)
		{
			sprite->RemoveUISprite(this);
		}
	}
#endif
}
#if WITH_EDITOR
void UUISpriteBase::EditorForceUpdateImmediately()
{
	Super::EditorForceUpdateImmediately();
}
void UUISpriteBase::CheckSpriteData()
{
	if (sprite == nullptr)
	{
		sprite = ULGUISpriteData::GetDefaultWhiteSolid();
		sprite->AddUISprite(this);
	}
}
void UUISpriteBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
void UUISpriteBase::OnPreChangeSpriteProperty()
{
	if (sprite != nullptr)
	{
		sprite->RemoveUISprite(this);
	}
}
void UUISpriteBase::OnPostChangeSpriteProperty()
{
	if (sprite != nullptr)
	{
		sprite->AddUISprite(this);
	}
}
#endif

DECLARE_CYCLE_STAT(TEXT("UISprite UpdateGeometry"), STAT_UISpriteUpdateGeometry, STATGROUP_LGUI);


void UUISpriteBase::UpdateGeometry(const bool& parentTransformChanged)
{
	SCOPE_CYCLE_COUNTER(STAT_UISpriteUpdateGeometry);
	if (IsUIActiveInHierarchy() == false)return;
	if (sprite == nullptr)
	{
		//UE_LOG(LGUI, Log, TEXT("[UISpriteBase::UpdateGeometry]UISpriteBase:%s sprite is null"), *(this->GetFullName()));
		if (cacheForThisUpdate_TextureChanged)
		{
			if (geometry->vertices.Num() != 0)//have geometry data, clear it
			{
				geometry->Clear();
				RenderUIPanel->MarkRebuildSpecificDrawcall(geometry->drawcallIndex);
			}
		}
		return;
	}
	if (sprite->GetAtlasTexture() == nullptr)
	{
		//UE_LOG(LGUI, Error, TEXT("[UISpriteBase::UpdateGeometry]UISpriteBase:%s atlasTexture is null"), *(this->GetFullName()));
		if (cacheForThisUpdate_TextureChanged)
		{
			if (geometry->vertices.Num() != 0)//have geometry data, clear it
			{
				geometry->Clear();
				RenderUIPanel->MarkRebuildSpecificDrawcall(geometry->drawcallIndex);
			}
		}
		return;
	}
	if (!CheckRenderUIPanel())return;

	if (geometry->vertices.Num() == 0)//if geometry not created yet
	{
		CreateGeometry();
		RenderUIPanel->MarkRebuildAllDrawcall();
	}
	else//if geometry is created, update data
	{
		if (cacheForThisUpdate_TextureChanged || cacheForThisUpdate_MaterialChanged)//texture change or material change, need to recreate drawcall
		{
			if (sprite == nullptr)//sprite is cleared
			{
				geometry->Clear();
				RenderUIPanel->MarkRebuildSpecificDrawcall(geometry->drawcallIndex);
				goto COMPLETE;
			}
			CreateGeometry();
			RenderUIPanel->MarkRebuildAllDrawcall();
			goto COMPLETE;
		}
		if (cacheForThisUpdate_DepthChanged)
		{
			if (CustomUIMaterial != nullptr)
			{
				CreateGeometry();
				RenderUIPanel->MarkRebuildAllDrawcall();
			}
			else
			{
				geometry->depth = widget.depth;
				RenderUIPanel->DepthChangeForDrawcall(this);
			}
		}
		if (cacheForThisUpdate_TriangleChanged)//triangle change, need to clear geometry then recreate the specific drawcall
		{
			CreateGeometry();
			RenderUIPanel->MarkRebuildSpecificDrawcall(geometry->drawcallIndex);
			goto COMPLETE;
		}
		else//update geometry
		{
			OnUpdateGeometry(cacheForThisUpdate_VertexPositionChanged, cacheForThisUpdate_UVChanged, cacheForThisUpdate_ColorChanged);

			if (parentTransformChanged)
			{
				cacheForThisUpdate_VertexPositionChanged = true;
			}
			if (cacheForThisUpdate_UVChanged || cacheForThisUpdate_ColorChanged || cacheForThisUpdate_VertexPositionChanged)//vertex data change, need panel to update geometry's vertex
			{
				if (ApplyGeometryModifier())
				{
					UIGeometry::CheckAndApplyAdditionalChannel(geometry);
					RenderUIPanel->MarkRebuildSpecificDrawcall(geometry->drawcallIndex);
				}
				else
				{
					RenderUIPanel->MarkUpdateSpecificDrawcallVertex(geometry->drawcallIndex, cacheForThisUpdate_VertexPositionChanged);
				}
				if (cacheForThisUpdate_VertexPositionChanged)
				{
					UIGeometry::TransformVertices(RenderUIPanel, this, geometry, RenderUIPanel->GetRequireNormal(), RenderUIPanel->GetRequireTangent());
				}
			}
		}
	}
COMPLETE:
	;
	if (!geometry->CheckDataValid())
	{
		UE_LOG(LGUI, Error, TEXT("[UISpriteBase::UpdateGeometry]UISpriteBase:%s geometry is not valid!"), *(this->GetFullName()));
	}
}

void UUISpriteBase::CreateGeometry()
{
	geometry->Clear();
	geometry->texture = sprite->GetAtlasTexture();
	geometry->material = CustomUIMaterial;
	geometry->depth = widget.depth;
	OnCreateGeometry();
	ApplyGeometryModifier();
	
	UIGeometry::CheckAndApplyAdditionalChannel(geometry);
	UIGeometry::TransformVertices(RenderUIPanel, this, geometry, RenderUIPanel->GetRequireNormal(), RenderUIPanel->GetRequireTangent());
}
void UUISpriteBase::OnCreateGeometry()
{
	UE_LOG(LGUI, Error, TEXT("[UUISpriteBase::OnCreateGeometry]This function must be override!"));
}
void UUISpriteBase::OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	UE_LOG(LGUI, Error, TEXT("[UUISpriteBase::OnUpdateGeometry]This function must be override!"));
}