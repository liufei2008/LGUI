// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "Components/MeshComponent.h"
#include "Core/LexUIMeshIndex.h"
#include "Core/LexUIMeshVertex.h"
#include "LexUIMeshComponent.generated.h"

class FLexUIDrawCall;
struct FLexUIRenderSectionProxy;
struct FLexUISectionProxy_Mesh;
struct FLexUIRenderSectionProxy_BackBufferReader;
struct FLexUIRenderSectionProxy_ChildCanvas;

#define DEBUG_PRINT_MESH_MEMORY 0

enum class ELexUIRenderSectionType :uint8
{
	Mesh, DirectMesh, BackBufferReader, ChildCanvas,
};
struct FLexUIRenderSection
{
	FLexUIRenderSection(){};
	virtual ~FLexUIRenderSection(){}
	ELexUIRenderSectionType Type = ELexUIRenderSectionType::Mesh;
	int RenderPriority = 0;
	FLexUIRenderSectionProxy* RenderProxy = nullptr;
	FBox BoundingBox = FBox(EForceInit::ForceInit);//world space bounding box

	virtual void ClearBeforePool() = 0;
};
struct FLexUIRenderSection_Mesh : public FLexUIRenderSection
{
	FLexUIRenderSection_Mesh() 
	{
		Type = ELexUIRenderSectionType::Mesh; 
	}
	virtual ~FLexUIRenderSection_Mesh()override{}

	TArray<FLexUIMeshIndex> TriangleIndices;
	TArray<FLexUIMeshVertex> Vertices;
	int32 ValidVerticesNum = 0;
	int32 ValidTriangleIndicesNum = 0;

	UMaterialInterface* Material = nullptr;

	void Reset()
	{
		Vertices.Reset();
		TriangleIndices.Reset();
	}
	virtual void ClearBeforePool() override;
};
struct FLexUIRenderSection_DirectMesh : public FLexUIRenderSection_Mesh
{
	FLexUIRenderSection_DirectMesh()
	{
		Type = ELexUIRenderSectionType::DirectMesh;
	}
	virtual ~FLexUIRenderSection_DirectMesh()override{}

	TWeakObjectPtr<class ULexVisualDirectMesh> DirectMeshVisualObject = nullptr;
};
struct FLexUIRenderSection_BackBufferReader : public FLexUIRenderSection
{
	FLexUIRenderSection_BackBufferReader()
	{
		Type = ELexUIRenderSectionType::BackBufferReader;
	}
	virtual ~FLexUIRenderSection_BackBufferReader()override{}

	TWeakObjectPtr<class ULexVisualBackBufferReader> BackBufferReaderVisualObject = nullptr;

	virtual void ClearBeforePool() override;
};
struct FLexUIRenderSection_ChildCanvas : public FLexUIRenderSection
{
	FLexUIRenderSection_ChildCanvas()
	{
		Type = ELexUIRenderSectionType::ChildCanvas;
	}
	virtual ~FLexUIRenderSection_ChildCanvas()override{}

	TWeakObjectPtr<class ULexUIMeshComponent> ChildCanvasMeshComponent = nullptr;

	virtual void ClearBeforePool() override;
};

class FLexUIRenderer;
class ILexUIRendererPrimitive;
class ULexCanvas;

DECLARE_MULTICAST_DELEGATE_TwoParams(FLexUIMeshSceneProxyCreateDeleteDelegate, class ULexUIMeshComponent*, class FLexUIRenderSceneProxy*);

