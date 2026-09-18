// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/LexUIRender/LexUIPostProcessVertex.h"
#include "RHI.h"


void FLexUIPostProcessVertexDeclaration::InitRHI(FRHICommandListBase& RHICmdList)
{
	FVertexDeclarationElementList Elements;
	uint16 Stride = sizeof(FLexUIPostProcessVertex);
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FLexUIPostProcessVertex, Position), VET_Float3, 0, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FLexUIPostProcessVertex, TextureCoordinate0), VET_Float2, 1, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FLexUIPostProcessVertex, TextureCoordinate1), VET_Float2, 2, Stride));
	VertexDeclarationRHI = RHICreateVertexDeclaration(Elements);
}
void FLexUIPostProcessVertexDeclaration::ReleaseRHI()
{
	VertexDeclarationRHI.SafeRelease();
}
TGlobalResource<FLexUIPostProcessVertexDeclaration> GLGUIPostProcessVertexDeclaration;
FVertexDeclarationRHIRef& GetLexUIPostProcessVertexDeclaration()
{
	return GLGUIPostProcessVertexDeclaration.VertexDeclarationRHI;
}




void FLexUIPostProcessCopyMeshRegionVertexDeclaration::InitRHI(FRHICommandListBase& RHICmdList)
{
	FVertexDeclarationElementList Elements;
	uint16 Stride = sizeof(FLexUIPostProcessCopyMeshRegionVertex);
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FLexUIPostProcessCopyMeshRegionVertex, ScreenPosition), VET_Float2, 0, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FLexUIPostProcessCopyMeshRegionVertex, TextureCoordinate), VET_Float2, 1, Stride));
	VertexDeclarationRHI = RHICreateVertexDeclaration(Elements);
}
void FLexUIPostProcessCopyMeshRegionVertexDeclaration::ReleaseRHI()
{
	VertexDeclarationRHI.SafeRelease();
}
TGlobalResource<FLexUIPostProcessCopyMeshRegionVertexDeclaration> GLGUIPostProcessCopyMeshRegionVertexDeclaration;
FVertexDeclarationRHIRef& GetLexUIPostProcessCopyMeshRegionVertexDeclaration()
{
	return GLGUIPostProcessCopyMeshRegionVertexDeclaration.VertexDeclarationRHI;
}

