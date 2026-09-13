// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "LexVisual.h"
#include "Core/LexUIRender/LexUIPostProcessVertex.h"
#include "LexVisualPostProcess.generated.h"

class FLexVisualPostProcessRenderProxy;
struct FLexUIPostProcessVertex;

UENUM(BlueprintType)
enum class ELexBackgroundBlurRenderType:uint8
{
	/** Render direct to screen */
	Screen,
	/** Output to a RenderTarget */
	RenderTarget,
};

/** 
 * UI element that can do post-processing effect on screen space.
 * Only valid on LexUIRenderer (ScreenSpaceUI or WorldSpace-LexUIRenderer).
 */
UCLASS(Abstract, NotBlueprintable)
class LGUI_API ULexVisualPostProcess : public ULexVisual
{
	GENERATED_BODY()

public:
	DECLARE_EVENT_OneParam(ULexVisualPostProcess, FRenderTargetChangedEvent, UTextureRenderTarget2D*);
	ULexVisualPostProcess(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
	virtual void OnUnregister() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif
	TSharedPtr<FLexUIGeometry> Geometry = nullptr;
	virtual void UpdateGeometry()override final;

	virtual void OnDimensionChanged(bool InPivotChange, bool InWidthChange, bool InHeightChange)override;
	virtual void OnTransformChanged(bool InPositionChanged, bool InScaleChanged) override;
	virtual void MarkAllDirty()override;

protected:
	friend class FLexVisualPostProcessCustomization;
	/** Use maskTexture's red channel to mask out effect result. */
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayThumbnail = "false"))
	TObjectPtr<UTexture2D> MaskTexture;
	/** MaskTexture UV offset and scale info. Only get good result when MaskTextureType is Simple */
	UPROPERTY(EditAnywhere, Category = "LGUI")
	FVector4 MaskTextureUVRect = FVector4(0, 0, 1, 1);
	UPROPERTY(EditAnywhere, Category = "LGUI")
	ELexBackgroundBlurRenderType RenderType = ELexBackgroundBlurRenderType::Screen;
	/**
	 * Blur result will output to this RenderTarget.
	 * Will create one if not specified.
	 */
	UPROPERTY(EditAnywhere, Category = "LGUI", meta=(EditCondition="RenderType==ELexBackgroundBlurRenderType::RenderTarget"))
	TObjectPtr<UTextureRenderTarget2D> OutputRenderTarget = nullptr;
	FRenderTargetChangedEvent OnRenderTargetChanged;
public:
	FRenderTargetChangedEvent& GetRenderTargetChangedEvent(){return OnRenderTargetChanged;}
	
	FLexUIGeometry* GetGeometry()const { return Geometry.Get(); }
	
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	UTexture2D* GetMaskTexture()const { return MaskTexture; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	const FVector4& GetMaskTextureUVRect()const { return MaskTextureUVRect; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	ELexBackgroundBlurRenderType GetRenderType()const { return RenderType; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	UTextureRenderTarget2D* GetOutputRenderTarget()const { return OutputRenderTarget; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
	void SetMaskTexture(UTexture2D* Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	void SetMaskTextureUVRect(const FVector4& Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	void SetRenderType(ELexBackgroundBlurRenderType Value);
public:
	void MarkVertexPositionDirty();
	void MarkUVDirty();
public:
	virtual FLexVisualPostProcessRenderProxy* GetRenderProxy()PURE_VIRTUAL(UUIPostProcessRenderable::GetRenderProxy, return 0;);
	virtual bool HaveValidData()const;

	virtual bool LineTraceUI(FLexUIHitResult& OutHit, const FVector& Start, const FVector& End)const override;
private:
	/** local vertex position changed */
	uint8 bLocalVertexPositionChanged : 1;
	/** vertex's uv change */
	uint8 bUVChanged : 1;
protected:
	FLexVisualPostProcessRenderProxy* RenderProxy = nullptr;
	/** update ui geometry */
	virtual void OnUpdateGeometry(bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged);
	/** update region vertex data */
	virtual void UpdateRegionVertex();
	void UpdateGeometryClipData(FLexUIGeometry& InMesh, int InDataStartPosition);
	TArray<FLexUIPostProcessCopyMeshRegionVertex> RenderScreenToMeshRegionVertexArray;
	TArray<FLexUIPostProcessVertex> RenderMeshRegionToScreenVertexArray;

	virtual void SendRegionVertexDataToRenderProxy();
	void SendMaskTextureToRenderProxy();
	void SendRenderTargetToRenderProxy();

	void UpdateRenderTarget();
};
