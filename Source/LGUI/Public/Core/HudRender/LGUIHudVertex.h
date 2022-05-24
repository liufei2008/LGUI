// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RenderResource.h"
#include "Core/LGUIMeshVertex.h"

struct LGUI_API FLGUIPostProcessVertex
{
	FVector Position;
	FVector2D TextureCoordinate0;
	FVector2D TextureCoordinate1;

	FLGUIPostProcessVertex(FVector InPosition, FVector2D InTextureCoordinate0)
	{
		Position = InPosition;
		TextureCoordinate0 = InTextureCoordinate0;
	}
	FLGUIPostProcessVertex(FVector InPosition, FVector2D InTextureCoordinate0, FVector2D InTextureCoordinate1)
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
	FVector ScreenPosition;
	FVector LocalPosition;

	FLGUIPostProcessCopyMeshRegionVertex(FVector InScreenPosition, FVector InLocalPosition)
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
	FVector Position = FVector::ZeroVector;
	FColor Color = FColor::White;

	FLGUIHelperLineVertex(FVector InPosition, FColor InColor)
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
