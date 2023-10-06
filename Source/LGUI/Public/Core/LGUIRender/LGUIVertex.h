// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RenderResource.h"
#include "Core/LGUIMeshVertex.h"

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
	virtual void InitRHI(FRHICommandListBase& RHICmdList) override;
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
	virtual void InitRHI(FRHICommandListBase& RHICmdList) override;
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
	virtual void InitRHI(FRHICommandListBase& RHICmdList) override;
	virtual void ReleaseRHI() override;
};
LGUI_API FVertexDeclarationRHIRef& GetLGUIHelperLineVertexDeclaration();
