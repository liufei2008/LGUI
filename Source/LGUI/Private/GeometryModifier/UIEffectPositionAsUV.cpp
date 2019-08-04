// Copyright 2019 LexLiu. All Rights Reserved.

#include "GeometryModifier/UIEffectPositionAsUV.h"
#include "LGUI.h"


UUIEffectPositionAsUV::UUIEffectPositionAsUV()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUIEffectPositionAsUV::ModifyUIGeometry(TSharedPtr<UIGeometry>& InGeometry, int32& InOutOriginVerticesCount, int32& InOutOriginTriangleIndicesCount, bool& OutTriangleChanged)
{
	auto uiRenderable = GetRenderableUIItem();
	if (!uiRenderable)return;
	auto uiPanel = uiRenderable->GetRenderUIPanel();
	auto& vertices = InGeometry->vertices;
	switch (uvChannel)
	{
	case 0:
	{
		auto& uvs0 = InGeometry->uvs;
		auto vertexCount = uvs0.Num();
		for (int i = 0; i < vertexCount; i++)
		{
			auto& vert = vertices[i];
			uvs0[i] = FVector2D(vert.X, vert.Y);
		}
	}
	break;
	case 1:
	{
		if (!uiPanel)return;
		if (!uiPanel->GetRequireUV1())
		{
			UE_LOG(LGUI, Error, TEXT("[UUIEffectPositionAsUV::ModifyUIGeometry]UIPanel/AdditionalShaderChannel/UV1 should be checked!"));
			return;
		}
		auto& uvs1 = InGeometry->uvs1;
		auto vertexCount = uvs1.Num();
		for (int i = 0; i < vertexCount; i++)
		{
			auto& vert = vertices[i];
			uvs1[i] = FVector2D(vert.X, vert.Y);
		}
	}
	break;
	case 2:
	{
		if (!uiPanel)return;
		if (!uiPanel->GetRequireUV2())
		{
			UE_LOG(LGUI, Error, TEXT("[UUIEffectPositionAsUV::ModifyUIGeometry]UIPanel/AdditionalShaderChannel/UV2 should be checked!"));
			return;
		}
		auto& uvs2 = InGeometry->uvs2;
		auto vertexCount = uvs2.Num();
		for (int i = 0; i < vertexCount; i++)
		{
			auto& vert = vertices[i];
			uvs2[i] = FVector2D(vert.X, vert.Y);
		}
	}
	break;
	case 3:
	{
		if (!uiPanel)return;
		if (!uiPanel->GetRequireUV3())
		{
			UE_LOG(LGUI, Error, TEXT("[UUIEffectPositionAsUV::ModifyUIGeometry]UIPanel/AdditionalShaderChannel/UV3 should be checked!"));
			return;
		}
		auto& uvs3 = InGeometry->uvs3;
		auto vertexCount = uvs3.Num();
		for (int i = 0; i < vertexCount; i++)
		{
			auto& vert = vertices[i];
			uvs3[i] = FVector2D(vert.X, vert.Y);
		}
	}
	break;
	}
}