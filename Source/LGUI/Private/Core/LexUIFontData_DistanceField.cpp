// Copyright 2019-present LexLiu. All Rights Reserved.

#include "Core/LexUIFontData_DistanceField.h"
#include "Core/Components/LexText.h"
#include "Materials/MaterialInterface.h"
#include "TextureResource.h"
#include "Engine/Texture2D.h"
#include "HAL/PlatformTime.h"
#define SDF_IMPLEMENTATION
#include "Core/Components/LexWidget.h"
#include "Engine/Texture2DArray.h"
#include "Utils/sdf/sdf.h"
#if WITH_FREETYPE
#include <ft2build.h>
#include FT_FREETYPE_H
#endif

#define LOCTEXT_NAMESPACE "LexUIFontData_DistanceField"

ULexUIFontData_DistanceField::ULexUIFontData_DistanceField()
{
	RectPackCellSizeType = ELexUIAtlasTextureSizeType::SIZE_512x512;

	PresetMaterials.Add(LoadObject<UMaterialInterface>(NULL, TEXT("/LGUI/Materials/TextEffects/MI_DropShadowSoft")));
	PresetMaterials.Add(LoadObject<UMaterialInterface>(NULL, TEXT("/LGUI/Materials/TextEffects/MI_DropShadowHard")));
	PresetMaterials.Add(LoadObject<UMaterialInterface>(NULL, TEXT("/LGUI/Materials/TextEffects/MI_Outline")));
	PresetMaterials.Add(LoadObject<UMaterialInterface>(NULL, TEXT("/LGUI/Materials/TextEffects/MI_OutlineOnly")));
}

bool ULexUIFontData_DistanceField::GetCharDataFromCache(uint32 CharCode, float CharSize, bool IsBold, FLexUICharData& OutResult)
{
	auto CharKey = FLexUIDistanceFieldCharKey(CharCode, IsBold);
	if (auto charData = CharDataMap.Find(CharKey))
	{
		OutResult = FLexUICharData(*charData);
		float vertexOffset;
		if (ExpandMeshSize <= 0)//shrink mesh to reduce empty area of SDFRadius
		{
			vertexOffset = SDFRadius - SampleFontSize * 0.02f;//0.02: slightly expand it in-case too sharp edge
		}
		else
		{
			vertexOffset = (SDFRadius - ExpandMeshSize) - SampleFontSize * 0.02f;//0.02: slightly expand it in-case too sharp edge
		}
		OutResult.Width -= vertexOffset + vertexOffset;
		OutResult.Height -= vertexOffset + vertexOffset;
		OutResult.XOffset += vertexOffset;
		OutResult.YOffset -= vertexOffset;
		float uvOffset = vertexOffset * OneDivideTextureSize;
		OutResult.MinUV.X += uvOffset;
		OutResult.MaxUV.Y -= uvOffset;
		OutResult.MaxUV.X -= uvOffset;
		OutResult.MinUV.Y += uvOffset;
		//scale char by font size
		float scale = CharSize * OneDivideFontSize;
		OutResult.Width *= scale;
		OutResult.Height *= scale;
		OutResult.XOffset *= scale;
		OutResult.YOffset *= scale;
		OutResult.XAdvance *= scale;
		return true;
	}
	return false;
}
void ULexUIFontData_DistanceField::AddCharDataToCache(uint32 CharCode, float CharSize, bool IsBold, FLexUICharData& CharData)
{
	CharDataMap.Add(FLexUIDistanceFieldCharKey(CharCode, IsBold), CharData);
}

