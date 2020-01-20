// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Core/LGUIAtlasData.h"
#include "LGUI.h"
#include "Core/LGUISettings.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "Core/ActorComponent/UISpriteBase.h"
#include "Core/LGUISpriteData.h"
#include "Utils/LGUIUtils.h"


void FLGUIAtlasData::EnsureAtlasTexture(const FName& packingTag)
{
#if WITH_EDITOR
	int32 defaultAtlasTextureSize = ULGUISettings::GetAtlasTextureInitialSize(packingTag);
#else
	static int32 defaultAtlasTextureSize = ULGUISettings::GetAtlasTextureInitialSize(packingTag);
#endif
	if (!IsValid(atlasTexture))
	{
		atlasBinPack.Init(defaultAtlasTextureSize, defaultAtlasTextureSize, 0, 0);
		CreateAtlasTexture(packingTag, 0, defaultAtlasTextureSize);
	}
}
void FLGUIAtlasData::CreateAtlasTexture(const FName& packingTag, int oldTextureSize, int newTextureSize)
{
#if WITH_EDITOR
	bool atlasSRGB = ULGUISettings::GetAtlasTextureSRGB(packingTag);
	auto filter = ULGUISettings::GetAtlasTextureFilter(packingTag);
#else
	static bool atlasSRGB = ULGUISettings::GetAtlasTextureSRGB(packingTag);
	static auto filter = ULGUISettings::GetAtlasTextureFilter(packingTag);
#endif
	auto texture = LGUIUtils::CreateTransientBlackTransparentTexture(newTextureSize);

	texture->CompressionSettings = TextureCompressionSettings::TC_EditorIcon;
	texture->LODGroup = TextureGroup::TEXTUREGROUP_UI;
	texture->SRGB = atlasSRGB;
	texture->Filter = filter;
	texture->UpdateResource();
	texture->AddToRoot();
	auto oldTexture = this->atlasTexture;
	this->atlasTexture = texture;

	//copy old texture to new one
	if (IsValid(oldTexture) && oldTextureSize > 0)
	{
		auto newTexture = texture;
		if (oldTexture->Resource != nullptr && newTexture->Resource != nullptr)
		{
			ENQUEUE_UNIQUE_RENDER_COMMAND_THREEPARAMETER(
				FLGUISpriteCopyAtlasTexture,
				UTexture2D*, oldTexture, oldTexture,
				UTexture2D*, newTexture, newTexture,
				int32, oldTextureSize, oldTextureSize,
				{
					FBox2D regionBox(FVector2D(0, 0), FVector2D(oldTextureSize, oldTextureSize));
					RHICmdList.CopySubTextureRegion(
						((FTexture2DResource*)oldTexture->Resource)->GetTexture2DRHI(),
						((FTexture2DResource*)newTexture->Resource)->GetTexture2DRHI(),
						regionBox,
						regionBox
					);
					oldTexture->RemoveFromRoot();//ready for gc
				}
			);
		}
	}
}
int32 FLGUIAtlasData::ExpendTextureSize(const FName& packingTag)
{
	int32 oldTextureSize = this->atlasBinPack.GetBinWidth();
	int32 newTextureSize = oldTextureSize * 2;

	this->atlasBinPack.ExpendSize(newTextureSize, newTextureSize);
	//create new texture
	this->CreateAtlasTexture(packingTag, oldTextureSize, newTextureSize);
	//scale down sprite uv
	for (ULGUISpriteData* spriteItem : this->spriteDataArray)
	{
		spriteItem->atlasTexture = this->atlasTexture;
		spriteItem->spriteInfo.ScaleUV(0.5f);
	}
	//tell UISprite to scale down uv
	for (auto itemSprite : this->renderSpriteArray)
	{
		if (itemSprite.IsValid())
		{
			itemSprite->ApplyAtlasTextureScaleUp();
		}
	}
	//callback function
	if (expandTextureSizeCallback.IsBound())
	{
		expandTextureSizeCallback.Broadcast(this->atlasTexture, newTextureSize);
	}

	return newTextureSize;
}


ULGUIAtlasManager* ULGUIAtlasManager::Instance = nullptr;
bool ULGUIAtlasManager::InitCheck()
{
	if (Instance == nullptr)
	{
		Instance = NewObject<ULGUIAtlasManager>();
		Instance->AddToRoot();
	}
	return true;
}
void ULGUIAtlasManager::BeginDestroy()
{
	ResetAtlasMap();
#if WITH_EDITOR
	ULGUISpriteData::MarkAllSpritesNeedToReinitialize();
#endif
	Instance = nullptr;
	Super::BeginDestroy();
}

const TMap<FName, FLGUIAtlasData>& ULGUIAtlasManager::GetAtlasMap()
{
	return atlasMap;
}
FLGUIAtlasData* ULGUIAtlasManager::FindOrAdd(const FName& packingTag)
{
	if (InitCheck())
	{
		return &(Instance->atlasMap.FindOrAdd(packingTag));
	}
	return nullptr;
}
FLGUIAtlasData* ULGUIAtlasManager::Find(const FName& packingTag)
{
	if (Instance != nullptr)
	{
		return Instance->atlasMap.Find(packingTag);
	}
	return nullptr;
}
void ULGUIAtlasManager::ResetAtlasMap()
{
	if (Instance != nullptr)
	{
		Instance->atlasMap.Reset();
		for (auto item : Instance->atlasMap)
		{
			item.Value.atlasTexture->RemoveFromRoot();
			item.Value.atlasTexture->ConditionalBeginDestroy();
		}
	}
}
