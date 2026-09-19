// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "LexVisual.h"
#include "Core/LexUIRender/LexUIPostProcessVertex.h"
#include "LexVisualBackBufferReader.generated.h"

class FLexVisualBackBufferRenderProxy;
struct FLexUIPostProcessVertex;

/** 
 * UI element that can access back-buffer image.
 * Only valid on LexUIRenderer (ScreenSpaceUI or WorldSpace-LexUIRenderer).
 */
UCLASS(Abstract, NotBlueprintable)
class LGUI_API ULexVisualBackBufferReader : public ULexVisual
{
	GENERATED_BODY()

public:
	ULexVisualBackBufferReader(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif
	TSharedPtr<FLexUIGeometry> Geometry = nullptr;
	virtual void UpdateGeometry()override final;
	virtual void PostUpdateDrawCall();

	virtual void OnDimensionChanged(bool InPivotChange, bool InWidthChange, bool InHeightChange)override;
	virtual void OnTransformChanged(bool InPositionChanged, bool InScaleChanged) override;
	virtual void MarkAllDirty()override;

protected:
	friend class FLexBackBufferReaderCustomization;
	
public:
	
	FLexUIGeometry* GetGeometry()const { return Geometry.Get(); }
	
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	FBox2f Get2dBoxInScreen()const{return MeshRectInScreen;}
	/** same as Get2dBoxInScreen but normalized to 0~1, xy- min, zw- size */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	FVector4f Get2dBoxInScreen01()const{return RectInScreen01;}

public:
	void MarkVertexPositionDirty();
	void MarkUVDirty();
public:
	virtual FLexVisualBackBufferRenderProxy* GetRenderProxy()PURE_VIRTUAL(UUIPostProcessRenderable::GetRenderProxy, return 0;);
	virtual bool HaveValidData()const;

	virtual bool LineTraceUI(FLexUIHitResult& OutHit, const FVector& Start, const FVector& End)const override;
protected:
	/** local vertex position changed */
	uint8 bLocalVertexPositionChanged : 1;
	/** vertex's uv change */
	uint8 bUVChanged : 1;
	/** widget's transform or visual's mesh changed */
	uint8 bWidgetOrGeometryDirty : 1;
	FMatrix CacheViewProjectionMatrix = FMatrix::Identity;
	FIntRect CacheViewRect = FIntRect();
	FLexVisualBackBufferRenderProxy* RenderProxy = nullptr;
	/** update ui geometry */
	virtual void OnUpdateGeometry(bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged);
	/** update region vertex data */
	virtual void UpdateRegionVertex(FIntPoint InViewportSize);
	void UpdateGeometryClipData(FLexUIGeometry& InMesh, int InDataStartPosition);
	TArray<FLexUIPostProcessCopyMeshRegionVertex, TFixedAllocator<4>> RenderScreenToMeshRegionVertexArray;
	TArray<FLexUIPostProcessVertex, TFixedAllocator<4>> RenderMeshRegionToScreenVertexArray;
	FBox2f MeshRectInScreen;
	/** xy- min, zw- size */
	FVector4f RectInScreen01;

	virtual void SendRegionVertexDataToRenderProxy();
};
