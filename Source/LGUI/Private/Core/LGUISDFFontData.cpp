// Copyright 2019-present LexLiu. All Rights Reserved.

#pragma once

#include "Core/LGUISDFFontData.h"
#include "Core/ActorComponent/UIText.h"
#include "Core/Actor/LGUIManager.h"
#include "Utils/LGUIUtils.h"
#include "Materials/MaterialInterface.h"
#include "TextureResource.h"
#define SDF_IMPLEMENTATION
#include "Utils/sdf/sdf.h"
#if WITH_FREETYPE
#include <ft2build.h>
#include FT_FREETYPE_H
#endif

#define LOCTEXT_NAMESPACE "LGUISDFFontData"

ULGUISDFFontData::ULGUISDFFontData()
{
	initialSize = ELGUIAtlasTextureSizeType::SIZE_1024x1024;
	rectPackCellSize = 1024;
}

bool ULGUISDFFontData::GetCharDataFromCache(const TCHAR& charCode, const float& charSize, FLGUICharData_HighPrecision& OutResult)
{
	if (auto charData = charDataMap.Find(charCode))
	{
		OutResult = FLGUICharData_HighPrecision(*charData);
		float vertexOffset = SDFRadius - SDFRadius * BoldRatio;
		OutResult.width -= vertexOffset + vertexOffset;
		OutResult.height -= vertexOffset + vertexOffset;
		OutResult.xoffset += vertexOffset;
		OutResult.yoffset -= vertexOffset;
		float uvOffset = vertexOffset * oneDivideTextureSize;
		OutResult.uv0X += uvOffset;
		OutResult.uv0Y -= uvOffset;
		OutResult.uv3X -= uvOffset;
		OutResult.uv3Y += uvOffset;
		float scale = charSize * oneDivideFontSize;
		OutResult.width *= scale;
		OutResult.height *= scale;
		OutResult.xoffset *= scale;
		OutResult.yoffset *= scale;
		OutResult.xadvance *= scale;
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
#if WITH_FREETYPE
	auto slot = RenderGlyphOnFreeType(charCode, FontSize);
	if (slot == nullptr)
	{
		return false;
	}
	//auto time = FDateTime::Now();
	int glyphWidth = slot->bitmap.width + SDFRadius + SDFRadius;
	int glyphHeight = slot->bitmap.rows + SDFRadius + SDFRadius;
	static TArray<unsigned char> sourceBuffer;
	static TArray<unsigned char> sdfTemp;
	sourceBuffer.SetNumUninitialized(glyphWidth * glyphHeight);
	sdfTemp.SetNumUninitialized(sourceBuffer.Num() * sizeof(float) * 3);
	unsigned char* sdfResult = new unsigned char[sourceBuffer.Num()];
	FMemory::Memzero(sourceBuffer.GetData(), sourceBuffer.Num());
	FMemory::Memzero(sdfResult, sourceBuffer.Num());
	int sourceBufferOffset = SDFRadius * glyphWidth + SDFRadius;
	int freetypeBufferOffset = 0;
	for (int h = 0, maxH = slot->bitmap.rows, maxW = slot->bitmap.width; h < maxH; h++)
	{
		FMemory::Memcpy(sourceBuffer.GetData() + sourceBufferOffset, slot->bitmap.buffer + freetypeBufferOffset, maxW);
		sourceBufferOffset += glyphWidth;
		freetypeBufferOffset += maxW;
	}
	sdfBuildDistanceFieldNoAlloc(sdfResult, glyphWidth, SDFRadius, sourceBuffer.GetData(), glyphWidth, glyphHeight, glyphWidth, sdfTemp.GetData());
	//UE_LOG(LGUI, Error, TEXT("Gen sdf time: %f(ms)"), (FDateTime::Now() - time).GetTotalMilliseconds());
	OutResult.width = glyphWidth;
	OutResult.height = glyphHeight;
	OutResult.hOffset = slot->bitmap_left - SDFRadius;
	OutResult.vOffset = slot->bitmap_top + SDFRadius;
	OutResult.hAdvance = slot->metrics.horiAdvance >> 6;
	OutResult.buffer = sdfResult;
	OutResult.pixelSize = 1;
	return true;
#else
	return false;
#endif
}
void ULGUISDFFontData::ClearCharDataCache()
{
	charDataMap.Empty();
	LineHeight = VerticalOffset = -1;
}

UTexture2D* ULGUISDFFontData::CreateFontTexture(int InTextureSize)
{
	auto ResultTexture = NewObject<UTexture2D>(
		this,
		FName(*FString::Printf(TEXT("LGUISDFFontData_Texture_%d"), LGUIUtils::LGUITextureNameSuffix++))
		);
	auto PlatformData = new FTexturePlatformData();
	PlatformData->SizeX = InTextureSize;
	PlatformData->SizeY = InTextureSize;
	PlatformData->PixelFormat = PF_R8;
	// Allocate first mipmap.
	int32 NumBlocksX = InTextureSize / GPixelFormats[PF_R8].BlockSizeX;
	int32 NumBlocksY = InTextureSize / GPixelFormats[PF_R8].BlockSizeY;
	FTexture2DMipMap* Mip = new FTexture2DMipMap();
	PlatformData->Mips.Add(Mip);
	Mip->SizeX = InTextureSize;
	Mip->SizeY = InTextureSize;
	int DataSize = NumBlocksX * NumBlocksY * GPixelFormats[PF_R8].BlockBytes;
	Mip->BulkData.Lock(LOCK_READ_WRITE);
	void* dataPtr = Mip->BulkData.Realloc(DataSize);
	FMemory::Memzero(dataPtr, DataSize);
	Mip->BulkData.Unlock();
	ResultTexture->SetPlatformData(PlatformData);

	ResultTexture->CompressionSettings = TextureCompressionSettings::TC_DistanceFieldFont;
	ResultTexture->LODGroup = TextureGroup::TEXTUREGROUP_UI;
	ResultTexture->SRGB = false;
	ResultTexture->Filter = TextureFilter::TF_Trilinear;
	ResultTexture->UpdateResource();

	return ResultTexture;
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
	italicSlop = FMath::Tan(FMath::DegreesToRadians(ItalicAngle));
	oneDivideFontSize = 1.0f / FontSize;
	auto CompScale = InText->GetComponentScale();
	objectScale = FMath::Max(CompScale.X, CompScale.Y);
	SDFRadius = FontSize * 0.25f;//use 1/4 of FontSize can get good result
}

uint8 ULGUISDFFontData::GetRequireAdditionalShaderChannels()
{
	return
		(1 << (int)ELGUICanvasAdditionalChannelType::UV1)//UV1.x = boldSize, UV1.y = richTextProperty.size * oneDivideFontSize * objectScale
		| (1 << (int)ELGUICanvasAdditionalChannelType::Normal)//for tilt look
		;
}

float ULGUISDFFontData::GetKerning(const TCHAR& leftCharIndex, const TCHAR& rightCharIndex, const float& charSize)
{
	auto KerningPair = FLGUISDFFontKerningPair(leftCharIndex, rightCharIndex);
	if (auto KerningValuePtr = KerningPairsMap.Find(KerningPair))
	{
		return (*KerningValuePtr) * charSize * oneDivideFontSize;
	}
	else
	{
		auto KerningValue = Super::GetKerning(leftCharIndex, rightCharIndex, FontSize);
		KerningPairsMap.Add(KerningPair, KerningValue);
		return KerningValue * charSize * oneDivideFontSize;
	}
}
float ULGUISDFFontData::GetLineHeight(const float& fontSize)
{
	if (LineHeight == -1)
	{
		LineHeight = Super::GetLineHeight(FontSize);
	}
	return LineHeight * fontSize * oneDivideFontSize;
}
float ULGUISDFFontData::GetVerticalOffset(const float& fontSize)
{
	if (VerticalOffset == -1)
	{
		VerticalOffset = Super::GetVerticalOffset(FontSize);
	}
	return VerticalOffset * fontSize * oneDivideFontSize;
}
UMaterialInterface* ULGUISDFFontData::GetFontMaterial(ELGUICanvasClipType clipType)
{
	return SDFDefaultMaterials[(int)clipType];
}

void ULGUISDFFontData::PushCharData(
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
	int32 newVerticesCount = verticesStartIndex + outAdditionalVerticesCount;
	UIGeometry::LGUIGeometrySetArrayNum(originVertices, newVerticesCount, false);
	UIGeometry::LGUIGeometrySetArrayNum(vertices, newVerticesCount, false);

	int32 newIndicesCount = indicesStartIndex + outAdditionalIndicesCount;
	UIGeometry::LGUIGeometrySetArrayNum(triangleIndices, newIndicesCount, false);

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
		auto tempFontScale = richTextProperty.size * objectScale;
		{
			vertices[verticesStartIndex].TextureCoordinate[0] = charData.GetUV0();
			vertices[verticesStartIndex + 1].TextureCoordinate[0] = charData.GetUV1();
			vertices[verticesStartIndex + 2].TextureCoordinate[0] = charData.GetUV2();
			vertices[verticesStartIndex + 3].TextureCoordinate[0] = charData.GetUV3();

			//bold and scale
			{
				auto tempBoldSize = richTextProperty.bold ? BoldRatio : 0.0f;
				vertices[verticesStartIndex].TextureCoordinate[1] = FVector2f(tempBoldSize, tempFontScale);
				vertices[verticesStartIndex + 1].TextureCoordinate[1] = FVector2f(tempBoldSize, tempFontScale);
				vertices[verticesStartIndex + 2].TextureCoordinate[1] = FVector2f(tempBoldSize, tempFontScale);
				vertices[verticesStartIndex + 3].TextureCoordinate[1] = FVector2f(tempBoldSize, tempFontScale);
			}

			addVertCount = 4;
		}
		if (richTextProperty.underline)
		{
			vertices[verticesStartIndex + addVertCount].TextureCoordinate[0] = underlineCharGeo.GetUV0();
			vertices[verticesStartIndex + addVertCount + 1].TextureCoordinate[0] = underlineCharGeo.GetUV1();
			vertices[verticesStartIndex + addVertCount + 2].TextureCoordinate[0] = underlineCharGeo.GetUV2();
			vertices[verticesStartIndex + addVertCount + 3].TextureCoordinate[0] = underlineCharGeo.GetUV3();

			//bold and scale, bold is not needed for underline and strikethrough, but scale is needed
			{
				auto tempBoldSize = 0.0f;
				vertices[verticesStartIndex + addVertCount].TextureCoordinate[1] = FVector2f(tempBoldSize, tempFontScale);
				vertices[verticesStartIndex + addVertCount + 1].TextureCoordinate[1] = FVector2f(tempBoldSize, tempFontScale);
				vertices[verticesStartIndex + addVertCount + 2].TextureCoordinate[1] = FVector2f(tempBoldSize, tempFontScale);
				vertices[verticesStartIndex + addVertCount + 3].TextureCoordinate[1] = FVector2f(tempBoldSize, tempFontScale);
			}

			addVertCount += 4;
		}
		if (richTextProperty.strikethrough)
		{
			vertices[verticesStartIndex + addVertCount].TextureCoordinate[0] = strikethroughCharGeo.GetUV0();
			vertices[verticesStartIndex + addVertCount + 1].TextureCoordinate[0] = strikethroughCharGeo.GetUV1();
			vertices[verticesStartIndex + addVertCount + 2].TextureCoordinate[0] = strikethroughCharGeo.GetUV2();
			vertices[verticesStartIndex + addVertCount + 3].TextureCoordinate[0] = strikethroughCharGeo.GetUV3();

			//bold and scale, bold is not needed for underline and strikethrough, but scale is needed
			{
				auto tempBoldSize = 0.0f;
				vertices[verticesStartIndex + addVertCount].TextureCoordinate[1] = FVector2f(tempBoldSize, tempFontScale);
				vertices[verticesStartIndex + addVertCount + 1].TextureCoordinate[1] = FVector2f(tempBoldSize, tempFontScale);
				vertices[verticesStartIndex + addVertCount + 2].TextureCoordinate[1] = FVector2f(tempBoldSize, tempFontScale);
				vertices[verticesStartIndex + addVertCount + 3].TextureCoordinate[1] = FVector2f(tempBoldSize, tempFontScale);
			}

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
			PropertyName == GET_MEMBER_NAME_CHECKED(ULGUISDFFontData, SDFDefaultMaterials)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(ULGUISDFFontData, ItalicAngle)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(ULGUISDFFontData, BoldRatio)
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

void ULGUISDFFontData::PostInitProperties()
{
	Super::PostInitProperties();
	CheckMaterials();
}

void ULGUISDFFontData::CheckMaterials()
{
	for (int i = 0; i < (int)ELGUICanvasClipType::Custom; i++)
	{
		if (SDFDefaultMaterials[i] == nullptr)
		{
			FString matPath;
			switch (i)
			{
			default:
			case 0: matPath = TEXT("/LGUI/Materials/LGUI_SDF_Font_NoClip"); break;
			case 1: matPath = TEXT("/LGUI/Materials/LGUI_SDF_Font_RectClip"); break;
			case 2: matPath = TEXT("/LGUI/Materials/LGUI_SDF_Font_TextureClip"); break;
			}
			auto mat = LoadObject<UMaterialInterface>(NULL, *matPath);
			if (mat == nullptr)
			{
				auto errMsg = FText::Format(LOCTEXT("MissingDefaultContent", "{0} Load material error! Missing some content of LGUI plugin, reinstall this plugin may fix the issue.")
					, FText::FromString(FString::Printf(TEXT("[%s].%d"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__)));
				UE_LOG(LGUI, Error, TEXT("%s"), *errMsg.ToString());
#if WITH_EDITOR
				LGUIUtils::EditorNotification(errMsg, 10);
#endif
				continue;
			}
			SDFDefaultMaterials[i] = mat;
			this->MarkPackageDirty();
		}
	}
}

#undef LOCTEXT_NAMESPACE
