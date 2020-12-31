// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Extensions/UITextureBox.h"
#include "LGUI.h"
#include "Core/UIGeometry.h"
#include "Core/ActorComponent/LGUICanvas.h"

UUITextureBox::UUITextureBox(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUITextureBox::BeginPlay()
{
	Super::BeginPlay();
}
#if WITH_EDITOR
void UUITextureBox::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
void UUITextureBox::EditorForceUpdateImmediately()
{
	Super::EditorForceUpdateImmediately();
}
#endif


AUITextureBoxActor::AUITextureBoxActor()
{
	PrimaryActorTick.bCanEverTick = false;

	UITextureBox = CreateDefaultSubobject<UUITextureBox>(TEXT("UITextureBoxComponent"));
	RootComponent = UITextureBox;
}





void UpdateUITextureBoxUV(TSharedPtr<UIGeometry> uiGeo)
{
	auto& vertices = uiGeo->vertices;
	if (vertices.Num() == 0)
	{
		vertices.SetNumUninitialized(24);
	}
	int vertIndex = 0;
	for (int i = 0; i < 6; i++)
	{
		(vertices[vertIndex++].TextureCoordinate[0]) = FVector2D(0, 0);
		(vertices[vertIndex++].TextureCoordinate[0]) = FVector2D(1, 0);
		(vertices[vertIndex++].TextureCoordinate[0]) = FVector2D(0, 1);
		(vertices[vertIndex++].TextureCoordinate[0]) = FVector2D(1, 1);
	}
}
void UpdateUITextureBoxVertex(TSharedPtr<UIGeometry> uiGeo, float& width, float& height, float& thickness, const FVector2D& pivot)
{
	//pivot offset
	float pivotOffsetX = 0, pivotOffsetY = 0, halfW = 0, halfH = 0;
	UIGeometry::CalculatePivotOffset(width, height, pivot, pivotOffsetX, pivotOffsetY, halfW, halfH);
	//vertices
	auto& vertices = uiGeo->originPositions;
	if (vertices.Num() == 0)
	{
		uiGeo->originVerticesCount = 24;
		vertices.Reserve(24);
		for (int i = 0; i < 24; i++)
		{
			vertices.Add(FVector());
		}
	}
	float x, y;
	int vertIndex = 0;
	//front
	x = -halfW + pivotOffsetX;
	y = -halfH + pivotOffsetY;
	vertices[vertIndex++] = FVector(x, y, 0);
	x = halfW + pivotOffsetX;
	vertices[vertIndex++] = FVector(x, y, 0);
	x = -halfW + pivotOffsetX;
	y = halfH + pivotOffsetY;
	vertices[vertIndex++] = FVector(x, y, 0);
	x = halfW + pivotOffsetX;
	vertices[vertIndex++] = FVector(x, y, 0);
	//back
	x = halfW + pivotOffsetX;
	y = -halfH + pivotOffsetY;
	vertices[vertIndex++] = FVector(x, y, thickness);
	x = -halfW + pivotOffsetX;
	vertices[vertIndex++] = FVector(x, y, thickness);
	x = halfW + pivotOffsetX;
	y = halfH + pivotOffsetY;
	vertices[vertIndex++] = FVector(x, y, thickness);
	x = -halfW + pivotOffsetX;
	vertices[vertIndex++] = FVector(x, y, thickness);
	//left
	x = -halfW + pivotOffsetX;
	y = -halfH + pivotOffsetY;
	vertices[vertIndex++] = FVector(x, y, thickness);
	vertices[vertIndex++] = FVector(x, y, 0);
	x = -halfW + pivotOffsetX;
	y = halfH + pivotOffsetY;
	vertices[vertIndex++] = FVector(x, y, thickness);
	vertices[vertIndex++] = FVector(x, y, 0);
	//right
	x = halfW + pivotOffsetX;
	y = -halfH + pivotOffsetY;
	vertices[vertIndex++] = FVector(x, y, 0);
	vertices[vertIndex++] = FVector(x, y, thickness);
	x = halfW + pivotOffsetX;
	y = halfH + pivotOffsetY;
	vertices[vertIndex++] = FVector(x, y, 0);
	vertices[vertIndex++] = FVector(x, y, thickness);
	//top
	x = -halfW + pivotOffsetX;
	y = halfH + pivotOffsetY;
	vertices[vertIndex++] = FVector(x, y, 0);
	x = halfW + pivotOffsetX;
	vertices[vertIndex++] = FVector(x, y, 0);
	x = -halfW + pivotOffsetX;
	y = halfH + pivotOffsetY;
	vertices[vertIndex++] = FVector(x, y, thickness);
	x = halfW + pivotOffsetX;
	vertices[vertIndex++] = FVector(x, y, thickness);
	//bottom
	x = -halfW + pivotOffsetX;
	y = -halfH + pivotOffsetY;
	vertices[vertIndex++] = FVector(x, y, thickness);
	x = halfW + pivotOffsetX;
	vertices[vertIndex++] = FVector(x, y, thickness);
	x = -halfW + pivotOffsetX;
	y = -halfH + pivotOffsetY;
	vertices[vertIndex++] = FVector(x, y, 0);
	x = halfW + pivotOffsetX;
	vertices[vertIndex++] = FVector(x, y, 0);
}
void FromUITextureBox(float& width, float& height, float& thickness, const FVector2D& pivot, FColor color, bool seperateFrontColor, FColor frontFaceColor, TSharedPtr<UIGeometry> uiGeo, bool requireNormal, bool requireTangent, bool requireUV1)
{
	//triangles
	auto& triangles = uiGeo->triangles;
	if (triangles.Num() == 0)
	{
		uiGeo->originTriangleCount = 12;
		triangles = {
			0,3,2,		0,1,3,
			4,7,6,		4,5,7,
			8,11,10,	8,9,11,
			12,15,14,	12,13,15,
			16,19,18,	16,17,19,
			20,23,22,	20,21,23
		};
	}
	//vertices
	UpdateUITextureBoxVertex(uiGeo, width, height, thickness, pivot);
	//uvs
	UpdateUITextureBoxUV(uiGeo);
	//colors
	if (seperateFrontColor)
	{
		auto& vertices = uiGeo->vertices;
		auto count = vertices.Num();
		if (count == 0)
		{
			vertices.SetNumUninitialized(24);
		}
		for (int i = 0; i < 4; i++)
		{
			vertices[i].Color = frontFaceColor;
		}
		for (int i = 4; i < 24; i++)
		{
			vertices[i].Color = color;
		}
	}
	else
	{
		UIGeometry::UpdateUIColor(uiGeo, color);
	}

	//normals
	if (requireNormal)
	{
		auto& normals = uiGeo->originNormals;
		if (normals.Num() == 0)
		{
			normals.AddUninitialized(24);
			int vertIndex = 0;
			FVector normalItem;
			for (int i = 0; i < 6; i++)
			{
				switch (i)
				{
				case 0://front
					normalItem = FVector(0, 0, -1);
					break;
				case 1://back
					normalItem = FVector(0, 0, 1);
					break;
				case 2://left
					normalItem = FVector(-1, 0, 0);
					break;
				case 3://right
					normalItem = FVector(1, 0, 0);
					break;
				case 4://top
					normalItem = FVector(0, 1, 0);
					break;
				case 5://bottom
					normalItem = FVector(0, -1, 0);
					break;
				}
				(normals[vertIndex++]) = normalItem;
				(normals[vertIndex++]) = normalItem;
				(normals[vertIndex++]) = normalItem;
				(normals[vertIndex++]) = normalItem;
			}
		}
	}
	//tangents
	if (requireTangent)
	{
		auto& tangents = uiGeo->originTangents;
		if (tangents.Num() == 0)
		{
			tangents.AddUninitialized(24);
			int vertIndex = 0;
			FVector tangentItem;
			for (int i = 0; i < 6; i++)
			{
				switch (i)
				{
				case 0://front
					tangentItem = FVector(1, 0, 0);
					break;
				case 1://back
					tangentItem = FVector(1, 0, 0);
					break;
				case 2://left
					tangentItem = FVector(0, 0, 1);
					break;
				case 3://right
					tangentItem = FVector(0, 0, 1);
					break;
				case 4://top
					tangentItem = FVector(1, 0, 0);
					break;
				case 5://bottom
					tangentItem = FVector(1, 0, 0);
					break;
				}
				tangents[vertIndex++] = tangentItem;
				tangents[vertIndex++] = tangentItem;
				tangents[vertIndex++] = tangentItem;
				tangents[vertIndex++] = tangentItem;
			}
		}
	}
	//uvs1
	if (requireUV1)
	{
		auto& vertices = uiGeo->vertices;
		int vertIndex = 0;
		for (int i = 0; i < 6; i++)
		{
			(vertices[vertIndex++].TextureCoordinate[1]) = FVector2D(0, 0);
			(vertices[vertIndex++].TextureCoordinate[1]) = FVector2D(1, 0);
			(vertices[vertIndex++].TextureCoordinate[1]) = FVector2D(0, 1);
			(vertices[vertIndex++].TextureCoordinate[1]) = FVector2D(1, 1);
		}
	}
}




void UUITextureBox::OnCreateGeometry()
{
	FromUITextureBox(widget.width, widget.height, thickness, widget.pivot, GetFinalColor(), seperateFrontColor, frontFaceColor, geometry, RenderCanvas->GetRequireNormal(), RenderCanvas->GetRequireTangent(), RenderCanvas->GetRequireUV1());
}
void UUITextureBox::OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	if (InVertexPositionChanged)
	{
		UpdateUITextureBoxVertex(geometry, widget.width, widget.height, thickness, widget.pivot);
	}
	if (InVertexColorChanged)
	{
		UIGeometry::UpdateUIColor(geometry, GetFinalColor());
	}
	if (InVertexUVChanged)
	{
		UpdateUITextureBoxUV(geometry);
	}
}


