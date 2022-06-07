// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RenderResource.h"
#include "PackedNormal.h"
#include "RenderUtils.h"

#define LGUI_VERTEX_TEXCOORDINATE_COUNT 4

struct LGUI_API FLGUIMeshVertex
{
	FLGUIMeshVertex(){}
	FLGUIMeshVertex(const FVector& InPosition):
		Position(InPosition),
		Color(FColor(255, 255, 255)),
		TangentX(FVector(1, 0, 0)),
		TangentZ(FVector(0, 0, 1))
	{
		// basis determinant default to +1.0
		TangentZ.Vector.W = 127;

		for (int i = 0; i < LGUI_VERTEX_TEXCOORDINATE_COUNT; i++)
		{
			TextureCoordinate[i] = FVector2D::ZeroVector;
		}
	}

	FVector Position;
	FColor Color;
	FVector2D TextureCoordinate[LGUI_VERTEX_TEXCOORDINATE_COUNT];
	FPackedNormal TangentX;
	FPackedNormal TangentZ;

	FVector GetTangentY() const
	{
		return GenerateYAxis(TangentX, TangentZ);
	};
};

class LGUI_API FLGUIMeshVertexDeclaration : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;
	virtual ~FLGUIMeshVertexDeclaration() {}
	virtual void InitRHI()override;
	virtual void ReleaseRHI()override;
};
LGUI_API FVertexDeclarationRHIRef& GetLGUIMeshVertexDeclaration();

