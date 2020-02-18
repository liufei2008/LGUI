// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Core/Render/LGUIHudVertex.h"


void FLGUIVertexDeclaration::InitRHI()
{
	FVertexDeclarationElementList Elements;
	uint32 Stride = sizeof(FLGUIHudVertex);
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FLGUIHudVertex, Position), VET_Float3, 0, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FLGUIHudVertex, Color), VET_Color, 1, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FLGUIHudVertex, TextureCoordinate0), VET_Float2, 2, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FLGUIHudVertex, TextureCoordinate1), VET_Float2, 3, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FLGUIHudVertex, TextureCoordinate2), VET_Float2, 4, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FLGUIHudVertex, TextureCoordinate3), VET_Float2, 5, Stride));
	VertexDeclarationRHI = RHICreateVertexDeclaration(Elements);
}
void FLGUIVertexDeclaration::ReleaseRHI()
{
	VertexDeclarationRHI.SafeRelease();
}
TGlobalResource<FLGUIVertexDeclaration> GLGUIVertexDeclaration;
FVertexDeclarationRHIRef& GetLGUIVertexDeclaration()
{
	return GLGUIVertexDeclaration.VertexDeclarationRHI;
}


void FLGUIPostProcessVertexDeclaration::InitRHI()
{
	FVertexDeclarationElementList Elements;
	uint16 Stride = sizeof(FLGUIPostProcessVertex);
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FLGUIPostProcessVertex, Position), VET_Float3, 0, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FLGUIPostProcessVertex, TextureCoordinate0), VET_Float2, 1, Stride));
	VertexDeclarationRHI = RHICreateVertexDeclaration(Elements);
}
void FLGUIPostProcessVertexDeclaration::ReleaseRHI()
{
	VertexDeclarationRHI.SafeRelease();
}
TGlobalResource<FLGUIPostProcessVertexDeclaration> GLGUIPostProcessVertexDeclaration;
FVertexDeclarationRHIRef& GetLGUIPostProcessVertexDeclaration()
{
	return GLGUIPostProcessVertexDeclaration.VertexDeclarationRHI;
}
