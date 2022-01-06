// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Core/LGUIFontData.h"
#include "LGUI.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"
#include "Core/ActorComponent/UIText.h"
#include "Core/LGUIAtlasData.h"
#include "Core/LGUISettings.h"
#include "Utils/LGUIUtils.h"
#include "TextureResource.h"
#include "Engine/Texture2D.h"
#include "Engine/FontFace.h"
#include "Rendering/Texture2DResource.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H

void ULGUIFontData::FinishDestroy()
{
	DeinitFreeType();
	Super::FinishDestroy();
}

const char* GetErrorMessage(FT_Error err)
{
#undef __FTERRORS_H__
#define FT_ERRORDEF( e, v, s )  case e: return s;
#define FT_ERROR_START_LIST     switch (err) {
#define FT_ERROR_END_LIST       }
#include FT_ERRORS_H
	return "(Unknown error)";
}

#if WITH_EDITOR
TArray<FString> ULGUIFontData::CacheSubFaces(FT_LibraryRec_* InFTLibrary, const TArray<uint8>& InMemory)
{
	TArray<FString> Result;
	FT_Face FTFace = nullptr;
	FT_New_Memory_Face(InFTLibrary, InMemory.GetData(), static_cast<FT_Long>(InMemory.Num()), -1, &FTFace);
	if (FTFace)
	{
		const int32 NumFaces = FTFace->num_faces;
		FT_Done_Face(FTFace);
		FTFace = nullptr;

		Result.Reserve(NumFaces);
		for (int32 FaceIndex = 0; FaceIndex < NumFaces; ++FaceIndex)
		{
			FT_New_Memory_Face(InFTLibrary, InMemory.GetData(), static_cast<FT_Long>(InMemory.Num()), FaceIndex, &FTFace);
			if (FTFace)
			{
				Result.Add(FString::Printf(TEXT("%s (%s)"), UTF8_TO_TCHAR(FTFace->family_name), UTF8_TO_TCHAR(FTFace->style_name)));
				FT_Done_Face(FTFace);
				FTFace = nullptr;
			}
		}
	}
	return Result;
}
#endif

