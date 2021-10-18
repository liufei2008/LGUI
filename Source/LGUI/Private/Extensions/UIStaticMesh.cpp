// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Extensions/UIStaticMesh.h"
#include "Core/UIGeometry.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Engine/StaticMesh.h"
#include "Engine/Engine.h"
#include "StaticMeshResources.h"
#include "Rendering/ColorVertexBuffer.h"
#include "LGUI.h"
#include "Core/LGUIMesh/LGUIMeshComponent.h"


UUIStaticMesh::UUIStaticMesh(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

#define ONE_DIVIDE_255 0.0039215686274509803921568627451f

bool UUIStaticMesh::CanCreateGeometry()
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
void UUIStaticMesh::UpdateGeometry()
{
	if (GetIsUIActiveInHierarchy() == false)return;
	if (!CheckRenderCanvas())return;
	if (!IsValid(mesh))return;

	Super::UpdateGeometry();
	if (!drawcall.IsValid()//not add to render yet
		)
	{
		RenderCanvas->AddUIRenderable(this);
	}

	if (MeshSection.IsValid())
	{
		if (cacheForThisUpdate_ColorChanged)
		{
			UpdateMeshColor();
		}
		if (cacheForThisUpdate_LocalVertexPositionChanged || cacheForThisUpdate_LayoutChanged)
		{
			UpdateMeshTransform();
		}
	}
}

void UUIStaticMesh::UpdateMeshColor()
{
	if (vertexColorType == UIStaticMeshVertexColorType::NotAffectByUIColor)return;
	FStaticMeshVertexBuffers& vertexBuffers = mesh->GetRenderData()->LODResources[0].VertexBuffers;
	FRawStaticIndexBuffer& indicesBuffers = mesh->GetRenderData()->LODResources[0].IndexBuffer;
	auto numVertices = (int32)vertexBuffers.PositionVertexBuffer.GetNumVertices();
	auto numIndices = indicesBuffers.GetNumIndices();
	if (numVertices > 0 && numIndices > 0)
	{
		FPositionVertexBuffer& positionBuffer = vertexBuffers.PositionVertexBuffer;
		FStaticMeshVertexBuffer& staticMeshVertexBuffer = vertexBuffers.StaticMeshVertexBuffer;
		{
			auto& VertexData = MeshSection.Pin()->vertices;

			VertexData.SetNumUninitialized(numVertices);
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
			}
		}
	}
	UIMesh->UpdateMeshSection(MeshSection.Pin(), true, this->GetRenderCanvas()->GetActualAdditionalShaderChannelFlags());
}
void UUIStaticMesh::CreateGeometry()
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
			auto& VertexData = MeshSection.Pin()->vertices;

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
			auto& IndexData = MeshSection.Pin()->triangles;
			IndexData.SetNumUninitialized(numIndices);
			for (int i = 0; i < numIndices; i++)
			{
				IndexData[i] = indicesBuffers.GetIndex(i);
			}
		}
	}
	UIMesh->CreateMeshSection(MeshSection.Pin());

	if (IsValid(replaceMat))
	{
		UIMesh->SetMeshSectionMaterial(MeshSection.Pin(), replaceMat);
	}
	else
	{
		UIMesh->SetMeshSectionMaterial(MeshSection.Pin(), mesh->GetMaterial(0));
	}

	UpdateMeshTransform();
}

void UUIStaticMesh::UpdateMeshTransform()
{
	FTransform itemToCanvasTf;
	auto canvasUIItem = RenderCanvas->GetUIItem();
	auto inverseCanvasTf = canvasUIItem->GetComponentTransform().Inverse();
	const auto& itemTf = this->GetComponentTransform();
	FTransform::Multiply(&itemToCanvasTf, &itemTf, &inverseCanvasTf);


	FStaticMeshVertexBuffers& vertexBuffers = mesh->RenderData->LODResources[0].VertexBuffers;
	auto numVertices = (int32)vertexBuffers.PositionVertexBuffer.GetNumVertices();
	FPositionVertexBuffer& positionBuffer = vertexBuffers.PositionVertexBuffer;
	FStaticMeshVertexBuffer& staticMeshVertexBuffer = vertexBuffers.StaticMeshVertexBuffer;
	{
		auto& VertexData = MeshSection.Pin()->vertices;
		VertexData.SetNumUninitialized(numVertices);
		bool needNormal = RenderCanvas->GetRequireNormal();
		bool needTangent = RenderCanvas->GetRequireTangent();
		for (int i = 0; i < numVertices; i++)
		{
			auto& vert = VertexData[i];
			vert.Position = positionBuffer.VertexPosition(i);
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


	auto& vertices = MeshSection.Pin()->vertices;
	auto vertexCount = vertices.Num();
	for (int i = 0; i < vertexCount; i++)
	{
		vertices[i].Position = itemToCanvasTf.TransformPosition(vertices[i].Position);
	}
	if (RenderCanvas->GetRequireNormal())
	{
		for (int i = 0; i < vertexCount; i++)
		{
			vertices[i].TangentZ = itemToCanvasTf.TransformVector(vertices[i].TangentZ.ToFVector());
			vertices[i].TangentZ.Vector.W = -127;
		}
	}
	if (RenderCanvas->GetRequireTangent())
	{
		for (int i = 0; i < vertexCount; i++)
		{
			vertices[i].TangentX = itemToCanvasTf.TransformVector(vertices[i].TangentX.ToFVector());
		}
	}


	UIMesh->UpdateMeshSection(MeshSection.Pin(), true, RenderCanvas->GetActualAdditionalShaderChannelFlags());
}

#if WITH_EDITOR
void UUIStaticMesh::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.Property)
	{
		auto PropName = Property->GetFName();
		if (
			PropName == GET_MEMBER_NAME_CHECKED(UUIStaticMesh, mesh)
			|| PropName == GET_MEMBER_NAME_CHECKED(UUIStaticMesh, vertexColorType)
			)
		{
			if (IsValid(mesh))
			{
				if (MeshSection.IsValid())
				{
					if (CanCreateGeometry())
					{
						CreateGeometry();
					}
				}
			}
			else
			{
				if (drawcall.IsValid())
				{
					RenderCanvas->RemoveUIRenderable(this);
				}
			}
		}
		else if (PropName == GET_MEMBER_NAME_CHECKED(UUIStaticMesh, replaceMat))
		{
			if (MeshSection.IsValid())
			{
				if (CanCreateGeometry())
				{
					CreateGeometry();
				}
			}
		}
	}
}
#endif

void UUIStaticMesh::SetMeshData(TWeakObjectPtr<ULGUIMeshComponent> InUIMesh, TWeakPtr<FLGUIMeshSection> InMeshSection)
{
	Super::SetMeshData(InUIMesh, InMeshSection);
	if (MeshSection.IsValid())
	{
		if (CanCreateGeometry())
		{
			CreateGeometry();
		}
	}
}

void UUIStaticMesh::SetMesh(UStaticMesh* value)
{
	if (mesh != value)
	{
		mesh = value;
		if (IsValid(mesh))
		{
			if (MeshSection.IsValid())
			{
				if (CanCreateGeometry())
				{
					CreateGeometry();
				}
			}
		}
		else
		{
			if (drawcall.IsValid())
			{
				RenderCanvas->RemoveUIRenderable(this);
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
