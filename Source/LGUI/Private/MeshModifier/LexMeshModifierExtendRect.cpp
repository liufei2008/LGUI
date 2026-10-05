// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LGUI/Public/MeshModifier/LexMeshModifierExtendRect.h"
#include "LGUI.h"
#include "Core/Components/LexWidget.h"
#include "Utils/LexUIUtils.h"


ULexMeshModifierExtendRect::ULexMeshModifierExtendRect()
{
}
void ULexMeshModifierExtendRect::ModifyUIGeometry(
	FLexUIGeometry& InGeometry, bool InTriangleChanged, bool InUVChanged, bool InColorChanged, bool InVertexPositionChanged
)
{
	auto& triangles = InGeometry.Triangles;
	auto& originVertices = InGeometry.OriginVertices;
	auto& vertices = InGeometry.Vertices;

	auto vertexCount = vertices.Num();
	int32 triangleCount = triangles.Num();
	if (triangleCount == 0 || vertexCount == 0)return;
	if (vertexCount != 4)return;
	
	originVertices[0].Position += FVector3f(0, -Extend.Left, -Extend.Bottom);
	originVertices[1].Position += FVector3f(0, Extend.Right, -Extend.Bottom);
	originVertices[2].Position += FVector3f(0, -Extend.Left, Extend.Top);
	originVertices[3].Position += FVector3f(0, Extend.Right, Extend.Top);
	
	auto Widget = GetWidget();
	float InvWidth = 1.0f / Widget->GetWidth();
	float InvHeight = 1.0f / Widget->GetHeight();
	FMargin ExtendUV = FMargin(Extend.Left * InvWidth, Extend.Top * InvHeight, Extend.Right * InvWidth, Extend.Bottom * InvHeight);
	vertices[0].TextureCoordinate[0] += FVector2f(-ExtendUV.Left, ExtendUV.Bottom);
	vertices[1].TextureCoordinate[0] += FVector2f(ExtendUV.Right, ExtendUV.Bottom);
	vertices[2].TextureCoordinate[0] += FVector2f(-ExtendUV.Left, -ExtendUV.Top);
	vertices[3].TextureCoordinate[0] += FVector2f(ExtendUV.Right, -ExtendUV.Top);
}

void ULexMeshModifierExtendRect::SetExtend(FMargin Value)
{
	if (Extend != Value)
	{
		Extend = Value;
		if (auto Visual = GetVisualBatchMesh())
		{
			Visual->MarkVerticesDirty(false, true, true, false);
		}
	}
}
