// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Engine/Texture.h"
#include "Core/LGUIIndexBuffer.h"

class UUIPostProcessRenderable;
class UIGeometry;
struct FDynamicMeshVertex;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UUIItem;
class UUIBatchGeometryRenderable;
class UUIDirectMeshRenderable;

enum class EUIDrawcallType :uint8
{
	BatchGeometry = 1,
	PostProcess,
	DirectMesh,
};

class LGUI_API UUIDrawcall
{
public:
	EUIDrawcallType type;

	TArray<TSharedPtr<UIGeometry>> geometryList;//UI geometries that construct this drawcall
	TWeakObjectPtr<UTexture> texture = nullptr;//drawcall used this texture to render
	TWeakObjectPtr<UMaterialInterface> material = nullptr;//drawcall use this material to render, can be null to use default material
	TWeakObjectPtr<UMaterialInstanceDynamic> materialInstanceDynamic = nullptr;//created MaterialInstanceDynamic that render this drawcall

	bool needToBeRebuild = false;//rebuild only if need to
	bool needToUpdateVertex = false;//update only if need to
	bool vertexPositionChanged = false;//if vertex position changed? use for update bounds

	TWeakObjectPtr<UUIPostProcessRenderable> postProcessObject;//post process object

	TWeakObjectPtr<UUIDirectMeshRenderable> directMeshRenderableObject;

	int depthMin = 0;//min depth of all geometries
	int depthMax = 0;//max depth of all geometries
#pragma region AutoManageDepth
	TArray<TWeakObjectPtr<UUIBatchGeometryRenderable>> renderObjectList;//render object collections belong to this drawcall
	bool is3DDrawcall = false;//transform relative to canvas is 3d or not? only 2d drawcall can batch
#pragma endregion
public:
	void GetCombined(TArray<FDynamicMeshVertex>& vertices, TArray<FLGUIIndexType>& triangles)const;
	void UpdateData(TArray<FDynamicMeshVertex>& vertices, TArray<FLGUIIndexType>& triangles);
	//update the max and min depth
	void UpdateDepthRange();
	
	bool IsDepthInsideDrawcall(int depth)
	{
		return depth >= depthMin && depth < depthMax;
	}
	void Clear();
	bool Equals(UUIDrawcall* Other);

	static bool CompareDrawcallList(const TArray<TSharedPtr<UUIDrawcall>>& A, const TArray<TSharedPtr<UUIDrawcall>>& B);
	static void CopyDrawcallList(const TArray<TSharedPtr<UUIDrawcall>>& From, TArray<TSharedPtr<UUIDrawcall>>& To);

	//create drawcall
	static void CreateDrawcall(TArray<TWeakObjectPtr<class UUIBaseRenderable>>& sortedList, TArray<TSharedPtr<class UUIDrawcall>>& drawcallList);
	static void CreateDrawcallForAutoManageDepth(TArray<TWeakObjectPtr<class UUIBaseRenderable>>& sortedList, TArray<TSharedPtr<class UUIDrawcall>>& drawcallList);
private:
	static TSharedPtr<class UUIDrawcall> GetAvailableDrawcall(TArray<TSharedPtr<UUIDrawcall>>& drawcallList, int& prevDrawcallListCount, int& drawcallCount);
	static bool Is2DUITransform(const FTransform& Transform);
};
