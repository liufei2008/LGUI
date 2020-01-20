// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "GeometryModifier/UIEffectGradientColor.h"
#include "LGUI.h"
#include "Utils/LGUIUtils.h"
#include "Core/ActorComponent/UIText.h"

UUIEffectGradientColor::UUIEffectGradientColor()
{
	PrimaryComponentTick.bCanEverTick = false;
}
void UUIEffectGradientColor::ModifyUIGeometry(TSharedPtr<UIGeometry>& InGeometry, int32& InOutOriginVerticesCount, int32& InOutOriginTriangleIndicesCount, bool& OutTriangleChanged)
{
	auto& triangles = InGeometry->triangles;
	auto& vertices = InGeometry->vertices;

	auto vertexCount = vertices.Num();
	int32 triangleCount = triangles.Num();
	if (triangleCount == 0 || vertexCount == 0)return;
	
	FColor baseColor, tintColor1, tintColor2;
	if (multiplySourceAlpha)
	{
		baseColor = GetRenderableUIItem()->GetFinalColor();
		tintColor1 = LGUIUtils::MultiplyColor(color1, baseColor);
		tintColor2 = LGUIUtils::MultiplyColor(color2, baseColor);
	}
	else
	{
		tintColor1 = color1;
		tintColor2 = color2;
	}

	switch (directionType)
	{
	case EUIEffectGradientColorDirection::BottomToTop:
	{
		for (int i = 0; i < vertexCount;)
		{
			vertices[i++].Color = tintColor1;
			vertices[i++].Color = tintColor1;
			vertices[i++].Color = tintColor2;
			vertices[i++].Color = tintColor2;
		}
	}
	break;
	case EUIEffectGradientColorDirection::TopToBottom:
	{
		for (int i = 0; i < vertexCount;)
		{
			vertices[i++].Color = tintColor2;
			vertices[i++].Color = tintColor2;
			vertices[i++].Color = tintColor1;
			vertices[i++].Color = tintColor1;
		}
	}
	break;
	case EUIEffectGradientColorDirection::LeftToRight:
	{
		for (int i = 0; i < vertexCount;)
		{
			vertices[i++].Color = tintColor1;
			vertices[i++].Color = tintColor2;
			vertices[i++].Color = tintColor1;
			vertices[i++].Color = tintColor2;
		}
	}
	break;
	case EUIEffectGradientColorDirection::RightToLeft:
	{
		for (int i = 0; i < vertexCount;)
		{
			vertices[i++].Color = tintColor2;
			vertices[i++].Color = tintColor1;
			vertices[i++].Color = tintColor2;
			vertices[i++].Color = tintColor1;
		}
	}
	break;
	case EUIEffectGradientColorDirection::FourCornor:
	{
		FColor tintColor3, tintColor4;
		if (multiplySourceAlpha)
		{
			baseColor = GetRenderableUIItem()->GetFinalColor();
			tintColor3 = LGUIUtils::MultiplyColor(color3, baseColor);
			tintColor4 = LGUIUtils::MultiplyColor(color4, baseColor);
		}
		else
		{
			tintColor3 = color3;
			tintColor4 = color4;
		}
		
		for (int i = 0; i < vertexCount;)
		{
			vertices[i++].Color = tintColor1;
			vertices[i++].Color = tintColor2;
			vertices[i++].Color = tintColor3;
			vertices[i++].Color = tintColor4;
		}
	}
	break;
	}
}
