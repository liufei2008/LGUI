// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/LGUIFontData.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIText.h"
#include "TextureResource.h"
#include "Engine/Texture2D.h"
#if WITH_FREETYPE
#include <ft2build.h>
#include FT_FREETYPE_H
#endif


void ULGUIFontData::PushCharData(
	TCHAR charCode, const FVector2f& inLineOffset, const FVector2f& fontSpace, const FLGUICharData_HighPrecision& charData,
	const LGUIRichTextParser::RichTextParseResult& richTextProperty,
	int verticesStartIndex, int indicesStartIndex,
	int& outAdditionalVerticesCount, int& outAdditionalIndicesCount,
	TArray<FLGUIOriginVertexData>& originVertices, TArray<FLGUIMeshVertex>& vertices, TArray<FLGUIMeshIndexBufferType>& triangleIndices
)
{
	auto GetUnderlineOrStrikethroughCharGeo = [&](TCHAR charCode, float overrideFontSize)
	{
		auto charData = this->GetCharData(charCode, overrideFontSize);
		charData.yoffset += this->GetVerticalOffset(overrideFontSize);

		float uvX = (charData.uv3X - charData.uv0X) * 0.5f + charData.uv0X;
		charData.uv0X = charData.uv3X = uvX;
		return charData;
	};

	outAdditionalVerticesCount = 4;
	outAdditionalIndicesCount = 6;

	FLGUICharData_HighPrecision underlineCharGeo;
	FLGUICharData_HighPrecision strikethroughCharGeo;
	//underline and strikethrough should not exist at same char
	if (richTextProperty.underline)
	{
		outAdditionalVerticesCount += 4;
		outAdditionalIndicesCount += 6;
		underlineCharGeo = GetUnderlineOrStrikethroughCharGeo('_', richTextProperty.size);
	}
	if (richTextProperty.strikethrough)
	{
		outAdditionalVerticesCount += 4;
		outAdditionalIndicesCount += 6;
		strikethroughCharGeo = GetUnderlineOrStrikethroughCharGeo('-', richTextProperty.size);
	}
	if (richTextProperty.bold)
	{
		outAdditionalVerticesCount += 12;
		outAdditionalIndicesCount += 18;
	}
	int32 newVerticesCount = verticesStartIndex + outAdditionalVerticesCount;
	UIGeometry::LGUIGeometrySetArrayNum(originVertices, newVerticesCount);
	UIGeometry::LGUIGeometrySetArrayNum(vertices, newVerticesCount);

	int32 newIndicesCount = indicesStartIndex + outAdditionalIndicesCount;
	UIGeometry::LGUIGeometrySetArrayNum(triangleIndices, newIndicesCount);

	auto lineOffset = inLineOffset;
	if (richTextProperty.supOrSubMode == LGUIRichTextParser::SupOrSubMode::Sup)
	{
		lineOffset.Y += richTextProperty.size * 0.5f;
	}
	else if (richTextProperty.supOrSubMode == LGUIRichTextParser::SupOrSubMode::Sub)
	{
		lineOffset.Y -= richTextProperty.size * 0.5f;
	}
	float offsetX = lineOffset.X + charData.xoffset;
	float offsetY = lineOffset.Y + charData.yoffset;

	float charWidth = charData.xadvance + fontSpace.X;
	//position
	{
		float x, y;

		int addVertCount = 0;
		if (richTextProperty.bold)
		{
			x = offsetX;
			y = offsetY - charData.height;
			auto vert0 = FVector3f(0, x, y);
			x = charData.width + offsetX;
			auto vert1 = FVector3f(0, x, y);
			x = offsetX;
			y = offsetY;
			auto vert2 = FVector3f(0, x, y);
			x = charData.width + offsetX;
			auto vert3 = FVector3f(0, x, y);
			if (richTextProperty.italic)
			{
				auto vert01ItalicOffset = (charData.height - charData.yoffset) * italicSlop;
				vert0.Y -= vert01ItalicOffset;
				vert1.Y -= vert01ItalicOffset;
				auto vert23ItalicOffset = charData.yoffset * italicSlop;
				vert2.Y += vert23ItalicOffset;
				vert3.Y += vert23ItalicOffset;
			}
			//bold left
			originVertices[verticesStartIndex].Position = vert0 + FVector3f(0, -boldSize, 0);
			originVertices[verticesStartIndex + 1].Position = vert1 + FVector3f(0, -boldSize, 0);
			originVertices[verticesStartIndex + 2].Position = vert2 + FVector3f(0, -boldSize, 0);
			originVertices[verticesStartIndex + 3].Position = vert3 + FVector3f(0, -boldSize, 0);
			//bold right
			originVertices[verticesStartIndex + 4].Position = vert0 + FVector3f(0, boldSize, 0);
			originVertices[verticesStartIndex + 5].Position = vert1 + FVector3f(0, boldSize, 0);
			originVertices[verticesStartIndex + 6].Position = vert2 + FVector3f(0, boldSize, 0);
			originVertices[verticesStartIndex + 7].Position = vert3 + FVector3f(0, boldSize, 0);
			//bold top
			originVertices[verticesStartIndex + 8].Position = vert0 + FVector3f(0, 0, boldSize);
			originVertices[verticesStartIndex + 9].Position = vert1 + FVector3f(0, 0, boldSize);
			originVertices[verticesStartIndex + 10].Position = vert2 + FVector3f(0, 0, boldSize);
			originVertices[verticesStartIndex + 11].Position = vert3 + FVector3f(0, 0, boldSize);
			//bold bottom
			originVertices[verticesStartIndex + 12].Position = vert0 + FVector3f(0, 0, -boldSize);
			originVertices[verticesStartIndex + 13].Position = vert1 + FVector3f(0, 0, -boldSize);
			originVertices[verticesStartIndex + 14].Position = vert2 + FVector3f(0, 0, -boldSize);
			originVertices[verticesStartIndex + 15].Position = vert3 + FVector3f(0, 0, -boldSize);

			addVertCount = 16;
		}
		else
		{
			x = offsetX;
			y = offsetY - charData.height;
			auto& vert0 = originVertices[verticesStartIndex].Position;
			vert0 = FVector3f(0, x, y);
			x = charData.width + offsetX;
			auto& vert1 = originVertices[verticesStartIndex + 1].Position;
			vert1 = FVector3f(0, x, y);
			x = offsetX;
			y = offsetY;
			auto& vert2 = originVertices[verticesStartIndex + 2].Position;
			vert2 = FVector3f(0, x, y);
			x = charData.width + offsetX;
			auto& vert3 = originVertices[verticesStartIndex + 3].Position;
			vert3 = FVector3f(0, x, y);
			if (richTextProperty.italic)
			{
				auto vert01ItalicOffset = (charData.height - charData.yoffset) * italicSlop;
				vert0.Y -= vert01ItalicOffset;
				vert1.Y -= vert01ItalicOffset;
				auto vert23ItalicOffset = charData.yoffset * italicSlop;
				vert2.Y += vert23ItalicOffset;
				vert3.Y += vert23ItalicOffset;
			}

			addVertCount = 4;
		}

		if (richTextProperty.underline)
		{
			offsetX = lineOffset.X;
			offsetY = lineOffset.Y + underlineCharGeo.yoffset;
			x = offsetX;
			y = offsetY - underlineCharGeo.height;
			originVertices[verticesStartIndex + addVertCount].Position = FVector3f(0, x, y);
			x = charWidth + offsetX;
			originVertices[verticesStartIndex + addVertCount + 1].Position = FVector3f(0, x, y);
			x = offsetX;
			y = offsetY;
			originVertices[verticesStartIndex + addVertCount + 2].Position = FVector3f(0, x, y);
			x = charWidth + offsetX;
			originVertices[verticesStartIndex + addVertCount + 3].Position = FVector3f(0, x, y);

			addVertCount += 4;
		}
		if (richTextProperty.strikethrough)
		{
			offsetX = lineOffset.X;
			offsetY = lineOffset.Y + strikethroughCharGeo.yoffset;
			x = offsetX;
			y = offsetY - strikethroughCharGeo.height;
			originVertices[verticesStartIndex + addVertCount].Position = FVector3f(0, x, y);
			x = charWidth + offsetX;
			originVertices[verticesStartIndex + addVertCount + 1].Position = FVector3f(0, x, y);
			x = offsetX;
			y = offsetY;
			originVertices[verticesStartIndex + addVertCount + 2].Position = FVector3f(0, x, y);
			x = charWidth + offsetX;
			originVertices[verticesStartIndex + addVertCount + 3].Position = FVector3f(0, x, y);

			addVertCount += 4;
		}
	}
	//uv
	{
		int addVertCount = 0;
		if (richTextProperty.bold)
		{
			//bold left
			vertices[verticesStartIndex].TextureCoordinate[0] = charData.GetUV0();
			vertices[verticesStartIndex + 1].TextureCoordinate[0] = charData.GetUV1();
			vertices[verticesStartIndex + 2].TextureCoordinate[0] = charData.GetUV2();
			vertices[verticesStartIndex + 3].TextureCoordinate[0] = charData.GetUV3();
			//bold right
			vertices[verticesStartIndex + 4].TextureCoordinate[0] = charData.GetUV0();
			vertices[verticesStartIndex + 5].TextureCoordinate[0] = charData.GetUV1();
			vertices[verticesStartIndex + 6].TextureCoordinate[0] = charData.GetUV2();
			vertices[verticesStartIndex + 7].TextureCoordinate[0] = charData.GetUV3();
			//bold top
			vertices[verticesStartIndex + 8].TextureCoordinate[0] = charData.GetUV0();
			vertices[verticesStartIndex + 9].TextureCoordinate[0] = charData.GetUV1();
			vertices[verticesStartIndex + 10].TextureCoordinate[0] = charData.GetUV2();
			vertices[verticesStartIndex + 11].TextureCoordinate[0] = charData.GetUV3();
			//bold bottom
			vertices[verticesStartIndex + 12].TextureCoordinate[0] = charData.GetUV0();
			vertices[verticesStartIndex + 13].TextureCoordinate[0] = charData.GetUV1();
			vertices[verticesStartIndex + 14].TextureCoordinate[0] = charData.GetUV2();
			vertices[verticesStartIndex + 15].TextureCoordinate[0] = charData.GetUV3();

			addVertCount = 16;
		}
		else
		{
			vertices[verticesStartIndex].TextureCoordinate[0] = charData.GetUV0();
			vertices[verticesStartIndex + 1].TextureCoordinate[0] = charData.GetUV1();
			vertices[verticesStartIndex + 2].TextureCoordinate[0] = charData.GetUV2();
			vertices[verticesStartIndex + 3].TextureCoordinate[0] = charData.GetUV3();

			addVertCount = 4;
		}
		if (richTextProperty.underline)
		{
			vertices[verticesStartIndex + addVertCount].TextureCoordinate[0] = underlineCharGeo.GetUV0();
			vertices[verticesStartIndex + addVertCount + 1].TextureCoordinate[0] = underlineCharGeo.GetUV1();
			vertices[verticesStartIndex + addVertCount + 2].TextureCoordinate[0] = underlineCharGeo.GetUV2();
			vertices[verticesStartIndex + addVertCount + 3].TextureCoordinate[0] = underlineCharGeo.GetUV3();

			addVertCount += 4;
		}
		if (richTextProperty.strikethrough)
		{
			vertices[verticesStartIndex + addVertCount].TextureCoordinate[0] = strikethroughCharGeo.GetUV0();
			vertices[verticesStartIndex + addVertCount + 1].TextureCoordinate[0] = strikethroughCharGeo.GetUV1();
			vertices[verticesStartIndex + addVertCount + 2].TextureCoordinate[0] = strikethroughCharGeo.GetUV2();
			vertices[verticesStartIndex + addVertCount + 3].TextureCoordinate[0] = strikethroughCharGeo.GetUV3();

			addVertCount += 4;
		}
	}
	//color
	{
		int addVertCount = 0;
		if (richTextProperty.bold)
		{
			//bold left
			vertices[verticesStartIndex].Color = richTextProperty.color;
			vertices[verticesStartIndex + 1].Color = richTextProperty.color;
			vertices[verticesStartIndex + 2].Color = richTextProperty.color;
			vertices[verticesStartIndex + 3].Color = richTextProperty.color;
			//bold right
			vertices[verticesStartIndex + 4].Color = richTextProperty.color;
			vertices[verticesStartIndex + 5].Color = richTextProperty.color;
			vertices[verticesStartIndex + 6].Color = richTextProperty.color;
			vertices[verticesStartIndex + 7].Color = richTextProperty.color;
			//bold top
			vertices[verticesStartIndex + 8].Color = richTextProperty.color;
			vertices[verticesStartIndex + 9].Color = richTextProperty.color;
			vertices[verticesStartIndex + 10].Color = richTextProperty.color;
			vertices[verticesStartIndex + 11].Color = richTextProperty.color;
			//bold bottom
			vertices[verticesStartIndex + 12].Color = richTextProperty.color;
			vertices[verticesStartIndex + 13].Color = richTextProperty.color;
			vertices[verticesStartIndex + 14].Color = richTextProperty.color;
			vertices[verticesStartIndex + 15].Color = richTextProperty.color;

			addVertCount = 16;
		}
		else
		{
			vertices[verticesStartIndex].Color = richTextProperty.color;
			vertices[verticesStartIndex + 1].Color = richTextProperty.color;
			vertices[verticesStartIndex + 2].Color = richTextProperty.color;
			vertices[verticesStartIndex + 3].Color = richTextProperty.color;

			addVertCount = 4;
		}
		if (richTextProperty.underline)
		{
			vertices[verticesStartIndex + addVertCount].Color = richTextProperty.color;
			vertices[verticesStartIndex + addVertCount + 1].Color = richTextProperty.color;
			vertices[verticesStartIndex + addVertCount + 2].Color = richTextProperty.color;
			vertices[verticesStartIndex + addVertCount + 3].Color = richTextProperty.color;

			addVertCount += 4;
		}
		if (richTextProperty.strikethrough)
		{
			vertices[verticesStartIndex + addVertCount].Color = richTextProperty.color;
			vertices[verticesStartIndex + addVertCount + 1].Color = richTextProperty.color;
			vertices[verticesStartIndex + addVertCount + 2].Color = richTextProperty.color;
			vertices[verticesStartIndex + addVertCount + 3].Color = richTextProperty.color;

			addVertCount += 4;
		}
	}
	//triangle
	{
		int addVertCount = 0;
		int addIndCount = 0;
		if (richTextProperty.bold)
		{
			//bold left
			triangleIndices[indicesStartIndex] = verticesStartIndex;
			triangleIndices[indicesStartIndex + 1] = verticesStartIndex + 3;
			triangleIndices[indicesStartIndex + 2] = verticesStartIndex + 2;
			triangleIndices[indicesStartIndex + 3] = verticesStartIndex;
			triangleIndices[indicesStartIndex + 4] = verticesStartIndex + 1;
			triangleIndices[indicesStartIndex + 5] = verticesStartIndex + 3;
			//bold right
			triangleIndices[indicesStartIndex + 6] = verticesStartIndex + 4;
			triangleIndices[indicesStartIndex + 7] = verticesStartIndex + 7;
			triangleIndices[indicesStartIndex + 8] = verticesStartIndex + 6;
			triangleIndices[indicesStartIndex + 9] = verticesStartIndex + 4;
			triangleIndices[indicesStartIndex + 10] = verticesStartIndex + 5;
			triangleIndices[indicesStartIndex + 11] = verticesStartIndex + 7;
			//bold top
			triangleIndices[indicesStartIndex + 12] = verticesStartIndex + 8;
			triangleIndices[indicesStartIndex + 13] = verticesStartIndex + 11;
			triangleIndices[indicesStartIndex + 14] = verticesStartIndex + 10;
			triangleIndices[indicesStartIndex + 15] = verticesStartIndex + 8;
			triangleIndices[indicesStartIndex + 16] = verticesStartIndex + 9;
			triangleIndices[indicesStartIndex + 17] = verticesStartIndex + 11;
			//bold bottom
			triangleIndices[indicesStartIndex + 18] = verticesStartIndex + 12;
			triangleIndices[indicesStartIndex + 19] = verticesStartIndex + 15;
			triangleIndices[indicesStartIndex + 20] = verticesStartIndex + 14;
			triangleIndices[indicesStartIndex + 21] = verticesStartIndex + 12;
			triangleIndices[indicesStartIndex + 22] = verticesStartIndex + 13;
			triangleIndices[indicesStartIndex + 23] = verticesStartIndex + 15;

			addVertCount = 16;
			addIndCount = 24;
		}
		else
		{
			triangleIndices[indicesStartIndex] = verticesStartIndex;
			triangleIndices[indicesStartIndex + 1] = verticesStartIndex + 3;
			triangleIndices[indicesStartIndex + 2] = verticesStartIndex + 2;
			triangleIndices[indicesStartIndex + 3] = verticesStartIndex;
			triangleIndices[indicesStartIndex + 4] = verticesStartIndex + 1;
			triangleIndices[indicesStartIndex + 5] = verticesStartIndex + 3;

			addVertCount = 4;
			addIndCount = 6;
		}
		if (richTextProperty.underline)
		{
			triangleIndices[indicesStartIndex + addIndCount] = verticesStartIndex + addVertCount;
			triangleIndices[indicesStartIndex + addIndCount + 1] = verticesStartIndex + addVertCount + 3;
			triangleIndices[indicesStartIndex + addIndCount + 2] = verticesStartIndex + addVertCount + 2;
			triangleIndices[indicesStartIndex + addIndCount + 3] = verticesStartIndex + addVertCount;
			triangleIndices[indicesStartIndex + addIndCount + 4] = verticesStartIndex + addVertCount + 1;
			triangleIndices[indicesStartIndex + addIndCount + 5] = verticesStartIndex + addVertCount + 3;

			addVertCount += 4;
			addIndCount += 6;
		}
		if (richTextProperty.strikethrough)
		{
			triangleIndices[indicesStartIndex + addIndCount] = verticesStartIndex + addVertCount;
			triangleIndices[indicesStartIndex + addIndCount + 1] = verticesStartIndex + addVertCount + 3;
			triangleIndices[indicesStartIndex + addIndCount + 2] = verticesStartIndex + addVertCount + 2;
			triangleIndices[indicesStartIndex + addIndCount + 3] = verticesStartIndex + addVertCount;
			triangleIndices[indicesStartIndex + addIndCount + 4] = verticesStartIndex + addVertCount + 1;
			triangleIndices[indicesStartIndex + addIndCount + 5] = verticesStartIndex + addVertCount + 3;

			addVertCount += 4;
			addIndCount += 6;
		}
	}
}


