// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/LGUIStaticSpriteAtlasData.h"
#include "LGUI.h"
#include "Core/LGUISpriteData.h"
#include "Core/ActorComponent/UISpriteBase.h"
#include "Utils/LGUIUtils.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "Core/IUISpriteRenderableInterface.h"

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
		MarkNotInitialized();
		auto PropertyName = Property->GetFName();
		if (PropertyName == GET_MEMBER_NAME_CHECKED(ULGUIStaticSpriteAtlasData, spriteArray))
		{
			//not allow empty
			prevSpriteArray.Remove(nullptr);
			spriteArray.Remove(nullptr);
			//not allow repeated
			TSet<ULGUISpriteData*> tempSet;
			for (int i = 0; i < prevSpriteArray.Num(); i++)
			{
				auto spriteItem = prevSpriteArray[i];
				if (tempSet.Contains(spriteItem))
				{
					prevSpriteArray.RemoveAt(i);
					i--;
				}
				else
				{
					tempSet.Add(spriteItem);
				}
			}
			tempSet.Empty();
			for (int i = 0; i < spriteArray.Num(); i++)
			{
				auto spriteItem = spriteArray[i];
				if (tempSet.Contains(spriteItem))
				{
					spriteArray.RemoveAt(i);
					i--;
				}
				else
				{
					tempSet.Add(spriteItem);
				}
			}

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

			auto TransferSprite = [=](ULGUISpriteData* spriteData) {
				spriteData->Modify();
				if (IsValid(spriteData->packingAtlas))
				{
					spriteData->packingAtlas->RemoveSpriteData(spriteData);
				}
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
					TransferSprite(Item);
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
						auto WeakThis = TWeakObjectPtr<ULGUIStaticSpriteAtlasData>(this);
						ULGUIEditorManagerObject::AddOneShotTickFunction([=] {
							if (WeakThis.IsValid())
							{
								bIsYesToAll = false;
								bIsNoToAll = false;
							}
							}, 0);
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

			//If we drag sprites to the spriteArray, the PostEditChangeProperty will be called foreach of the dragged sprites which is a long time wait, so we do the pack after the iteration.
			if (!bIsAddedToDelayedCall)
			{
				bIsAddedToDelayedCall = true;
				auto WeakThis = TWeakObjectPtr<ULGUIStaticSpriteAtlasData>(this);
				ULGUIEditorManagerObject::AddOneShotTickFunction([=] {
					if (WeakThis.IsValid())
					{
						MarkNotInitialized();
						InitCheck();
						MarkPackageDirty();
						bIsAddedToDelayedCall = false;
					}
					}, 0);
			}
		}
		else if (PropertyName == GET_MEMBER_NAME_CHECKED(ULGUIStaticSpriteAtlasData, maxAtlasTextureSize))
		{
			maxAtlasTextureSize = FMath::RoundUpToPowerOfTwo(maxAtlasTextureSize);
			maxAtlasTextureSize = FMath::Clamp(maxAtlasTextureSize, (uint32)256, (uint32)8192);
		}
	}
}
void ULGUIStaticSpriteAtlasData::AddSpriteData(ULGUISpriteData* InSpriteData)
{
	if (!spriteArray.Contains(InSpriteData))
	{
		spriteArray.Add(InSpriteData);
		MarkPackageDirty();
		MarkNotInitialized();
	}
}
void ULGUIStaticSpriteAtlasData::RemoveSpriteData(ULGUISpriteData* InSpriteData)
{
	if (spriteArray.Contains(InSpriteData))
	{
		spriteArray.Remove(InSpriteData);
		MarkPackageDirty();
		MarkNotInitialized();
	}
}
void ULGUIStaticSpriteAtlasData::AddRenderSprite(TScriptInterface<IUISpriteRenderableInterface> InSprite)
{
	renderSpriteArray.AddUnique(InSprite);
}
void ULGUIStaticSpriteAtlasData::RemoveRenderSprite(TScriptInterface<IUISpriteRenderableInterface> InSprite)
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
		if (IsValid(itemSprite.GetObject()))
		{
			if (!IsValid(IUISpriteRenderableInterface::Execute_GetSprite(itemSprite.GetObject())))
			{
				this->renderSpriteArray.RemoveAt(i);
			}
			else
			{
				if (auto spriteData = Cast<ULGUISpriteData>(IUISpriteRenderableInterface::Execute_GetSprite(itemSprite.GetObject())))
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
	atlasTexture = nullptr;

	if (spriteArray.Num() <= 0)return false;
	for (int i = 0; i < spriteArray.Num(); i++)
	{
		ULGUISpriteData* spriteDataItem = spriteArray[i];
		if (!IsValid(spriteDataItem))
		{
			if (!bWarningIsAlreadyAppearedAtCurrentPackingSession)
			{
				bWarningIsAlreadyAppearedAtCurrentPackingSession = true;
				auto ErrMsg = FText::Format(LOCTEXT("SpriteDataError", "{0} Packing atlas for LGUIStaticSpriteAtlasData: '{1}', but SpriteData is not valid in spriteArray at index {2}")
					, FText::FromString(FString::Printf(TEXT("[%s].%d"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__))
					, FText::FromString(this->GetPathName()), i);
				UE_LOG(LGUI, Error, TEXT("%s"), *ErrMsg.ToString());
				LGUIUtils::EditorNotification(ErrMsg, 10.0f);
			}
			return false;
		}
		if (!IsValid(spriteDataItem->GetSpriteTexture()))
		{
			if (!bWarningIsAlreadyAppearedAtCurrentPackingSession)
			{
				bWarningIsAlreadyAppearedAtCurrentPackingSession = true;
				auto ErrMsg = FText::Format(LOCTEXT("SpriteDataTextureError", "{0} Packing atlas for LGUIStaticSpriteAtlasData: '{1}', but SpriteData's texture is not valid of spriteData: '{2}'")
					, FText::FromString(FString::Printf(TEXT("[%s].%d"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__))
					, FText::FromString(this->GetPathName()), FText::FromString(spriteDataItem->GetPathName()));
				UE_LOG(LGUI, Error, TEXT("%s"), *ErrMsg.ToString());
				LGUIUtils::EditorNotification(ErrMsg, 10.0f);
			}
			return false;
		}
		if (spriteDataItem->packingAtlas != this)
		{
			if (!bWarningIsAlreadyAppearedAtCurrentPackingSession)
			{
				bWarningIsAlreadyAppearedAtCurrentPackingSession = true;
				auto ErrMsg = FText::Format(LOCTEXT("SpritePackingAtlasError", "{0} Packing atlas for LGUIStaticSpriteAtlasData: '{1}', but SpriteData's packingAtlas is not this one, spriteData '{2}', at index: {3}")
					, FText::FromString(FString::Printf(TEXT("[%s].%d"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__))
					, FText::FromString(this->GetPathName()), FText::FromString(spriteDataItem->GetPathName()), i);
				UE_LOG(LGUI, Error, TEXT("%s"), *ErrMsg.ToString());
				LGUIUtils::EditorNotification(ErrMsg, 10.0f);
			}
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
		if (!bWarningIsAlreadyAppearedAtCurrentPackingSession)
		{
			bWarningIsAlreadyAppearedAtCurrentPackingSession = true;
			auto ErrMsg = FText::Format(LOCTEXT("AtlasSizeTooLargeError", "{0} Package sprite atlas fail! Atlas texture size {1} larger than {2}: {3}! Please remove some large size sprite, or split to multiple atlas.")
				, FText::FromString(FString::Printf(TEXT("[%s].%d"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__))
				, packSize
				, FText::FromName(GET_MEMBER_NAME_CHECKED(ULGUIStaticSpriteAtlasData, maxAtlasTextureSize))
				, maxAtlasTextureSize);
			UE_LOG(LGUI, Error, TEXT("%s"), *ErrMsg.ToString());
			LGUIUtils::EditorNotification(ErrMsg, 10.0f);
		}
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
			ULGUISpriteData::CheckAndApplySpriteTextureSetting(spriteTexture);
			int32 spriteWidth = spriteTexture->GetSizeX();
			int32 spriteHeight = spriteTexture->GetSizeY();
			const FColor* spriteColorBuffer = reinterpret_cast<const FColor*>(spriteTexture->PlatformData->Mips[0].BulkData.LockReadOnly());
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
			spriteTexture->PlatformData->Mips[0].BulkData.Unlock();
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
	bWarningIsAlreadyAppearedAtCurrentPackingSession = false;
}
bool ULGUIStaticSpriteAtlasData::CheckInvalidSpriteData()const
{
	for (int i = 0; i < spriteArray.Num(); i++)
	{
		ULGUISpriteData* spriteDataItem = spriteArray[i];
		if (!IsValid(spriteDataItem))
		{
			return true;
		}
		else if (!IsValid(spriteDataItem->GetSpriteTexture()))
		{
			return true;
		}
		else if (spriteDataItem->packingAtlas != this)
		{
			return true;
		}
	}
	return false;
}
void ULGUIStaticSpriteAtlasData::CleanupInvalidSpriteData()
{
	auto PrevCount = spriteArray.Num();
	for (int i = 0; i < spriteArray.Num(); i++)
	{
		ULGUISpriteData* spriteDataItem = spriteArray[i];
		if (!IsValid(spriteDataItem))
		{
			spriteArray.RemoveAt(i);
			i--;
		}
		else if (!IsValid(spriteDataItem->GetSpriteTexture()))
		{
			spriteArray.RemoveAt(i);
			i--;
		}
		else if (spriteDataItem->packingAtlas != this)
		{
			spriteArray.RemoveAt(i);
			i--;
		}
	}
	if (PrevCount != spriteArray.Num())
	{
		this->MarkNotInitialized();
		this->InitCheck();
		this->MarkPackageDirty();
	}
}
#endif

void ULGUIStaticSpriteAtlasData::BeginDestroy()
{
	for (auto& item : spriteArray)
	{
		item->isInitialized = false;
	}
	Super::BeginDestroy();
}

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
		bIsInitialized = true;

		//create texture
		auto texture = NewObject<UTexture2D>(
			this, 
			FName(*FString::Printf(TEXT("LGUIStaticSpriteAtlasData_Texture_%d"), LGUIUtils::LGUITextureNameSuffix++))
			);
		auto PlatformData = new FTexturePlatformData();
		PlatformData->SizeX = textureSize;
		PlatformData->SizeY = textureSize;
		PlatformData->PixelFormat = PF_B8G8R8A8;
		texture->PlatformData = PlatformData;

		//mipmaps
		{
			uint32 textureDataOffset = 0;
			int mipSize = textureSize;
			while (true)
			{
				// Allocate next mipmap.
				FTexture2DMipMap* mip = new FTexture2DMipMap;
				texture->PlatformData->Mips.Add(mip);
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

#if !WITH_EDITOR
		//empty it to reduce memory usage
		textureMipData.Empty();
#endif

		texture->CompressionSettings = TextureCompressionSettings::TC_EditorIcon;
		texture->LODGroup = TextureGroup::TEXTUREGROUP_UI;
		texture->SRGB = atlasTextureUseSRGB;
		texture->Filter = atlasTextureFilter;
		texture->UpdateResource();

		this->atlasTexture = texture;
#if WITH_EDITOR
		for (auto& sprite : renderSpriteArray)
		{
			if (IsValid(sprite.GetObject()))
			{
				IUISpriteRenderableInterface::Execute_ApplyAtlasTextureChange(sprite.GetObject());
			}
		}
#endif
	}
	return bIsInitialized;
}
UTexture2D* ULGUIStaticSpriteAtlasData::GetAtlasTexture()
{
	InitCheck();
	return atlasTexture;
}
bool ULGUIStaticSpriteAtlasData::ReadPixel(const FVector2D& InUV, FColor& OutPixel)
{
	InitCheck();

	auto PlatformData = atlasTexture->PlatformData;
	if (PlatformData && PlatformData->Mips.Num() > 0)
	{
		auto Pixels = (FColor*)(PlatformData->Mips[0].BulkData.Lock(LOCK_READ_ONLY));
		{
			auto uvInFullSize = FIntPoint(InUV.X * textureSize, InUV.Y * textureSize);
			auto PixelIndex = uvInFullSize.Y * textureSize + uvInFullSize.X;
			OutPixel = Pixels[PixelIndex];
		}
		PlatformData->Mips[0].BulkData.Unlock();
		return true;
	}
	return false;
}

#undef LOCTEXT_NAMESPACE
