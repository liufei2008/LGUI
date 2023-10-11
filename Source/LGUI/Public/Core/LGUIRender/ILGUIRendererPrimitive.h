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

	virtual bool CanRender() const = 0;
	virtual int GetRenderPriority() const = 0;
	/** For world space renderer to tell visibility, eg SceneCapture2D */
	virtual FPrimitiveComponentId GetPrimitiveComponentId() const = 0;
	virtual FVector3f GetWorldPositionForSortTranslucent()const = 0;
	virtual FBoxSphereBounds GetWorldBounds()const = 0;

	virtual void CollectRenderData(TArray<FLGUIPrimitiveDataContainer>& OutRenderData) = 0;
	virtual void GetMeshElements(const FSceneViewFamily& ViewFamilyclass, FMeshElementCollector* Collector, const FLGUIPrimitiveDataContainer& PrimitiveData, TArray<FLGUIMeshBatchContainer>& ResultArray) = 0;
	virtual FUIPostProcessRenderProxy* GetPostProcessElement(const void* SectionPtr)const = 0;
	virtual bool PostProcessRequireOriginScreenColorTexture()const = 0;
};
