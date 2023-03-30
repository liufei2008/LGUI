// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/LGUIStaticSpriteAtlasData.h"
#include "LGUI.h"
#include "Core/LGUISpriteData.h"
#include "Core/ActorComponent/UISpriteBase.h"
#include "TextureCompiler.h"
#include "Utils/LGUIUtils.h"

#define LOCTEXT_NAMESPACE "LGUIStaticSpriteAtlasData"

#if WITH_EDITOR
void ULGUIStaticSpriteAtlasData::PreEditChange(FProperty* PropertyAboutToChange)
{
	Super::PreEditChange(PropertyAboutToChange);
	auto PropertyName = PropertyAboutToChange->GetFName();
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ULGUIStaticSpriteAtlasData, spriteArray))
	{
		prevSpriteArray = spriteArray;
	}
}
void ULGUIStaticSpriteAtlasData::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.MemberProperty)
	{
		auto PropertyName = Property->GetFName();
		if (PropertyName == GET_MEMBER_NAME_CHECKED(ULGUIStaticSpriteAtlasData, maxAtlasTextureSize))
		{
			maxAtlasTextureSize = FMath::RoundUpToPowerOfTwo(maxAtlasTextureSize);
			maxAtlasTextureSize = FMath::Clamp(maxAtlasTextureSize, (uint32)256, (uint32)8192);
		}
		else if (PropertyName == GET_MEMBER_NAME_CHECKED(ULGUIStaticSpriteAtlasData, spriteArray))
		{
			//not allow empty
			prevSpriteArray.Remove(nullptr);
			spriteArray.Remove(nullptr);

			TArray<ULGUISpriteData*> AddedArray;
			TArray<ULGUISpriteData*> RemovedArray;
			for (auto Item : spriteArray)
			{
				if (!prevSpriteArray.Contains(Item))
				{
					AddedArray.Add(Item);
				}
			}
			for (auto Item : prevSpriteArray)
			{
				if (!spriteArray.Contains(Item))
				{
					RemovedArray.Add(Item);
				}
			}

			bool bIsYesToAll = false;
			bool bIsNoToAll = false;
			auto TransferSprite = [=](ULGUISpriteData* spriteData) {
				spriteData->Modify();
				spriteData->packingAtlas->RemoveSpriteData(spriteData);
				spriteData->packingAtlas = this;
				spriteData->isInitialized = false;
				spriteData->MarkPackageDirty();
			};
			auto KeepOldSprite = [=](ULGUISpriteData* spriteData) {
				spriteArray.Remove(spriteData);
			};
			for (auto Item : AddedArray)
			{
				if (Item->packingAtlas == nullptr)
				{
					Item->packingAtlas = this;
				}
				else
				{
					if (bIsYesToAll || bIsNoToAll)
					{
						if (bIsYesToAll)
						{
							TransferSprite(Item);
						}
						if (bIsNoToAll)
						{
							KeepOldSprite(Item);
						}
					}
					else
					{
						auto WarningMsg = FText::Format(LOCTEXT("TransferSpriteWarning", "Sprite: '{0}' was belongs to atlas: '{1}', do you want to transfer the sprite to this atlas?")
							, FText::FromString(Item->GetPathName()), FText::FromString(Item->packingAtlas->GetPathName()));
						auto Result = FMessageDialog::Open(EAppMsgType::YesNoYesAllNoAll, WarningMsg);
						switch (Result)
						{
						case EAppReturnType::No:
							KeepOldSprite(Item);
							break;
						case EAppReturnType::Yes:
							TransferSprite(Item);
							break;
						case EAppReturnType::YesAll:
							bIsYesToAll = true;
							TransferSprite(Item);
							break;
						case EAppReturnType::NoAll:
							bIsNoToAll = true;
							KeepOldSprite(Item);
							break;
						}
					}
				}
			}

			for (auto Item : RemovedArray)
			{
				Item->Modify();
				Item->packingAtlas = nullptr;
				Item->isInitialized = false;
				Item->MarkPackageDirty();
			}

			MarkNotInitialized();
			InitCheck();
			MarkPackageDirty();
		}
		bIsInitialized = false;
	}
}
void ULGUIStaticSpriteAtlasData::AddSpriteData(ULGUISpriteData* InSpriteData)
{
	spriteArray.AddUnique(InSpriteData);
	bIsInitialized = false;
}
void ULGUIStaticSpriteAtlasData::RemoveSpriteData(ULGUISpriteData* InSpriteData)
{
	spriteArray.Remove(InSpriteData);
	bIsInitialized = false;
}
void ULGUIStaticSpriteAtlasData::AddRenderSprite(UUISpriteBase* InSprite)
{
	renderSpriteArray.AddUnique(InSprite);
}
void ULGUIStaticSpriteAtlasData::RemoveRenderSprite(UUISpriteBase* InSprite)
{
	renderSpriteArray.Remove(InSprite);
}
void ULGUIStaticSpriteAtlasData::CheckSprite()
{
	for (int i = this->spriteArray.Num() - 1; i >= 0; i--)
	{
		auto itemSprite = this->spriteArray[i];
		if (IsValid(itemSprite))
		{
			if (itemSprite->GetPackingAtlas() != this)
			{
				this->spriteArray.RemoveAt(i);
			}
		}
		else
		{
			this->spriteArray.RemoveAt(i);
		}
	}
	for (int i = this->renderSpriteArray.Num() - 1; i >= 0; i--)
	{
		auto itemSprite = this->renderSpriteArray[i];
		if (itemSprite.IsValid())
		{
			if (!IsValid(itemSprite->GetSprite()))
			{
				this->renderSpriteArray.RemoveAt(i);
			}
			else
			{
				if (auto spriteData = Cast<ULGUISpriteData>(itemSprite->GetSprite()))
				{
					if (spriteData->GetPackingAtlas() != this)
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
bool ULGUIStaticSpriteAtlasData::PackAtlas()
{
	bIsInitialized = false;
	atlasTexture = nullptr;

	if (spriteArray.Num() <= 0)return false;
	for (int i = 0; i < spriteArray.Num(); i++)
	{
		ULGUISpriteData* spriteDataItem = spriteArray[i];
		if (!IsValid(spriteDataItem))
		{
			auto ErrMsg = FText::Format(LOCTEXT("SpriteDataError", "SpriteData is not valid in spriteArray at index {0}"), i);
			UE_LOG(LGUI, Error, TEXT("%s"), *ErrMsg.ToString());
			LGUIUtils::EditorNotification(ErrMsg);
			return false;
		}
		if (!IsValid(spriteDataItem->GetSpriteTexture()))
		{
			auto ErrMsg = FText::Format(LOCTEXT("SpriteDataTextureError", "SpriteData's texture is not valid of spriteData: '{0}'"), FText::FromString(spriteDataItem->GetPathName()));
			UE_LOG(LGUI, Error, TEXT("%s"), *ErrMsg.ToString());
			LGUIUtils::EditorNotification(ErrMsg);
			return false;
		}
	}

	//pack
	uint32 packSize = 16;//start from minimal size 16
	TArray<rbp::Rect> packResult;
	packResult.SetNumUninitialized(spriteArray.Num());
	while (!PackAtlasTest(packSize, packResult))
	{
		packSize *= 2;
	}
	if (packSize > maxAtlasTextureSize)
	{
		auto ErrMsg = FText::Format(LOCTEXT("AtlasSizeTooLargeError", "Atlas texture size coule be {0}, larger than {1}: {2}! Please remove some large size sprite, or split to multiple atlas.")
			, packSize
			, FText::FromName(GET_MEMBER_NAME_CHECKED(ULGUIStaticSpriteAtlasData, maxAtlasTextureSize))
			, maxAtlasTextureSize);
		UE_LOG(LGUI, Error, TEXT("%s"), *ErrMsg.ToString());
		LGUIUtils::EditorNotification(ErrMsg);
		return false;
	}

	//create texture
	auto PlatformData = new FTexturePlatformData();
	PlatformData->SizeX = packSize;
	PlatformData->SizeY = packSize;
	PlatformData->PixelFormat = PF_B8G8R8A8;

	int32 atlasSize = packSize;
	auto pixelBufferLength = atlasSize * atlasSize * GPixelFormats[PF_B8G8R8A8].BlockBytes;
	uint8* pixelData = new uint8[pixelBufferLength];
	FMemory::Memset(pixelData, 0, pixelBufferLength);//default is transparent black
	//copy pixels
	FColor* atlasColorBuffer = static_cast<FColor*>((void*)pixelData);
	float atlasTextureSizeInv = 1.0f / atlasSize;
	for (int spriteIndex = 0; spriteIndex < spriteArray.Num(); spriteIndex++)
	{
		auto spriteDataItem = spriteArray[spriteIndex];
		if (IsValid(spriteDataItem) && IsValid(spriteDataItem->GetSpriteTexture()))
		{
			auto spriteTexture = spriteDataItem->GetSpriteTexture();
			spriteTexture->CompressionSettings = TextureCompressionSettings::TC_EditorIcon;
			spriteTexture->SRGB = false;
			spriteTexture->UpdateResource();
#if WITH_EDITOR
			FTextureCompilingManager::Get().FinishCompilation({ spriteTexture });
#endif
			int32 spriteWidth = spriteTexture->GetSizeX();
			int32 spriteHeight = spriteTexture->GetSizeY();
			const FColor* spriteColorBuffer = reinterpret_cast<const FColor*>(spriteTexture->GetPlatformData()->Mips[0].BulkData.LockReadOnly());
			rbp::Rect rect = packResult[spriteIndex];

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
			if (spriteDataItem->GetUseEdgePixelPadding() && edgePixelPadding > 0)
			{
				//left
				destY = rect.y * atlasSize;
				for (int paddingIndex = 0; paddingIndex < edgePixelPadding; paddingIndex++)
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
				for (int paddingIndex = 0; paddingIndex < edgePixelPadding; paddingIndex++)
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
				for (int paddingIndex = 0; paddingIndex < edgePixelPadding; paddingIndex++)
				{
					int destX = destY + rect.x;
					int dstPixelIndex = destX - edgePixelPadding;
					for (int widthIndex = -edgePixelPadding; widthIndex < spriteWidth + edgePixelPadding; widthIndex++)
					{
						atlasColorBuffer[dstPixelIndex] = atlasColorBuffer[dstPixelIndex + atlasSize];
						dstPixelIndex += 1;
					}
					destY -= atlasSize;
				}
				//bottom, with corner
				destY = (rect.y + rect.height) * atlasSize;
				for (int paddingIndex = 0; paddingIndex < edgePixelPadding; paddingIndex++)
				{
					int destX = destY + rect.x;
					int dstPixelIndex = destX - edgePixelPadding;
					for (int widthIndex = -edgePixelPadding; widthIndex < spriteWidth + edgePixelPadding; widthIndex++)
					{
						atlasColorBuffer[dstPixelIndex] = atlasColorBuffer[dstPixelIndex - atlasSize];
						dstPixelIndex += 1;
					}
					destY += atlasSize;
				}
			}

			spriteDataItem->ApplySpriteInfoAfterStaticPack(rect, atlasTextureSizeInv);
			spriteTexture->GetPlatformData()->Mips[0].BulkData.Unlock();
		}
	}

	//store data
	textureMipData.SetNumUninitialized(pixelBufferLength);
	FMemory::Memcpy(textureMipData.GetData(), pixelData, pixelBufferLength);
	textureSize = packSize;

	//generate mipmaps
	{
		int mipsAdd = 0;
		//Declaring buffers here to reduce reallocs
		//We double buffer mips, using the prior buffer to build the next buffer
		TArray<FColor> mipRGBAs1;
		TArray<FColor> mipRGBAs2;

		//Access source data
		auto priorData = reinterpret_cast<const FColor*>(pixelData);
		int mipSize = atlasSize;

		while (true)
		{
			auto* mipRGBAs = mipsAdd & 1 ? &mipRGBAs1 : &mipRGBAs2;
			auto srcWidth = mipSize;
			mipSize = mipSize >> 1;
			if (mipSize == 0)
			{
				break;
			}

			mipRGBAs->Reset();
			mipRGBAs->AddUninitialized(mipSize* mipSize);

			//Average out the values
			auto* dataOut = mipRGBAs->GetData();
			for (int y = 0; y < mipSize; y++)
			{
				auto* srcData0 = priorData + (srcWidth * y * 2);
				auto* srcData1 = srcData0 + srcWidth;
				for (int x = 0; x < mipSize; x++)
				{
					auto srcColor1 = *srcData0++;
					auto srcColor2 = *srcData0++;
					auto srcColor3 = *srcData1++;
					auto srcColor4 = *srcData1++;
					int totalR = srcColor1.R;
					int totalG = srcColor1.G;
					int totalB = srcColor1.B;
					int totalA = srcColor1.A;

					totalR += srcColor2.R;
					totalG += srcColor2.G;
					totalB += srcColor2.B;
					totalA += srcColor2.A;

					totalR += srcColor3.R;
					totalG += srcColor3.G;
					totalB += srcColor3.B;
					totalA += srcColor3.A;

					totalR += srcColor4.R;
					totalG += srcColor4.G;
					totalB += srcColor4.B;
					totalA += srcColor4.A;

					totalR >>= 2;
					totalG >>= 2;
					totalB >>= 2;
					totalA >>= 2;

					*dataOut = FColor((uint8)totalR, (uint8)totalG, (uint8)totalB, (uint8)totalA);
					dataOut++;
				}
			}

			auto mipBufferLength = mipRGBAs->Num() * GPixelFormats[PF_B8G8R8A8].BlockBytes;
			priorData = mipRGBAs->GetData();
			mipsAdd++;

			//store mip data
			auto prevLength = textureMipData.Num();
			textureMipData.AddUninitialized(mipBufferLength);
			FMemory::Memcpy(textureMipData.GetData() + prevLength, mipRGBAs->GetData(), mipBufferLength);
		}
	}

	delete[] pixelData;

	return true;
}
bool ULGUIStaticSpriteAtlasData::PackAtlasTest(uint32 size, TArray<rbp::Rect>& result)
{
	rbp::MaxRectsBinPack atlasBinPack;
	atlasBinPack.Init(size, size, false);
	auto methold = rbp::MaxRectsBinPack::FreeRectChoiceHeuristic::RectBestAreaFit;
	for (int i = 0; i < spriteArray.Num(); i++)
	{
		auto spriteDataItem = spriteArray[i];
		auto calculatedEdgePixelPadding = spriteDataItem->GetUseEdgePixelPadding() ? edgePixelPadding : 0;
		auto spriteTexture = spriteDataItem->GetSpriteTexture();
		auto space = spaceBetweenSprites + calculatedEdgePixelPadding + calculatedEdgePixelPadding;
		//add space
#if WITH_EDITOR
		FTextureCompilingManager::Get().FinishCompilation({ spriteTexture });
#endif
		int insertRectWidth = spriteTexture->GetSizeX() + space;
		int insertRectHeight = spriteTexture->GetSizeY() + space;
		auto rect = atlasBinPack.Insert(insertRectWidth, insertRectHeight, methold);
		if (rect.width <= 0)//cannot fit, should expend size
		{
			return false;
		}
		//remove space
		rect.x += calculatedEdgePixelPadding;
		rect.y += calculatedEdgePixelPadding;
		rect.width -= space;
		rect.height -= space;

		result[i] = rect;
	}
	return true;
}
void ULGUIStaticSpriteAtlasData::BeginCacheForCookedPlatformData(const ITargetPlatform* TargetPlatform)
{
	
}
void ULGUIStaticSpriteAtlasData::WillNeverCacheCookedPlatformDataAgain()
{
	
}
void ULGUIStaticSpriteAtlasData::ClearCachedCookedPlatformData(const ITargetPlatform* TargetPlatform)
{
	
}
void ULGUIStaticSpriteAtlasData::MarkNotInitialized()
{
	bIsInitialized = false;
}
#endif

bool ULGUIStaticSpriteAtlasData::InitCheck()
{
	if (!bIsInitialized)
	{
#if WITH_EDITOR
		if (!PackAtlas())
		{
			return false;
		}
#endif

		//create texture
		auto texture = NewObject<UTexture2D>(
			this, 
			FName(*FString::Printf(TEXT("LGUIStaticSpriteAtlasData_Texture_%d"), LGUIUtils::LGUITextureNameSuffix++))
			);
		auto PlatformData = new FTexturePlatformData();
		PlatformData->SizeX = textureSize;
		PlatformData->SizeY = textureSize;
		PlatformData->PixelFormat = PF_B8G8R8A8;
		texture->SetPlatformData(PlatformData);

		//mipmaps
		{
			uint32 textureDataOffset = 0;
			int mipSize = textureSize;
			while (true)
			{
				// Allocate next mipmap.
				FTexture2DMipMap* mip = new FTexture2DMipMap;
				texture->GetPlatformData()->Mips.Add(mip);
				mip->SizeX = mipSize;
				mip->SizeY = mipSize;
				mip->BulkData.Lock(LOCK_READ_WRITE);
				auto pixelBufferLength = mipSize * mipSize * GPixelFormats[PF_B8G8R8A8].BlockBytes;
				void* mipData = mip->BulkData.Realloc(pixelBufferLength);
				FMemory::Memcpy(mipData, textureMipData.GetData() + textureDataOffset, pixelBufferLength);
				mip->BulkData.Unlock();

				mipSize = mipSize >> 1;
				if (mipSize == 0)
				{
					break;
				}
				textureDataOffset += pixelBufferLength;
			}
		}

		texture->CompressionSettings = TextureCompressionSettings::TC_EditorIcon;
		texture->LODGroup = TextureGroup::TEXTUREGROUP_UI;
		texture->SRGB = atlasTextureUseSRGB;
		texture->Filter = atlasTextureFilter;
		texture->UpdateResource();

		this->atlasTexture = texture;

		bIsInitialized = true;
	}
	return bIsInitialized;
}
UTexture2D* ULGUIStaticSpriteAtlasData::GetAtlasTexture()
{
	InitCheck();
	return atlasTexture;
}

#undef LOCTEXT_NAMESPACE
