// Copyright 2019-present LexLiu. All Rights Reserved.

#pragma once

#include "LGUISDFFontData.h"
#include "LGUISDFFontModule.h"
#include "Core/ActorComponent/UIText.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "Core/LGUIAtlasData.h"
#define SDF_IMPLEMENTATION
#include "sdf/sdf.h"
#include <ft2build.h>
#include FT_FREETYPE_H

ULGUISDFFontData::ULGUISDFFontData()
{
	initialSize = ELGUIAtlasTextureSizeType::SIZE_1024x1024;
	boldRatio = 0.15f;
}

bool ULGUISDFFontData::GetCharDataFromCache(const TCHAR& charCode, const float& charSize, FLGUICharData_HighPrecision& OutResult)
{
	if (auto charData = charDataMap.Find(charCode))
	{
		OutResult = FLGUICharData_HighPrecision(*charData, charSize * oneDivideFontSize);
		return true;
	}
	return false;
}
void ULGUISDFFontData::AddCharDataToCache(const TCHAR& charCode, const float& charSize, const FLGUICharData& charData)
{
	charDataMap.Add(charCode, charData);
}
void ULGUISDFFontData::ScaleDownUVofCachedChars()
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
bool ULGUISDFFontData::RenderGlyph(const TCHAR& charCode, const float& charSize, FGlyphBitmap& OutResult)
{
	auto slot = RenderGlyphOnFreeType(charCode, FontSize);
	if (slot == nullptr)
	{
		return false;
	}
	//auto time = FDateTime::Now();
	int glyphWidth = slot->bitmap.width + SDFRadius + SDFRadius;
	int glyphHeight = slot->bitmap.rows + SDFRadius + SDFRadius;
	static TArray<unsigned char> sourceBuffer;
	static TArray<unsigned char> sdfResult;
	static TArray<unsigned char> sdfTemp;
	sourceBuffer.SetNumZeroed(glyphWidth * glyphHeight);
	sdfResult.SetNumZeroed(sourceBuffer.Num());
	sdfTemp.SetNumZeroed(sourceBuffer.Num() * sizeof(float) * 3);
	FMemory::Memzero(sourceBuffer.GetData(), sourceBuffer.Num());
	for (int h = 0, maxH = slot->bitmap.rows; h < maxH; h++)
	{
		for (int w = 0, maxW = slot->bitmap.width; w < maxW; w++)
		{
			sourceBuffer[(h + SDFRadius) * glyphWidth + w + SDFRadius] = slot->bitmap.buffer[h * maxW + w];
		}
	}
	sdfBuildDistanceFieldNoAlloc(sdfResult.GetData(), glyphWidth, SDFRadius, sourceBuffer.GetData(), glyphWidth, glyphHeight, glyphWidth, sdfTemp.GetData());
	//UE_LOG(LGUI, Error, TEXT("Gen sdf time: %f"), (FDateTime::Now() - time).GetTotalMilliseconds());
	OutResult.width = glyphWidth;
	OutResult.height = glyphHeight;
	OutResult.hOffset = slot->bitmap_left - SDFRadius;
	OutResult.vOffset = slot->bitmap_top + SDFRadius;
	OutResult.hAdvance = slot->metrics.horiAdvance >> 6;
	OutResult.buffer = sdfResult.GetData();
	return true;
}
void ULGUISDFFontData::ClearCharDataCache()
{
	charDataMap.Empty();
}

void ULGUISDFFontData::ApplyPackingAtlasTextureExpand(UTexture2D* newTexture, int newTextureSize)
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

void ULGUISDFFontData::PrepareForPushCharData(UUIText* InText)
{
	boldSize = boldRatio * FontSize * 0.02f;
	italicSlop = FMath::Tan(FMath::DegreesToRadians(italicAngle));
	oneDivideFontSize = 1.0f / FontSize;
}

uint8 ULGUISDFFontData::GetRequireAdditionalShaderChannels()
{
	return
		(1 << (int)ELGUICanvasAdditionalChannelType::UV1)
		| (1 << (int)ELGUICanvasAdditionalChannelType::UV2)
		;
}

