// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/LGUIMeshVertex.h"
#include "RHI.h"


void FLGUIMeshVertexDeclaration::InitRHI()
{
	FVertexDeclarationElementList Elements;
	uint32 Stride = sizeof(FLGUIMeshVertex);
	uint16 Index = 0;
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FLGUIMeshVertex, Position), VET_Float3, Index++, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FLGUIMeshVertex, Color), VET_Color, Index++, Stride));
	for (int i = 0; i < LGUI_VERTEX_TEXCOORDINATE_COUNT; i++)
	{
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FLGUIMeshVertex, TextureCoordinate) + i * 8, VET_Float2, Index++, Stride));
	}
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FLGUIMeshVertex, TangentX), VET_PackedNormal, Index++, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FLGUIMeshVertex, TangentZ), VET_PackedNormal, Index++, Stride));
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
