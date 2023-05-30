// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/LGUIFreeTypeRenderFontData.h"
#include "LGUI.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Core/ActorComponent/UIText.h"
#include "Core/LGUISettings.h"
#include "Utils/LGUIUtils.h"
#include "TextureResource.h"
#include "Engine/Texture2D.h"
#include "Engine/FontFace.h"
#include "Rendering/Texture2DResource.h"
#if WITH_FREETYPE
#include <ft2build.h>
#include FT_FREETYPE_H
#endif

void ULGUIFreeTypeRenderFontData::FinishDestroy()
{
#if WITH_FREETYPE
	DeinitFreeType();
#endif
	Super::FinishDestroy();
}

#if WITH_FREETYPE
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
		UE_LOG(LGUI, Error, TEXT("[%s].%d Font:%s, error:%s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(this->GetName()), ANSI_TO_TCHAR(GetErrorMessage(error)));
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
			UE_LOG(LGUI, Error, TEXT("[%s].%d Font:%s, have no face!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(this->GetName()));
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
					UE_LOG(LGUI, Warning, TEXT("[%s].%d Failed to load or process '%s'"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *unrealFont->GetFontFilename());
					return;
				}
				else
				{
					NewFontFace(tempFontBinaryArray);
				}
			}
		}
		else
		{
			UE_LOG(LGUI, Error, TEXT("[%s].%d Font:%s, trying to load Unreal's font face, but not valid!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(this->GetName()));
			return;
		}
	}
	else
	{
#if WITH_EDITOR
		if (true)
		{
			FString FontFilePathStr = fontFilePath;
			FontFilePathStr = useRelativeFilePath ? FPaths::ProjectDir() + fontFilePath : fontFilePath;
			if (!FPaths::FileExists(*FontFilePathStr))
			{
				if (fontBinaryArray.Num() > 0 && !useExternalFileOrEmbedInToUAsset)
				{
					UE_LOG(LGUI, Warning, TEXT("[%s].%d Font:%s, file: \"%s\" not exist! Will use cache data"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(this->GetName()), *FontFilePathStr);
				}
				else
				{
					UE_LOG(LGUI, Error, TEXT("[%s].%d Font:%s, file: \"%s\" not exist!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(this->GetName()), *FontFilePathStr);
					return;
				}
			}

			if (useExternalFileOrEmbedInToUAsset)
			{
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
		}
		else
#endif	
		{
			if (useExternalFileOrEmbedInToUAsset)
			{
				auto FontFilePathStr = useRelativeFilePath ? FPaths::ProjectDir() + fontFilePath : fontFilePath;
				if (!FPaths::FileExists(*FontFilePathStr))
				{
					UE_LOG(LGUI, Error, TEXT("[%s].%d Font:%s, file: \"%s\" not exist!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(this->GetName()), *FontFilePathStr);
					return;
				}

				fontBinaryArray.Empty();
				FFileHelper::LoadFileToArray(tempFontBinaryArray, *FontFilePathStr);
				NewFontFace(tempFontBinaryArray);
			}
			else
			{
				NewFontFace(fontBinaryArray);
			}
		}
	}

	if (error)
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d Font:%s, error:%s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(this->GetName()), ANSI_TO_TCHAR(GetErrorMessage(error)));
		face = nullptr;
		return;
	}
	else
	{
		UE_LOG(LGUI, Log, TEXT("[%s].%d Success, font:%s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(this->GetName()));
		alreadyInitialized = true;
		hasKerning = FT_HAS_KERNING(face) != 0;

		texture = nullptr;
		textureSize = ULGUISettings::ConvertAtlasTextureSizeTypeToSize(initialSize);
		binPack = rbp::MaxRectsBinPack(rectPackCellSize, rectPackCellSize);
		if (initialSize != ELGUIAtlasTextureSizeType::SIZE_256x256)
		{
			binPack.PrepareExpendSizeForText(textureSize, textureSize, freeRects, rectPackCellSize, false);
		}
		RenewFontTexture(0, textureSize);
		oneDivideTextureSize = 1.0f / textureSize;

		ClearCharDataCache();
	}
}

void ULGUIFreeTypeRenderFontData::DeinitFreeType()
{
	alreadyInitialized = false;
	if (library != nullptr)
	{
		auto error = FT_Done_FreeType(library);
		if (error)
		{
			UE_LOG(LGUI, Error, TEXT("[%s].%d Font:%s, error:%s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(this->GetName()), ANSI_TO_TCHAR(GetErrorMessage(error)));
		}
		else
		{
			UE_LOG(LGUI, Log, TEXT("[%s].%d Success, font:%s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(this->GetName()));
		}
	}
	face = nullptr;
	library = nullptr;
	freeRects.Empty();
	binPack = rbp::MaxRectsBinPack(256, 256);
#if WITH_EDITORONLY_DATA
	subFaces.Reset();
#endif
	fontFace = 0;
	hasKerning = false;
	ClearCharDataCache();
}
#endif

#if WITH_FREETYPE
FT_GlyphSlot ULGUIFreeTypeRenderFontData::RenderGlyphOnFreeType(const TCHAR& charCode, const float& charSize)
{
	InitFreeType();
	if (alreadyInitialized == false)
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d Font '%s' is not initialized"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *this->GetPathName());
		return nullptr;
	}

	auto error = FT_Set_Pixel_Sizes(face, 0, charSize);
	if (error)
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d Font '%s' FT_Set_Pixel_Sizes error:%s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *this->GetPathName(), ANSI_TO_TCHAR(GetErrorMessage(error)));
		return nullptr;
	}
	FT_GlyphSlot slot = face->glyph;
	error = FT_Load_Glyph(face, FT_Get_Char_Index(face, charCode), FT_LOAD_DEFAULT);
	if (slot->glyph_index == 0 && slot->metrics.width == 0 && slot->metrics.height == 0)//missing char in this font
	{
		if (fallbackFontArray.Num() > 0)
		{
			UE_LOG(LGUI, Log, TEXT("[%s].%d Font '%s' Can't find glyph, will search in fallbacks"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *this->GetPathName());
			for (int i = 0; i < fallbackFontArray.Num(); i++)
			{
				if (fallbackFontArray[i] == nullptr)continue;
				if (auto fallbackSlot = fallbackFontArray[i]->RenderGlyphOnFreeType(charCode, charSize))
				{
					return fallbackSlot;
				}
			}
		}
		UE_LOG(LGUI, Error, TEXT("[%s].%d Font '%s' FT_Load_Glyph error:%s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *this->GetPathName(), ANSI_TO_TCHAR(GetErrorMessage(error)));
		return nullptr;
	}
	if (error)
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d Font '%s' FT_Load_Glyph error:%s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *this->GetPathName(), ANSI_TO_TCHAR(GetErrorMessage(error)));
		return nullptr;
	}
	error = FT_Render_Glyph(face->glyph, FT_Render_Mode::FT_RENDER_MODE_NORMAL);
	if (error)
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d Font '%s' FT_Render_Glyph error:%s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *this->GetPathName(), ANSI_TO_TCHAR(GetErrorMessage(error)));
		return nullptr;
	}
	return slot;
}
#endif

UTexture2D* ULGUIFreeTypeRenderFontData::GetFontTexture()
{
	return texture;
}
void ULGUIFreeTypeRenderFontData::InitFont()
{
#if WITH_FREETYPE
	InitFreeType();
#endif
}

float ULGUIFreeTypeRenderFontData::GetKerning(const TCHAR& leftCharIndex, const TCHAR& rightCharIndex, const float& charSize)
{
#if WITH_FREETYPE
	if (face == nullptr)return 0;
	if (!hasKerning)return 0;
	auto error = FT_Set_Pixel_Sizes(face, 0, charSize);
	if (error)
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d FT_Set_Pixel_Sizes error:%s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, ANSI_TO_TCHAR(GetErrorMessage(error)));
		return 0;
	}
	FT_Vector kerning;
	error = FT_Get_Kerning(face, FT_Get_Char_Index(face, leftCharIndex), FT_Get_Char_Index(face, rightCharIndex), FT_KERNING_DEFAULT, &kerning);
	if (error)
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d FT_Get_Kerning error:%s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, ANSI_TO_TCHAR(GetErrorMessage(error)));
		return 0;
	}
	return kerning.x >> 6;
#else
	return 0;
#endif
}
float ULGUIFreeTypeRenderFontData::GetLineHeight(const float& fontSize)
{
#if WITH_FREETYPE
	if (face == nullptr)return fontSize;
	auto error = FT_Set_Pixel_Sizes(face, 0, fontSize);
	if (error)
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d FT_Set_Pixel_Sizes error:%s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, ANSI_TO_TCHAR(GetErrorMessage(error)));
		return fontSize;
	}
	return lineHeightType == ELGUIDynamicFontLineHeightType::FromFontFace ? (face->size->metrics.height >> 6) : fontSize;
#else
	return fontSize;
#endif
}
float ULGUIFreeTypeRenderFontData::GetVerticalOffset(const float& fontSize)
{
#if WITH_FREETYPE
	if (face == nullptr)return fontSize;
	auto error = FT_Set_Pixel_Sizes(face, 0, fontSize);
	if (error)
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d FT_Set_Pixel_Sizes error:%s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, ANSI_TO_TCHAR(GetErrorMessage(error)));
		return 0;
	}
	return -((face->size->metrics.ascender + face->size->metrics.descender) >> 6) * 0.5f;
#else
	return fontSize;
#endif
}

