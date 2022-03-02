// Copyright 2019-2022 LexLiu. All Rights Reserved.

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
class ULGUIMeshComponent;
struct FLGUIMeshSection;

enum class EUIDrawcallType :uint8
{
	None,
	BatchGeometry = 1,
	PostProcess,
	DirectMesh,
};

class LGUI_API UUIDrawcall
{
public:
	EUIDrawcallType type = EUIDrawcallType::None;

	TWeakObjectPtr<UTexture> texture = nullptr;//drawcall used this texture to render
	TWeakObjectPtr<UMaterialInterface> material = nullptr;//drawcall use this material to render, can be null to use default material
	TWeakObjectPtr<UMaterialInstanceDynamic> materialInstanceDynamic = nullptr;//created MaterialInstanceDynamic that render this drawcall
	TWeakObjectPtr<ULGUIMeshComponent> drawcallMesh = nullptr;//mesh for render this drawcall
	TWeakPtr<FLGUIMeshSection> drawcallMeshSection = nullptr;//section of mesh which render this drawcall

	bool materialChanged = true;
	bool materialNeedToReassign = true;//once a mesh section is recreated, and the material is still valid, then we need to re-assign the material to newly created mesh section
	bool textureChanged = true;

	bool needToUpdateVertex = true;
	bool vertexPositionChanged = true;//if vertex position changed? use for update bounds

	TWeakObjectPtr<UUIPostProcessRenderable> postProcessRenderableObject;//post process object
	bool needToAddPostProcessRenderProxyToRender = true;//this parameter is needed when: post process object have added to render before (means PostProcessRenderProxy is already created) and then removed, and now add to render again, so this will be true by default newly created drawcall

	TWeakObjectPtr<UUIDirectMeshRenderable> directMeshRenderableObject;

	TArray<TWeakObjectPtr<UUIBatchGeometryRenderable>> renderObjectList;//render object collections belong to this drawcall, must sorted on hierarchy-index
	bool shouldSortRenderObjectList = false;//should sort renderObjectList

	bool bIs2DSpace = false;//transform relative to canvas is 2d or not? only 2d drawcall can batch

	TWeakObjectPtr<ULGUICanvas> manageCanvas;//this drawcall's manage canvas. a canvas could render by other canvas, but this manageCanvas stores the direct manage canvas, not the actual render canvas
public:
	void GetCombined(TArray<FDynamicMeshVertex>& vertices, TArray<FLGUIIndexType>& triangles)const;
	void CopyUpdateState(UUIDrawcall* Target);
};