void ULGUIFontData::InitFreeType()
{
	if (alreadyInitialized)return;
	FT_Error error = 0;
	error = FT_Init_FreeType(&library);
	if (error)
	{
		UE_LOG(LGUI, Error, TEXT("[InitFreeType]font:%s, error:%s"), *(this->GetName()), ANSI_TO_TCHAR(GetErrorMessage(error)));
		return;
	}

	auto NewFontFace = [&error, this](const TArray<uint8>& InFontBinary) {
#if WITH_EDITOR
		subFaces = CacheSubFaces(library, InFontBinary);
		if (subFaces.Num() > 0)
		{
			fontFace = FMath::Clamp(fontFace, 0, subFaces.Num());
#endif
			error = FT_New_Memory_Face(library, InFontBinary.GetData(), InFontBinary.Num(), fontFace, &face);
#if WITH_EDITOR
		}
		else
		{
			UE_LOG(LGUI, Error, TEXT("[InitFreeType]font:%s, have no face!"), *(this->GetName()));
		}
#endif
	};

	if (fontType == ELGUIDynamicFontDataType::UnrealFont)
	{
		if (IsValid(unrealFont))
		{
			if (unrealFont->GetFontFaceData()->HasData())
			{
				NewFontFace(unrealFont->GetFontFaceData()->GetData());
			}
			else
			{
				if (!FFileHelper::LoadFileToArray(tempFontBinaryArray, *unrealFont->GetFontFilename()))
				{
					UE_LOG(LGUI, Warning, TEXT("failed to load or process '%s'"), *unrealFont->GetFontFilename());
					return;
				}
				else
				{
					NewFontFace(tempFontBinaryArray);
				}
			}
			if (error == 0)
			{
				fontBinaryArray.Empty();
			}
		}
		else
		{
			UE_LOG(LGUI, Error, TEXT("[InitFreeType]font:%s, trying to load Unreal's font face, but not valid!"), *(this->GetName()));
		}
	}
	else
	{
#if WITH_EDITOR
		FString FontFilePathStr = fontFilePath;
		FontFilePathStr = useRelativeFilePath ? FPaths::ProjectDir() + fontFilePath : fontFilePath;
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		if (!PlatformFile.FileExists(*FontFilePathStr))
		{
			if (fontBinaryArray.Num() > 0 && !useExternalFileOrEmbedInToUAsset)
			{
				UE_LOG(LGUI, Log, TEXT("[InitFreeType]font:%s, file: \"%s\" not exist! Will use cache data"), *(this->GetName()), *FontFilePathStr);
			}
			else
			{
				UE_LOG(LGUI, Error, TEXT("[InitFreeType]font:%s, file: \"%s\" not exist!"), *(this->GetName()), *FontFilePathStr);
				return;
			}
		}

		if (useExternalFileOrEmbedInToUAsset)
		{
			tempFontBinaryArray.Empty();
			FFileHelper::LoadFileToArray(tempFontBinaryArray, *FontFilePathStr);
			NewFontFace(tempFontBinaryArray);
			if (error == 0)
			{
				fontBinaryArray.Empty();
			}
		}
		else
		{
			FFileHelper::LoadFileToArray(fontBinaryArray, *FontFilePathStr);
			NewFontFace(fontBinaryArray);
		}
#else
		if (useExternalFileOrEmbedInToUAsset)
		{
			auto FontFilePathStr = useRelativeFilePath ? FPaths::ProjectDir() + fontFilePath : fontFilePath;
			IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
			if (!PlatformFile.FileExists(*FontFilePathStr))
			{
				UE_LOG(LGUI, Error, TEXT("[InitFreeType]font:%s, file: \"%s\" not exist!"), *(this->GetName()), *FontFilePathStr);
				return;
			}

			tempFontBinaryArray.Empty();
			fontBinaryArray.Empty();
			FFileHelper::LoadFileToArray(tempFontBinaryArray, *FontFilePathStr);
			error = FT_New_Memory_Face(library, tempFontBinaryArray.GetData(), tempFontBinaryArray.Num(), fontFace, &face);
		}
		else
		{
			error = FT_New_Memory_Face(library, fontBinaryArray.GetData(), fontBinaryArray.Num(), fontFace, &face);
		}
#endif	
	}
	
	if (error)
	{
		UE_LOG(LGUI, Error, TEXT("[InitFreeType]font:%s, error:%s"), *(this->GetName()), ANSI_TO_TCHAR(GetErrorMessage(error)));
		face = nullptr;
		return;
	}
	else
	{
		UE_LOG(LGUI, Log, TEXT("[InitFreeType]success, font:%s"), *(this->GetName()));
		alreadyInitialized = true;
		usePackingTag = !packingTag.IsNone();
		if (usePackingTag)
		{
			if (packingAtlasData == nullptr)
			{
				packingAtlasData = ULGUIAtlasManager::FindOrAdd(packingTag);
				packingAtlasTextureExpandDelegateHandle = packingAtlasData->expandTextureSizeCallback.AddUObject(this, &ULGUIFontData::ApplyPackingAtlasTextureExpand);
			}
			packingAtlasData->EnsureAtlasTexture(packingTag);
			texture = packingAtlasData->atlasTexture;
			textureSize = texture->GetSizeX();
			fullTextureSizeReciprocal = 1.0f / textureSize;
			charDataMap.Empty();
		}
		else
		{
			texture = nullptr;

			textureSize = 256;//default font texture size is 256
			binPack = rbp::MaxRectsBinPack(textureSize, textureSize);
			if (initialSize != ELGUIAtlasTextureSizeType::SIZE_256x256)
			{
				textureSize = ULGUISettings::ConvertAtlasTextureSizeTypeToSize(initialSize);
				binPack.PrepareExpendSizeForText(textureSize, textureSize, freeRects, false);
			}
			CreateFontTexture(0, textureSize);
			fullTextureSizeReciprocal = 1.0f / textureSize;
			charDataMap.Empty();
		}
	}
}
void ULGUIFontData::DeinitFreeType()
{
	alreadyInitialized = false;
	usePackingTag = false;
	packingAtlasData = ULGUIAtlasManager::Find(packingTag);
	if (packingAtlasData != nullptr)
	{
		packingAtlasData->expandTextureSizeCallback.Remove(packingAtlasTextureExpandDelegateHandle);
		packingAtlasData = nullptr;
	}
	if (face != nullptr)
	{
		face = nullptr;
		auto error = FT_Done_FreeType(library);
		if (error)
		{
			UE_LOG(LGUI, Error, TEXT("[DeintFreeType]font:%s, error:%s"), *(this->GetName()), ANSI_TO_TCHAR(GetErrorMessage(error)));
		}
		else
		{
			UE_LOG(LGUI, Log, TEXT("[DeintFreeType]success, font:%s"), *(this->GetName()));
		}
	}
	freeRects.Empty();
	binPack = rbp::MaxRectsBinPack(256, 256);
}
UTexture2D* ULGUIFontData::GetFontTexture()
{
	return texture;
}
void ULGUIFontData::InitFont()
{
	InitFreeType();
}
void ULGUIFontData::AddUIText(UUIText* InText)
{
	renderTextArray.AddUnique(InText);
}
void ULGUIFontData::RemoveUIText(UUIText* InText)
{
	renderTextArray.Remove(InText);
}

