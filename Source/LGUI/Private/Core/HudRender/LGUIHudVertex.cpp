// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Core/HudRender/LGUIHudVertex.h"


void FLGUIHudVertexDeclaration::InitRHI()
{
	FVertexDeclarationElementList Elements;
	uint32 Stride = sizeof(FLGUIHudVertex);
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FLGUIHudVertex, Position), VET_Float3, 0, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FLGUIHudVertex, Color), VET_Color, 1, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FLGUIHudVertex, TextureCoordinate) + 0, VET_Float2, 2, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FLGUIHudVertex, TextureCoordinate) + 8, VET_Float2, 3, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FLGUIHudVertex, TextureCoordinate) + 16, VET_Float2, 4, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FLGUIHudVertex, TextureCoordinate) + 24, VET_Float2, 5, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FLGUIHudVertex, TangentX), VET_PackedNormal, 6, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FLGUIHudVertex, TangentZ), VET_PackedNormal, 7, Stride));
	VertexDeclarationRHI = RHICreateVertexDeclaration(Elements);
}
void FLGUIHudVertexDeclaration::ReleaseRHI()
{
	VertexDeclarationRHI.SafeRelease();
}
TGlobalResource<FLGUIHudVertexDeclaration> GLGUIVertexDeclaration;
FVertexDeclarationRHIRef& GetLGUIHudVertexDeclaration()
{
	return GLGUIVertexDeclaration.VertexDeclarationRHI;
}


void FLGUIPostProcessVertexDeclaration::InitRHI()
{
	FVertexDeclarationElementList Elements;
	uint16 Stride = sizeof(FLGUIPostProcessVertex);
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FLGUIPostProcessVertex, Position), VET_Float3, 0, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FLGUIPostProcessVertex, TextureCoordinate0), VET_Float2, 1, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FLGUIPostProcessVertex, TextureCoordinate1), VET_Float2, 2, Stride));
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




void FLGUIPostProcessCopyMeshRegionVertexDeclaration::InitRHI()
{
	FVertexDeclarationElementList Elements;
	uint16 Stride = sizeof(FLGUIPostProcessCopyMeshRegionVertex);
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FLGUIPostProcessCopyMeshRegionVertex, ScreenPosition), VET_Float3, 0, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FLGUIPostProcessCopyMeshRegionVertex, LocalPosition), VET_Float3, 1, Stride));
	VertexDeclarationRHI = RHICreateVertexDeclaration(Elements);
}
void FLGUIPostProcessCopyMeshRegionVertexDeclaration::ReleaseRHI()
{
	VertexDeclarationRHI.SafeRelease();
}
TGlobalResource<FLGUIPostProcessCopyMeshRegionVertexDeclaration> GLGUIPostProcessCopyMeshRegionVertexDeclaration;
FVertexDeclarationRHIRef& GetLGUIPostProcessCopyMeshRegionVertexDeclaration()
{
	return GLGUIPostProcessCopyMeshRegionVertexDeclaration.VertexDeclarationRHI;
}



void FLGUIHelperLineVertexDeclaration::InitRHI()
{
	FVertexDeclarationElementList Elements;
	uint16 Stride = sizeof(FLGUIHelperLineVertex);
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FLGUIHelperLineVertex, Position), VET_Float3, 0, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FLGUIHelperLineVertex, Color), VET_Color, 1, Stride));
	VertexDeclarationRHI = RHICreateVertexDeclaration(Elements);
}
void FLGUIHelperLineVertexDeclaration::ReleaseRHI()
{
	VertexDeclarationRHI.SafeRelease();
}
TGlobalResource<FLGUIHelperLineVertexDeclaration> GLGUIHelperLineVertexDeclaration;
FVertexDeclarationRHIRef& GetLGUIHelperLineVertexDeclaration()
{
	return GLGUIHelperLineVertexDeclaration.VertexDeclarationRHI;
}
