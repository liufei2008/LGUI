// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Extensions/UIStaticMesh.h"
#include "Core/UIGeometry.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Engine/StaticMesh.h"
#include "Engine/Engine.h"
#include "StaticMeshResources.h"
#include "Rendering/ColorVertexBuffer.h"
#include "LGUI.h"
#include "Core/LGUIMesh/UIDrawcallMesh.h"


UUIStaticMesh::UUIStaticMesh(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	bIsSelfRender = true;
}

bool UUIStaticMesh::HaveDataToCreateGeometry()
{
	if (IsValid(mesh))
	{
		if (mesh->GetRenderData() != nullptr)
		{
			if (mesh->GetRenderData()->LODResources.Num() > 0)
			{
				FStaticMeshVertexBuffers& vertexBuffers = mesh->GetRenderData()->LODResources[0].VertexBuffers;
				FRawStaticIndexBuffer& indicesBuffers = mesh->GetRenderData()->LODResources[0].IndexBuffer;
				auto numVertices = vertexBuffers.PositionVertexBuffer.GetNumVertices();
				auto numIndices = indicesBuffers.GetNumIndices();
				if (numVertices > 0 && numIndices > 0)
				{
					return true;
				}
			}
		}
	}
	return false;
}
#define ONE_DIVIDE_255 0.0039215686274509803921568627451f
void UUIStaticMesh::OnCreateGeometry()
{
	FStaticMeshVertexBuffers& vertexBuffers = mesh->GetRenderData()->LODResources[0].VertexBuffers;
	FRawStaticIndexBuffer& indicesBuffers = mesh->GetRenderData()->LODResources[0].IndexBuffer;
	auto numVertices = (int32)vertexBuffers.PositionVertexBuffer.GetNumVertices();
	auto numIndices = indicesBuffers.GetNumIndices();
	if (numVertices > 0 && numIndices > 0)
	{
		FPositionVertexBuffer& positionBuffer = vertexBuffers.PositionVertexBuffer;
		FStaticMeshVertexBuffer& staticMeshVertexBuffer = vertexBuffers.StaticMeshVertexBuffer;

		{
			geometry->originVerticesCount = numVertices;
			geometry->originPositions.SetNumUninitialized(geometry->originVerticesCount);
			geometry->vertices.SetNumUninitialized(geometry->originVerticesCount);
			bool needNormal = RenderCanvas->GetRequireNormal();
			bool needTangent = RenderCanvas->GetRequireTangent();
			if (needNormal)
			{
				geometry->originNormals.SetNumUninitialized(geometry->originVerticesCount);
			}
			if (needTangent)
			{
				geometry->originTangents.SetNumUninitialized(geometry->originVerticesCount);
			}
			auto numTexCoords = staticMeshVertexBuffer.GetNumTexCoords();
			bool needUV1 = RenderCanvas->GetRequireUV1();
			bool needUV2 = RenderCanvas->GetRequireUV2();
			bool needUV3 = RenderCanvas->GetRequireUV3();
			auto tempVertexColorType = vertexColorType;
			if (vertexBuffers.ColorVertexBuffer.VertexBufferRHI == nullptr)
			{
				tempVertexColorType = UIStaticMeshVertexColorType::ReplaceByUIColor;
			}
			FMemory::Memcpy(geometry->originPositions.GetData(), positionBuffer.GetVertexData(), numVertices * positionBuffer.GetStride());
			for (int i = 0; i < numVertices; i++)
			{
				auto& vert = geometry->vertices[i];
				switch (tempVertexColorType)
				{
				case UIStaticMeshVertexColorType::MultiplyWithUIColor:
				{
				vert.Color = vertexBuffers.ColorVertexBuffer.VertexColor(i);
				auto uiFinalColor = GetFinalColor();
				vert.Color.R = (uint8)((float)vert.Color.R * uiFinalColor.R * ONE_DIVIDE_255);
				vert.Color.G = (uint8)((float)vert.Color.G * uiFinalColor.G * ONE_DIVIDE_255);
				vert.Color.B = (uint8)((float)vert.Color.B * uiFinalColor.B * ONE_DIVIDE_255);
				vert.Color.A = (uint8)((float)vert.Color.A * uiFinalColor.A * ONE_DIVIDE_255);
				}
				break;
				case UIStaticMeshVertexColorType::NotAffectByUIColor:
				{
					vert.Color = vertexBuffers.ColorVertexBuffer.VertexColor(i);
				}
				break;
				case UIStaticMeshVertexColorType::ReplaceByUIColor:
				{
					vert.Color = GetFinalColor();
				}
				break;
				}

				if (numTexCoords >= 1)
				{
					vert.TextureCoordinate[0] = staticMeshVertexBuffer.GetVertexUV(i, 0);
				}
				if (needUV1 && numTexCoords >= 2)
				{
					vert.TextureCoordinate[1] = staticMeshVertexBuffer.GetVertexUV(i, 1);
				}
				if (needUV2 && numTexCoords >= 3)
				{
					vert.TextureCoordinate[2] = staticMeshVertexBuffer.GetVertexUV(i, 2);
				}
				if (needUV3 && numTexCoords >= 4)
				{
					vert.TextureCoordinate[3] = staticMeshVertexBuffer.GetVertexUV(i, 3);
				}

				if (needNormal)
				{
					geometry->originNormals[i] = staticMeshVertexBuffer.VertexTangentZ(i);
				}
				if (needTangent)
				{
					geometry->originTangents[i] = staticMeshVertexBuffer.VertexTangentX(i);
				}
			}
		}
		{
			geometry->originTriangleCount = numIndices;
			geometry->triangles.SetNumUninitialized(numIndices);
			for (int i = 0; i < numIndices; i++)
			{
				geometry->triangles[i] = indicesBuffers.GetIndex(i);
			}
		}
	}
	if (CustomUIMaterial)
	{
		geometry->material = CustomUIMaterial;
	}
	else
	{
		geometry->material = mesh->GetMaterial(0);
	}
}
void UUIStaticMesh::OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	if (InVertexUVChanged)
	{
		//UIGeometry::UpdateUIMeshUV(geometry, (uint8)uvType, startAngle, endAngle, segment, sprite->InitAndGetSpriteInfo());
	}
	if (InVertexColorChanged)
	{
		FStaticMeshVertexBuffers& vertexBuffers = mesh->GetRenderData()->LODResources[0].VertexBuffers;
		auto tempVertexColorType = vertexColorType;
		if (vertexBuffers.ColorVertexBuffer.VertexBufferRHI == nullptr)
		{
			tempVertexColorType = UIStaticMeshVertexColorType::ReplaceByUIColor;
		}

		switch (tempVertexColorType)
		{
		case UIStaticMeshVertexColorType::MultiplyWithUIColor:
		{
			auto numVertices = (int32)vertexBuffers.PositionVertexBuffer.GetNumVertices();
			if (numVertices > 0)
			{
				auto& vertices = geometry->vertices;
				for (int i = 0; i < vertices.Num(); i++)
				{
					auto& vert = vertices[i];
					vert.Color = vertexBuffers.ColorVertexBuffer.VertexColor(i);
					auto uiFinalColor = GetFinalColor();
					vert.Color.R = (uint8)((float)vert.Color.R * uiFinalColor.R * ONE_DIVIDE_255);
					vert.Color.G = (uint8)((float)vert.Color.G * uiFinalColor.G * ONE_DIVIDE_255);
					vert.Color.B = (uint8)((float)vert.Color.B * uiFinalColor.B * ONE_DIVIDE_255);
					vert.Color.A = (uint8)((float)vert.Color.A * uiFinalColor.A * ONE_DIVIDE_255);
				}
			}
		}
		break;
		case UIStaticMeshVertexColorType::NotAffectByUIColor:
		{
			auto numVertices = (int32)vertexBuffers.PositionVertexBuffer.GetNumVertices();
			if (numVertices > 0)
			{
				FPositionVertexBuffer& positionBuffer = vertexBuffers.PositionVertexBuffer;
				FStaticMeshVertexBuffer& staticMeshVertexBuffer = vertexBuffers.StaticMeshVertexBuffer;

				auto& vertices = geometry->vertices;
				for (int i = 0; i < vertices.Num(); i++)
				{
					auto& vert = vertices[i];
					vert.Color = vertexBuffers.ColorVertexBuffer.VertexColor(i);
				}
			}
		}
		break;
		case UIStaticMeshVertexColorType::ReplaceByUIColor:
		{
			UIGeometry::UpdateUIColor(geometry, GetFinalColor());
		}
		break;
		}
	}
	if (InVertexPositionChanged)
	{
		//UIGeometry::UpdateUIMeshVertex(geometry, widget.width, widget.height, widget.pivot, startAngle, endAngle, segment);
	}
}

