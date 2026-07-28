// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/LexUIFontData_FreeTypeRender.h"
#include "LGUI.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Core/Components/LexText.h"
#include "TextureResource.h"
#include "Engine/Texture2D.h"
#include "Engine/FontFace.h"
#include "Engine/Texture2DArray.h"
#include "Internationalization/Culture.h"
#include "Rendering/Texture2DResource.h"
#include "Runtime/Engine/Private/Rendering/Texture2DArrayResource.h"
#if WITH_FREETYPE
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#endif

void ULexUIFontData_FreeTypeRender::UpdateFontOnCultureChanged()
{
	FString CurrentCulture = FInternationalization::Get().GetCurrentCulture()->GetName();
	if (CultureFontMap.Contains(CurrentCulture))
		EngineFont = CultureFontMap[CurrentCulture].LoadSynchronous();

	if (FontType == ELexUIDynamicFontDataType::EngineFont)
	{
		FontBinaryArray.Empty();//clear cache font data when switch to EngineFont
	}

#if WITH_FREETYPE
	DeinitFreeType();
	InitFreeType();
#endif
}

void ULexUIFontData_FreeTypeRender::FinishDestroy()
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
TArray<FString> ULexUIFontData_FreeTypeRender::CacheSubFaces(FT_LibraryRec_* InFTLibrary, const TArray<uint8>& InMemory)
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

