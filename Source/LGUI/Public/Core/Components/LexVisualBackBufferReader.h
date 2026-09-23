// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "LexVisual.h"
#include "Core/LexUIRender/LexUIPostProcessVertex.h"
#include "LexVisualBackBufferReader.generated.h"

class FLexVisualBackBufferRenderProxy;
struct FLexUIPostProcessVertex;

UENUM(BlueprintType)
enum class ELexVisualBackBufferReaderMode : uint8
{
	/** use the UI element's rect in viewport */
	Rect,
	/** use full viewport rect */
	Viewport,
};

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

	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
	virtual void OnRegister() override;
	virtual void OnUnregister() override;

	TSharedPtr<FLexUIGeometry> Geometry = nullptr;
	virtual void UpdateGeometry()override final;
	virtual void PostUpdateDrawCall();

	virtual void OnDimensionChanged(bool InPivotChange, bool InWidthChange, bool InHeightChange)override;
	virtual void OnTransformChanged(bool InPositionChanged, bool InScaleChanged) override;
	virtual void MarkAllDirty()override;

protected:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif
	
	friend class FLexBackBufferReaderCustomization;
	UPROPERTY(EditAnywhere, Category="LGUI", BlueprintReadWrite, Getter, Setter)
	ELexVisualBackBufferReaderMode BackBufferReaderType = ELexVisualBackBufferReaderMode::Rect;
public:
	
	FLexUIGeometry* GetGeometry()const { return Geometry.Get(); }
	
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	FBox2f Get2dBoxInScreen()const{return MeshRectInScreen;}
	/** same as Get2dBoxInScreen but normalized to 0~1, xy- min, zw- size */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	FVector4f Get2dBoxInScreen01()const{return RectInScreen01;}
	
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	ELexVisualBackBufferReaderMode GetBackBufferReaderType()const{return BackBufferReaderType;}
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	void SetBackBufferReaderType(ELexVisualBackBufferReaderMode InBackBufferReaderType);

	void MarkVertexPositionDirty();
	void MarkUVDirty();
	
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
	virtual void OnUpdateGeometry(bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged);
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
