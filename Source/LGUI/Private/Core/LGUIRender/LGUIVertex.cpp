// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/LGUIRender/LGUIVertex.h"


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