void ULGUIFreeTypeRenderFontData::AddUIText(UUIText* InText)
{
	renderTextArray.AddUnique(InText);
}
void ULGUIFreeTypeRenderFontData::RemoveUIText(UUIText* InText)
{
	renderTextArray.Remove(InText);
}

FLGUICharData_HighPrecision ULGUIFreeTypeRenderFontData::GetCharData(const TCHAR& charCode, const float& charSize)
{
	auto Result = FLGUICharData_HighPrecision();
	if (charSize <= 0.0f)return Result;
	if (!GetCharDataFromCache(charCode, charSize, Result))//if charData not cached, then create it and add to cache
	{
		FGlyphBitmap glyphBitmap;
		if (!RenderGlyph(charCode, charSize, glyphBitmap))
		{
			return Result;
		}

		auto& calcBinpack = this->binPack;
		auto& calcTexture = this->texture;
		FLGUICharData uiCharData;
	PACK_AND_INSERT:
		if (PackRectAndInsertChar(glyphBitmap, calcBinpack, calcTexture, uiCharData))
		{

		}
		else
		{
			int32 newTextureSize = 0;
			if (freeRects.Num() > 0)
			{
				calcBinpack.DoExpendSizeForText(freeRects[freeRects.Num() - 1]);
				freeRects.RemoveAt(freeRects.Num() - 1, 1, false);
			}
			else
			{
				newTextureSize = textureSize + textureSize;
				UE_LOG(LGUI, Log, TEXT("[%s].%d Expend font texture size to:%d"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, newTextureSize);
				//expend by multiply 2
				calcBinpack.PrepareExpendSizeForText(newTextureSize, newTextureSize, freeRects, rectPackCellSize);
				calcBinpack.DoExpendSizeForText(freeRects[freeRects.Num() - 1]);
				freeRects.RemoveAt(freeRects.Num() - 1, 1, false);

				RenewFontTexture(textureSize, newTextureSize);
				textureSize = newTextureSize;
				oneDivideTextureSize = 1.0f / textureSize;

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

			goto PACK_AND_INSERT;
		}

		AddCharDataToCache(charCode, charSize, uiCharData);
		GetCharDataFromCache(charCode, charSize, Result);
	}
	return Result;
}

bool ULGUIFreeTypeRenderFontData::PackRectAndInsertChar(const FGlyphBitmap& InGlyphBitmap, rbp::MaxRectsBinPack& InOutBinpack, UTexture2D* InTexture, FLGUICharData& OutResult)
{
	if (InGlyphBitmap.width <= 0 || InGlyphBitmap.height <= 0)//glyph no need to display, could be space
	{
		OutResult.width = InGlyphBitmap.width;
		OutResult.height = InGlyphBitmap.height;
		OutResult.xoffset = InGlyphBitmap.hOffset;
		OutResult.yoffset = InGlyphBitmap.vOffset;
		OutResult.xadvance = InGlyphBitmap.hAdvance;
		OutResult.uv0X = OutResult.uv0Y = OutResult.uv3X = OutResult.uv3Y = 0.0f;//(0,0) point is transparent
		return true;
	}
	const auto SPACE_NEED_EXPEND = this->Get_SPACE_NEED_EXPEND();
	const auto SPACE_NEED_EXPENDx2 = SPACE_NEED_EXPEND + SPACE_NEED_EXPEND;
	const auto SPACE_BETWEEN_GLYPH_RECT = this->Get_SPACE_BETWEEN_GLYPH() + SPACE_NEED_EXPEND;
	const auto SPACE_BETWEEN_GLYPH_RECTx2 = SPACE_BETWEEN_GLYPH_RECT + SPACE_BETWEEN_GLYPH_RECT;

	int charRectWidth = InGlyphBitmap.width + SPACE_BETWEEN_GLYPH_RECTx2;
	int charRectHeight = InGlyphBitmap.height + SPACE_BETWEEN_GLYPH_RECTx2;
	auto method = rbp::MaxRectsBinPack::RectBestAreaFit;

	auto packedRect = InOutBinpack.Insert(charRectWidth, charRectHeight, method);
	if (packedRect.height <= 0)//means this area cannot fit the char
	{
		return false;
	}
	else//this area can fit the char, so copy pixel color into texture
	{
		//remove space
		packedRect.x += SPACE_BETWEEN_GLYPH_RECT;
		packedRect.y += SPACE_BETWEEN_GLYPH_RECT;
		packedRect.width -= SPACE_BETWEEN_GLYPH_RECTx2;
		packedRect.height -= SPACE_BETWEEN_GLYPH_RECTx2;

		auto region = new FUpdateTextureRegion2D(packedRect.x, packedRect.y, 0, 0, InGlyphBitmap.width, InGlyphBitmap.height);
		UpdateFontTextureRegion(InTexture, region, packedRect.width * InGlyphBitmap.pixelSize, InGlyphBitmap.pixelSize, (uint8*)InGlyphBitmap.buffer);

		OutResult.width = InGlyphBitmap.width + SPACE_NEED_EXPENDx2;
		OutResult.height = InGlyphBitmap.height + SPACE_NEED_EXPENDx2;
		OutResult.xoffset = InGlyphBitmap.hOffset - SPACE_NEED_EXPEND;
		OutResult.yoffset = InGlyphBitmap.vOffset + SPACE_NEED_EXPEND;
		OutResult.xadvance = InGlyphBitmap.hAdvance;
		OutResult.uv0X = oneDivideTextureSize * (packedRect.x - SPACE_NEED_EXPEND);
		OutResult.uv0Y = oneDivideTextureSize * (packedRect.y - SPACE_NEED_EXPEND + OutResult.height);
		OutResult.uv3X = oneDivideTextureSize * (packedRect.x - SPACE_NEED_EXPEND + OutResult.width);
		OutResult.uv3Y = oneDivideTextureSize * (packedRect.y - SPACE_NEED_EXPEND);
		return true;
	}
	return false;
}
void ULGUIFreeTypeRenderFontData::ApplyPackingAtlasTextureExpand(UTexture2D* newTexture, int newTextureSize)
{
	this->texture = newTexture;
	textureSize = newTextureSize;
	oneDivideTextureSize = 1.0f / textureSize;
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
void ULGUIFreeTypeRenderFontData::RenewFontTexture(int oldTextureSize, int newTextureSize)
{
	//store old texutre pointer
	auto oldTexture = texture;
	//create new texture
	texture = CreateFontTexture(newTextureSize);
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
#if WITH_FREETYPE
	DeinitFreeType();
	InitFreeType();
#endif
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
			|| PropertyName == GET_MEMBER_NAME_CHECKED(ULGUIFreeTypeRenderFontData, fontFace)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(ULGUIFreeTypeRenderFontData, fontType)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(ULGUIFreeTypeRenderFontData, lineHeightType)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(ULGUIFreeTypeRenderFontData, unrealFont)
			)
		{
			if (PropertyName == GET_MEMBER_NAME_CHECKED(ULGUIFreeTypeRenderFontData, fontType))
			{
				if (fontType == ELGUIDynamicFontDataType::UnrealFont)
				{
					fontBinaryArray.Empty();//clear cache font data when swich to UnrealFont
				}
			}
			ReloadFont();
		}

		{
			int powerValue = 0;
			while (rectPackCellSize > 0)
			{
				rectPackCellSize = rectPackCellSize >> 1;
				powerValue++;
			}
			rectPackCellSize = 1;
			while (powerValue > 0)
			{
				rectPackCellSize = rectPackCellSize << 1;
				powerValue--;
			}
			
			rectPackCellSize = FMath::Clamp(rectPackCellSize, 64, ULGUISettings::ConvertAtlasTextureSizeTypeToSize(initialSize));
		}
	}
}
#endif

