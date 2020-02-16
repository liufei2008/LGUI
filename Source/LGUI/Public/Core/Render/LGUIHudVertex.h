// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RenderResource.h"

struct LGUI_API FLGUIHudVertex
{
	FVector Position;
	FColor Color;
	FVector2D TextureCoordinate0;
	FVector2D TextureCoordinate1;
	FVector2D TextureCoordinate2;
	FVector2D TextureCoordinate3;
};

class FLGUIVertexDeclaration : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;
	virtual ~FLGUIVertexDeclaration() {}
	virtual void InitRHI()override;
	virtual void ReleaseRHI()override;
};

TGlobalResource<FLGUIVertexDeclaration> GLGUIVertexDeclaration;

struct LGUI_API FLGUIPostProcessVertex
{
	FVector Position;
	FVector2D TextureCoordinate0;

	FLGUIPostProcessVertex(FVector InPosition, FVector2D InTextureCoordinate0)
	{
		Position = InPosition;
		TextureCoordinate0 = InTextureCoordinate0;
	}
};

class FLGUIPostProcessVertexDeclaration : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;
	virtual void InitRHI() override;
	virtual void ReleaseRHI() override;
};

TGlobalResource<FLGUIPostProcessVertexDeclaration> GLGUIPostProcessVertexDeclaration;
