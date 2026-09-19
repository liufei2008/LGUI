// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SceneManagement.h"
#include "MeshBatch.h"
#include "RHIResources.h"
#include "GlobalShader.h"

class FLexUIRenderer;
class FSceneViewFamily;
class FLexVisualBackBufferRenderProxy;

struct FLexUIMeshBatchContainer
{
	FMeshBatch Mesh;
	FBufferRHIRef VertexBufferRHI;
	FBufferRHIRef IndexBufferRHI;
	int32 NumVerts = 0;

	FLexUIMeshBatchContainer() {}
};

enum class ELexUIRendererPrimitiveType :uint8
{
	Mesh,
	BackBufferReader,
};

struct FLexUIRenderSectionProxy;
struct FLexUIPrimitiveSectionDataContainer
{
	FLexUIRenderSectionProxy* SectionPointer = nullptr;
};
struct FLexUIPrimitiveDataContainer
{
	class ILexUIRendererPrimitive* Primitive = nullptr;
	ELexUIRendererPrimitiveType Type;
	TArray<FLexUIPrimitiveSectionDataContainer> Sections;
};

class ILexUIRendererPrimitive
{
public:
	virtual ~ILexUIRendererPrimitive() {}

#if !UE_BUILD_SHIPPING
	FString DebugName = TEXT("DebugNameNone");
#endif
	virtual bool LexUI_CanRender() const = 0;
	virtual int LexUI_GetRenderPriority() const = 0;
	/** For world space renderer to tell visibility, e.g. SceneCapture2D */
	virtual FPrimitiveComponentId LexUI_GetPrimitiveComponentId() const = 0;
	virtual FVector3f LexUI_GetWorldPositionForSortTranslucent()const = 0;
	virtual FBoxSphereBounds LexUI_GetWorldBounds()const = 0;

	virtual void LexUI_CollectRenderData(TArray<FLexUIPrimitiveDataContainer>& OutRenderData) = 0;
	virtual void LexUI_GetMeshElements(const FSceneViewFamily& ViewFamily, FMeshElementCollector& Collector, const FLexUIPrimitiveDataContainer& PrimitiveData, TArray<FLexUIMeshBatchContainer>& ResultArray) = 0;
	virtual FLexVisualBackBufferRenderProxy* LexUI_GetBackBufferReaderElement(FLexUIRenderSectionProxy* SectionPtr)const = 0;
};
