// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RenderResource.h"
#include "PackedNormal.h"

#define LGUI_VERTEX_TEXCOORDINATE_COUNT 4

struct LGUI_API FLGUIHudVertex
{
	FLGUIHudVertex(){}
	FLGUIHudVertex(const FVector3f& InPosition):
		Position(InPosition),
		Color(FColor(255, 255, 255)),
		TangentX(FVector3f(1, 0, 0)),
		TangentZ(FVector3f(0, 0, 1))
	{
		// basis determinant default to +1.0
		TangentZ.Vector.W = 127;

		for (int i = 0; i < LGUI_VERTEX_TEXCOORDINATE_COUNT; i++)
		{
			TextureCoordinate[i] = FVector2f::ZeroVector;
		}
	}

	FVector3f Position;
	FColor Color;
	FVector2f TextureCoordinate[LGUI_VERTEX_TEXCOORDINATE_COUNT];
	FPackedNormal TangentX;
	FPackedNormal TangentZ;

	FVector3f GetTangentY() const
	{
		return FVector3f(GenerateYAxis(TangentX, TangentZ));
	};
};

class LGUI_API FLGUIHudVertexDeclaration : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;
	virtual ~FLGUIHudVertexDeclaration() {}
	virtual void InitRHI()override;
	virtual void ReleaseRHI()override;
};
LGUI_API FVertexDeclarationRHIRef& GetLGUIHudVertexDeclaration();


struct LGUI_API FLGUIPostProcessVertex
{
	FVector3f Position;
	FVector2f TextureCoordinate0;
	FVector2f TextureCoordinate1;

	FLGUIPostProcessVertex(FVector3f InPosition, FVector2f InTextureCoordinate0)
	{
		Position = InPosition;
		TextureCoordinate0 = InTextureCoordinate0;
	}
	FLGUIPostProcessVertex(FVector3f InPosition, FVector2f InTextureCoordinate0, FVector2f InTextureCoordinate1)
	{
		Position = InPosition;
		TextureCoordinate0 = InTextureCoordinate0;
		TextureCoordinate1 = InTextureCoordinate1;
	}
};

class LGUI_API FLGUIPostProcessVertexDeclaration : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;
	virtual void InitRHI() override;
	virtual void ReleaseRHI() override;
};
LGUI_API FVertexDeclarationRHIRef& GetLGUIPostProcessVertexDeclaration();




struct LGUI_API FLGUIPostProcessCopyMeshRegionVertex
{
	FVector3f ScreenPosition;
	FVector3f LocalPosition;

	FLGUIPostProcessCopyMeshRegionVertex(FVector3f InScreenPosition, FVector3f InLocalPosition)
	{
		ScreenPosition = InScreenPosition;
		LocalPosition = InLocalPosition;
	}
};

class LGUI_API FLGUIPostProcessCopyMeshRegionVertexDeclaration : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;
	virtual void InitRHI() override;
	virtual void ReleaseRHI() override;
};
LGUI_API FVertexDeclarationRHIRef& GetLGUIPostProcessCopyMeshRegionVertexDeclaration();


/** Parameters for render editor helper line */
struct LGUI_API FLGUIHelperLineVertex
{
public:
	FVector3f Position = FVector3f::ZeroVector;
	FColor Color = FColor::White;

	FLGUIHelperLineVertex(FVector3f InPosition, FColor InColor)
	{
		Position = InPosition;
		Color = InColor;
	}
};
struct LGUI_API FLGUIHelperLineVertexDeclaration : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;
	virtual void InitRHI() override;
	virtual void ReleaseRHI() override;
};
LGUI_API FVertexDeclarationRHIRef& GetLGUIHelperLineVertexDeclaration();
