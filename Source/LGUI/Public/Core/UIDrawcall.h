// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Engine/Texture.h"
#include "Core/LGUIMeshIndex.h"
#include "Core/UIQuadTree.h"

class UUIPostProcessRenderable;
class UIGeometry;
struct FLGUIMeshVertex;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UUIItem;
class UUIBatchMeshRenderable;
class UUIDirectMeshRenderable;
class ULGUIMeshComponent;
struct FLGUIRenderSection;

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
		RenderObjectListTreeRootNode = MakeUnique<UIQuadTree::Node>(InCanvasRect);
	}
	~UUIDrawcall()
	{
		
	}
	EUIDrawcallType Type = EUIDrawcallType::BatchGeometry;

	TWeakObjectPtr<UTexture> Texture = nullptr;//drawcall used this texture to render
	TWeakObjectPtr<UMaterialInterface> Material = nullptr;//drawcall use this material to render, can be null to use default material
	TWeakObjectPtr<UMaterialInterface> RenderMaterial = nullptr;//material that render this drawcall
	TWeakObjectPtr<ULGUIMeshComponent> DrawcallMesh = nullptr;//mesh for render this drawcall
	TWeakPtr<FLGUIRenderSection> DrawcallRenderSection = nullptr;//section of mesh which render this drawcall

	bool bMaterialContainsLGUIParameter = false;//if Material contains LGUI's parameter, then a MaterialInstanceDynamic will be created and stored as RenderMaterial, other wise RenderMaterial is same as Material
	bool bMaterialChanged = true;
	bool bMaterialNeedToReassign = true;//once a mesh section is recreated, and the material is still valid, then we need to re-assign the material to newly created mesh section
	bool bTextureChanged = true;
	bool bIsSDFFont = false;//sdf font need special material

	bool bNeedToUpdateVertex = true;
	bool bVertexPositionChanged = true;//if vertex position changed? use for update bounds

	TWeakObjectPtr<UUIPostProcessRenderable> PostProcessRenderableObject;//post process object

	TWeakObjectPtr<UUIDirectMeshRenderable> DirectMeshRenderableObject;

	TArray<TWeakObjectPtr<UUIBatchMeshRenderable>> RenderObjectList;//render object collections belong to this drawcall, must sorted on hierarchy-index
	bool bNeedToSortRenderObjectList = false;//need to sort RenderObjectList?
	TUniquePtr<UIQuadTree::Node> RenderObjectListTreeRootNode = nullptr;
	int32 VerticesCount = 0;//vertices count of all renderObjectList
	int32 IndicesCount = 0;//triangle indices count of all renderObjectList

	bool bIs2DSpace = false;//transform relative to canvas is 2d or not? only 2d drawcall can batch

	TWeakObjectPtr<class ULGUICanvas> ChildCanvas;//insert point to sort child canvas
public:
	void GetCombined(TArray<FLGUIMeshVertex>& vertices, TArray<FLGUIMeshIndexBufferType>& triangles)const;
	void CopyUpdateState(UUIDrawcall* Target);
	bool CanConsumeUIBatchMeshRenderable(UIGeometry* geo, int32 itemVertCount);
};
