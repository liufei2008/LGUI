// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "Components/MeshComponent.h"
#include "Core/LGUIMeshIndex.h"
#include "Core/LGUIMeshVertex.h"
#include "DynamicMeshBuilder.h"
#include "LGUIMeshComponent.generated.h"

struct FLGUIRenderSectionProxy;
struct FLGUIMeshSectionProxy;
struct FLGUIPostProcessSectionProxy;
struct FLGUIChildCanvasSectionProxy;

enum class ELGUIRenderSectionType :uint8
{
	Mesh, PostProcess, ChildCanvas,
};
struct FLGUIRenderSection
{
	ELGUIRenderSectionType Type;
	int RenderPriority = 0;
	FLGUIRenderSectionProxy* RenderProxy = nullptr;
	FBox BoundingBox;//world space bounding box

	virtual void UpdateSectionBox(const FTransform& LocalToWorld) = 0;
};
struct FLGUIMeshSection : public FLGUIRenderSection
{
	FLGUIMeshSection() 
	{
		Type = ELGUIRenderSectionType::Mesh; 
	}
	virtual ~FLGUIMeshSection()
	{

	}

	TArray<FLGUIMeshIndexBufferType> triangles;
	TArray<FLGUIMeshVertex> vertices;

	int prevVertexCount = 0;
	int prevIndexCount = 0;

	UMaterialInterface* material = nullptr;

	void Reset()
	{
		vertices.Reset();
		triangles.Reset();
	}
	virtual void UpdateSectionBox(const FTransform& LocalToWorld) override;
};
struct FLGUIPostProcessSection : public FLGUIRenderSection
{
	FLGUIPostProcessSection()
	{
		Type = ELGUIRenderSectionType::PostProcess;
	}

	TWeakObjectPtr<class UUIPostProcessRenderable> PostProcessRenderableObject = nullptr;

	virtual void UpdateSectionBox(const FTransform& LocalToWorld) override;
};
struct FLGUIChildCanvasSection : public FLGUIRenderSection
{
	FLGUIChildCanvasSection()
	{
		Type = ELGUIRenderSectionType::ChildCanvas;
	}

	class ULGUIMeshComponent* ChildCanvasMeshComponent = nullptr;

	virtual void UpdateSectionBox(const FTransform& LocalToWorld) override;
};

class FLGUIRenderer;
class ILGUIRendererPrimitive;
class ULGUICanvas;

//LGUI's mesh
//@todo: split this class to: one for UE renderer && one for LGUI renderer, will it be more efficient? or maybe a class without additional shader channels? 
UCLASS(ClassGroup = (LGUI), Blueprintable)
class LGUI_API ULGUIMeshComponent : public UMeshComponent
{
	GENERATED_BODY()

public:
	ULGUIMeshComponent();
	void CreateRenderSectionRenderData(TSharedPtr<FLGUIRenderSection> InRenderSection);
	void UpdateMeshSectionRenderData(TSharedPtr<FLGUIRenderSection> InRenderSection, bool InVertexPositionChanged, int8 AdditionalShaderChannelFlags);
	void DeleteRenderSection(TSharedPtr<FLGUIRenderSection> InRenderSection);
	TSharedPtr<FLGUIRenderSection> CreateRenderSection(ELGUIRenderSectionType type);
	void SetRenderSectionRenderPriority(TSharedPtr<FLGUIRenderSection> InRenderSection, int32 InSortPriority);
	void SetMeshSectionMaterial(TSharedPtr<FLGUIRenderSection> InMeshSection, UMaterialInterface* InMaterial);

	void SetRenderCanvas(ULGUICanvas* InCanvas);
	void SetSupportLGUIRenderer(bool supportOrNot, TWeakPtr<FLGUIRenderer, ESPMode::ThreadSafe> HudRenderer, bool InIsRenderToWorld);
	void SetSupportUERenderer(bool supportOrNot);
	void ClearRenderData();

	void SetUITranslucentSortPriority(int32 NewTranslucentSortPriority);

	void VarifyMaterials();

	//~ Begin UPrimitiveComponent Interface.
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	//~ End UPrimitiveComponent Interface.

	//~ Begin UMeshComponent Interface.
	virtual int32 GetNumMaterials() const override;
	//~ End UMeshComponent Interface.

	/** Update LocalBounds member from the local box of each section */
	void UpdateLocalBounds();
	void UpdateChildCanvasSectionBox();
private:
	TArray<TSharedPtr<FLGUIRenderSection>> RenderSections;
	//~ Begin USceneComponent Interface.
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	//~ Begin USceneComponent Interface.

	friend class FLGUIRenderSceneProxy;

protected:
	TWeakPtr<FLGUIRenderer, ESPMode::ThreadSafe> LGUIRenderer;
	bool bIsLGUIRenderToWorld = false;//LGUI renderer render to world or screen
	TWeakObjectPtr<ULGUICanvas> RenderCanvas = nullptr;
	bool bIsSupportUERenderer = true;

public:
	DECLARE_MULTICAST_DELEGATE_TwoParams(FLGUIMeshSceneProxyCreateDeleteDelegate, ULGUIMeshComponent*, class FLGUIRenderSceneProxy*);
	FLGUIMeshSceneProxyCreateDeleteDelegate OnSceneProxyCreated;
	FLGUIMeshSceneProxyCreateDeleteDelegate OnSceneProxyDeleted_RenderThread;
};


