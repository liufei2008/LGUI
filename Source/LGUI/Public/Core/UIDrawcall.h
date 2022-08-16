// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Engine/Texture.h"
#include "Core/LGUIIndexBuffer.h"
#include "Core/UIQuadTree.h"

class UUIPostProcessRenderable;
class UIGeometry;
struct FLGUIMeshVertex;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UUIItem;
class UUIBatchGeometryRenderable;
class UUIDirectMeshRenderable;
class ULGUIMeshComponent;
struct FLGUIMeshSection;

enum class EUIDrawcallType :uint8
{
	BatchGeometry = 1,
	PostProcess,
	DirectMesh,
	ChildCanvas,
};

class LGUI_API UUIDrawcall
{
public:
	UUIDrawcall(EUIDrawcallType InType)
	{
		Type = InType;
	}
	UUIDrawcall(UIQuadTree::Rectangle InCanvasRect)
	{
		Type = EUIDrawcallType::BatchGeometry;
		RenderObjectListTreeRootNode = new UIQuadTree::Node(InCanvasRect);
	}
	~UUIDrawcall()
	{
		if (RenderObjectListTreeRootNode != nullptr)
		{
			delete RenderObjectListTreeRootNode;
			RenderObjectListTreeRootNode = nullptr;
		}
	}
	EUIDrawcallType Type = EUIDrawcallType::BatchGeometry;

	TWeakObjectPtr<UTexture> Texture = nullptr;//drawcall used this texture to render
	TWeakObjectPtr<UMaterialInterface> Material = nullptr;//drawcall use this material to render, can be null to use default material
	TWeakObjectPtr<UMaterialInterface> RenderMaterial = nullptr;//material that render this drawcall
	TWeakObjectPtr<ULGUIMeshComponent> DrawcallMesh = nullptr;//mesh for render this drawcall
	TWeakPtr<FLGUIMeshSection> DrawcallMeshSection = nullptr;//section of mesh which render this drawcall

	bool bMaterialContainsLGUIParameter = false;//if Material contains LGUI's parameter, then a MaterialInstanceDynamic will be created and stored as RenderMaterial, other wise RenderMaterial is same as Material
	bool bMaterialChanged = true;
	bool bMaterialNeedToReassign = true;//once a mesh section is recreated, and the material is still valid, then we need to re-assign the material to newly created mesh section
	bool bTextureChanged = true;

	bool bNeedToUpdateVertex = true;
	bool bVertexPositionChanged = true;//if vertex position changed? use for update bounds

	TWeakObjectPtr<UUIPostProcessRenderable> PostProcessRenderableObject;//post process object
	bool bNeedToAddPostProcessRenderProxyToRender = true;//this parameter is needed when: post process object have added to render before (means PostProcessRenderProxy is already created) and then removed, and now add to render again, so this will be true by default newly created drawcall

	TWeakObjectPtr<UUIDirectMeshRenderable> DirectMeshRenderableObject;

	TArray<TWeakObjectPtr<UUIBatchGeometryRenderable>> RenderObjectList;//render object collections belong to this drawcall, must sorted on hierarchy-index
	bool bNeedToSortRenderObjectList = false;//need to sort RenderObjectList?
	UIQuadTree::Node* RenderObjectListTreeRootNode = nullptr;
	int32 VerticesCount = 0;//vertices count of all renderObjectList
	int32 IndicesCount = 0;//triangle indices count of all renderObjectList

	bool bIs2DSpace = false;//transform relative to canvas is 2d or not? only 2d drawcall can batch

	TWeakObjectPtr<class ULGUICanvas> ChildCanvas;//insert point to sort child canvas
public:
	void GetCombined(TArray<FLGUIMeshVertex>& vertices, TArray<FLGUIIndexType>& triangles)const;
	void CopyUpdateState(UUIDrawcall* Target);
	bool CanConsumeUIBatchGeometryRenderable(UIGeometry* geo, int32 itemVertCount);
};
