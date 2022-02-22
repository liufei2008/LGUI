// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Core/LGUIAtlasData.h"
#include "LGUI.h"
#include "Core/LGUISettings.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "Core/ActorComponent/UISpriteBase.h"
#include "Core/LGUISpriteData.h"
#include "Utils/LGUIUtils.h"
#include "Rendering/Texture2DResource.h"


void FLGUIAtlasData::EnsureAtlasTexture(const FName& packingTag)
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
void FLGUIAtlasData::CreateAtlasTexture(const FName& packingTag, int oldTextureSize, int newTextureSize)
{
#if WITH_EDITOR
	bool atlasSRGB = ULGUISettings::GetAtlasTextureSRGB(packingTag);
	auto filter = ULGUISettings::GetAtlasTextureFilter(packingTag);
#else
	static bool atlasSRGB = ULGUISettings::GetAtlasTextureSRGB(packingTag);
	static auto filter = ULGUISettings::GetAtlasTextureFilter(packingTag);
#endif
	auto texture = LGUIUtils::CreateTexture(newTextureSize);

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
		if (oldTexture->Resource != nullptr && newTexture->Resource != nullptr)
		{
			ENQUEUE_RENDER_COMMAND(FLGUISpriteCopyAtlasTexture)(
				[oldTexture, newTexture, oldTextureSize](FRHICommandListImmediate& RHICmdList)
			{
				FRHICopyTextureInfo CopyInfo;
				CopyInfo.SourcePosition = FIntVector(0, 0, 0);
				CopyInfo.Size = FIntVector(oldTextureSize, oldTextureSize, 0);
				CopyInfo.DestPosition = FIntVector(0, 0, 0);
				RHICmdList.CopyTexture(
					((FTexture2DResource*)oldTexture->Resource)->GetTexture2DRHI(),
					((FTexture2DResource*)newTexture->Resource)->GetTexture2DRHI(),
					CopyInfo
				);
				oldTexture->RemoveFromRoot();//ready for gc
			});
		}
	}
}
int32 FLGUIAtlasData::ExpendTextureSize(const FName& packingTag)
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
		if (itemSprite.IsValid())
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
int32 FLGUIAtlasData::GetWillExpendTextureSize()const
{
	int32 oldTextureSize = this->atlasBinPack.GetBinWidth();
	return oldTextureSize + oldTextureSize;
}
bool FLGUIAtlasData::StaticPacking(const FName& packingTag)
{
	bool canPack = true;
	for (int i = 0; i < spriteDataArray.Num(); i++)
	{
		ULGUISpriteData* spriteDataItem = spriteDataArray[i];
		if (!IsValid(spriteDataItem))
		{
			UE_LOG(LGUI, Error, TEXT("[LGUIAtlas::PackIt]sprite item not valid at index:%d"), i);
			canPack = false;
		}
		if (canPack)
		{
			if (!IsValid(spriteDataItem->GetSpriteTexture()))
			{
				UE_LOG(LGUI, Error, TEXT("[LGUIAtlas::PackIt]sprite item:%s texture not valid"), *(spriteDataItem->GetPathName()));
				canPack = false;
			}
		}
	}
	if (!canPack)return false;
	//find largest sprite size, convert to power of 2
	uint32 packSize = ULGUISettings::GetAtlasTextureInitialSize(packingTag);
	int32 spaceBetweenSprites = ULGUISettings::GetAtlasTexturePadding(packingTag);

	bool atlasSRGB = ULGUISettings::GetAtlasTextureSRGB(packingTag);
	auto filter = ULGUISettings::GetAtlasTextureFilter(packingTag);

	//pack
	bool packSuccess = false;
	TArray<rbp::Rect> packResult;
	do
	{
		packSuccess = PackAtlasTest(packSize, packResult, spaceBetweenSprites);
		if (!packSuccess)
		{
			packSize *= 2;
		}
	} while (!packSuccess);

	//create texture
	auto texture = NewObject<UTexture2D>();
	texture->PlatformData = new FTexturePlatformData();
	texture->PlatformData->SizeX = packSize;
	texture->PlatformData->SizeY = packSize;
	texture->PlatformData->PixelFormat = PF_B8G8R8A8;
	texture->AddToRoot();

	int32 atlasSize = packSize;
	auto pixelBufferLength = atlasSize * atlasSize * GPixelFormats[PF_B8G8R8A8].BlockBytes;
	uint8* pixelData = new uint8[pixelBufferLength];
	FMemory::Memset(pixelData, 0, pixelBufferLength);//default is transparent black
	//copy pixels
	FColor* atlasColorBuffer = static_cast<FColor*>((void*)pixelData);
	float atlasTextureSizeInv = 1.0f / atlasSize;
	for (int spriteIndex = 0; spriteIndex < spriteDataArray.Num(); spriteIndex++)
	{
		ULGUISpriteData* spriteDataItem = spriteDataArray[spriteIndex];
		if (IsValid(spriteDataItem) && IsValid(spriteDataItem->GetSpriteTexture()))
		{
			auto spriteTexture = spriteDataItem->GetSpriteTexture();
			spriteTexture->CompressionSettings = TextureCompressionSettings::TC_EditorIcon;
			spriteTexture->SRGB = false;
			spriteTexture->UpdateResource();
			const FColor* spriteColorBuffer = reinterpret_cast<const FColor*>(spriteTexture->PlatformData->Mips[0].BulkData.LockReadOnly());
			rbp::Rect rect = packResult[spriteIndex];
			int32 spriteWidth = spriteTexture->GetSizeX();
			int32 spriteHeight = spriteTexture->GetSizeY();
			if (spriteTexture->GetSizeX() == rect.width)
			{
				int destY = rect.y * atlasSize;
				int spritePixelIndex = 0;
				for (int32 texY = 0; texY < spriteHeight; texY++)
				{
					int destX = rect.x + destY;
					for (int32 texX = 0; texX < spriteWidth; texX++)
					{
						int dstPixelIndex = destX + texX;
						atlasColorBuffer[dstPixelIndex] = spriteColorBuffer[spritePixelIndex];
						spritePixelIndex++;
					}
					destY += atlasSize;
				}
				//pixel padding
				{
					//left
					destY = rect.y * atlasSize;
					for (int paddingIndex = 0; paddingIndex < spaceBetweenSprites; paddingIndex++)
					{
						int destX = destY + rect.x - paddingIndex - 1;
						int dstPixelIndex = destX;
						for (int heightIndex = 0; heightIndex < spriteHeight; heightIndex++)
						{
							atlasColorBuffer[dstPixelIndex] = atlasColorBuffer[dstPixelIndex + 1];
							dstPixelIndex += atlasSize;
						}
					}
					//right
					destY = rect.y * atlasSize;
					for (int paddingIndex = 0; paddingIndex < spaceBetweenSprites; paddingIndex++)
					{
						int destX = destY + rect.x + rect.width + paddingIndex;
						int dstPixelIndex = destX;
						for (int heightIndex = 0; heightIndex < spriteHeight; heightIndex++)
						{
							atlasColorBuffer[dstPixelIndex] = atlasColorBuffer[dstPixelIndex - 1];
							dstPixelIndex += atlasSize;
						}
					}
					//top, with corner
					destY = (rect.y - 1) * atlasSize;
					for (int paddingIndex = 0; paddingIndex < spaceBetweenSprites; paddingIndex++)
					{
						int destX = destY + rect.x;
						int dstPixelIndex = destX - spaceBetweenSprites;
						for (int widthIndex = -spaceBetweenSprites; widthIndex < spriteWidth + spaceBetweenSprites; widthIndex++)
						{
							atlasColorBuffer[dstPixelIndex] = atlasColorBuffer[dstPixelIndex + atlasSize];
							dstPixelIndex += 1;
						}
						destY -= atlasSize;
					}
					//bottom, with corner
					destY = (rect.y + rect.height) * atlasSize;
					for (int paddingIndex = 0; paddingIndex < spaceBetweenSprites; paddingIndex++)
					{
						int destX = destY + rect.x;
						int dstPixelIndex = destX - spaceBetweenSprites;
						for (int widthIndex = -spaceBetweenSprites; widthIndex < spriteWidth + spaceBetweenSprites; widthIndex++)
						{
							atlasColorBuffer[dstPixelIndex] = atlasColorBuffer[dstPixelIndex - atlasSize];
							dstPixelIndex += 1;
						}
						destY += atlasSize;
					}
				}

				spriteDataItem->ApplySpriteInfoAfterStaticPack(rect, atlasTextureSizeInv, texture);
			}
			else//flipped
			{
				/*UE_LOG(LGUI, Error, TEXT("flipped:%s"), *(spriteTexture->GetPathName()));

				int destY = rect.y * atlasSize;
				int spritePixelIndex = 0;
				for (int32 texY = 0; texY < rect.height; texY++)
				{
					int destX = rect.x + destY;
					for (int32 texX = 0; texX < rect.width; texX++)
					{
						int dstPixelIndex = destX + texX;
						atlasColorBuffer[dstPixelIndex] = spriteColorBuffer[spritePixelIndex];
						spritePixelIndex++;
					}
					destY += atlasSize;
				}*/
			}
			spriteTexture->PlatformData->Mips[0].BulkData.Unlock();
		}
	}

	{
		FTexture2DMipMap* textureMip = new FTexture2DMipMap();
		texture->PlatformData->Mips.Add(textureMip);
		textureMip->SizeX = packSize;
		textureMip->SizeY = packSize;
		textureMip->BulkData.Lock(LOCK_READ_WRITE);
		void* textureData = textureMip->BulkData.Realloc(packSize * packSize * GPixelFormats[PF_B8G8R8A8].BlockBytes);
		FMemory::Memcpy(textureData, pixelData, pixelBufferLength);
		texture->PlatformData->Mips[0].BulkData.Unlock();
		//texture->Source.Init(atlasSize, atlasSize, 1, 1, ETextureSourceFormat::TSF_BGRA8, pixelData);
		delete[] pixelData;
	}

	//generate mipmaps
	{
		int mipsAdd = 0;// pNewTexture->RequestedMips - 1;

		//Declaring buffers here to reduce reallocs
		//We double buffer mips, using the prior buffer to build the next buffer
		TArray<uint8> _mipRGBAs;
		TArray<uint8> _mipRGBBs;

		//Access source data
		auto* priorData = (const uint8*)texture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
		int priorwidth = texture->PlatformData->Mips[0].SizeX;
		int priorheight = texture->PlatformData->Mips[0].SizeY;

		while (true)
		{
			auto* mipRGBAs = mipsAdd & 1 ? &_mipRGBAs : &_mipRGBBs;

			int mipwidth = priorwidth >> 1;
			int mipheight = priorheight >> 1;
			if ((mipwidth == 0) || (mipheight == 0))
			{
				break;
			}

			mipRGBAs->Reset();
			mipRGBAs->AddUninitialized(mipwidth * mipheight * GPixelFormats[PF_B8G8R8A8].BlockBytes);

			int dataPerRow = priorwidth * GPixelFormats[PF_B8G8R8A8].BlockBytes;

			//Average out the values
			auto* dataOut = mipRGBAs->GetData();
			for (int y = 0; y < mipheight; y++)
			{
				auto* dataInRow0 = priorData + (dataPerRow * y * 2);
				auto* dataInRow1 = dataInRow0 + dataPerRow;
				for (int x = 0; x < mipwidth; x++)
				{
					int totalB = *dataInRow0++;
					int totalG = *dataInRow0++;
					int totalR = *dataInRow0++;
					int totalA = *dataInRow0++;
					totalB += *dataInRow0++;
					totalG += *dataInRow0++;
					totalR += *dataInRow0++;
					totalA += *dataInRow0++;

					totalB += *dataInRow1++;
					totalG += *dataInRow1++;
					totalR += *dataInRow1++;
					totalA += *dataInRow1++;
					totalB += *dataInRow1++;
					totalG += *dataInRow1++;
					totalR += *dataInRow1++;
					totalA += *dataInRow1++;

					totalB >>= 2;
					totalG >>= 2;
					totalR >>= 2;
					totalA >>= 2;

					*dataOut++ = (uint8)totalB;
					*dataOut++ = (uint8)totalG;
					*dataOut++ = (uint8)totalR;
					*dataOut++ = (uint8)totalA;
				}
				dataInRow0 += priorwidth * 2;
				dataInRow1 += priorwidth * 2;
			}

			// Allocate next mipmap.
			FTexture2DMipMap* mip = new FTexture2DMipMap;
			texture->PlatformData->Mips.Add(mip);
			mip->SizeX = mipwidth;
			mip->SizeY = mipheight;
			mip->BulkData.Lock(LOCK_READ_WRITE);
			void* mipData = mip->BulkData.Realloc(mipRGBAs->Num());
			FMemory::Memcpy(mipData, mipRGBAs->GetData(), mipRGBAs->Num());
			mip->BulkData.Unlock();

			priorData = mipRGBAs->GetData();
			priorwidth = mipwidth;
			priorheight = mipheight;
			mipsAdd++;
		}

		texture->PlatformData->Mips[0].BulkData.Unlock();
		texture->UpdateResource();
	}


	texture->CompressionSettings = TextureCompressionSettings::TC_EditorIcon;
	//texture->MipGenSettings = TextureMipGenSettings::TMGS_FromTextureGroup;
	texture->LODGroup = TextureGroup::TEXTUREGROUP_UI;
	texture->SRGB = atlasSRGB;
	texture->Filter = filter;
	texture->UpdateResource();

	if (IsValid(this->atlasTexture))
	{
		this->atlasTexture->RemoveFromRoot();
	}
	this->atlasTexture = texture;
	return true;
}
bool FLGUIAtlasData::PackAtlasTest(uint32 size, TArray<rbp::Rect>& result, int32 spaceBetweenSprites)
{
	result.Reset();
	result.AddDefaulted(spriteDataArray.Num());

	rbp::MaxRectsBinPack testAtlasBinPack;
	testAtlasBinPack.Init(size, size, false);
	auto methold = rbp::MaxRectsBinPack::FreeRectChoiceHeuristic::RectBestAreaFit;
	for (int i = 0; i < spriteDataArray.Num(); i++)
	{
		ULGUISpriteData* spriteDataItem = spriteDataArray[i];
		if (IsValid(spriteDataItem) && IsValid(spriteDataItem->GetSpriteTexture()))
		{
			//add space
			int insertRectWidth = spriteDataItem->GetSpriteTexture()->GetSizeX() + spaceBetweenSprites + spaceBetweenSprites;
			int insertRectHeight = spriteDataItem->GetSpriteTexture()->GetSizeY() + spaceBetweenSprites + spaceBetweenSprites;
			auto rect = testAtlasBinPack.Insert(insertRectWidth, insertRectHeight, methold);
			if (rect.width <= 0)//cannot fit, should expend size
			{
				return false;
			}
			//remote space
			rect.x += spaceBetweenSprites;
			rect.y += spaceBetweenSprites;
			rect.width -= spaceBetweenSprites + spaceBetweenSprites;
			rect.height -= spaceBetweenSprites + spaceBetweenSprites;

			result[i] = rect;
		}
	}
	return true;
}