FLGUICharData* ULGUIFontData::PushCharIntoFont(const TCHAR& charIndex, const uint16& charSize)
{
	if (alreadyInitialized == false)
	{
		InitFreeType();
		if (alreadyInitialized == false)
		{
			UE_LOG(LGUI, Error, TEXT("[PushCharIntoFont]Font is not initialized"));
			cacheCharData = FLGUICharData();
			return &cacheCharData;
		}
	}

	auto error = FT_Set_Pixel_Sizes(face, 0, charSize);
	if (error)
	{
		UE_LOG(LGUI, Error, TEXT("FT_Set_Pixel_Sizes error:%s"), ANSI_TO_TCHAR(GetErrorMessage(error)));
		return &cacheCharData;
	}
	FT_GlyphSlot slot = face->glyph;
	error = FT_Load_Glyph(face, FT_Get_Char_Index(face, charIndex), FT_LOAD_DEFAULT);
	if (error)
	{
		UE_LOG(LGUI, Error, TEXT("FT_Load_Glyph error:%s"), ANSI_TO_TCHAR(GetErrorMessage(error)));
		return &cacheCharData;
	}
	error = FT_Render_Glyph(face->glyph, FT_Render_Mode::FT_RENDER_MODE_NORMAL);
	if (error)
	{
		UE_LOG(LGUI, Error, TEXT("FT_Render_Glyph error:%s"), ANSI_TO_TCHAR(GetErrorMessage(error)));
		return &cacheCharData;
	}

	auto& calcBinpack = usePackingTag ? packingAtlasData->atlasBinPack : this->binPack;
	auto& calcTexture = usePackingTag ? packingAtlasData->atlasTexture : this->texture;
PACK_AND_INSERT:
	if (PackRectAndInsertChar(slot, calcBinpack, calcTexture))
	{

	}
	else
	{
		int32 newTextureSize = 0;
		if (usePackingTag)
		{
			newTextureSize = packingAtlasData->ExpendTextureSize(packingTag);
			UE_LOG(LGUI, Log, TEXT("[PushCharIntoFont]Expend font texture size to:%d"), newTextureSize);
			texture = packingAtlasData->atlasTexture;
			textureSize = newTextureSize;
			fullTextureSizeReciprocal = 1.0f / textureSize;
		}
		else
		{
			if (freeRects.Num() > 0)
			{
				calcBinpack.DoExpendSizeForText(freeRects[freeRects.Num() - 1]);
				freeRects.RemoveAt(freeRects.Num() - 1, 1, false);
			}
			else
			{
				newTextureSize = textureSize + textureSize;
				UE_LOG(LGUI, Log, TEXT("[PushCharIntoFont]Expend font texture size to:%d"), newTextureSize);
				//expend by multiply 2
				calcBinpack.PrepareExpendSizeForText(newTextureSize, newTextureSize, freeRects);
				calcBinpack.DoExpendSizeForText(freeRects[freeRects.Num() - 1]);
				freeRects.RemoveAt(freeRects.Num() - 1, 1, false);

				CreateFontTexture(textureSize, newTextureSize);
				textureSize = newTextureSize;
				fullTextureSizeReciprocal = 1.0f / textureSize;

				//scale down uv of prev chars
				for (auto& charDataItem : charDataMap)
				{
					auto& mapValue = charDataItem.Value;
					mapValue.uv0X *= 0.5f;
					mapValue.uv0Y *= 0.5f;
					mapValue.uv3X *= 0.5f;
					mapValue.uv3Y *= 0.5f;
				}
				//tell UIText to scale down uv
				for (auto textItem : renderTextArray)
				{
					if (textItem.IsValid())
					{
						textItem->ApplyFontTextureScaleUp();
					}
				}
			}
		}

		goto PACK_AND_INSERT;
	}

	charDataMap.Add(cacheFontKey, cacheCharData);

	return &cacheCharData;
}
bool ULGUIFontData::PackRectAndInsertChar(FT_GlyphSlotRec_* InSlot, rbp::MaxRectsBinPack& InOutBinpack, UTexture2D* InTexture)
{
	const auto& charBitmap = InSlot->bitmap;
	int charRectWidth = charBitmap.width + SPACE_BETWEEN_GLYPHx2;
	int charRectHeight = charBitmap.rows + SPACE_BETWEEN_GLYPHx2;
	auto method = rbp::MaxRectsBinPack::RectBestAreaFit;

	auto packedRect = InOutBinpack.Insert(charRectWidth, charRectHeight, method);
	if (packedRect.height <= 0)//means this area cannot fit the char
	{
		return false;
	}
	else//this area can fit the char, so copy pixel color into texture
	{
		//remove space
		packedRect.x += SPACE_BETWEEN_GLYPH;
		packedRect.y += SPACE_BETWEEN_GLYPH;
		packedRect.width -= SPACE_BETWEEN_GLYPHx2;
		packedRect.height -= SPACE_BETWEEN_GLYPHx2;

		//pixel color
		int pixelCount = charBitmap.width * charBitmap.rows;
		FColor* regionColor = new FColor[pixelCount];
		for (int i = 0; i < pixelCount; i++)
		{
			auto& pixelColor = regionColor[i];
			pixelColor.R = pixelColor.G = pixelColor.B = 255;
			pixelColor.A = charBitmap.buffer[i];
		}
		auto region = new FUpdateTextureRegion2D(packedRect.x, packedRect.y, 0, 0, charBitmap.width, charBitmap.rows);
		UpdateFontTextureRegion(InTexture, region, packedRect.width * 4, 4, (uint8*)regionColor);

		cacheCharData.width = charBitmap.width + SPACE_NEED_EXPENDx2;
		cacheCharData.height = charBitmap.rows + SPACE_NEED_EXPENDx2;
		cacheCharData.xoffset = InSlot->bitmap_left - SPACE_NEED_EXPEND;
		cacheCharData.yoffset = InSlot->bitmap_top - SPACE_NEED_EXPEND;
		cacheCharData.xadvance = InSlot->metrics.horiAdvance >> 6;
		cacheCharData.uv0X = fullTextureSizeReciprocal * (packedRect.x - SPACE_NEED_EXPEND);
		cacheCharData.uv0Y = fullTextureSizeReciprocal * (packedRect.y - SPACE_NEED_EXPEND + cacheCharData.height);
		cacheCharData.uv3X = fullTextureSizeReciprocal * (packedRect.x - SPACE_NEED_EXPEND + cacheCharData.width);
		cacheCharData.uv3Y = fullTextureSizeReciprocal * (packedRect.y - SPACE_NEED_EXPEND);
		return true;
	}
	return false;
}
void ULGUIFontData::ApplyPackingAtlasTextureExpand(UTexture2D* newTexture, int newTextureSize)
{
	this->texture = newTexture;
	textureSize = newTextureSize;
	fullTextureSizeReciprocal = 1.0f / textureSize;
	//scale down uv of prev chars
	for (auto& charDataItem : charDataMap)
	{
		auto& mapValue = charDataItem.Value;
		mapValue.uv0X *= 0.5f;
		mapValue.uv0Y *= 0.5f;
		mapValue.uv3X *= 0.5f;
		mapValue.uv3Y *= 0.5f;
	}
	//tell UIText to scale down uv
	for (auto textItem : renderTextArray)
	{
		if (textItem.IsValid())
		{
			textItem->ApplyFontTextureScaleUp();
		}
	}
}

