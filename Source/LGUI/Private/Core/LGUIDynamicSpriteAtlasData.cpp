// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/LGUIDynamicSpriteAtlasData.h"
#include "Core/LGUIStaticSpriteAtlasData.h"
#include "LGUI.h"
#include "Core/LGUISettings.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "Core/ActorComponent/UISpriteBase.h"
#include "Core/LGUISpriteData.h"
#include "Utils/LGUIUtils.h"
#include "Rendering/Texture2DResource.h"
#include "Core/IUISpriteRenderableInterface.h"
#include "RenderingThread.h"


void FLGUIDynamicSpriteAtlasData::EnsureAtlasTexture(const FName& packingTag)
{
	if (!IsValid(atlasTexture))
	{
#if WITH_EDITOR
		int32 defaultAtlasTextureSize = ULGUISettings::GetAtlasTextureInitialSize(packingTag);
#else
		static int32 defaultAtlasTextureSize = ULGUISettings::GetAtlasTextureInitialSize(packingTag);
#endif
		atlasBinPack.Init(defaultAtlasTextureSize, defaultAtlasTextureSize);
		CreateAtlasTexture(packingTag, 0, defaultAtlasTextureSize);
	}
}
void FLGUIDynamicSpriteAtlasData::CreateAtlasTexture(const FName& packingTag, int oldTextureSize, int newTextureSize)
{
#if WITH_EDITOR
	bool atlasSRGB = ULGUISettings::GetAtlasTextureSRGB(packingTag);
	auto filter = ULGUISettings::GetAtlasTextureFilter(packingTag);
#else
	static bool atlasSRGB = ULGUISettings::GetAtlasTextureSRGB(packingTag);
	static auto filter = ULGUISettings::GetAtlasTextureFilter(packingTag);
#endif
	auto texture = LGUIUtils::CreateTexture(newTextureSize, FColor::Transparent
		, ULGUIDynamicSpriteAtlasManager::Instance
		, FName(*FString::Printf(TEXT("LGUIDynamicSpriteAtlasData_Texture_%d"), LGUIUtils::LGUITextureNameSuffix++))
	);

	texture->CompressionSettings = TextureCompressionSettings::TC_EditorIcon;
	texture->LODGroup = TextureGroup::TEXTUREGROUP_UI;
	texture->SRGB = atlasSRGB;
	texture->Filter = filter;
	texture->UpdateResource();
	texture->AddToRoot();//@todo: is this really need to AddToRoot?
	auto oldTexture = this->atlasTexture;
	this->atlasTexture = texture;

	//copy old texture to new one
	if (IsValid(oldTexture) && oldTextureSize > 0)
	{
		auto newTexture = texture;
		if (oldTexture->GetResource() != nullptr && newTexture->GetResource() != nullptr)
		{
			ENQUEUE_RENDER_COMMAND(FLGUIDynamicSpriteAtlas_CopyAtlasTexture)(
				[oldTexture, newTexture, oldTextureSize](FRHICommandListImmediate& RHICmdList)
			{
				FRHICopyTextureInfo CopyInfo;
				CopyInfo.SourcePosition = FIntVector(0, 0, 0);
				CopyInfo.Size = FIntVector(oldTextureSize, oldTextureSize, 0);
				CopyInfo.DestPosition = FIntVector(0, 0, 0);
				RHICmdList.CopyTexture(
					((FTexture2DResource*)oldTexture->GetResource())->GetTexture2DRHI(),
					((FTexture2DResource*)newTexture->GetResource())->GetTexture2DRHI(),
					CopyInfo
				);
				oldTexture->RemoveFromRoot();//ready for gc
			});
		}
	}
}
int32 FLGUIDynamicSpriteAtlasData::ExpendTextureSize(const FName& packingTag)
{
	int32 oldTextureSize = this->atlasBinPack.GetBinWidth();
	int32 newTextureSize = oldTextureSize + oldTextureSize;

	this->atlasBinPack.ExpendSize(newTextureSize, newTextureSize);
	//create new texture
	this->CreateAtlasTexture(packingTag, oldTextureSize, newTextureSize);
	//scale down sprite uv
	for (ULGUISpriteData* spriteItem : this->spriteDataArray)
	{
		if (IsValid(spriteItem))
		{
			spriteItem->atlasTexture = this->atlasTexture;
			spriteItem->spriteInfo.ScaleUV(0.5f);
		}
	}
	//tell UISprite to scale down uv
	for (auto itemSprite : this->renderSpriteArray)
	{
		if (IsValid(itemSprite.GetObject()))
		{
			itemSprite->ApplyAtlasTextureScaleUp();
		}
	}
	//callback function
	if (OnTextureSizeExpanded.IsBound())
	{
		OnTextureSizeExpanded.Broadcast(this->atlasTexture, newTextureSize);
	}

	return newTextureSize;
}
int32 FLGUIDynamicSpriteAtlasData::GetWillExpendTextureSize()const
{
	int32 oldTextureSize = this->atlasBinPack.GetBinWidth();
	return oldTextureSize + oldTextureSize;
}
void FLGUIDynamicSpriteAtlasData::CheckSprite(const FName& packingTag)
{
	for (int i = this->spriteDataArray.Num() - 1; i >= 0; i--)
	{
		auto itemSprite = this->spriteDataArray[i];
		if (IsValid(itemSprite))
		{
			if (IsValid(itemSprite->GetPackingAtlas()))
			{
				this->spriteDataArray.RemoveAt(i);
			}
			else
			{
				if (itemSprite->GetPackingTag() != packingTag)
				{
					this->spriteDataArray.RemoveAt(i);
				}
			}
		}
		else
		{
			this->spriteDataArray.RemoveAt(i);
		}
	}
	for (int i = this->renderSpriteArray.Num() - 1; i >= 0; i--)
	{
		auto itemSprite = this->renderSpriteArray[i];
		if (IsValid(itemSprite.GetObject()))
		{
			if (!IsValid(itemSprite->GetSprite()))
			{
				this->renderSpriteArray.RemoveAt(i);
			}
			else
			{
				if (auto spriteData = Cast<ULGUISpriteData>(itemSprite->GetSprite()))
				{
					if (spriteData->GetPackingTag() != packingTag)
					{
						this->renderSpriteArray.RemoveAt(i);
					}
				}
				else
				{
					this->renderSpriteArray.RemoveAt(i);
				}
			}
		}
		else
		{
			this->renderSpriteArray.RemoveAt(i);
		}
	}
}

