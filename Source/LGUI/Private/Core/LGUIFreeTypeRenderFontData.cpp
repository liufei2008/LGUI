// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Core/LGUIFreeTypeRenderFontData.h"
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

void ULGUIFreeTypeRenderFontData::FinishDestroy()
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
TArray<FString> ULGUIFreeTypeRenderFontData::CacheSubFaces(FT_LibraryRec_* InFTLibrary, const TArray<uint8>& InMemory)
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

void ULGUIFreeTypeRenderFontData::InitFreeType()
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
			return;
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
		hasKerning = FT_HAS_KERNING(face) != 0;
		if (usePackingTag)
		{
			if (packingAtlasData == nullptr)
			{
				packingAtlasData = ULGUIAtlasManager::FindOrAdd(packingTag);
				packingAtlasTextureExpandDelegateHandle = packingAtlasData->OnTextureSizeExpanded.AddUObject(this, &ULGUIFreeTypeRenderFontData::ApplyPackingAtlasTextureExpand);
			}
			packingAtlasData->EnsureAtlasTexture(packingTag);
			texture = packingAtlasData->atlasTexture;
			textureSize = texture->GetSizeX();
			oneDiviceTextureSize = 1.0f / textureSize;
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
			oneDiviceTextureSize = 1.0f / textureSize;
		}
		ClearCharDataCache();
	}
}
void ULGUIFreeTypeRenderFontData::DeinitFreeType()
{
	alreadyInitialized = false;
	usePackingTag = false;
	packingAtlasData = ULGUIAtlasManager::Find(packingTag);
	if (packingAtlasData != nullptr)
	{
		packingAtlasData->OnTextureSizeExpanded.Remove(packingAtlasTextureExpandDelegateHandle);
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
	library = nullptr;
	freeRects.Empty();
	binPack = rbp::MaxRectsBinPack(256, 256);
	ClearCharDataCache();
}
UTexture2D* ULGUIFreeTypeRenderFontData::GetFontTexture()
{
	return texture;
}
void ULGUIFreeTypeRenderFontData::InitFont()
{
	InitFreeType();
}

float ULGUIFreeTypeRenderFontData::GetKerning(const TCHAR& leftCharIndex, const TCHAR& rightCharIndex, const float& charSize)
{
	if (face == nullptr)return 0;
	if (!hasKerning)return 0;
	auto error = FT_Set_Pixel_Sizes(face, 0, charSize);
	if (error)
	{
		UE_LOG(LGUI, Error, TEXT("[GetKerning] FT_Set_Pixel_Sizes error:%s"), ANSI_TO_TCHAR(GetErrorMessage(error)));
		return 0;
	}
	FT_Vector kerning;
	error = FT_Get_Kerning(face, FT_Get_Char_Index(face, leftCharIndex), FT_Get_Char_Index(face, rightCharIndex), FT_KERNING_DEFAULT, &kerning);
	if (error)
	{
		UE_LOG(LGUI, Error, TEXT("[GetKerning] FT_Get_Kerning error:%s"), ANSI_TO_TCHAR(GetErrorMessage(error)));
		return 0;
	}
	return kerning.x >> 6;
}
float ULGUIFreeTypeRenderFontData::GetLineHeight(const float& fontSize)
{
	if (face == nullptr)return fontSize;
	auto error = FT_Set_Pixel_Sizes(face, 0, fontSize);
	if (error)
	{
		UE_LOG(LGUI, Error, TEXT("[GetLineHeight] FT_Set_Pixel_Sizes error:%s"), ANSI_TO_TCHAR(GetErrorMessage(error)));
		return fontSize;
	}
	return lineHeightType == ELGUIDynamicFontLineHeightType::FromFontFace ? (face->size->metrics.height >> 6) : fontSize;
}
float ULGUIFreeTypeRenderFontData::GetVerticalOffset(const float& fontSize)
{
	if (face == nullptr)return fontSize;
	auto error = FT_Set_Pixel_Sizes(face, 0, fontSize);
	if (error)
	{
		UE_LOG(LGUI, Error, TEXT("[GetVerticalOffset] FT_Set_Pixel_Sizes error:%s"), ANSI_TO_TCHAR(GetErrorMessage(error)));
		return 0;
	}
	return -((face->size->metrics.ascender + face->size->metrics.descender) >> 6) * 0.5f;
}


void ULGUIFreeTypeRenderFontData::AddUIText(UUIText* InText)
{
	renderTextArray.AddUnique(InText);
}
void ULGUIFreeTypeRenderFontData::RemoveUIText(UUIText* InText)
{
	renderTextArray.Remove(InText);
}

FT_GlyphSlot ULGUIFreeTypeRenderFontData::RenderGlyphOnFreeType(const TCHAR& charCode, const float& charSize)
{
	InitFreeType();
	if (alreadyInitialized == false)
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIFreeTypeRenderFontData::RenderGlyphOnFreeType] Font is not initialized"));
		return nullptr;
	}

	auto error = FT_Set_Pixel_Sizes(face, 0, charSize);
	if (error)
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIFreeTypeRenderFontData::RenderGlyphOnFreeType] FT_Set_Pixel_Sizes error:%s"), ANSI_TO_TCHAR(GetErrorMessage(error)));
		return nullptr;
	}
	FT_GlyphSlot slot = face->glyph;
	error = FT_Load_Glyph(face, FT_Get_Char_Index(face, charCode), FT_LOAD_DEFAULT);
	if (error)
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIFreeTypeRenderFontData::RenderGlyphOnFreeType] FT_Load_Glyph error:%s"), ANSI_TO_TCHAR(GetErrorMessage(error)));
		return nullptr;
	}
	error = FT_Render_Glyph(face->glyph, FT_Render_Mode::FT_RENDER_MODE_NORMAL);
	if (error)
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIFreeTypeRenderFontData::RenderGlyphOnFreeType] FT_Render_Glyph error:%s"), ANSI_TO_TCHAR(GetErrorMessage(error)));
		return nullptr;
	}
	return slot;
}