void ULGUISDFFontData::PushCharData(
	TCHAR charCode, const FVector2D& inLineOffset, const FVector2D& fontSpace, const FLGUICharData_HighPrecision& charData,
	const LGUIRichTextParser::RichTextParseResult& richTextProperty,
	int verticesStartIndex, int indicesStartIndex,
	int& outAdditionalVerticesCount, int& outAdditionalIndicesCount,
	TArray<FLGUIOriginVertexData>& originVertices, TArray<FDynamicMeshVertex>& vertices, TArray<FLGUIIndexType>& triangleIndices
)
{
	auto GetUnderlineOrStrikethroughCharGeo = [&](TCHAR charCode, int overrideFontSize)
	{
		auto charData = this->GetCharData(charCode, (uint16)overrideFontSize);
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
		{
			x = offsetX;
			y = offsetY - charData.height;
			auto& vert0 = originVertices[verticesStartIndex].Position;
			vert0 = FVector(0, x, y);
			x = charData.width + offsetX;
			auto& vert1 = originVertices[verticesStartIndex + 1].Position;
			vert1 = FVector(0, x, y);
			x = offsetX;
			y = offsetY;
			auto& vert2 = originVertices[verticesStartIndex + 2].Position;
			vert2 = FVector(0, x, y);
			x = charData.width + offsetX;
			auto& vert3 = originVertices[verticesStartIndex + 3].Position;
			vert3 = FVector(0, x, y);
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
			originVertices[verticesStartIndex + addVertCount].Position = FVector(0, x, y);
			x = charWidth + offsetX;
			originVertices[verticesStartIndex + addVertCount + 1].Position = FVector(0, x, y);
			x = offsetX;
			y = offsetY;
			originVertices[verticesStartIndex + addVertCount + 2].Position = FVector(0, x, y);
			x = charWidth + offsetX;
			originVertices[verticesStartIndex + addVertCount + 3].Position = FVector(0, x, y);

			addVertCount += 4;
		}
		if (richTextProperty.strikethrough)
		{
			offsetX = lineOffset.X;
			offsetY = lineOffset.Y + strikethroughCharGeo.yoffset;
			x = offsetX;
			y = offsetY - strikethroughCharGeo.height;
			originVertices[verticesStartIndex + addVertCount].Position = FVector(0, x, y);
			x = charWidth + offsetX;
			originVertices[verticesStartIndex + addVertCount + 1].Position = FVector(0, x, y);
			x = offsetX;
			y = offsetY;
			originVertices[verticesStartIndex + addVertCount + 2].Position = FVector(0, x, y);
			x = charWidth + offsetX;
			originVertices[verticesStartIndex + addVertCount + 3].Position = FVector(0, x, y);

			addVertCount += 4;
		}
	}
	//uv
	{
		int addVertCount = 0;
		{
			vertices[verticesStartIndex].TextureCoordinate[0] = charData.GetUV0();
			vertices[verticesStartIndex + 1].TextureCoordinate[0] = charData.GetUV1();
			vertices[verticesStartIndex + 2].TextureCoordinate[0] = charData.GetUV2();
			vertices[verticesStartIndex + 3].TextureCoordinate[0] = charData.GetUV3();

			auto tempBoldSize = richTextProperty.bold ? boldSize : 0.0f;
			vertices[verticesStartIndex].TextureCoordinate[1] = FVector2D(tempBoldSize, richTextProperty.size * oneDivideFontSize);
			vertices[verticesStartIndex + 1].TextureCoordinate[1] = FVector2D(tempBoldSize, richTextProperty.size * oneDivideFontSize);
			vertices[verticesStartIndex + 2].TextureCoordinate[1] = FVector2D(tempBoldSize, richTextProperty.size * oneDivideFontSize);
			vertices[verticesStartIndex + 3].TextureCoordinate[1] = FVector2D(tempBoldSize, richTextProperty.size * oneDivideFontSize);

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

		triangleIndices[indicesStartIndex] = verticesStartIndex;
		triangleIndices[indicesStartIndex + 1] = verticesStartIndex + 3;
		triangleIndices[indicesStartIndex + 2] = verticesStartIndex + 2;
		triangleIndices[indicesStartIndex + 3] = verticesStartIndex;
		triangleIndices[indicesStartIndex + 4] = verticesStartIndex + 1;
		triangleIndices[indicesStartIndex + 5] = verticesStartIndex + 3;

		addVertCount = 4;
		addIndCount = 6;

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

#if WITH_EDITOR
void ULGUISDFFontData::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.Property)
	{
		auto PropertyName = Property->GetFName();
		if (
			PropertyName == GET_MEMBER_NAME_CHECKED(ULGUISDFFontData, FontSize)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(ULGUISDFFontData, SDFRadius)
			)
		{
			ReloadFont();
		}
		if (
			PropertyName == GET_MEMBER_NAME_CHECKED(ULGUISDFFontData, DefaultMaterials)
			)
		{
			for (auto& textItem : renderTextArray)
			{
				if (textItem.IsValid())
				{
					textItem->ApplyFontMaterialChange();
				}
			}
		}
	}
}
#endif