ULGUIDynamicSpriteAtlasManager* ULGUIDynamicSpriteAtlasManager::Instance = nullptr;
bool ULGUIDynamicSpriteAtlasManager::InitCheck()
{
	if (Instance == nullptr)
	{
		Instance = NewObject<ULGUIDynamicSpriteAtlasManager>();
		Instance->AddToRoot();
	}
	return true;
}
void ULGUIDynamicSpriteAtlasManager::BeginDestroy()
{
	ResetAtlasMap();
#if WITH_EDITOR
	ULGUISpriteData::MarkAllSpritesNeedToReinitialize();
#endif
	Instance = nullptr;
	Super::BeginDestroy();
}

FLGUIDynamicSpriteAtlasData* ULGUIDynamicSpriteAtlasManager::FindOrAdd(const FName& packingTag)
{
	if (InitCheck())
	{
		if (!Instance->atlasMap.Contains(packingTag))
		{
			auto Result = &(Instance->atlasMap.Add(packingTag));
			if (Instance->OnAtlasMapChanged.IsBound())
			{
				Instance->OnAtlasMapChanged.Broadcast();
			}
			return Result;
		}
		return Instance->atlasMap.Find(packingTag);
	}
	return nullptr;
}
FLGUIDynamicSpriteAtlasData* ULGUIDynamicSpriteAtlasManager::Find(const FName& packingTag)
{
	if (Instance != nullptr)
	{
		return Instance->atlasMap.Find(packingTag);
	}
	return nullptr;
}
void ULGUIDynamicSpriteAtlasManager::ResetAtlasMap()
{
	if (Instance != nullptr)
	{
		for (auto& item : Instance->atlasMap)
		{
			if (IsValid(item.Value.atlasTexture))
			{
				item.Value.atlasTexture->RemoveFromRoot();
				item.Value.atlasTexture->ConditionalBeginDestroy();
			}
		}
		Instance->atlasMap.Empty();
		if (Instance->OnAtlasMapChanged.IsBound())
		{
			Instance->OnAtlasMapChanged.Broadcast();
		}
	}
}

void ULGUIDynamicSpriteAtlasManager::DisposeAtlasByPackingTag(FName inPackingTag)
{
	if (Instance != nullptr)
	{
		if (auto atlasData = Find(inPackingTag))
		{
			atlasData->atlasTexture->RemoveFromRoot();
			Instance->atlasMap.Remove(inPackingTag);
		}
	}
}