bool ULGUIFontData::GetCharDataFromCache(const TCHAR& charCode, const float& charSize, FLGUICharData_HighPrecision& OutResult)
{
	auto fontKey = FLGUIFontKeyData(charCode, charSize);
	if (auto charData = charDataMap.Find(fontKey))
	{
		OutResult = FLGUICharData_HighPrecision(*charData);
		return true;
	}
	return false;
}
void ULGUIFontData::AddCharDataToCache(const TCHAR& charCode, const float& charSize, const FLGUICharData& charData)
{
	charDataMap.Add(FLGUIFontKeyData(charCode, charSize), charData);
}
void ULGUIFontData::ScaleDownUVofCachedChars()
{
	for (auto& charDataItem : charDataMap)
	{
		auto& mapValue = charDataItem.Value;
		mapValue.uv0X *= 0.5f;
		mapValue.uv0Y *= 0.5f;
		mapValue.uv3X *= 0.5f;
		mapValue.uv3Y *= 0.5f;
	}
}
bool ULGUIFontData::RenderGlyph(const TCHAR& charCode, const float& charSize, FGlyphBitmap& OutResult)
{
#if WITH_FREETYPE
	//InSlot->bitmap_left equals (InSlot->metrics.horiBearingX >> 6), InSlot->bitmap_top equals (InSlot->metrics.horiBearingY >> 6)

	auto slot = RenderGlyphOnFreeType(charCode, charSize);
	if (slot == nullptr)
	{
		return false;
	}
	OutResult.width = slot->bitmap.width;
	OutResult.height = slot->bitmap.rows;
	OutResult.hOffset = slot->bitmap_left;
	OutResult.vOffset = slot->bitmap_top;
	OutResult.hAdvance = slot->metrics.horiAdvance >> 6;
	OutResult.pixelSize = 4;
	//pixel color
	int pixelCount = OutResult.width * OutResult.height;
	FColor* regionColor = new FColor[pixelCount];
	for (int i = 0; i < pixelCount; i++)
	{
		auto& pixelColor = regionColor[i];
		pixelColor.R = pixelColor.G = pixelColor.B = 255;
		pixelColor.A = slot->bitmap.buffer[i];
	}
	OutResult.buffer = (unsigned char*)regionColor;
	return true;
#else
	return false;
#endif
}
void ULGUIFontData::ClearCharDataCache()
{
	charDataMap.Empty();
}