bool ULexUIFontData_DistanceField::RenderGlyph(uint32 CharCode, float CharSize, bool IsBold, FGlyphBitmap& OutResult)
{
#if WITH_FREETYPE
	auto slot = RenderGlyphOnFreeType(CharCode, SampleFontSize, IsBold ? SampleFontSize * BoldRatio : 0);
	if (slot == nullptr)
	{
		return false;
	}
	//const double Time = FPlatformTime::Seconds();
	int glyphWidth = slot->bitmap.width + SDFRadius + SDFRadius;
	int glyphHeight = slot->bitmap.rows + SDFRadius + SDFRadius;
	static TArray<unsigned char> sourceBuffer;
	static TArray<unsigned char> sdfTemp;
	sourceBuffer.SetNumUninitialized(glyphWidth * glyphHeight);
	sdfTemp.SetNumUninitialized(sourceBuffer.Num() * sizeof(float) * 3);
	TArray<unsigned char> sdfResult;
	sdfResult.SetNumUninitialized(sourceBuffer.Num());
	FMemory::Memzero(sourceBuffer.GetData(), sourceBuffer.Num());
	FMemory::Memzero(sdfResult.GetData(), sourceBuffer.Num());
	int sourceBufferOffset = SDFRadius * glyphWidth + SDFRadius;
	int freetypeBufferOffset = 0;
	for (int h = 0, maxH = slot->bitmap.rows, maxW = slot->bitmap.width; h < maxH; h++)
	{
		FMemory::Memcpy(sourceBuffer.GetData() + sourceBufferOffset, slot->bitmap.buffer + freetypeBufferOffset, maxW);
		sourceBufferOffset += glyphWidth;
		freetypeBufferOffset += maxW;
	}
	sdfBuildDistanceFieldNoAlloc(sdfResult.GetData(), glyphWidth, SDFRadius, sourceBuffer.GetData(), glyphWidth, glyphHeight, glyphWidth, sdfTemp.GetData());
	//UE_LOG(LGUI, Error, TEXT("Gen sdf time: %f(ms)"), (FPlatformTime::Seconds() - Time) * 1000.0);
	OutResult.width = glyphWidth;
	OutResult.height = glyphHeight;
	OutResult.hOffset = slot->bitmap_left - SDFRadius;
	OutResult.vOffset = slot->bitmap_top + SDFRadius;
	OutResult.hAdvance = slot->metrics.horiAdvance * ONE_DIVIDE_64;
	OutResult.buffer = MoveTemp(sdfResult);
	OutResult.pixelSize = 1;
	return true;
#else
	return false;
#endif
}
void ULexUIFontData_DistanceField::ClearCharDataCache()
{
	CharDataMap.Empty();
	LineHeight = VerticalOffset = -1;
}

UTexture2DArray* ULexUIFontData_DistanceField::CreateFontTexture(int InTextureSize, int InSliceCount)
{
	static int TextureNameSuffix = 0;
	auto NewTexture = NewObject<UTexture2DArray>(
		GetTransientPackage()
		, FName(*FString::Printf(TEXT("LexUIFontData_DistanceField_Texture_%d"), TextureNameSuffix++))
		, RF_Transient);
	auto PixelFormat = PF_R8;

	auto PlatformData = new FTexturePlatformData();
	PlatformData->SizeX = InTextureSize;
	PlatformData->SizeY = InTextureSize;
	PlatformData->PixelFormat = PixelFormat;
	PlatformData->SetNumSlices(InSliceCount);
	NewTexture->SetPlatformData(PlatformData);

	// Allocate first mipmap.
	int32 NumBlocksX = InTextureSize / GPixelFormats[PixelFormat].BlockSizeX;
	int32 NumBlocksY = InTextureSize / GPixelFormats[PixelFormat].BlockSizeY;
	FTexture2DMipMap* Mip = new FTexture2DMipMap(InTextureSize, InTextureSize, InSliceCount);
	PlatformData->Mips.Add(Mip);
	auto DataSize = (int64)GPixelFormats[PixelFormat].BlockBytes * NumBlocksX * NumBlocksY * InSliceCount;
	Mip->BulkData.Lock(LOCK_READ_WRITE);
	Mip->BulkData.Realloc(DataSize);
	void* DataPtr = Mip->BulkData.Realloc(DataSize);
	FMemory::Memzero(DataPtr, DataSize);
	Mip->BulkData.Unlock();
	
	NewTexture->CompressionSettings = TextureCompressionSettings::TC_DistanceFieldFont;
	NewTexture->LODGroup = TextureGroup::TEXTUREGROUP_UI;
	NewTexture->SRGB = false;
	NewTexture->Filter = TextureFilter::TF_Bilinear;
	NewTexture->UpdateResource();

	return NewTexture;
}

