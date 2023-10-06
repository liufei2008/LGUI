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
	int renderPriority = 0;
	FLGUIRenderSectionProxy* RenderProxy = nullptr;
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
};
struct FLGUIPostProcessSection : public FLGUIRenderSection
{
	FLGUIPostProcessSection()
	{
		Type = ELGUIRenderSectionType::PostProcess;
	}

	TWeakPtr<class FUIPostProcessRenderProxy> PostProcessRenderProxy = nullptr;
};
struct FLGUIChildCanvasSection : public FLGUIRenderSection
{
	FLGUIChildCanvasSection()
	{
		Type = ELGUIRenderSectionType::ChildCanvas;
	}

	class ULGUIMeshComponent* ChildCanvasMeshComponent = nullptr;
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
	void UpdateChildCanvasSectionRenderData(TSharedPtr<FLGUIRenderSection> InRenderSection);
	void DeleteRenderSection(TSharedPtr<FLGUIRenderSection> InRenderSection);
	TSharedPtr<FLGUIRenderSection> CreateRenderSection(ELGUIRenderSectionType type);
	void SetRenderSectionRenderPriority(TSharedPtr<FLGUIRenderSection> InRenderSection, int32 InSortPriority);
	void SortRenderSectionRenderPriority();
	void SetMeshSectionMaterial(TSharedPtr<FLGUIRenderSection> InMeshSection, UMaterialInterface* InMaterial);

	void SetRenderCanvas(ULGUICanvas* InCanvas);
	void SetSupportLGUIRenderer(bool supportOrNot, TWeakPtr<FLGUIRenderer, ESPMode::ThreadSafe> HudRenderer, bool InIsRenderToWorld);
	void SetSupportUERenderer(bool supportOrNot);

	void SetUITranslucentSortPriority(int32 NewTranslucentSortPriority);

	void VarifyMaterials();

	//~ Begin UPrimitiveComponent Interface.
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	//~ End UPrimitiveComponent Interface.

	//~ Begin UMeshComponent Interface.
	virtual int32 GetNumMaterials() const override;
	//~ End UMeshComponent Interface.

private:
	TArray<TSharedPtr<FLGUIRenderSection>> RenderSections;
	//~ Begin USceneComponent Interface.
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	//~ Begin USceneComponent Interface.

	/** Update LocalBounds member from the local box of each section */
	void UpdateLocalBounds();

	virtual void DestroyRenderState_Concurrent()override;

	friend class FLGUIRenderSceneProxy;

protected:
	TWeakPtr<FLGUIRenderer, ESPMode::ThreadSafe> LGUIRenderer;
	bool IsLGUIRenderToWorld = false;//LGUI renderer render to world or screen
	TWeakObjectPtr<ULGUICanvas> RenderCanvas = nullptr;
	bool IsSupportUERenderer = true;

public:
	DECLARE_MULTICAST_DELEGATE_TwoParams(FLGUIMeshSceneProxyCreateDeleteDelegate, ULGUIMeshComponent*, FLGUIRenderSceneProxy*);
	FLGUIMeshSceneProxyCreateDeleteDelegate OnSceneProxyCreated;
	FLGUIMeshSceneProxyCreateDeleteDelegate OnSceneProxyDeleted_RenderThread;
};


