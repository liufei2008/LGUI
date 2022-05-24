// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Core/LGUIMeshVertex.h"


void FLGUIMeshVertexDeclaration::InitRHI()
{
	FVertexDeclarationElementList Elements;
	uint32 Stride = sizeof(FLGUIMeshVertex);
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FLGUIMeshVertex, Position), VET_Float3, 0, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FLGUIMeshVertex, Color), VET_Color, 1, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FLGUIMeshVertex, TextureCoordinate) + 0, VET_Float2, 2, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FLGUIMeshVertex, TextureCoordinate) + 8, VET_Float2, 3, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FLGUIMeshVertex, TextureCoordinate) + 16, VET_Float2, 4, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FLGUIMeshVertex, TextureCoordinate) + 24, VET_Float2, 5, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FLGUIMeshVertex, TangentX), VET_PackedNormal, 6, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FLGUIMeshVertex, TangentZ), VET_PackedNormal, 7, Stride));
	VertexDeclarationRHI = RHICreateVertexDeclaration(Elements);
}
void FLGUIMeshVertexDeclaration::ReleaseRHI()
{
	VertexDeclarationRHI.SafeRelease();
}
TGlobalResource<FLGUIMeshVertexDeclaration> GLGUIVertexDeclaration;
FVertexDeclarationRHIRef& GetLGUIMeshVertexDeclaration()
{
	return GLGUIVertexDeclaration.VertexDeclarationRHI;
}