void ULexUIFontData_FreeTypeRender::InitFreeType()
{
	if (bAlreadyInitialized)return;
	FT_Error error = 0;
	error = FT_Init_FreeType(&Library);
	if (error)
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d Font:%s, error:%s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(this->GetName()), ANSI_TO_TCHAR(GetErrorMessage(error)));
		return;
	}

	auto NewFontFace = [&error, this](const TArray<uint8>& InFontBinary) {
#if WITH_EDITOR
		SubFaces = CacheSubFaces(Library, InFontBinary);
		if (SubFaces.Num() > 0)
		{
			FontFace = FMath::Clamp(FontFace, 0, SubFaces.Num());
#endif
			error = FT_New_Memory_Face(Library, InFontBinary.GetData(), InFontBinary.Num(), FontFace, &Face);
#if WITH_EDITOR
		}
		else
		{
			UE_LOG(LGUI, Error, TEXT("[%s].%d Font:%s, have no face!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(this->GetName()));
		}
#endif
	};

	if (FontType == ELexUIDynamicFontDataType::EngineFont)
	{
#if WITH_EDITOR
		//editor use data from EngineFont
		if (IsValid(EngineFont))
		{
			if (EngineFont->GetFontFaceData()->HasData())
			{
				NewFontFace(EngineFont->GetFontFaceData()->GetData());
			}
			else
			{
				if (!FFileHelper::LoadFileToArray(TempFontBinaryArray, *EngineFont->GetFontFilename()))
				{
					UE_LOG(LGUI, Warning, TEXT("[%s].%d Failed to load or process '%s'"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *EngineFont->GetFontFilename());
					return;
				}
				else
				{
					NewFontFace(TempFontBinaryArray);
				}
			}
		}
		else
		{
			UE_LOG(LGUI, Error, TEXT("[%s].%d Font:%s, trying to load Unreal's font face, but not valid!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(this->GetName()));
			return;
		}
#else
		//from UE5.6, runtime use cached data, because UnrealFont's runtime data is not usable for freetype
		NewFontFace(FontBinaryArray);
#endif
	}
	else
	{
#if WITH_EDITOR
		if (true)
		{
			FString FontFilePathStr = FontFilePath;
			FontFilePathStr = bUseRelativeFilePath ? FPaths::ProjectDir() + FontFilePath : FontFilePath;
			if (!FPaths::FileExists(*FontFilePathStr))
			{
				if (FontBinaryArray.Num() > 0 && !bUseExternalFileOrEmbedInToUAsset)
				{
					UE_LOG(LGUI, Warning, TEXT("[%s].%d Font:%s, file: \"%s\" not exist! Will use cache data"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(this->GetName()), *FontFilePathStr);
				}
				else
				{
					UE_LOG(LGUI, Error, TEXT("[%s].%d Font:%s, file: \"%s\" not exist!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(this->GetName()), *FontFilePathStr);
					return;
				}
			}

			if (bUseExternalFileOrEmbedInToUAsset)
			{
				FFileHelper::LoadFileToArray(TempFontBinaryArray, *FontFilePathStr);
				NewFontFace(TempFontBinaryArray);
				if (error == 0)
				{
					FontBinaryArray.Empty();
				}
			}
			else
			{
				FFileHelper::LoadFileToArray(FontBinaryArray, *FontFilePathStr);
				NewFontFace(FontBinaryArray);
			}
		}
		else
#endif	
		{
			if (bUseExternalFileOrEmbedInToUAsset)
			{
				auto FontFilePathStr = bUseRelativeFilePath ? FPaths::ProjectDir() + FontFilePath : FontFilePath;
				if (!FPaths::FileExists(*FontFilePathStr))
				{
					UE_LOG(LGUI, Error, TEXT("[%s].%d Font:%s, file: \"%s\" not exist!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(this->GetName()), *FontFilePathStr);
					return;
				}

				FontBinaryArray.Empty();
				FFileHelper::LoadFileToArray(TempFontBinaryArray, *FontFilePathStr);
				NewFontFace(TempFontBinaryArray);
			}
			else
			{
				NewFontFace(FontBinaryArray);
			}
		}
	}

	if (error)
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d Font:%s, error:%s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(this->GetName()), ANSI_TO_TCHAR(GetErrorMessage(error)));
		Face = nullptr;
		return;
	}
	else
	{
		UE_LOG(LGUI, Log, TEXT("[%s].%d Success, font:%s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(this->GetName()));
		bAlreadyInitialized = true;
		bHasKerning = FT_HAS_KERNING(Face) != 0;

		Texture = nullptr;
		auto RectPackCellSize = ULexUISettings::ConvertAtlasTextureSizeTypeToSize(RectPackCellSizeType);
		BinPack = rbp::MaxRectsBinPack(RectPackCellSize, RectPackCellSize);
		auto TextureSize = ULexUISettings::ConvertAtlasTextureSizeTypeToSize(TextureSizeType);
		BinPack.PrepareRectCellsForText(TextureSize, TextureSize, FreeRectCells, RectPackCellSize, false);
		RenewFontTexture();
		//@todo: use small size for intermediate texture, and share same intermediate texture for different fonts
		IntermediateTexture = CreateIntermediateTexture(RectPackCellSize);
		IntermediateTexture->AddToRoot();
		OneDivideTextureSize = 1.0f / TextureSize;

		ClearCharDataCache();
	}
}

void ULexUIFontData_FreeTypeRender::DeinitFreeType()
{
	bAlreadyInitialized = false;
	if (Library != nullptr)
	{
		auto error = FT_Done_FreeType(Library);
		if (error)
		{
			UE_LOG(LGUI, Error, TEXT("[%s].%d Font:%s, error:%s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(this->GetName()), ANSI_TO_TCHAR(GetErrorMessage(error)));
		}
		else
		{
			UE_LOG(LGUI, Log, TEXT("[%s].%d Success, font:%s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(this->GetName()));
		}
	}
	Face = nullptr;
	Library = nullptr;
	FreeRectCells.Empty();
	BinPack = rbp::MaxRectsBinPack(256, 256);
#if WITH_EDITORONLY_DATA
	SubFaces.Reset();
#endif
	FontFace = 0;
	bHasKerning = false;
	ClearCharDataCache();
}
#endif

#if WITH_FREETYPE
FT_GlyphSlot ULexUIFontData_FreeTypeRender::RenderGlyphOnFreeType(uint32 CharCode, float CharSize, float BoldSize)
{
	InitFreeType();
	if (bAlreadyInitialized == false)
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d Font '%s' is not initialized"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *this->GetPathName());
		return nullptr;
	}

	auto error = FT_Set_Pixel_Sizes(Face, 0, CharSize);
	if (error)
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d Font '%s' FT_Set_Pixel_Sizes error:%s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *this->GetPathName(), ANSI_TO_TCHAR(GetErrorMessage(error)));
		return nullptr;
	}
	FT_GlyphSlot slot = Face->glyph;
	error = FT_Load_Glyph(Face, FT_Get_Char_Index(Face, CharCode), FT_LOAD_DEFAULT);
	if (slot->glyph_index == 0//missing char in this font
		&& slot->metrics.width == 0 && slot->metrics.height == 0//some chars (/r, /n, space) only have width and height, no pixels
		)
	{
		if (FallbackFontArray.Num() > 0)
		{
			UE_LOG(LGUI, Log, TEXT("[%s].%d Font '%s' Can't find glyph (code:%d), will search in fallbacks"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *this->GetPathName(), (int)CharCode);
			for (int i = 0; i < FallbackFontArray.Num(); i++)
			{
				if (FallbackFontArray[i] == nullptr)continue;
				if (auto fallbackSlot = FallbackFontArray[i]->RenderGlyphOnFreeType(CharCode, CharSize, BoldSize))
				{
					return fallbackSlot;
				}
			}
		}
		UE_LOG(LGUI, Error, TEXT("[%s].%d Font '%s' Can't find glyph (code:%d) in fallbacks too"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *this->GetPathName(), (int)CharCode);
		return nullptr;
	}
	if (error)
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d Font '%s' FT_Load_Glyph error:%s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *this->GetPathName(), ANSI_TO_TCHAR(GetErrorMessage(error)));
		return nullptr;
	}
	if (BoldSize > 0)
	{
		error = FT_Outline_Embolden(&slot->outline, static_cast<FT_Pos>(BoldSize * 64.0f));
		if (error)
		{
			UE_LOG(LGUI, Warning, TEXT("[%s].%d Font '%s' FT_Outline_Embolden error:%s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *this->GetPathName(), ANSI_TO_TCHAR(GetErrorMessage(error)));
		}
	}
	error = FT_Render_Glyph(Face->glyph, FT_Render_Mode::FT_RENDER_MODE_NORMAL);
	if (error)
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d Font '%s' FT_Render_Glyph error:%s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *this->GetPathName(), ANSI_TO_TCHAR(GetErrorMessage(error)));
		return nullptr;
	}
	if (BoldSize > 0)
	{
		slot->metrics.horiAdvance += BoldSize * 64.0f;
	}
	return slot;
}
#endif

UTexture2DArray* ULexUIFontData_FreeTypeRender::GetFontTexture()
{
	return Texture;
}

void ULexUIFontData_FreeTypeRender::PostLoad()
{
	Super::PostLoad();
	if (!bCultureFont)
		return;

	//localization
	OnCultureChangedDelegateHandle = FInternationalization::Get().OnCultureChanged().AddUObject(this, &ULexUIFontData_FreeTypeRender::UpdateFontOnCultureChanged);

	FString CurrentCulture = FInternationalization::Get().GetCurrentCulture()->GetName();
	if (CultureFontMap.Contains(CurrentCulture))
		EngineFont = CultureFontMap[CurrentCulture].LoadSynchronous();
}

void ULexUIFontData_FreeTypeRender::BeginDestroy()
{
	if (bCultureFont)
	{
		if (OnCultureChangedDelegateHandle.IsValid())
		{
			FInternationalization::Get().OnCultureChanged().Remove(OnCultureChangedDelegateHandle);
		}

		if (IsValid(IntermediateTexture))
		{
			IntermediateTexture->RemoveFromRoot();
		}
		if (IsValid(Texture))
		{
			Texture->RemoveFromRoot();
		}
	}
	Super::BeginDestroy();
}

void ULexUIFontData_FreeTypeRender::InitFont()
{
#if WITH_FREETYPE
	InitFreeType();
#endif
}

float ULexUIFontData_FreeTypeRender::GetKerning(uint32 LeftCharCode, uint32 RightCharCode, float CharSize)
{
#if WITH_FREETYPE
	if (Face == nullptr)return 0;
	if (!bHasKerning)return 0;
	auto error = FT_Set_Pixel_Sizes(Face, 0, CharSize);
	if (error)
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d FT_Set_Pixel_Sizes error:%s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, ANSI_TO_TCHAR(GetErrorMessage(error)));
		return 0;
	}
	FT_Vector kerning;
	error = FT_Get_Kerning(Face, FT_Get_Char_Index(Face, LeftCharCode), FT_Get_Char_Index(Face, RightCharCode), FT_KERNING_DEFAULT, &kerning);
	if (error)
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d FT_Get_Kerning error:%s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, ANSI_TO_TCHAR(GetErrorMessage(error)));
		return 0;
	}
	return kerning.x * ONE_DIVIDE_64;
#else
	return 0;
#endif
}
float ULexUIFontData_FreeTypeRender::GetLineHeight(float FontSize)
{
#if WITH_FREETYPE
	if (Face == nullptr)return FontSize;
	auto error = FT_Set_Pixel_Sizes(Face, 0, FontSize);
	if (error)
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d FT_Set_Pixel_Sizes error:%s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, ANSI_TO_TCHAR(GetErrorMessage(error)));
		return FontSize;
	}
	return Face->size->metrics.height * ONE_DIVIDE_64;
#else
	return fontSize;
#endif
}
float ULexUIFontData_FreeTypeRender::GetVerticalOffset(float FontSize)
{
#if WITH_FREETYPE
	if (Face == nullptr)return FontSize;
	auto error = FT_Set_Pixel_Sizes(Face, 0, FontSize);
	if (error)
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d FT_Set_Pixel_Sizes error:%s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, ANSI_TO_TCHAR(GetErrorMessage(error)));
		return 0;
	}
	return -((Face->size->metrics.ascender + Face->size->metrics.descender) * ONE_DIVIDE_64) * 0.5f;
#else
	return fontSize;
#endif
}

void ULexUIFontData_FreeTypeRender::AddUIText(ULexText* InText)
{
	RenderTextArray.AddUnique(InText);
}
void ULexUIFontData_FreeTypeRender::RemoveUIText(ULexText* InText)
{
	RenderTextArray.Remove(InText);
}

void ULexUIFontData_FreeTypeRender::SetFontType(ELexUIDynamicFontDataType Value)
{
	FontType = Value;
}

void ULexUIFontData_FreeTypeRender::SetEngineFont(UFontFace* Value)
{
	EngineFont = Value;
}

FLexUICharData ULexUIFontData_FreeTypeRender::GetCharData(uint32 CharCode, float CharSize, bool IsBold)
{
	auto Result = FLexUICharData();
	if (CharSize <= 0.0f)return Result;
	if (!GetCharDataFromCache(CharCode, CharSize, IsBold, Result))//if charData not cached, then create it and add to cache
	{
		FGlyphBitmap glyphBitmap;
		if (!RenderGlyph(CharCode, CharSize, IsBold, glyphBitmap))//no valid glyph
		{
			return Result;//@todo: use an error char to display
		}

		FLexUICharData uiCharData;
	PACK_AND_INSERT:
		if (!PackRectAndInsertChar(glyphBitmap, BinPack, Texture, uiCharData))
		{
			if (FreeRectCells.Num() > 0)//use free cells
			{
				BinPack.DoRectCellsForText(FreeRectCells[FreeRectCells.Num() - 1]);
				FreeRectCells.RemoveAt(FreeRectCells.Num() - 1, 1, EAllowShrinking::No);
			}
			else//no free cells, move to next slice of Texture2DArray
			{
				CurrentTextureSlice++;
				UE_LOG(LGUI, Log, TEXT("[%s].%d Expend Texture2DArray slice to: %d"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, Texture->GetArraySize() + 1);
				//add new slice to Texture2DArray
				auto RectPackCellSize = ULexUISettings::ConvertAtlasTextureSizeTypeToSize(RectPackCellSizeType);
				BinPack = rbp::MaxRectsBinPack(RectPackCellSize, RectPackCellSize);
				auto TextureSize = ULexUISettings::ConvertAtlasTextureSizeTypeToSize(TextureSizeType);
				BinPack.PrepareRectCellsForText(TextureSize, TextureSize, FreeRectCells, RectPackCellSize, false);
				
				FreeRectCells.RemoveAt(FreeRectCells.Num() - 1, 1, EAllowShrinking::No);

				RenewFontTexture();
				OneDivideTextureSize = 1.0f / TextureSize;
			}

			goto PACK_AND_INSERT;
		}

		AddCharDataToCache(CharCode, CharSize, IsBold, uiCharData);
		GetCharDataFromCache(CharCode, CharSize, IsBold, Result);
	}
	return Result;
}

bool ULexUIFontData_FreeTypeRender::PackRectAndInsertChar(const FGlyphBitmap& InGlyphBitmap, rbp::MaxRectsBinPack& InOutBinPack, UTexture2DArray* InTexture, FLexUICharData& OutResult)
{
	if (InGlyphBitmap.width <= 0 || InGlyphBitmap.height <= 0)//glyph no need to display, could be space
	{
		OutResult.Width = InGlyphBitmap.width;
		OutResult.Height = InGlyphBitmap.height;
		OutResult.XOffset = InGlyphBitmap.hOffset;
		OutResult.YOffset = InGlyphBitmap.vOffset;
		OutResult.XAdvance = InGlyphBitmap.hAdvance;
		OutResult.MinUV.X = OutResult.MaxUV.Y = OutResult.MaxUV.X = OutResult.MinUV.Y = 0.0f;//(0,0) point is transparent
		return true;
	}
	const auto SPACE_NEED_EXPEND = this->Get_SPACE_NEED_EXPEND();
	const auto SPACE_NEED_EXPENDx2 = SPACE_NEED_EXPEND + SPACE_NEED_EXPEND;
	const auto SPACE_BETWEEN_GLYPH_RECT = this->Get_SPACE_BETWEEN_GLYPH() + SPACE_NEED_EXPEND;
	const auto SPACE_BETWEEN_GLYPH_RECTx2 = SPACE_BETWEEN_GLYPH_RECT + SPACE_BETWEEN_GLYPH_RECT;

	int charRectWidth = InGlyphBitmap.width + SPACE_BETWEEN_GLYPH_RECTx2;
	int charRectHeight = InGlyphBitmap.height + SPACE_BETWEEN_GLYPH_RECTx2;
	auto method = rbp::MaxRectsBinPack::RectBestAreaFit;

	auto packedRect = InOutBinPack.Insert(charRectWidth, charRectHeight, method);
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

		auto UpdateRegion = FUpdateTextureRegion2D(0, 0, 0, 0, InGlyphBitmap.width, InGlyphBitmap.height);
		UpdateFontTextureRegion(packedRect.x, packedRect.y, CurrentTextureSlice, MoveTemp(UpdateRegion), packedRect.width * InGlyphBitmap.pixelSize, InGlyphBitmap.pixelSize, MoveTemp(const_cast<FGlyphBitmap&>(InGlyphBitmap).buffer));

		OutResult.Width = InGlyphBitmap.width + SPACE_NEED_EXPENDx2;
		OutResult.Height = InGlyphBitmap.height + SPACE_NEED_EXPENDx2;
		OutResult.XOffset = InGlyphBitmap.hOffset - SPACE_NEED_EXPEND;
		OutResult.YOffset = InGlyphBitmap.vOffset + SPACE_NEED_EXPEND;
		OutResult.XAdvance = InGlyphBitmap.hAdvance;
		OutResult.MinUV.X = OneDivideTextureSize * (packedRect.x - SPACE_NEED_EXPEND);
		OutResult.MaxUV.Y = OneDivideTextureSize * (packedRect.y - SPACE_NEED_EXPEND + OutResult.Height);
		OutResult.MaxUV.X = OneDivideTextureSize * (packedRect.x - SPACE_NEED_EXPEND + OutResult.Width);
		OutResult.MinUV.Y = OneDivideTextureSize * (packedRect.y - SPACE_NEED_EXPEND);
		OutResult.SliceIndex = CurrentTextureSlice;
		return true;
	}
}
void ULexUIFontData_FreeTypeRender::ApplyPackingAtlasTextureExpand(UTexture2D* newTexture, int newTextureSize)
{

}

void ULexUIFontData_FreeTypeRender::UpdateFontTextureRegion(uint32 PosX, uint32 PosY, uint32 Slice, FUpdateTextureRegion2D Region, uint32 SrcPitch, uint32 SrcBpp, TArray<uint8> SrcData)
{
	if (!IntermediateTexture->GetResource() || !Texture->GetResource())
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d Texture Resource is null!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		return;
	}
	
	struct FUpdateTextureRegionsData
	{
		FUpdateTextureRegion2D Region;
		uint32 SrcPitch;
		uint32 SrcBpp;
		TArray<uint8> SrcData;
		uint32 Slice;
		uint32 PosX;
		uint32 PosY;
	};
	FUpdateTextureRegionsData RegionData;
	RegionData.Region = MoveTemp(Region);
	RegionData.SrcPitch = SrcPitch;
	RegionData.SrcBpp = SrcBpp;
	RegionData.SrcData = MoveTemp(SrcData);
	RegionData.Slice = Slice;
	RegionData.PosX = PosX;
	RegionData.PosY = PosY;
	auto Texture2DArrayRes = (FTexture2DArrayResource*)Texture->GetResource();
	auto IntermediateTexture2DRes = (FTexture2DResource*)IntermediateTexture->GetResource();
	ENQUEUE_RENDER_COMMAND(FLexUIFontData_UpdateFontTextureRegionData)(
		[RegionData = MoveTemp(RegionData), IntermediateTexture2DRes, Texture2DArrayRes](FRHICommandListImmediate& RHICmdList)
		{
			auto IntermediateTexRHI = IntermediateTexture2DRes->GetTexture2DRHI();
			auto TexRHI = Texture2DArrayRes->GetTexture2DArrayRHI();
			check(IntermediateTexRHI && IntermediateTexRHI->IsValid());
			check(TexRHI && TexRHI->IsValid());
			RHICmdList.UpdateTexture2D(
				IntermediateTexRHI,
				0,
				RegionData.Region,
				RegionData.SrcPitch,
				RegionData.SrcData.GetData()
				+ RegionData.Region.SrcY * RegionData.SrcPitch
				+ RegionData.Region.SrcX * RegionData.SrcBpp
			);

			FRHICopyTextureInfo CopyInfo;
			CopyInfo.SourceMipIndex = 0;
			CopyInfo.NumMips = 1;
			CopyInfo.SourceSliceIndex = 0;
			CopyInfo.NumSlices = 1;
			CopyInfo.DestSliceIndex = RegionData.Slice;
			CopyInfo.SourcePosition = FIntVector(0, 0, 0);
			CopyInfo.DestPosition = FIntVector(RegionData.PosX, RegionData.PosY, 0);
			CopyInfo.Size = FIntVector(RegionData.Region.Width, RegionData.Region.Height, 0);
			RHICmdList.CopyTexture(IntermediateTexRHI, TexRHI, CopyInfo);
		});
}
void ULexUIFontData_FreeTypeRender::RenewFontTexture()
{
	//get old texture pointer
	auto OldTexture = Texture; 
	//create new texture
	auto TextureSize = ULexUISettings::ConvertAtlasTextureSizeTypeToSize(TextureSizeType);
	Texture = CreateFontTexture(TextureSize, OldTexture ? OldTexture->GetArraySize() + 1 : 1);
	Texture->AddToRoot();

	//copy old texture to new one
	if (OldTexture)
	{
		auto OldTextureResource = OldTexture->GetResource();
		auto NewTextureResource = Texture->GetResource();
		check(OldTextureResource);
		check(NewTextureResource);
		ENQUEUE_RENDER_COMMAND(FLexUIFontData_UpdateAndCopyFontTexture)([
				OldTextureResource
				, NewTextureResource
				, TextureSize = TextureSize
				, SliceCount = OldTexture->GetArraySize()
				](FRHICommandListImmediate& RHICmdList)
		{
			auto OldTextureRHI = OldTextureResource->GetTexture2DArrayRHI();
			auto NewTextureRHI = NewTextureResource->GetTexture2DArrayRHI();
			check(OldTextureRHI && OldTextureRHI->IsValid());
			check(NewTextureRHI && NewTextureRHI->IsValid());
			FRHICopyTextureInfo CopyInfo;
			CopyInfo.SourcePosition = FIntVector(0, 0, 0);
			CopyInfo.Size = FIntVector(TextureSize, TextureSize, 0);
			CopyInfo.DestPosition = FIntVector(0, 0, 0);
			CopyInfo.SourceSliceIndex = 0;
			CopyInfo.DestSliceIndex = 0;
			CopyInfo.NumMips = 1;
			CopyInfo.NumSlices = SliceCount;
			RHICmdList.CopyTexture(
				OldTextureRHI
				, NewTextureRHI
				, CopyInfo
				);
		});
		OldTexture->RemoveFromRoot();//we should not worry about gc because render thread only need texture resource, not texture object
	}

	for (auto textItem : RenderTextArray)
	{
		if (textItem.IsValid())
		{
			textItem->ApplyFontTextureChange();
		}
	}
}

#if WITH_EDITOR
void ULexUIFontData_FreeTypeRender::ReloadFont()
{
#if WITH_FREETYPE
	DeinitFreeType();
	InitFreeType();
#endif
}
void ULexUIFontData_FreeTypeRender::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.Property)
	{
		auto PropertyName = Property->GetFName();
		if (RectPackCellSizeType > TextureSizeType)
		{
			RectPackCellSizeType = TextureSizeType;
		}
		if (PropertyName == GET_MEMBER_NAME_CHECKED(ULexUIFontData_FreeTypeRender, bUseExternalFileOrEmbedInToUAsset)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(ULexUIFontData_FreeTypeRender, FontFace)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(ULexUIFontData_FreeTypeRender, FontType)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(ULexUIFontData_FreeTypeRender, EngineFont)
			)
		{
			if (PropertyName == GET_MEMBER_NAME_CHECKED(ULexUIFontData_FreeTypeRender, FontType))
			{
				if (FontType == ELexUIDynamicFontDataType::EngineFont)
				{
					FontBinaryArray.Empty();//clear cache font data when swich to UnrealFont
				}
			}
			ReloadFont();
		}
	}
}

void ULexUIFontData_FreeTypeRender::BeginCacheForCookedPlatformData(const ITargetPlatform* TargetPlatform)
{
	if (FontType == ELexUIDynamicFontDataType::EngineFont)
	{
		FontBinaryArray = EngineFont->GetFontFaceData()->GetData();
	}
}

void ULexUIFontData_FreeTypeRender::ClearCachedCookedPlatformData(const ITargetPlatform* TargetPlatform)
{
	if (FontType == ELexUIDynamicFontDataType::EngineFont)
	{
		FontBinaryArray.Empty();
	}
}
#endif