UTexture2D* ULexUIFontData_DistanceField::CreateIntermediateTexture(int InTextureSize)
{
	static int TextureNameSuffix = 0;
	auto ResultTexture = NewObject<UTexture2D>(
		GetTransientPackage(),
		FName(*FString::Printf(TEXT("LexUIFontData_DistanceField_Intermediate_%d"), TextureNameSuffix++)),
		RF_Transient
	);
	auto PixelFormat = PF_R8;
	
	auto PlatformData = new FTexturePlatformData();
	PlatformData->SizeX = InTextureSize;
	PlatformData->SizeY = InTextureSize;
	PlatformData->PixelFormat = PixelFormat;
	ResultTexture->SetPlatformData(PlatformData);
	
	// Allocate first mipmap.
	int32 NumBlocksX = InTextureSize / GPixelFormats[PixelFormat].BlockSizeX;
	int32 NumBlocksY = InTextureSize / GPixelFormats[PixelFormat].BlockSizeY;
	FTexture2DMipMap* Mip = new FTexture2DMipMap();
	PlatformData->Mips.Add(Mip);
	Mip->SizeX = InTextureSize;
	Mip->SizeY = InTextureSize;
	int DataSize = NumBlocksX * NumBlocksY * GPixelFormats[PixelFormat].BlockBytes;
	Mip->BulkData.Lock(LOCK_READ_WRITE);
	void* DataPtr = Mip->BulkData.Realloc(DataSize);
	FMemory::Memzero(DataPtr, DataSize);
	Mip->BulkData.Unlock();

	ResultTexture->CompressionSettings = TextureCompressionSettings::TC_DistanceFieldFont;
	ResultTexture->LODGroup = TextureGroup::TEXTUREGROUP_UI;
	ResultTexture->SRGB = false;
	ResultTexture->Filter = TextureFilter::TF_Bilinear;
	ResultTexture->UpdateResource();

	return ResultTexture;
}

void ULexUIFontData_DistanceField::ApplyPackingAtlasTextureExpand(UTexture2D* newTexture, int newTextureSize)
{
	Super::ApplyPackingAtlasTextureExpand(newTexture, newTextureSize);
	//scale down uv of prev chars
	for (auto& charDataItem : CharDataMap)
	{
		auto& mapValue = charDataItem.Value;
		mapValue.MinUV.X *= 0.5f;
		mapValue.MaxUV.Y *= 0.5f;
		mapValue.MaxUV.X *= 0.5f;
		mapValue.MinUV.Y *= 0.5f;
	}
}

void ULexUIFontData_DistanceField::PrepareForPushCharData(ULexText* InText)
{
	ItalicSlop = FMath::Tan(FMath::DegreesToRadians(ItalicAngle));
	OneDivideFontSize = 1.0f / SampleFontSize;
	ExpandMeshSize = InText->GetExpandMeshSize();
	auto CompScale = InText->GetWidget()->GetWorldScale();
	ObjectScale = FMath::Max(CompScale.X, CompScale.Y)
		* SampleFontSize / SDFRadius
		;
}

bool ULexUIFontData_DistanceField::GetRequireNormalAndTangent()
{
	return true;//for tilt look
}

float ULexUIFontData_DistanceField::GetKerning(uint32 leftCharIndex, uint32 rightCharIndex, float charSize)
{
	auto KerningPair = FLexUIDistanceFieldFontKerningPair(leftCharIndex, rightCharIndex);
	if (auto KerningValuePtr = KerningPairsMap.Find(KerningPair))
	{
		return (*KerningValuePtr) * charSize * OneDivideFontSize;
	}
	else
	{
		auto KerningValue = Super::GetKerning(leftCharIndex, rightCharIndex, SampleFontSize);
		KerningPairsMap.Add(KerningPair, KerningValue);
		return KerningValue * charSize * OneDivideFontSize;
	}
}
float ULexUIFontData_DistanceField::GetLineHeight(float fontSize)
{
	if (LineHeight == -1)
	{
		LineHeight = Super::GetLineHeight(SampleFontSize);
	}
	return LineHeight * fontSize * OneDivideFontSize;
}
float ULexUIFontData_DistanceField::GetVerticalOffset(float fontSize)
{
	if (VerticalOffset == -1)
	{
		VerticalOffset = Super::GetVerticalOffset(SampleFontSize);
	}
	return (VerticalOffset + AdditionalVerticalOffset) * fontSize * OneDivideFontSize;
}
UMaterialInterface* ULexUIFontData_DistanceField::GetFontMaterial()
{
	return nullptr;
}