//LexUI render mesh
//@todo: split this class to: one for UE renderer && one for LexUI renderer, will it be more efficient?
UCLASS(ClassGroup = (LGUI))
class LGUI_API ULexUIMeshComponent : public UMeshComponent
{
	GENERATED_BODY()

public:
	ULexUIMeshComponent();
	virtual void PostInitProperties() override;
private:
	void UpdateMeshSectionRenderData(FLexUIRenderSection_Mesh* InMeshSection, bool InRequireNormalAndTangent);
	void ExpandMeshSectionRenderData(FLexUIRenderSection_Mesh* InMeshSection);
public:
	TSharedPtr<FLexUIRenderSection> SetupRenderSection(ELexUIRenderSectionType InType, FLexUIDrawCall* InDrawCallData);
	void UpdateMeshSection(int Index, FLexUIDrawCall* InDrawCallData);
	void SetupDirectMeshRenderSection(FLexUIRenderSection_DirectMesh* InDirectMeshSection, bool bNeedExpandMeshSection, UMaterialInterface* InMaterial);
	void SetDirectMeshRenderSectionMaterial(FLexUIRenderSection_DirectMesh* InDirectMeshSection, UMaterialInterface* InMaterial);
	void PoolAllRenderSection();
	void SetRenderSectionRenderPriority(int32 InSectionIndex, int32 InSortPriority);
	void SetMeshSectionMaterial(int32 InSectionIndex, UMaterialInterface* InMaterial);

	void Init(ULexCanvas* InCanvas);
	void SetSupportLexUIRenderer(bool InSupportOrNot, TWeakPtr<FLexUIRenderer, ESPMode::ThreadSafe> InLexUIRenderer, bool InIsRenderToWorld);
	void SetSupportUERenderer(bool InSupportOrNot);
	void ClearRenderData();
	void FlushRenderCommand();

	void SetUITranslucentSortPriority(int32 NewTranslucentSortPriority);

	void VerifyMaterials();
	void SetParentCanvasMeshComp(ULexUIMeshComponent* InParentCanvasMeshComp);
	void ClearParentCanvasMeshComp(ULexUIMeshComponent* InParentCanvasMeshComp);

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
	TArray<TSharedPtr<FLexUIRenderSection>> RenderSectionArray;
	TDoubleLinkedList<TSharedPtr<FLexUIRenderSection>> RenderSectionPool;
	struct FMeshRenderSectionPool
	{
		FMeshRenderSectionPool() = default;
		FMeshRenderSectionPool(const FMeshRenderSectionPool& Other)
		{
		}
		TDoubleLinkedList<TSharedPtr<FLexUIRenderSection_Mesh>> RenderSections;
	};
	TArray<FMeshRenderSectionPool> RenderSectionMesh_CascadePool;//for mesh section pool, sorted by vertex buffer size, to prevent memory waste of big vertex and index buffer
	TDoubleLinkedList<TSharedPtr<FLexUIRenderSection_Mesh>>& GetRenderSectionMeshPool(int32 InNumVertices);
#if DEBUG_PRINT_MESH_MEMORY
	int ExpandMeshSectionCount = 0;
#endif
	//~ Begin USceneComponent Interface.
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	//~ Begin USceneComponent Interface.

	struct UpdateMeshSectionDataStruct
	{
		TArray<FLexUIMeshVertex> VertexBufferData;
		int32 NumVerts;
		int32 NumTriangles;
		TArray<FLexUIMeshIndex> IndexBufferData;
		bool RequireNormalAndTangent;
		FLexUISectionProxy_Mesh* Section;
	};
	TArray<UpdateMeshSectionDataStruct> PendingUpdateMeshSectionDataArray;
	struct UpdateRenderSectionPriority
	{
		FLexUIRenderSectionProxy* SectionProxy;
		int RenderPriority;
	};
	TArray<UpdateRenderSectionPriority> PendingUpdateRenderSectionPriorityArray;
	struct UpdateMeshSectionMaterialDataStruct
	{
		FLexUIRenderSectionProxy* SectionProxy;
		UMaterialInterface* Material;
	};
	TArray<UpdateMeshSectionMaterialDataStruct> PendingUpdateMeshSectionMaterialDataArray;

	friend class FLexUIRenderSceneProxy;

protected:
	TWeakPtr<FLexUIRenderer, ESPMode::ThreadSafe> LexUIRenderer;
	bool bIsLexUIRenderToWorld = false;//LexUI renderer render to world or screen
	TWeakObjectPtr<ULexCanvas> RenderCanvas = nullptr;
	bool bIsSupportUERenderer = true;
	TWeakObjectPtr<ULexUIMeshComponent> ParentCanvasMeshComp = nullptr;

public:
	FLexUIMeshSceneProxyCreateDeleteDelegate OnSceneProxyCreated;
};


