// Copyright 2019 LexLiu. All Rights Reserved.

#include "Core/LGUIFontData.h"
#include "LGUI.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"
#include "Core/ActorComponent/UIText.h"
#include FT_OUTLINE_H

ULGUIFontData::ULGUIFontData()
{
	
}
ULGUIFontData::~ULGUIFontData()
{
	
}
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

void ULGUIFontData::InitFreeType()
{
	if (alreadyInitialized)return;
	FT_Error error = 0;
	FString FontFilePathStr = fontFilePath;
#if WITH_EDITOR
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
	error = FT_Init_FreeType(&library);
	if (error)
	{
		UE_LOG(LGUI, Error, TEXT("[InitFreeType]font:%s, error:%s"), *(this->GetName()), ANSI_TO_TCHAR(GetErrorMessage(error)));
		return;
	}
	if (useExternalFileOrEmbedInToUAsset)
	{
		tempFontBinaryArray.Empty();
		FFileHelper::LoadFileToArray(tempFontBinaryArray, *FontFilePathStr);
		error = FT_New_Memory_Face(library, tempFontBinaryArray.GetData(), tempFontBinaryArray.Num(), 0, &face);
		if (error == 0)
		{
			fontBinaryArray.Empty();
		}
	}
	else
	{
		FFileHelper::LoadFileToArray(fontBinaryArray, *FontFilePathStr);
		error = FT_New_Memory_Face(library, fontBinaryArray.GetData(), fontBinaryArray.Num(), 0, &face);
	}
#else
	if (useExternalFileOrEmbedInToUAsset)
	{
		FontFilePathStr = useRelativeFilePath ? FPaths::ProjectDir() + fontFilePath : fontFilePath;
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		if (!PlatformFile.FileExists(*FontFilePathStr))
		{
			UE_LOG(LGUI, Error, TEXT("[InitFreeType]font:%s, file: \"%s\" not exist!"), *(this->GetName()), *FontFilePathStr);
			return;
		}
		error = FT_Init_FreeType(&library);
		if (error)
		{
			UE_LOG(LGUI, Error, TEXT("[InitFreeType]font:%s, error:%s"), *(this->GetName()), ANSI_TO_TCHAR(GetErrorMessage(error)));
			return;
		}

		tempFontBinaryArray.Empty();
		fontBinaryArray.Empty();
		FFileHelper::LoadFileToArray(tempFontBinaryArray, *FontFilePathStr);
		error = FT_New_Memory_Face(library, tempFontBinaryArray.GetData(), tempFontBinaryArray.Num(), 0, &face);
	}
	else
	{
		error = FT_Init_FreeType(&library);
		if (error)
		{
			UE_LOG(LGUI, Error, TEXT("[InitFreeType]font:%s, error:%s"), *(this->GetName()), ANSI_TO_TCHAR(GetErrorMessage(error)));
			return;
		}

		error = FT_New_Memory_Face(library, fontBinaryArray.GetData(), fontBinaryArray.Num(), 0, &face);
	}
#endif	
	
	if (error)
	{
		UE_LOG(LGUI, Error, TEXT("[InitFreeType]font:%s, error:%s"), *(this->GetName()), ANSI_TO_TCHAR(GetErrorMessage(error)));
		face = nullptr;
		return;
	}
	else
	{
		UE_LOG(LGUI, Log, TEXT("[InitFreeType]succeed, font:%s"), *(this->GetName()));
		alreadyInitialized = true;
		texture = nullptr;
		textureSize = 256;//default font texture size is 256
		fullTextureSizeReciprocal = 1.0f / textureSize;
		CreateFontTexture(0, textureSize);
		charDataMap.Empty();
		binPack = rbp::MaxRectsBinPack(textureSize, textureSize, 0, 0);
	}
}
void ULGUIFontData::DeinitFreeType()
{
	alreadyInitialized = false;
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
			UE_LOG(LGUI, Log, TEXT("[DeintFreeType]succeed, font:%s"), *(this->GetName()));
		}
	}
}
void ULGUIFontData::AddUIText(UUIText* InText)
{
	renderTextArray.AddUnique(InText);
}
void ULGUIFontData::RemoveUIText(UUIText* InText)
{
	renderTextArray.Remove(InText);
}
FT_Matrix ULGUIFontData::GetItalicMatrix()
{
	FT_Matrix italicMatrix;
	italicMatrix.xx = 1 << 16;
	italicMatrix.xy = 0x4000;
	italicMatrix.yx = 0;
	italicMatrix.yy = 1 << 16;
	return italicMatrix;
}
FLGUICharData* ULGUIFontData::PushCharIntoFont(const TCHAR& charIndex, const uint16& charSize, const bool& bold, const bool& italic)
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

	cacheFontKey = FLGUIFontKeyData(charIndex, charSize, bold, italic);
	auto error = FT_Set_Pixel_Sizes(face, 0, charSize);
	if (error)
	{
		UE_LOG(LGUI, Error, TEXT("FT_Set_Pixel_Sizes error:%s"), ANSI_TO_TCHAR(GetErrorMessage(error)));
	}
	FT_GlyphSlot slot = face->glyph;
	error = FT_Load_Glyph(face, FT_Get_Char_Index(face, charIndex), FT_LOAD_DEFAULT);
	if (error)
	{
		UE_LOG(LGUI, Error, TEXT("FT_Load_Glyph error:%s"), ANSI_TO_TCHAR(GetErrorMessage(error)));
		return &cacheCharData;
	}
	if (bold)
	{
		error = FT_Outline_Embolden(&face->glyph->outline, charSize * 2);
		if (error)
		{
			UE_LOG(LGUI, Error, TEXT("FT_Outline_Embolden error:%s"), ANSI_TO_TCHAR(GetErrorMessage(error)));
		}
	}
	if (italic)
	{
		static FT_Matrix italicMatrix = GetItalicMatrix();
		FT_Outline_Transform(&face->glyph->outline, &italicMatrix);
	}
	error = FT_Render_Glyph(face->glyph, FT_Render_Mode::FT_RENDER_MODE_NORMAL);
	if (error)
	{
		UE_LOG(LGUI, Error, TEXT("FT_Render_Glyph error:%s"), ANSI_TO_TCHAR(GetErrorMessage(error)));
		return &cacheCharData;
	}
	FT_Bitmap bitmap = slot->bitmap;

	bool firstTimeExpend = true;
	if (!IsValid(texture))
	{
		CreateFontTexture(0, textureSize);
	}