FLGUICharData_HighPrecision ULGUIFreeTypeRenderFontData::GetCharData(const TCHAR& charCode, const float& charSize)
{
	auto Result = FLGUICharData_HighPrecision();
	if (charSize <= 0.0f)return Result;
	if (GetCharDataFromCache(charCode, charSize, Result))
	{
		return Result;
	}
	else
	{
		FGlyphBitmap glyphBitmap;
		if (!RenderGlyph(charCode, charSize, glyphBitmap))
		{
			return Result;
		}

		auto& calcBinpack = usePackingTag ? packingAtlasData->atlasBinPack : this->binPack;
		auto& calcTexture = usePackingTag ? packingAtlasData->atlasTexture : this->texture;
		FLGUICharData resultCharData;
	PACK_AND_INSERT:
		if (PackRectAndInsertChar(glyphBitmap, calcBinpack, calcTexture, resultCharData))
		{

		}
		else
		{
			int32 newTextureSize = 0;
			if (usePackingTag)
			{
				newTextureSize = packingAtlasData->ExpendTextureSize(packingTag);
				UE_LOG(LGUI, Log, TEXT("[ULGUIFontData::GetCharData]Expend font texture size to:%d"), newTextureSize);
				texture = packingAtlasData->atlasTexture;
				textureSize = newTextureSize;
				oneDiviceTextureSize = 1.0f / textureSize;
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
					UE_LOG(LGUI, Log, TEXT("[ULGUIFontData::GetCharData]Expend font texture size to:%d"), newTextureSize);
					//expend by multiply 2
					calcBinpack.PrepareExpendSizeForText(newTextureSize, newTextureSize, freeRects);
					calcBinpack.DoExpendSizeForText(freeRects[freeRects.Num() - 1]);
					freeRects.RemoveAt(freeRects.Num() - 1, 1, false);

					CreateFontTexture(textureSize, newTextureSize);
					textureSize = newTextureSize;
					oneDiviceTextureSize = 1.0f / textureSize;

					//scale down uv of prev chars
					ScaleDownUVofCachedChars();
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

		AddCharDataToCache(charCode, charSize, resultCharData);
		return resultCharData;
	}
	return Result;
}

bool ULGUIFreeTypeRenderFontData::PackRectAndInsertChar(const FGlyphBitmap& InGlyphBitmap, rbp::MaxRectsBinPack& InOutBinpack, UTexture2D* InTexture, FLGUICharData& OutResult)
{
	int charRectWidth = InGlyphBitmap.width + SPACE_BETWEEN_GLYPHx2;
	int charRectHeight = InGlyphBitmap.height + SPACE_BETWEEN_GLYPHx2;
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
		int pixelCount = InGlyphBitmap.width * InGlyphBitmap.height;
		FColor* regionColor = new FColor[pixelCount];
		for (int i = 0; i < pixelCount; i++)
		{
			auto& pixelColor = regionColor[i];
			pixelColor.R = pixelColor.G = pixelColor.B = 255;
			pixelColor.A = InGlyphBitmap.buffer[i];
		}
		auto region = new FUpdateTextureRegion2D(packedRect.x, packedRect.y, 0, 0, InGlyphBitmap.width, InGlyphBitmap.height);
		UpdateFontTextureRegion(InTexture, region, packedRect.width * 4, 4, (uint8*)regionColor);

		OutResult.width = InGlyphBitmap.width + SPACE_NEED_EXPENDx2;
		OutResult.height = InGlyphBitmap.height + SPACE_NEED_EXPENDx2;
		OutResult.xoffset = InGlyphBitmap.hOffset - SPACE_NEED_EXPEND;
		OutResult.yoffset = InGlyphBitmap.vOffset + SPACE_NEED_EXPEND;
		OutResult.xadvance = InGlyphBitmap.hAdvance;
		OutResult.uv0X = oneDiviceTextureSize * (packedRect.x - SPACE_NEED_EXPEND);
		OutResult.uv0Y = oneDiviceTextureSize * (packedRect.y - SPACE_NEED_EXPEND + OutResult.height);
		OutResult.uv3X = oneDiviceTextureSize * (packedRect.x - SPACE_NEED_EXPEND + OutResult.width);
		OutResult.uv3Y = oneDiviceTextureSize * (packedRect.y - SPACE_NEED_EXPEND);
		return true;
	}
	return false;
}
void ULGUIFreeTypeRenderFontData::ApplyPackingAtlasTextureExpand(UTexture2D* newTexture, int newTextureSize)
{
	this->texture = newTexture;
	textureSize = newTextureSize;
	oneDiviceTextureSize = 1.0f / textureSize;
	//tell UIText to scale down uv
	for (auto textItem : renderTextArray)
	{
		if (textItem.IsValid())
		{
			textItem->ApplyFontTextureScaleUp();
		}
	}
}

void ULGUIFreeTypeRenderFontData::UpdateFontTextureRegion(UTexture2D* Texture, FUpdateTextureRegion2D* Region, uint32 SrcPitch, uint32 SrcBpp, uint8* SrcData)
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
void ULGUIFreeTypeRenderFontData::CreateFontTexture(int oldTextureSize, int newTextureSize)
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

#if WITH_EDITOR
void ULGUIFreeTypeRenderFontData::ReloadFont()
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
void ULGUIFreeTypeRenderFontData::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.Property)
	{
		auto PropertyName = Property->GetFName();
		if (PropertyName == GET_MEMBER_NAME_CHECKED(ULGUIFreeTypeRenderFontData, useExternalFileOrEmbedInToUAsset)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(ULGUIFreeTypeRenderFontData, fontFilePath)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(ULGUIFreeTypeRenderFontData, fontFace)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(ULGUIFreeTypeRenderFontData, fontType)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(ULGUIFreeTypeRenderFontData, packingTag)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(ULGUIFreeTypeRenderFontData, lineHeightType)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(ULGUIFreeTypeRenderFontData, unrealFont)
			)
		{
			ReloadFont();
		}
	}
}
#endif

