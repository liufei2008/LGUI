// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LGUI/Public/MeshModifier/LexMeshModifierGradientColor.h"
#include "LGUI.h"
#include "Utils/LexUIUtils.h"
#include "Core/Components/LexText.h"

ULexMeshModifierGradientColor::ULexMeshModifierGradientColor()
{
}
void ULexMeshModifierGradientColor::ApplyColorAndAlpha(FColor& InOutColor, FColor InTintColor)
{
	if (bMultiplySourceAlpha)
	{
		InOutColor.A = (uint8)(FLexUIUtils::ByteToFloat01(InOutColor.A) * InTintColor.A);
		InOutColor.R = InTintColor.R;
		InOutColor.G = InTintColor.G;
		InOutColor.B = InTintColor.B;
	}
	else
	{
		InOutColor = InTintColor;
	}
}
void ULexMeshModifierGradientColor::ModifyUIGeometry(
	FLexUIGeometry& InGeometry, bool InTriangleChanged, bool InUVChanged, bool InColorChanged, bool InVertexPositionChanged
)
{
	auto& triangles = InGeometry.Triangles;
	auto& vertices = InGeometry.Vertices;

	auto vertexCount = vertices.Num();
	int32 triangleCount = triangles.Num();
	if (triangleCount == 0 || vertexCount == 0)return;

	switch (DirectionType)
	{
	case ELexMeshModifierGradientColorDirection::BottomToTop:
	{
		for (int i = 0; i < vertexCount;)
		{
			ApplyColorAndAlpha(vertices[i++].Color, Color1);
			ApplyColorAndAlpha(vertices[i++].Color, Color1);
			ApplyColorAndAlpha(vertices[i++].Color, Color2);
			ApplyColorAndAlpha(vertices[i++].Color, Color2);
		}
	}
	break;
	case ELexMeshModifierGradientColorDirection::TopToBottom:
	{
		for (int i = 0; i < vertexCount;)
		{
			ApplyColorAndAlpha(vertices[i++].Color, Color2);
			ApplyColorAndAlpha(vertices[i++].Color, Color2);
			ApplyColorAndAlpha(vertices[i++].Color, Color1);
			ApplyColorAndAlpha(vertices[i++].Color, Color1);
		}
	}
	break;
	case ELexMeshModifierGradientColorDirection::LeftToRight:
	{
		for (int i = 0; i < vertexCount;)
		{
			ApplyColorAndAlpha(vertices[i++].Color, Color1);
			ApplyColorAndAlpha(vertices[i++].Color, Color2);
			ApplyColorAndAlpha(vertices[i++].Color, Color1);
			ApplyColorAndAlpha(vertices[i++].Color, Color2);
		}
	}
	break;
	case ELexMeshModifierGradientColorDirection::RightToLeft:
	{
		for (int i = 0; i < vertexCount;)
		{
			ApplyColorAndAlpha(vertices[i++].Color, Color2);
			ApplyColorAndAlpha(vertices[i++].Color, Color1);
			ApplyColorAndAlpha(vertices[i++].Color, Color2);
			ApplyColorAndAlpha(vertices[i++].Color, Color1);
		}
	}
	break;
	case ELexMeshModifierGradientColorDirection::FourCorner:
	{
		for (int i = 0; i < vertexCount;)
		{
			ApplyColorAndAlpha(vertices[i++].Color, Color1);
			ApplyColorAndAlpha(vertices[i++].Color, Color2);
			ApplyColorAndAlpha(vertices[i++].Color, Color3);
			ApplyColorAndAlpha(vertices[i++].Color, Color4);
		}
	}
	break;
	}
}

void ULexMeshModifierGradientColor::SetDirectionType(ELexMeshModifierGradientColorDirection Value)
{
	if (DirectionType != Value)
	{
		DirectionType = Value;
		if (GetVisualBatchMesh())GetVisualBatchMesh()->MarkColorDirty();
	}
}
void ULexMeshModifierGradientColor::SetMultiplySourceAlpha(bool Value)
{
	if (bMultiplySourceAlpha != Value)
	{
		bMultiplySourceAlpha = Value;
		if (GetVisualBatchMesh())GetVisualBatchMesh()->MarkColorDirty();
	}
}
void ULexMeshModifierGradientColor::SetColor1(FColor Value)
{
	if (Color1 != Value)
	{
		Color1 = Value;
		if (GetVisualBatchMesh())GetVisualBatchMesh()->MarkColorDirty();
	}
}
void ULexMeshModifierGradientColor::SetColor2(FColor Value)
{
	if (Color2 != Value)
	{
		Color2 = Value;
		if (GetVisualBatchMesh())GetVisualBatchMesh()->MarkColorDirty();
	}
}
void ULexMeshModifierGradientColor::SetColor3(FColor Value)
{
	if (Color3 != Value)
	{
		Color3 = Value;
		if (GetVisualBatchMesh())GetVisualBatchMesh()->MarkColorDirty();
	}
}
void ULexMeshModifierGradientColor::SetColor4(FColor Value)
{
	if (Color4 != Value)
	{
		Color4 = Value;
		if (GetVisualBatchMesh())GetVisualBatchMesh()->MarkColorDirty();
	}
}