PACK_AND_INSERT:
	if (PackRectAndInsertChar(bold ? charSize * 0.05f : 0, bitmap, slot))
	{
		
	}
	else//if all rect area cannot fit in this char, then expend texture
	{
		checkf(firstTimeExpend, TEXT("[PushCharIntoFont]Expend texture size second time on one character! Is the font size too large?"));//can only expend once for every char
		int32 newTextureSize = textureSize + textureSize;
		UE_LOG(LGUI, Log, TEXT("[PushCharIntoFont]Expend font texture size to:%d"), newTextureSize);
		//expend by multiply 2
		binPack.ExpendSize(newTextureSize, newTextureSize);

		CreateFontTexture(textureSize, newTextureSize);
		textureSize = newTextureSize;
		fullTextureSizeReciprocal = 1.0f / textureSize;
		firstTimeExpend = false;

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

		goto PACK_AND_INSERT;
	}

	charDataMap.Add(cacheFontKey, cacheCharData);

	//UE_LOG(LGUI, Error, TEXT("InsertCharTakeTime:%f"), GWorld->GetRealTimeSeconds() - prevTime);
	return &cacheCharData;
}
bool ULGUIFontData::PackRectAndInsertChar(const int32 InBoldOffset, const FT_Bitmap& InCharBitmap, const FT_GlyphSlot& InSlot)
{
	int charRectWidth = InCharBitmap.width + SPACE_BETWEEN_GLYPHx2;
	int charRectHeight = InCharBitmap.rows + SPACE_BETWEEN_GLYPHx2;
	auto method = rbp::MaxRectsBinPack::RectBestAreaFit;

	auto packedRect = binPack.Insert(charRectWidth, charRectHeight, method);
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
		int32 sourceTexturePixelIndexY = packedRect.y;
		int32 sourceTextureCharStartPixelXWithSpace = packedRect.x + SPACE_BETWEEN_GLYPH;
		auto region = new FUpdateTextureRegion2D(sourceTextureCharStartPixelXWithSpace, sourceTexturePixelIndexY, 0, 0, packedRect.width, packedRect.height);

		int pixelCount = packedRect.width * packedRect.height;
		FColor* regionColor = new FColor[pixelCount];
		for (int i = 0; i < pixelCount; i++)
		{
			auto& pixelColor = regionColor[i];
			pixelColor.R = pixelColor.G = pixelColor.B = 255;
			pixelColor.A = InCharBitmap.buffer[i];
		}
		UpdateFontTextureRegion(texture, region, packedRect.width * 4, 4, (uint8*)regionColor);

		cacheCharData.width = InCharBitmap.width + SPACE_NEED_EXPENDx2;
		cacheCharData.height = InCharBitmap.rows + SPACE_NEED_EXPENDx2;
		cacheCharData.xoffset = InSlot->bitmap_left - SPACE_NEED_EXPEND;
		cacheCharData.yoffset = InSlot->bitmap_top - SPACE_NEED_EXPEND - InBoldOffset * 0.5f;
		cacheCharData.xadvance = (uint16)(InSlot->linearHoriAdvance * MAX_UINT16_RECEPROCAL + InBoldOffset);
		cacheCharData.uv0X = fullTextureSizeReciprocal * (sourceTextureCharStartPixelXWithSpace - SPACE_NEED_EXPEND);
		cacheCharData.uv0Y = fullTextureSizeReciprocal * (packedRect.y + cacheCharData.height - SPACE_NEED_EXPEND);
		cacheCharData.uv3X = fullTextureSizeReciprocal * (sourceTextureCharStartPixelXWithSpace - SPACE_NEED_EXPEND + cacheCharData.width);
		cacheCharData.uv3Y = fullTextureSizeReciprocal * (packedRect.y - SPACE_NEED_EXPEND);

		return true;
	}
	return false;
}

