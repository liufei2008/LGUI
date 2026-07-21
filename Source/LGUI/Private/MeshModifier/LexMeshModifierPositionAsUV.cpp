// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LGUI/Public/MeshModifier/LexMeshModifierPositionAsUV.h"
#include "Core/Components/LexCanvas.h"
#include "LGUI.h"
#include "Core/Components/LexWidget.h"


ULexMeshModifierPositionAsUV::ULexMeshModifierPositionAsUV()
{
}

void ULexMeshModifierPositionAsUV::ModifyUIGeometry(
	FLexUIGeometry& InGeometry, bool InTriangleChanged, bool InUVChanged, bool InColorChanged, bool InVertexPositionChanged
)
{
	auto LexVisual = GetVisualBatchMesh();
	if (!LexVisual)return;
	auto RenderCanvas = LexVisual->GetWidget()->GetRenderCanvas();
	auto& originVertices = InGeometry.OriginVertices;
	switch (UVChannel)
	{
	case 0:
	{
		auto& vertices = InGeometry.Vertices;
		auto vertexCount = vertices.Num();
		for (int i = 0; i < vertexCount; i++)
		{
			auto& vert = originVertices[i].Position;
			vertices[i].TextureCoordinate[0] = FVector2f(vert.Y, vert.Z) * Scale;
		}
	}
	break;
	case 1:
	{
		if (!RenderCanvas)return;
		auto& vertices = InGeometry.Vertices;
		auto vertexCount = vertices.Num();
		for (int i = 0; i < vertexCount; i++)
		{
			auto& vert = originVertices[i].Position;
			vertices[i].TextureCoordinate[1] = FVector2f(vert.Y, vert.Z) * Scale;
		}
	}
	break;
	case 2:
	{
		if (!RenderCanvas)return;
		auto& vertices = InGeometry.Vertices;
		auto vertexCount = vertices.Num();
		for (int i = 0; i < vertexCount; i++)
		{
			auto& vert = originVertices[i].Position;
			vertices[i].TextureCoordinate[2] = FVector2f(vert.Y, vert.Z) * Scale;
		}
	}
	break;
	case 3:
	{
		if (!RenderCanvas)return;
		auto& vertices = InGeometry.Vertices;
		auto vertexCount = vertices.Num();
		for (int i = 0; i < vertexCount; i++)
		{
			auto& vert = originVertices[i].Position;
			vertices[i].TextureCoordinate[3] = FVector2f(vert.Y, vert.Z) * Scale;
		}
	}
	break;
	}
}