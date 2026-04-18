// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SceneManagement.h"
#include "MeshBatch.h"
#include "RHIResources.h"
#include "GlobalShader.h"
#include "SceneTextures.h"

class FLGUIRenderer;
class FSceneViewFamily;
class FUIPostProcessRenderProxy;
enum class ELGUICanvasDepthMode :uint8;

struct FLGUIMeshBatchContainer
{
	FMeshBatch Mesh;
	FBufferRHIRef VertexBufferRHI;
	int32 NumVerts = 0;

	FLGUIMeshBatchContainer() {}
};

enum class ELGUIRendererPrimitiveType :uint8
{
	Mesh,
	PostProcess,
};

struct FLGUIPrimitiveSectionDataContainer
{
	void* SectionPointer = nullptr;
};
struct FLGUIPrimitiveDataContainer
{
	class ILGUIRendererPrimitive* Primitive = nullptr;
	ELGUIRendererPrimitiveType Type;
	TArray<FLGUIPrimitiveSectionDataContainer> Sections;
};

class ILGUIRendererPrimitive
{
public:
	virtual ~ILGUIRendererPrimitive() {}

	virtual bool LGUI_CanRender() const = 0;
	virtual int LGUI_GetRenderPriority() const = 0;
	/** For world space renderer to tell visibility, eg SceneCapture2D */
	virtual FPrimitiveComponentId LGUI_GetPrimitiveComponentId() const = 0;
	virtual FVector3f LGUI_GetWorldPositionForSortTranslucent()const = 0;
	virtual FBoxSphereBounds LGUI_GetWorldBounds()const = 0;

	virtual void LGUI_CollectRenderData(TArray<FLGUIPrimitiveDataContainer>& OutRenderData, float CurrentWorldTime) = 0;
	virtual void LGUI_GetMeshElements(const FSceneViewFamily& ViewFamily, FMeshElementCollector* Collector, const FLGUIPrimitiveDataContainer& PrimitiveData, TArray<FLGUIMeshBatchContainer>& ResultArray) = 0;
	virtual FUIPostProcessRenderProxy* LGUI_GetPostProcessElement(const void* SectionPtr)const = 0;
};