void UUIStaticMesh::UpdateSelfRenderMaterial(bool textureChange, bool materialChange)
{
	if (uiMesh.IsValid())
	{
		if (materialChange)
		{
			UMaterialInterface* SrcMaterial = CustomUIMaterial;
			if (!IsValid(SrcMaterial))
			{
				SrcMaterial = mesh->GetMaterial(0);
				if (!IsValid(SrcMaterial))
				{
					SrcMaterial = LoadObject<UMaterialInterface>(NULL, TEXT("/LGUI/LGUI_Standard"));
				}
			}
			uiMesh->SetMaterial(0, SrcMaterial);
		}
	}
}

void UUIStaticMesh::SetMesh(UStaticMesh* value)
{
	if (mesh != value)
	{
		mesh = value;
		MarkTriangleDirty();
		MarkMaterialDirty();
	}
}
void UUIStaticMesh::SetVertexColorType(UIStaticMeshVertexColorType value)
{
	if (vertexColorType != value)
	{
		vertexColorType = value;
		MarkColorDirty();
	}
}


AUIStaticMeshActor::AUIStaticMeshActor()
{
	PrimaryActorTick.bCanEverTick = false;

	UIStaticMesh = CreateDefaultSubobject<UUIStaticMesh>(TEXT("UIStaticMeshComponent"));
	RootComponent = UIStaticMesh;
}