ULGUIAtlasManager* ULGUIAtlasManager::Instance = nullptr;
bool ULGUIAtlasManager::InitCheck()
{
	if (Instance == nullptr)
	{
		Instance = NewObject<ULGUIAtlasManager>();
		Instance->AddToRoot();
	}
	if (!Instance->isStaticAtlasPacked)
	{
		Instance->isStaticAtlasPacked = true;
		PackStaticAtlas();
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

FLGUIAtlasData* ULGUIAtlasManager::FindOrAdd(const FName& packingTag)
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
		for (auto& item : Instance->atlasMap)
		{
			if (IsValid(item.Value.atlasTexture))
			{
				item.Value.atlasTexture->RemoveFromRoot();
				item.Value.atlasTexture->ConditionalBeginDestroy();
			}
		}
		Instance->atlasMap.Empty();
		Instance->isStaticAtlasPacked = false;
		if (Instance->OnAtlasMapChanged.IsBound())
		{
			Instance->OnAtlasMapChanged.Broadcast();
		}
	}
}
void ULGUIAtlasManager::PackStaticAtlas()
{
#if 0
	auto& allAtlasSettings = ULGUISettings::GetAllAtlasSettings();
	if (allAtlasSettings.Num() > 0)
	{
		TArray<FName> staticPackingTagArray;
		for (auto keyValue : allAtlasSettings)
		{
			if (keyValue.Value.packingType == ELGUIAtlasPackingType::Static)
			{
				staticPackingTagArray.Add(keyValue.Key);
			}
		}
		if (staticPackingTagArray.Num() > 0)
		{
			//collect all sprite data for packing tag
			for (TObjectIterator<ULGUISpriteData> Itr; Itr; ++Itr)
			{
				auto spriteDataItem = *Itr;
				auto packingTag = spriteDataItem->GetPackingTag();
				if (!packingTag.IsNone())
				{
					if (staticPackingTagArray.Contains(packingTag))
					{
						FLGUIAtlasData* atlasData = FindOrAdd(packingTag);
						atlasData->spriteDataArray.Add(spriteDataItem);
					}
				}
			}
			//static packing
			for (auto& item : Instance->atlasMap)
			{
				item.Value.StaticPacking(item.Key);
			}
		}
	}
#endif
}

void ULGUIAtlasManager::DisposeAtlasByPackingTag(FName inPackingTag)
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
