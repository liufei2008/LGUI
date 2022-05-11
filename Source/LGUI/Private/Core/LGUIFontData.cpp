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
		hasKerning = FT_HAS_KERNING(face) != 0;
		if (usePackingTag)
		{
			if (packingAtlasData == nullptr)
			{
				packingAtlasData = ULGUIAtlasManager::FindOrAdd(packingTag);
				packingAtlasTextureExpandDelegateHandle = packingAtlasData->OnTextureSizeExpanded.AddUObject(this, &ULGUIFontData::ApplyPackingAtlasTextureExpand);
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
}
UTexture2D* ULGUIFontData::GetFontTexture()
{
	return texture;
}
void ULGUIFontData::InitFont()
{
	InitFreeType();
}
void ULGUIFontData::PushCharData(
	TCHAR charCode, const FVector2D& lineOffset, const FVector2D& fontSpace, const FUITextCharGeometry& charGeo,
	bool bold, float boldSize, bool italic, float italicSlop, const FColor& color,
	int verticesStartIndex, int indicesStartIndex,
	int& outAdditionalVerticesCount, int& outAdditionalIndicesCount,
	TArray<FLGUIOriginVertexData>& originVertices, TArray<FDynamicMeshVertex>& vertices, TArray<FLGUIIndexType>& triangleIndices
)
{
	outAdditionalVerticesCount = 4;
	outAdditionalIndicesCount = 6;
	if (bold)
	{
		outAdditionalVerticesCount = 16;
		outAdditionalIndicesCount = 24;
	}

	int32 newVerticesCount = verticesStartIndex + outAdditionalVerticesCount;
	UIGeometry::LGUIGeometrySetArrayNum(originVertices, newVerticesCount);
	UIGeometry::LGUIGeometrySetArrayNum(vertices, newVerticesCount);

	int32 newIndicesCount = indicesStartIndex + outAdditionalIndicesCount;
	UIGeometry::LGUIGeometrySetArrayNum(triangleIndices, newIndicesCount);

	float offsetX = lineOffset.X + charGeo.xoffset;
	float offsetY = lineOffset.Y + charGeo.yoffset;

	//position
	{
		if (bold)
		{
			float x = offsetX;
			float y = offsetY - charGeo.geoHeight;
			auto vert0 = FVector(0, x, y);
			x = charGeo.geoWidth + offsetX;
			auto vert1 = FVector(0, x, y);
			x = offsetX;
			y = offsetY;
			auto vert2 = FVector(0, x, y);
			x = charGeo.geoWidth + offsetX;
			auto vert3 = FVector(0, x, y);
			if (italic)
			{
				auto vert01ItalicOffset = (charGeo.geoHeight - charGeo.yoffset) * italicSlop;
				vert0.Y -= vert01ItalicOffset;
				vert1.Y -= vert01ItalicOffset;
				auto vert23ItalicOffset = charGeo.yoffset * italicSlop;
				vert2.Y += vert23ItalicOffset;
				vert3.Y += vert23ItalicOffset;
			}
			//bold left
			originVertices[verticesStartIndex].Position = vert0 + FVector(0, -boldSize, 0);
			originVertices[verticesStartIndex + 1].Position = vert1 + FVector(0, -boldSize, 0);
			originVertices[verticesStartIndex + 2].Position = vert2 + FVector(0, -boldSize, 0);
			originVertices[verticesStartIndex + 3].Position = vert3 + FVector(0, -boldSize, 0);
			//bold right
			originVertices[verticesStartIndex + 4].Position = vert0 + FVector(0, boldSize, 0);
			originVertices[verticesStartIndex + 5].Position = vert1 + FVector(0, boldSize, 0);
			originVertices[verticesStartIndex + 6].Position = vert2 + FVector(0, boldSize, 0);
			originVertices[verticesStartIndex + 7].Position = vert3 + FVector(0, boldSize, 0);
			//bold top
			originVertices[verticesStartIndex + 8].Position = vert0 + FVector(0, 0, boldSize);
			originVertices[verticesStartIndex + 9].Position = vert1 + FVector(0, 0, boldSize);
			originVertices[verticesStartIndex + 10].Position = vert2 + FVector(0, 0, boldSize);
			originVertices[verticesStartIndex + 11].Position = vert3 + FVector(0, 0, boldSize);
			//bold bottom
			originVertices[verticesStartIndex + 12].Position = vert0 + FVector(0, 0, -boldSize);
			originVertices[verticesStartIndex + 13].Position = vert1 + FVector(0, 0, -boldSize);
			originVertices[verticesStartIndex + 14].Position = vert2 + FVector(0, 0, -boldSize);
			originVertices[verticesStartIndex + 15].Position = vert3 + FVector(0, 0, -boldSize);
		}
		else
		{
			float x = offsetX;
			float y = offsetY - charGeo.geoHeight;
			auto& vert0 = originVertices[verticesStartIndex].Position;
			vert0 = FVector(0, x, y);
			x = charGeo.geoWidth + offsetX;
			auto& vert1 = originVertices[verticesStartIndex + 1].Position;
			vert1 = FVector(0, x, y);
			x = offsetX;
			y = offsetY;
			auto& vert2 = originVertices[verticesStartIndex + 2].Position;
			vert2 = FVector(0, x, y);
			x = charGeo.geoWidth + offsetX;
			auto& vert3 = originVertices[verticesStartIndex + 3].Position;
			vert3 = FVector(0, x, y);
			if (italic)
			{
				auto vert01ItalicOffset = (charGeo.geoHeight - charGeo.yoffset) * italicSlop;
				vert0.Y -= vert01ItalicOffset;
				vert1.Y -= vert01ItalicOffset;
				auto vert23ItalicOffset = charGeo.yoffset * italicSlop;
				vert2.Y += vert23ItalicOffset;
				vert3.Y += vert23ItalicOffset;
			}
		}
	}
	//uv
	{
		if (bold)
		{
			//bold left
			vertices[verticesStartIndex].TextureCoordinate[0] = charGeo.uv0;
			vertices[verticesStartIndex + 1].TextureCoordinate[0] = charGeo.uv1;
			vertices[verticesStartIndex + 2].TextureCoordinate[0] = charGeo.uv2;
			vertices[verticesStartIndex + 3].TextureCoordinate[0] = charGeo.uv3;
			//bold right
			vertices[verticesStartIndex + 4].TextureCoordinate[0] = charGeo.uv0;
			vertices[verticesStartIndex + 5].TextureCoordinate[0] = charGeo.uv1;
			vertices[verticesStartIndex + 6].TextureCoordinate[0] = charGeo.uv2;
			vertices[verticesStartIndex + 7].TextureCoordinate[0] = charGeo.uv3;
			//bold top
			vertices[verticesStartIndex + 8].TextureCoordinate[0] = charGeo.uv0;
			vertices[verticesStartIndex + 9].TextureCoordinate[0] = charGeo.uv1;
			vertices[verticesStartIndex + 10].TextureCoordinate[0] = charGeo.uv2;
			vertices[verticesStartIndex + 11].TextureCoordinate[0] = charGeo.uv3;
			//bold bottom
			vertices[verticesStartIndex + 12].TextureCoordinate[0] = charGeo.uv0;
			vertices[verticesStartIndex + 13].TextureCoordinate[0] = charGeo.uv1;
			vertices[verticesStartIndex + 14].TextureCoordinate[0] = charGeo.uv2;
			vertices[verticesStartIndex + 15].TextureCoordinate[0] = charGeo.uv3;
		}
		else
		{
			vertices[verticesStartIndex].TextureCoordinate[0] = charGeo.uv0;
			vertices[verticesStartIndex + 1].TextureCoordinate[0] = charGeo.uv1;
			vertices[verticesStartIndex + 2].TextureCoordinate[0] = charGeo.uv2;
			vertices[verticesStartIndex + 3].TextureCoordinate[0] = charGeo.uv3;
		}
	}
	//color
	{
		if (bold)
		{
			//bold left
			vertices[verticesStartIndex].Color = color;
			vertices[verticesStartIndex + 1].Color = color;
			vertices[verticesStartIndex + 2].Color = color;
			vertices[verticesStartIndex + 3].Color = color;
			//bold right
			vertices[verticesStartIndex + 4].Color = color;
			vertices[verticesStartIndex + 5].Color = color;
			vertices[verticesStartIndex + 6].Color = color;
			vertices[verticesStartIndex + 7].Color = color;
			//bold top
			vertices[verticesStartIndex + 8].Color = color;
			vertices[verticesStartIndex + 9].Color = color;
			vertices[verticesStartIndex + 10].Color = color;
			vertices[verticesStartIndex + 11].Color = color;
			//bold bottom
			vertices[verticesStartIndex + 12].Color = color;
			vertices[verticesStartIndex + 13].Color = color;
			vertices[verticesStartIndex + 14].Color = color;
			vertices[verticesStartIndex + 15].Color = color;
		}
		else
		{
			vertices[verticesStartIndex].Color = color;
			vertices[verticesStartIndex + 1].Color = color;
			vertices[verticesStartIndex + 2].Color = color;
			vertices[verticesStartIndex + 3].Color = color;
		}
	}
	//triangle
	{
		if (bold)
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
		}
		else
		{
			triangleIndices[indicesStartIndex] = verticesStartIndex;
			triangleIndices[indicesStartIndex + 1] = verticesStartIndex + 3;
			triangleIndices[indicesStartIndex + 2] = verticesStartIndex + 2;
			triangleIndices[indicesStartIndex + 3] = verticesStartIndex;
			triangleIndices[indicesStartIndex + 4] = verticesStartIndex + 1;
			triangleIndices[indicesStartIndex + 5] = verticesStartIndex + 3;
		}
	}
}
void ULGUIFontData::PushCharData(
	TCHAR charCode, const FVector2D& inLineOffset, const FVector2D& fontSpace, const FUITextCharGeometry& charGeo,
	float boldSize, float italicSlop, const LGUIRichTextParser::RichTextParseResult& richTextProperty,
	int verticesStartIndex, int indicesStartIndex,
	int& outAdditionalVerticesCount, int& outAdditionalIndicesCount,
	TArray<FLGUIOriginVertexData>& originVertices, TArray<FDynamicMeshVertex>& vertices, TArray<FLGUIIndexType>& triangleIndices
)
{
	auto GetUnderlineOrStrikethroughCharGeo = [&](TCHAR charCode, int overrideFontSize)
	{
		FUITextCharGeometry charGeo;
		{
			auto charData = this->GetCharData(charCode, (uint16)overrideFontSize);
			charGeo.geoHeight = charData.height;
			charGeo.xoffset = charData.xoffset;
			charGeo.yoffset = charData.yoffset + this->GetVerticalOffset(overrideFontSize);

			float uvX = (charData.uv3X - charData.uv0X) * 0.5f + charData.uv0X;
			charGeo.uv0 = charGeo.uv1 = FVector2D(uvX, charData.uv0Y);
			charGeo.uv2 = charGeo.uv3 = FVector2D(uvX, charData.uv3Y);
		}
		return charGeo;
	};

	outAdditionalVerticesCount = 4;
	outAdditionalIndicesCount = 6;

	FUITextCharGeometry underlineCharGeo;
	FUITextCharGeometry strikethroughCharGeo;
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
	float offsetX = lineOffset.X + charGeo.xoffset;
	float offsetY = lineOffset.Y + charGeo.yoffset;

	float charWidth = charGeo.xadvance + fontSpace.X;
	//position
	{
		float x, y;

		int addVertCount = 0;
		if (richTextProperty.bold)
		{
			x = offsetX;
			y = offsetY - charGeo.geoHeight;
			auto vert0 = FVector(0, x, y);
			x = charGeo.geoWidth + offsetX;
			auto vert1 = FVector(0, x, y);
			x = offsetX;
			y = offsetY;
			auto vert2 = FVector(0, x, y);
			x = charGeo.geoWidth + offsetX;
			auto vert3 = FVector(0, x, y);
			if (richTextProperty.italic)
			{
				auto vert01ItalicOffset = (charGeo.geoHeight - charGeo.yoffset) * italicSlop;
				vert0.Y -= vert01ItalicOffset;
				vert1.Y -= vert01ItalicOffset;
				auto vert23ItalicOffset = charGeo.yoffset * italicSlop;
				vert2.Y += vert23ItalicOffset;
				vert3.Y += vert23ItalicOffset;
			}
			//bold left
			originVertices[verticesStartIndex].Position = vert0 + FVector(0, -boldSize, 0);
			originVertices[verticesStartIndex + 1].Position = vert1 + FVector(0, -boldSize, 0);
			originVertices[verticesStartIndex + 2].Position = vert2 + FVector(0, -boldSize, 0);
			originVertices[verticesStartIndex + 3].Position = vert3 + FVector(0, -boldSize, 0);
			//bold right
			originVertices[verticesStartIndex + 4].Position = vert0 + FVector(0, boldSize, 0);
			originVertices[verticesStartIndex + 5].Position = vert1 + FVector(0, boldSize, 0);
			originVertices[verticesStartIndex + 6].Position = vert2 + FVector(0, boldSize, 0);
			originVertices[verticesStartIndex + 7].Position = vert3 + FVector(0, boldSize, 0);
			//bold top
			originVertices[verticesStartIndex + 8].Position = vert0 + FVector(0, 0, boldSize);
			originVertices[verticesStartIndex + 9].Position = vert1 + FVector(0, 0, boldSize);
			originVertices[verticesStartIndex + 10].Position = vert2 + FVector(0, 0, boldSize);
			originVertices[verticesStartIndex + 11].Position = vert3 + FVector(0, 0, boldSize);
			//bold bottom
			originVertices[verticesStartIndex + 12].Position = vert0 + FVector(0, 0, -boldSize);
			originVertices[verticesStartIndex + 13].Position = vert1 + FVector(0, 0, -boldSize);
			originVertices[verticesStartIndex + 14].Position = vert2 + FVector(0, 0, -boldSize);
			originVertices[verticesStartIndex + 15].Position = vert3 + FVector(0, 0, -boldSize);

			addVertCount = 16;
		}
		else
		{
			x = offsetX;
			y = offsetY - charGeo.geoHeight;
			auto& vert0 = originVertices[verticesStartIndex].Position;
			vert0 = FVector(0, x, y);
			x = charGeo.geoWidth + offsetX;
			auto& vert1 = originVertices[verticesStartIndex + 1].Position;
			vert1 = FVector(0, x, y);
			x = offsetX;
			y = offsetY;
			auto& vert2 = originVertices[verticesStartIndex + 2].Position;
			vert2 = FVector(0, x, y);
			x = charGeo.geoWidth + offsetX;
			auto& vert3 = originVertices[verticesStartIndex + 3].Position;
			vert3 = FVector(0, x, y);
			if (richTextProperty.italic)
			{
				auto vert01ItalicOffset = (charGeo.geoHeight - charGeo.yoffset) * italicSlop;
				vert0.Y -= vert01ItalicOffset;
				vert1.Y -= vert01ItalicOffset;
				auto vert23ItalicOffset = charGeo.yoffset * italicSlop;
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
			y = offsetY - underlineCharGeo.geoHeight;
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
			y = offsetY - strikethroughCharGeo.geoHeight;
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
		if (richTextProperty.bold)
		{
			//bold left
			vertices[verticesStartIndex].TextureCoordinate[0] = charGeo.uv0;
			vertices[verticesStartIndex + 1].TextureCoordinate[0] = charGeo.uv1;
			vertices[verticesStartIndex + 2].TextureCoordinate[0] = charGeo.uv2;
			vertices[verticesStartIndex + 3].TextureCoordinate[0] = charGeo.uv3;
			//bold right
			vertices[verticesStartIndex + 4].TextureCoordinate[0] = charGeo.uv0;
			vertices[verticesStartIndex + 5].TextureCoordinate[0] = charGeo.uv1;
			vertices[verticesStartIndex + 6].TextureCoordinate[0] = charGeo.uv2;
			vertices[verticesStartIndex + 7].TextureCoordinate[0] = charGeo.uv3;
			//bold top
			vertices[verticesStartIndex + 8].TextureCoordinate[0] = charGeo.uv0;
			vertices[verticesStartIndex + 9].TextureCoordinate[0] = charGeo.uv1;
			vertices[verticesStartIndex + 10].TextureCoordinate[0] = charGeo.uv2;
			vertices[verticesStartIndex + 11].TextureCoordinate[0] = charGeo.uv3;
			//bold bottom
			vertices[verticesStartIndex + 12].TextureCoordinate[0] = charGeo.uv0;
			vertices[verticesStartIndex + 13].TextureCoordinate[0] = charGeo.uv1;
			vertices[verticesStartIndex + 14].TextureCoordinate[0] = charGeo.uv2;
			vertices[verticesStartIndex + 15].TextureCoordinate[0] = charGeo.uv3;

			addVertCount = 16;
		}
		else
		{
			vertices[verticesStartIndex].TextureCoordinate[0] = charGeo.uv0;
			vertices[verticesStartIndex + 1].TextureCoordinate[0] = charGeo.uv1;
			vertices[verticesStartIndex + 2].TextureCoordinate[0] = charGeo.uv2;
			vertices[verticesStartIndex + 3].TextureCoordinate[0] = charGeo.uv3;

			addVertCount = 4;
		}
		if (richTextProperty.underline)
		{
			vertices[verticesStartIndex + addVertCount].TextureCoordinate[0] = underlineCharGeo.uv0;
			vertices[verticesStartIndex + addVertCount + 1].TextureCoordinate[0] = underlineCharGeo.uv1;
			vertices[verticesStartIndex + addVertCount + 2].TextureCoordinate[0] = underlineCharGeo.uv2;
			vertices[verticesStartIndex + addVertCount + 3].TextureCoordinate[0] = underlineCharGeo.uv3;

			addVertCount += 4;
		}
		if (richTextProperty.strikethrough)
		{
			vertices[verticesStartIndex + addVertCount].TextureCoordinate[0] = strikethroughCharGeo.uv0;
			vertices[verticesStartIndex + addVertCount + 1].TextureCoordinate[0] = strikethroughCharGeo.uv1;
			vertices[verticesStartIndex + addVertCount + 2].TextureCoordinate[0] = strikethroughCharGeo.uv2;
			vertices[verticesStartIndex + addVertCount + 3].TextureCoordinate[0] = strikethroughCharGeo.uv3;

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

int16 ULGUIFontData::GetKerning(const TCHAR& leftCharIndex, const TCHAR& rightCharIndex, const uint16& charSize)
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
uint16 ULGUIFontData::GetLineHeight(const uint16& fontSize)
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
float ULGUIFontData::GetVerticalOffset(const uint16& fontSize)
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
			UE_LOG(LGUI, Error, TEXT("[PushCharIntoFont] Font is not initialized"));
			cacheCharData = FLGUICharData();
			return &cacheCharData;
		}
	}

	auto error = FT_Set_Pixel_Sizes(face, 0, charSize);
	if (error)
	{
		UE_LOG(LGUI, Error, TEXT("[PushCharIntoFont] FT_Set_Pixel_Sizes error:%s"), ANSI_TO_TCHAR(GetErrorMessage(error)));
		return &cacheCharData;
	}
	FT_GlyphSlot slot = face->glyph;
	error = FT_Load_Glyph(face, FT_Get_Char_Index(face, charIndex), FT_LOAD_DEFAULT);
	if (error)
	{
		UE_LOG(LGUI, Error, TEXT("[PushCharIntoFont] FT_Load_Glyph error:%s"), ANSI_TO_TCHAR(GetErrorMessage(error)));
		return &cacheCharData;
	}
	error = FT_Render_Glyph(face->glyph, FT_Render_Mode::FT_RENDER_MODE_NORMAL);
	if (error)
	{
		UE_LOG(LGUI, Error, TEXT("[PushCharIntoFont] FT_Render_Glyph error:%s"), ANSI_TO_TCHAR(GetErrorMessage(error)));
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
		//InSlot->bitmap_left equals (InSlot->metrics.horiBearingX >> 6), InSlot->bitmap_top equals (InSlot->metrics.horiBearingY >> 6)
		cacheCharData.xoffset = InSlot->bitmap_left - SPACE_NEED_EXPEND;
		cacheCharData.yoffset = InSlot->bitmap_top + SPACE_NEED_EXPEND;
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
			|| PropertyName == GET_MEMBER_NAME_CHECKED(ULGUIFontData, packingTag)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(ULGUIFontData, lineHeightType)
			)
		{
			ReloadFont();
		}
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
