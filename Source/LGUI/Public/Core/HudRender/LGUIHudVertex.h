// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RenderResource.h"

struct LGUI_API FLGUIHudVertex
{
	FVector3f Position;
	FColor Color;
	FVector2f TextureCoordinate0;
	FVector2f TextureCoordinate1;
	FVector2f TextureCoordinate2;
	FVector2f TextureCoordinate3;
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

	FLGUIPostProcessVertex(FVector3f InPosition, FVector2f InTextureCoordinate0)
	{
		Position = InPosition;
		TextureCoordinate0 = InTextureCoordinate0;
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

