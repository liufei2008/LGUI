// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RenderResource.h"

struct LGUI_API FLexUIPostProcessVertex
{
	FVector3f Position;
	FVector2f TextureCoordinate0;//widget's full rect uv
	FVector2f TextureCoordinate1;//clip data uv, check ULexCanvas UV1

	FLexUIPostProcessVertex(FVector3f InPosition, FVector2f InTextureCoordinate0)
	{
		Position = InPosition;
		TextureCoordinate0 = InTextureCoordinate0;
	}
	FLexUIPostProcessVertex(FVector3f InPosition, FVector2f InTextureCoordinate0, FVector2f InTextureCoordinate1)
	{
		Position = InPosition;
		TextureCoordinate0 = InTextureCoordinate0;
		TextureCoordinate1 = InTextureCoordinate1;
	}
};

class LGUI_API FLexUIPostProcessVertexDeclaration : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;
	virtual void InitRHI(FRHICommandListBase& RHICmdList) override;
	virtual void ReleaseRHI() override;
};
LGUI_API FVertexDeclarationRHIRef& GetLexUIPostProcessVertexDeclaration();




struct LGUI_API FLexUIPostProcessCopyMeshRegionVertex
{
	FVector2f ScreenPosition;
	FVector2f TextureCoordinate;

	FLexUIPostProcessCopyMeshRegionVertex(FVector2f InScreenPosition, FVector2f InTextureCoordinate)
	{
		ScreenPosition = InScreenPosition;
		TextureCoordinate = InTextureCoordinate;
	}
};

class LGUI_API FLexUIPostProcessCopyMeshRegionVertexDeclaration : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;
	virtual void InitRHI(FRHICommandListBase& RHICmdList) override;
	virtual void ReleaseRHI() override;
};
LGUI_API FVertexDeclarationRHIRef& GetLexUIPostProcessCopyMeshRegionVertexDeclaration();

