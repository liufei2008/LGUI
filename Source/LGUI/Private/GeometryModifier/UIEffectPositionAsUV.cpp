// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "GeometryModifier/UIEffectPositionAsUV.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "LGUI.h"


UUIEffectPositionAsUV::UUIEffectPositionAsUV()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUIEffectPositionAsUV::ModifyUIGeometry(
	UIGeometry& InGeometry, bool InTriangleChanged, bool InUVChanged, bool InColorChanged, bool InVertexPositionChanged
)
{
	auto uiRenderable = GetUIRenderable();
	if (!uiRenderable)return;
	auto renderCanvas = uiRenderable->GetRenderCanvas();
	auto& originVertices = InGeometry.originVertices;
	switch (uvChannel)
	{
	case 0:
	{
		auto& vertices = InGeometry.vertices;
		auto vertexCount = vertices.Num();
		for (int i = 0; i < vertexCount; i++)
		{
			auto& vert = originVertices[i].Position;
			vertices[i].TextureCoordinate[0] = FVector2D(vert.Y, vert.Z);
		}
	}
	break;
	case 1:
	{
		if (!renderCanvas)return;
		if (!renderCanvas->GetRequireUV1())
		{
			UE_LOG(LGUI, Error, TEXT("[UUIEffectPositionAsUV::ModifyUIGeometry]LGUICanvas/AdditionalShaderChannel/UV1 should be checked!"));
			return;
		}
		auto& vertices = InGeometry.vertices;
		auto vertexCount = vertices.Num();
		for (int i = 0; i < vertexCount; i++)
		{
			auto& vert = originVertices[i].Position;
			vertices[i].TextureCoordinate[1] = FVector2D(vert.Y, vert.Z);
		}
	}
	break;
	case 2:
	{
		if (!renderCanvas)return;
		if (!renderCanvas->GetRequireUV2())
		{
			UE_LOG(LGUI, Error, TEXT("[UUIEffectPositionAsUV::ModifyUIGeometry]LGUICanvas/AdditionalShaderChannel/UV2 should be checked!"));
			return;
		}
		auto& vertices = InGeometry.vertices;
		auto vertexCount = vertices.Num();
		for (int i = 0; i < vertexCount; i++)
		{
			auto& vert = originVertices[i].Position;
			vertices[i].TextureCoordinate[2] = FVector2D(vert.Y, vert.Z);
		}
	}
	break;
	case 3:
	{
		if (!renderCanvas)return;
		if (!renderCanvas->GetRequireUV3())
		{
			UE_LOG(LGUI, Error, TEXT("[UUIEffectPositionAsUV::ModifyUIGeometry]LGUICanvas/AdditionalShaderChannel/UV3 should be checked!"));
			return;
		}
		auto& vertices = InGeometry.vertices;
		auto vertexCount = vertices.Num();
		for (int i = 0; i < vertexCount; i++)
		{
			auto& vert = originVertices[i].Position;
			vertices[i].TextureCoordinate[3] = FVector2D(vert.Y, vert.Z);
		}
	}
	break;
	}
}