UTexture2D* ULGUIFontData::CreateFontTexture(int InTextureSize)
{
	auto ResultTexture = NewObject<UTexture2D>(
		GetTransientPackage(),
		FName(*FString::Printf(TEXT("LGUIFontData_Texture_%d"), LGUIUtils::LGUITextureNameSuffix++)),
		EObjectFlags::RF_Transient
	);
	auto PlatformData = new FTexturePlatformData();
	PlatformData->SizeX = InTextureSize;
	PlatformData->SizeY = InTextureSize;
	PlatformData->PixelFormat = PF_B8G8R8A8;
	// Allocate first mipmap.
	int32 NumBlocksX = InTextureSize / GPixelFormats[PF_B8G8R8A8].BlockSizeX;
	int32 NumBlocksY = InTextureSize / GPixelFormats[PF_B8G8R8A8].BlockSizeY;
	FTexture2DMipMap* Mip = new FTexture2DMipMap();
	PlatformData->Mips.Add(Mip);
	Mip->SizeX = InTextureSize;
	Mip->SizeY = InTextureSize;
	Mip->BulkData.Lock(LOCK_READ_WRITE);
	void* dataPtr = Mip->BulkData.Realloc(NumBlocksX * NumBlocksY * GPixelFormats[PF_B8G8R8A8].BlockBytes);
	FColor* pixelPtr = static_cast<FColor*>(dataPtr);
	const FColor DefaultColor = FColor(255, 255, 255, 0);
	for (int i = 0, count = InTextureSize * InTextureSize; i < count; i++)
	{
		pixelPtr[i] = DefaultColor;
	}
	Mip->BulkData.Unlock();
	ResultTexture->SetPlatformData(PlatformData);

	ResultTexture->CompressionSettings = TextureCompressionSettings::TC_EditorIcon;
	ResultTexture->LODGroup = TextureGroup::TEXTUREGROUP_UI;
	ResultTexture->SRGB = false;
	ResultTexture->Filter = TextureFilter::TF_Trilinear;
	ResultTexture->UpdateResource();

	return ResultTexture;
}

void ULGUIFontData::ApplyPackingAtlasTextureExpand(UTexture2D* newTexture, int newTextureSize)
{
	Super::ApplyPackingAtlasTextureExpand(newTexture, newTextureSize);
	//scale down uv of prev chars
	for (auto& charDataItem : charDataMap)
	{
		auto& mapValue = charDataItem.Value;
		mapValue.uv0X *= 0.5f;
		mapValue.uv0Y *= 0.5f;
		mapValue.uv3X *= 0.5f;
		mapValue.uv3Y *= 0.5f;
	}
}

void ULGUIFontData::PrepareForPushCharData(UUIText* InText)
{
	boldSize = InText->GetFontSize() * boldRatio;
	italicSlop = FMath::Tan(FMath::DegreesToRadians(italicAngle));
}

#if WITH_EDITOR
void ULGUIFontData::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.Property)
	{
		auto PropertyName = Property->GetFName();
		if (PropertyName == GET_MEMBER_NAME_CHECKED(ULGUIFontData, italicAngle)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(ULGUIFontData, boldRatio)
			)
		{
			for (auto& textItem : renderTextArray)
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