void ULGUIFontData::UpdateTextureRegions(UTexture2D* Texture, int32 MipIndex, uint32 NumRegions, FUpdateTextureRegion2D* Regions, uint32 SrcPitch, uint32 SrcBpp, uint8* SrcData, bool bFreeData)
{
	if (Texture->Resource)
	{
		struct FUpdateTextureRegionsData
		{
			FTexture2DResource* Texture2DResource;
			int32 MipIndex;
			uint32 NumRegions;
			FUpdateTextureRegion2D* Regions;
			uint32 SrcPitch;
			uint32 SrcBpp;
			uint8* SrcData;
		};

		FUpdateTextureRegionsData* RegionData = new FUpdateTextureRegionsData;

		RegionData->Texture2DResource = (FTexture2DResource*)Texture->Resource;
		RegionData->MipIndex = MipIndex;
		RegionData->NumRegions = NumRegions;
		RegionData->Regions = Regions;
		RegionData->SrcPitch = SrcPitch;
		RegionData->SrcBpp = SrcBpp;
		RegionData->SrcData = SrcData;
		ENQUEUE_RENDER_COMMAND(FLGUIFontUpdateFontTextureRegionsData)(
			[RegionData, bFreeData](FRHICommandListImmediate& RHICmdList)
			{
				for (uint32 RegionIndex = 0; RegionIndex < RegionData->NumRegions; ++RegionIndex)
				{
					int32 CurrentFirstMip = RegionData->Texture2DResource->GetCurrentFirstMip();
					if (RegionData->MipIndex >= CurrentFirstMip)
					{
						RHIUpdateTexture2D(
							RegionData->Texture2DResource->GetTexture2DRHI(),
							RegionData->MipIndex - CurrentFirstMip,
							RegionData->Regions[RegionIndex],
							RegionData->SrcPitch,
							RegionData->SrcData
							+ RegionData->Regions[RegionIndex].SrcY * RegionData->SrcPitch
							+ RegionData->Regions[RegionIndex].SrcX * RegionData->SrcBpp
						);
					}
				}
				if (bFreeData)
				{
					FMemory::Free(RegionData->SrcData);
					FMemory::Free(RegionData->Regions);
				}
				delete RegionData;
			});
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

		RegionData->Texture2DResource = (FTexture2DResource*)Texture->Resource;
		RegionData->Region = Region;
		RegionData->SrcPitch = SrcPitch;
		RegionData->SrcBpp = SrcBpp;
		RegionData->SrcData = SrcData;
		ENQUEUE_RENDER_COMMAND(FLGUIFontUpdateFontTextureRegionData)(
			[RegionData](FRHICommandListImmediate& RHICmdList)
			{
				RHIUpdateTexture2D(
					RegionData->Texture2DResource->GetTexture2DRHI(),
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
	texture = UTexture2D::CreateTransient(newTextureSize, newTextureSize, PF_B8G8R8A8);
	texture->CompressionSettings = TextureCompressionSettings::TC_EditorIcon;
	texture->LODGroup = TextureGroup::TEXTUREGROUP_UI;
	texture->UpdateResource();
	FColor defaultColor = FColor(255, 255, 255, 0);
	int pixelCount = newTextureSize * newTextureSize;
	FColor* transparentColor = new FColor[pixelCount];
	for (int i = 0; i < pixelCount; i++)
	{
		transparentColor[i] = defaultColor;
	}
	if (oldTexture != nullptr)
	{
		oldTexture->AddToRoot();
	}
	//fill and copy font texture
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
		RegionData->Texture2DResource = (FTexture2DResource*)texture->Resource;
		RegionData->Region = new FUpdateTextureRegion2D(0, 0, 0, 0, newTextureSize, newTextureSize);
		RegionData->SrcPitch = newTextureSize * 4;
		RegionData->SrcBpp = 4;
		RegionData->SrcData = (uint8*)transparentColor;
		auto newTexture = texture;
		ENQUEUE_RENDER_COMMAND(FLGUIFontUpdateAndCopyFontTexture)(
			[RegionData, oldTexture, newTexture, oldTextureSize](FRHICommandListImmediate& RHICmdList)
			{
				//fill with transparent pixels
				RHIUpdateTexture2D(
					RegionData->Texture2DResource->GetTexture2DRHI(),
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

				//copy old texture pixels
				if (oldTextureSize != 0 && oldTexture != nullptr)
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
				}
			});
	}
}

FLGUICharData* ULGUIFontData::GetCharData(const TCHAR& charIndex, const uint16& charSize, const bool& bold, const bool& italic)
{
	cacheFontKey = FLGUIFontKeyData(charIndex, charSize, bold, italic);
	if (auto charData = charDataMap.Find(cacheFontKey))
	{
		return charData;
	}
	else
	{
		return PushCharIntoFont(charIndex, charSize, bold, italic);
	}
	return nullptr;
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
		auto PropertyName = Property->GetName();
		if (PropertyName == TEXT("useExternalFileOrEmbedInToUAsset"))
		{
			ReloadFont();
		}
		if (PropertyName == TEXT("fixedVerticalOffset"))
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