void ULGUIFontData::UpdateFontTextureRegion(UTexture2D* Texture, FUpdateTextureRegion2D* Region, uint32 SrcPitch, uint32 SrcBpp, uint8* SrcData)
{
	if (Texture->Resource)
	{
		struct FUpdateTextureRegionsData
		{
			FTexture2DResource* Texture2DResource;
			FUpdateTextureRegion2D* Region;
			uint32 SrcPitch;
			uint32 SrcBpp;
			uint8* SrcData;
		};
		FUpdateTextureRegionsData* RegionData = new FUpdateTextureRegionsData;

		auto Texture2DRes = (FTexture2DResource*)Texture->Resource;
		RegionData->Region = Region;
		RegionData->SrcPitch = SrcPitch;
		RegionData->SrcBpp = SrcBpp;
		RegionData->SrcData = SrcData;
		ENQUEUE_RENDER_COMMAND(FLGUIFontUpdateFontTextureRegionData)(
			[RegionData, Texture2DRes](FRHICommandListImmediate& RHICmdList)
			{
				RHICmdList.UpdateTexture2D(
					Texture2DRes->GetTexture2DRHI(),
					0,
					*RegionData->Region,
					RegionData->SrcPitch,
					RegionData->SrcData
					+ RegionData->Region->SrcY * RegionData->SrcPitch
					+ RegionData->Region->SrcX * RegionData->SrcBpp
				);
				FMemory::Free(RegionData->SrcData);
				FMemory::Free(RegionData->Region);
				delete RegionData;
			});
	}
}
void ULGUIFontData::CreateFontTexture(int oldTextureSize, int newTextureSize)
{
	//store old texutre pointer
	auto oldTexture = texture;
	//create new texture
	texture = LGUIUtils::CreateTexture(newTextureSize, FColor(255, 255, 255, 0));
	texture->CompressionSettings = TextureCompressionSettings::TC_EditorIcon;
	texture->LODGroup = TextureGroup::TEXTUREGROUP_UI;
	texture->SRGB = true;
	texture->Filter = TextureFilter::TF_Trilinear;
	texture->UpdateResource();
	texture->AddToRoot();//@todo: is this really need to AddToRoot?

	//copy old texture to new one
	if (IsValid(oldTexture) && oldTextureSize > 0)
	{
		auto newTexture = texture;
		if (oldTexture->Resource != nullptr && newTexture->Resource != nullptr)
		{
			ENQUEUE_RENDER_COMMAND(FLGUIFontUpdateAndCopyFontTexture)(
				[oldTexture, newTexture, oldTextureSize](FRHICommandListImmediate& RHICmdList)
			{
				//copy old texture pixels
				if (oldTextureSize != 0)
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
					RHICmdList.ImmediateFlush(EImmediateFlushType::FlushRHIThread);//if remove this line, then texture will go wrong if expand texture size and write font pixels, looks like copy-pixels hanppens after write-font-pixels.
					oldTexture->RemoveFromRoot();//ready for gc
				}
			});
		}
	}
}

