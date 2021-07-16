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
}

bool UUIStaticMesh::CanCreateGeometry()
{
	if (IsValid(mesh))
	{
		if (mesh->RenderData.IsValid())
		{
			if (mesh->RenderData->LODResources.Num() > 0)
			{
				FStaticMeshVertexBuffers& vertexBuffers = mesh->RenderData->LODResources[0].VertexBuffers;
				FRawStaticIndexBuffer& indicesBuffers = mesh->RenderData->LODResources[0].IndexBuffer;
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
void UUIStaticMesh::CreateGeometry()
{
	FStaticMeshVertexBuffers& vertexBuffers = mesh->RenderData->LODResources[0].VertexBuffers;
	FRawStaticIndexBuffer& indicesBuffers = mesh->RenderData->LODResources[0].IndexBuffer;
	auto numVertices = (int32)vertexBuffers.PositionVertexBuffer.GetNumVertices();
	auto numIndices = indicesBuffers.GetNumIndices();
	if (numVertices > 0 && numIndices > 0)
	{
		FPositionVertexBuffer& positionBuffer = vertexBuffers.PositionVertexBuffer;
		FStaticMeshVertexBuffer& staticMeshVertexBuffer = vertexBuffers.StaticMeshVertexBuffer;
		{
			auto& VertexData = UIDrawcallMesh->MeshSection.vertices;

			VertexData.SetNumUninitialized(numVertices);
			bool needNormal = RenderCanvas->GetRequireNormal();
			bool needTangent = RenderCanvas->GetRequireTangent();
			auto numTexCoords = staticMeshVertexBuffer.GetNumTexCoords();
			bool needUV1 = RenderCanvas->GetRequireUV1();
			bool needUV2 = RenderCanvas->GetRequireUV2();
			bool needUV3 = RenderCanvas->GetRequireUV3();
			auto tempVertexColorType = vertexColorType;
			if (vertexBuffers.ColorVertexBuffer.VertexBufferRHI == nullptr)
			{
				tempVertexColorType = UIStaticMeshVertexColorType::ReplaceByUIColor;
			}

			for (int i = 0; i < numVertices; i++)
			{
				auto& vert = VertexData[i];
				vert.Position = positionBuffer.VertexPosition(i);
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
					vert.TangentZ = staticMeshVertexBuffer.VertexTangentZ(i);
				}
				if (needTangent)
				{
					vert.TangentX = staticMeshVertexBuffer.VertexTangentX(i);
				}
			}
		}
		{
			auto& IndexData = UIDrawcallMesh->MeshSection.triangles;
			IndexData.SetNumUninitialized(numIndices);
			for (int i = 0; i < numIndices; i++)
			{
				IndexData[i] = indicesBuffers.GetIndex(i);
			}
		}
	}
	UIDrawcallMesh->GenerateOrUpdateMesh();
	if (Material.IsValid())
	{
		UIDrawcallMesh->SetMaterial(0, Material.Get());
	}
	else
	{
		UIDrawcallMesh->SetMaterial(0, mesh->GetMaterial(0));
	}
}

void UUIStaticMesh::SetDrawcallMesh(UUIDrawcallMesh* InUIDrawcallMesh)
{
	Super::SetDrawcallMesh(InUIDrawcallMesh);
	if (CanCreateGeometry())
	{
		CreateGeometry();
	}
}

void UUIStaticMesh::SetMesh(UStaticMesh* value)
{
	if (mesh != value)
	{
		mesh = value;
		if (UIDrawcallMesh.IsValid())
		{
			if (CanCreateGeometry())
			{
				CreateGeometry();
			}
		}
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