void ULexUIFontData_DistanceField::PushCharData(
	uint32 charCode, FVector2f inLineOffset, FVector2f fontSpace, const FLexUICharData& charData,
	const LexUIRichTextParser::FRichTextParseResult& richTextProperty,
	int verticesStartIndex, int indicesStartIndex,
	int& outAdditionalVerticesCount, int& outAdditionalIndicesCount,
	TArray<FLexUIOriginVertexData>& originVertices, TArray<FLexUIMeshVertex>& vertices, TArray<FLexUIMeshIndex>& triangleIndices
)
{
	auto GetUnderlineOrStrikethroughCharGeo = [&](uint32 charCode, float overrideFontSize, bool bold)
	{
		auto charData = this->GetCharData(charCode, overrideFontSize, bold);
		charData.YOffset += this->GetVerticalOffset(overrideFontSize);

		float uvX = (charData.MaxUV.X - charData.MinUV.X) * 0.5f + charData.MinUV.X;
		charData.MinUV.X = charData.MaxUV.X = uvX;
		return charData;
	};

	outAdditionalVerticesCount = 4;
	outAdditionalIndicesCount = 6;

	FLexUICharData underlineCharGeo;
	FLexUICharData strikethroughCharGeo;
	//underline and strikethrough should not exist at same char
	if (richTextProperty.Underline)
	{
		outAdditionalVerticesCount += 4;
		outAdditionalIndicesCount += 6;
		underlineCharGeo = GetUnderlineOrStrikethroughCharGeo('_', richTextProperty.Size, richTextProperty.Bold);
	}
	if (richTextProperty.Strikethrough)
	{
		outAdditionalVerticesCount += 4;
		outAdditionalIndicesCount += 6;
		strikethroughCharGeo = GetUnderlineOrStrikethroughCharGeo('-', richTextProperty.Size, richTextProperty.Bold);
	}
	int32 newVerticesCount = verticesStartIndex + outAdditionalVerticesCount;
	FLexUIGeometry::LexUIGeometrySetArrayNum(originVertices, newVerticesCount, false);
	FLexUIGeometry::LexUIGeometrySetArrayNum(vertices, newVerticesCount, false);

	int32 newIndicesCount = indicesStartIndex + outAdditionalIndicesCount;
	FLexUIGeometry::LexUIGeometrySetArrayNum(triangleIndices, newIndicesCount, false);

	auto lineOffset = inLineOffset;
	if (richTextProperty.SupOrSubMode == LexUIRichTextParser::ESupOrSubMode::Sup)
	{
		lineOffset.Y += richTextProperty.Size * 0.5f;
	}
	else if (richTextProperty.SupOrSubMode == LexUIRichTextParser::ESupOrSubMode::Sub)
	{
		lineOffset.Y -= richTextProperty.Size * 0.5f;
	}
	
	//position
	{
		float offsetX = lineOffset.X + charData.XOffset;
		float offsetY = lineOffset.Y + charData.YOffset;
		float charAdvanceWidth = charData.XAdvance + fontSpace.X;
		float x, y;

		int addVertCount = 0;
		{
			float charWidth = charData.Width;
			float charHeight = charData.Height;
			x = offsetX;
			y = offsetY - charHeight;
			auto& vert0 = originVertices[verticesStartIndex].Position;
			vert0 = FVector3f(0, x, y);
			x = charWidth + offsetX;
			auto& vert1 = originVertices[verticesStartIndex + 1].Position;
			vert1 = FVector3f(0, x, y);
			x = offsetX;
			y = offsetY;
			auto& vert2 = originVertices[verticesStartIndex + 2].Position;
			vert2 = FVector3f(0, x, y);
			x = charWidth + offsetX;
			auto& vert3 = originVertices[verticesStartIndex + 3].Position;
			vert3 = FVector3f(0, x, y);
			if (richTextProperty.Italic)
			{
				auto vert01ItalicOffset = (charHeight - charData.YOffset) * ItalicSlop;
				vert0.Y -= vert01ItalicOffset;
				vert1.Y -= vert01ItalicOffset;
				auto vert23ItalicOffset = charData.YOffset * ItalicSlop;
				vert2.Y += vert23ItalicOffset;
				vert3.Y += vert23ItalicOffset;
			}

			addVertCount = 4;
		}
		if (richTextProperty.Underline)
		{
			offsetX = lineOffset.X;
			offsetY = lineOffset.Y + underlineCharGeo.YOffset;
			x = offsetX;
			y = offsetY - underlineCharGeo.Height;
			originVertices[verticesStartIndex + addVertCount].Position = FVector3f(0, x, y);
			x = charAdvanceWidth + offsetX;
			originVertices[verticesStartIndex + addVertCount + 1].Position = FVector3f(0, x, y);
			x = offsetX;
			y = offsetY;
			originVertices[verticesStartIndex + addVertCount + 2].Position = FVector3f(0, x, y);
			x = charAdvanceWidth + offsetX;
			originVertices[verticesStartIndex + addVertCount + 3].Position = FVector3f(0, x, y);

			addVertCount += 4;
		}
		if (richTextProperty.Strikethrough)
		{
			offsetX = lineOffset.X;
			offsetY = lineOffset.Y + strikethroughCharGeo.YOffset;
			x = offsetX;
			y = offsetY - strikethroughCharGeo.Height;
			originVertices[verticesStartIndex + addVertCount].Position = FVector3f(0, x, y);
			x = charAdvanceWidth + offsetX;
			originVertices[verticesStartIndex + addVertCount + 1].Position = FVector3f(0, x, y);
			x = offsetX;
			y = offsetY;
			originVertices[verticesStartIndex + addVertCount + 2].Position = FVector3f(0, x, y);
			x = charAdvanceWidth + offsetX;
			originVertices[verticesStartIndex + addVertCount + 3].Position = FVector3f(0, x, y);

			addVertCount += 4;
		}
	}
	//uv
	{
		int addVertCount = 0;
		auto tempFontScale = richTextProperty.Size * ObjectScale;
		{
			{
				vertices[verticesStartIndex].TextureCoordinate[0] = charData.GetUV0();
				vertices[verticesStartIndex + 1].TextureCoordinate[0] = charData.GetUV1();
				vertices[verticesStartIndex + 2].TextureCoordinate[0] = charData.GetUV2();
				vertices[verticesStartIndex + 3].TextureCoordinate[0] = charData.GetUV3();
			}

			//text-scale
			{
				vertices[verticesStartIndex].TextureCoordinate[2] = FVector2f(tempFontScale, 0);
				vertices[verticesStartIndex + 1].TextureCoordinate[2] = FVector2f(tempFontScale, 0);
				vertices[verticesStartIndex + 2].TextureCoordinate[2] = FVector2f(tempFontScale, 0);
				vertices[verticesStartIndex + 3].TextureCoordinate[2] = FVector2f(tempFontScale, 0);
			}
			//slice of texture array
			{
				vertices[verticesStartIndex].TextureCoordinate[1].Y = charData.SliceIndex;
				vertices[verticesStartIndex + 1].TextureCoordinate[1].Y = charData.SliceIndex;
				vertices[verticesStartIndex + 2].TextureCoordinate[1].Y = charData.SliceIndex;
				vertices[verticesStartIndex + 3].TextureCoordinate[1].Y = charData.SliceIndex;
			}

			addVertCount = 4;
		}
		if (richTextProperty.Underline)
		{
			vertices[verticesStartIndex + addVertCount].TextureCoordinate[0] = underlineCharGeo.GetUV0();
			vertices[verticesStartIndex + addVertCount + 1].TextureCoordinate[0] = underlineCharGeo.GetUV1();
			vertices[verticesStartIndex + addVertCount + 2].TextureCoordinate[0] = underlineCharGeo.GetUV2();
			vertices[verticesStartIndex + addVertCount + 3].TextureCoordinate[0] = underlineCharGeo.GetUV3();

			//font scale
			{
				vertices[verticesStartIndex + addVertCount].TextureCoordinate[2] = FVector2f(tempFontScale, 0);
				vertices[verticesStartIndex + addVertCount + 1].TextureCoordinate[2] = FVector2f(tempFontScale, 0);
				vertices[verticesStartIndex + addVertCount + 2].TextureCoordinate[2] = FVector2f(tempFontScale, 0);
				vertices[verticesStartIndex + addVertCount + 3].TextureCoordinate[2] = FVector2f(tempFontScale, 0);
			}
			//slice of texture array
			{
				vertices[verticesStartIndex].TextureCoordinate[1].Y = underlineCharGeo.SliceIndex;
				vertices[verticesStartIndex + 1].TextureCoordinate[1].Y = underlineCharGeo.SliceIndex;
				vertices[verticesStartIndex + 2].TextureCoordinate[1].Y = underlineCharGeo.SliceIndex;
				vertices[verticesStartIndex + 3].TextureCoordinate[1].Y = underlineCharGeo.SliceIndex;
			}

			addVertCount += 4;
		}
		if (richTextProperty.Strikethrough)
		{
			vertices[verticesStartIndex + addVertCount].TextureCoordinate[0] = strikethroughCharGeo.GetUV0();
			vertices[verticesStartIndex + addVertCount + 1].TextureCoordinate[0] = strikethroughCharGeo.GetUV1();
			vertices[verticesStartIndex + addVertCount + 2].TextureCoordinate[0] = strikethroughCharGeo.GetUV2();
			vertices[verticesStartIndex + addVertCount + 3].TextureCoordinate[0] = strikethroughCharGeo.GetUV3();

			//font scale
			{
				vertices[verticesStartIndex + addVertCount].TextureCoordinate[2] = FVector2f(tempFontScale, 0);
				vertices[verticesStartIndex + addVertCount + 1].TextureCoordinate[2] = FVector2f(tempFontScale, 0);
				vertices[verticesStartIndex + addVertCount + 2].TextureCoordinate[2] = FVector2f(tempFontScale, 0);
				vertices[verticesStartIndex + addVertCount + 3].TextureCoordinate[2] = FVector2f(tempFontScale, 0);
			}
			//slice of texture array
			{
				vertices[verticesStartIndex].TextureCoordinate[1].Y = strikethroughCharGeo.SliceIndex;
				vertices[verticesStartIndex + 1].TextureCoordinate[1].Y = strikethroughCharGeo.SliceIndex;
				vertices[verticesStartIndex + 2].TextureCoordinate[1].Y = strikethroughCharGeo.SliceIndex;
				vertices[verticesStartIndex + 3].TextureCoordinate[1].Y = strikethroughCharGeo.SliceIndex;
			}

			addVertCount += 4;
		}
	}
	//color
	{
		int addVertCount = 0;
		{
			vertices[verticesStartIndex].Color = richTextProperty.Color;
			vertices[verticesStartIndex + 1].Color = richTextProperty.Color;
			vertices[verticesStartIndex + 2].Color = richTextProperty.Color;
			vertices[verticesStartIndex + 3].Color = richTextProperty.Color;

			addVertCount = 4;
		}
		if (richTextProperty.Underline)
		{
			vertices[verticesStartIndex + addVertCount].Color = richTextProperty.Color;
			vertices[verticesStartIndex + addVertCount + 1].Color = richTextProperty.Color;
			vertices[verticesStartIndex + addVertCount + 2].Color = richTextProperty.Color;
			vertices[verticesStartIndex + addVertCount + 3].Color = richTextProperty.Color;

			addVertCount += 4;
		}
		if (richTextProperty.Strikethrough)
		{
			vertices[verticesStartIndex + addVertCount].Color = richTextProperty.Color;
			vertices[verticesStartIndex + addVertCount + 1].Color = richTextProperty.Color;
			vertices[verticesStartIndex + addVertCount + 2].Color = richTextProperty.Color;
			vertices[verticesStartIndex + addVertCount + 3].Color = richTextProperty.Color;

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

		if (richTextProperty.Underline)
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
		if (richTextProperty.Strikethrough)
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
void ULexUIFontData_DistanceField::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	ReloadFont();
}
#endif

void ULexUIFontData_DistanceField::PostInitProperties()
{
	Super::PostInitProperties();
}
#undef LOCTEXT_NAMESPACE