FLGUICharData_HighPrecision ULGUIFontData::GetCharData(const TCHAR& charIndex, const uint16& charSize)
{
	cacheFontKey = FLGUIFontKeyData(charIndex, charSize);
	if (auto charData = charDataMap.Find(cacheFontKey))
	{
		return *charData;
	}
	else
	{
		return *PushCharIntoFont(charIndex, charSize);
	}
	return FLGUICharData();
}
#if WITH_EDITOR
void ULGUIFontData::ReloadFont()
{
	DeinitFreeType();
	InitFreeType();
	for (auto textItem : renderTextArray)
	{
		if (textItem.IsValid())
		{
			textItem->ApplyFontTextureChange();
		}
	}
}
void ULGUIFontData::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.Property)
	{
		auto PropertyName = Property->GetFName();
		if (PropertyName == GET_MEMBER_NAME_CHECKED(ULGUIFontData, useExternalFileOrEmbedInToUAsset)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(ULGUIFontData, fontFace)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(ULGUIFontData, fontType)
			)
		{
			ReloadFont();
		}
		if (PropertyName == GET_MEMBER_NAME_CHECKED(ULGUIFontData, fixedVerticalOffset) 
			|| PropertyName == GET_MEMBER_NAME_CHECKED(ULGUIFontData, italicAngle) 
			|| PropertyName == GET_MEMBER_NAME_CHECKED(ULGUIFontData, boldRatio)
			)
		{
			for (auto textItem : renderTextArray)
			{
				if (textItem.IsValid())
				{
					textItem->ApplyRecreateText();
				}
			}
		}
	}
}
#endif
ULGUIFontData* ULGUIFontData::GetDefaultFont()
{
	static auto defaultFont = LoadObject<ULGUIFontData>(NULL, TEXT("/LGUI/DefaultFont"));
	if (defaultFont == nullptr)
	{
		UE_LOG(LGUI, Error, TEXT("[LGUIFontData::GetDefaultFont]Load default font error! Missing some content of LGUI plugin, reinstall this plugin may fix the issure."));
		return nullptr;
	}
	return defaultFont;
}
