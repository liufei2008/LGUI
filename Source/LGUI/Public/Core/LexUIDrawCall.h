// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "LexUIGeometry.h"
#include "Engine/Texture.h"
#include "Core/LexUIMeshIndex.h"
#include "Core/LexUIQuadTree.h"

class ULexVisualBackBufferReader;
struct FLexUIMeshVertex;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class ULexWidget;
class ULexVisualBatchMesh;
class ULexVisualDirectMesh;
class ULexUIMeshComponent;
struct FLexUIRenderSection;

enum class ELexUIDrawCallType :uint8
{
	BatchMesh = 1,
	BackBufferReader,
	DirectMesh,
	ChildCanvas,
};

class FLexUIRenderData
{
public:
	FLexUIRenderData(ELexUIDrawCallType InType)
	{
		Type = InType;
	}
	ELexUIDrawCallType Type = ELexUIDrawCallType::BatchMesh;

	FLexUIGeometry BatchMeshGeometry;
	TWeakObjectPtr<ULexVisualBatchMesh> BatchMeshVisualObject;

	TWeakObjectPtr<ULexVisualBackBufferReader> BackBufferReaderVisualObject;//post process object

	TWeakObjectPtr<ULexVisualDirectMesh> DirectMeshVisualObject;

	TWeakObjectPtr<ULexCanvas> ChildCanvas;
};

class LGUI_API FLexUIDrawCall
{
public:
	FLexUIDrawCall(ELexUIDrawCallType InType)
	{
		Type = InType;
	}
	FLexUIDrawCall(LexUIQuadTree::Rectangle InCanvasRect)
	{
		Type = ELexUIDrawCallType::BatchMesh;
		BatchMeshTreeNode = MakeShared<LexUIQuadTree::Node>(InCanvasRect);
	}
	~FLexUIDrawCall()
	{
		
	}
	ELexUIDrawCallType Type = ELexUIDrawCallType::BatchMesh;

	TWeakObjectPtr<UTexture> Texture = nullptr;//draw-call use this texture to render
	TWeakObjectPtr<UTexture> FontTexture = nullptr;//draw-call use this texture to render font
	TWeakObjectPtr<UMaterialInterface> Material = nullptr;//draw-call use this material to render, can be null to use default material
	TWeakObjectPtr<UMaterialInterface> RenderMaterial = nullptr;//actual material that render this draw-call

	TWeakObjectPtr<ULexVisualBackBufferReader> BackBufferReaderVisualObject;//post process object

	TWeakObjectPtr<ULexVisualDirectMesh> DirectMeshVisualObject;

	TArray<TWeakObjectPtr<ULexVisualBatchMesh>> BatchMeshVisualArray;
	TArray<FLexUIGeometry> BatchMeshGeometryArray;//BatchMesh's geometry collections belong to this draw-call, must be sorted on hierarchy-index
	TArray<FLexUIMeshVertex> CombinedBatchMeshGeometryVertices;
	TArray<FLexUIMeshIndex> CombinedBatchMeshGeometryTriangles;
	FBox CombinedBounds;
	bool bNeedToSortBatchMeshVisualObjectList = false;//need to sort BatchMeshRenderObjectList?
	TSharedPtr<LexUIQuadTree::Node> BatchMeshTreeNode = nullptr;
	int32 VerticesCount = 0;//vertices count of all BatchMeshRenderObjectList
	int32 IndicesCount = 0;//triangle indices count of all BatchMeshRenderObjectList

	bool bIs2DSpace = false;//transform relative to canvas is 2d or not? only 2d draw-call can batch

	TWeakObjectPtr<class ULexCanvas> ChildCanvas;//insert point to sort child canvas
public:
	void CopyBatchMeshGeometry();
	void ApplyBatchMeshGeometryToCombined();
	bool CanConsumeUIGeometryForBatchMesh(const FLexUIGeometry& geo)const;
};
