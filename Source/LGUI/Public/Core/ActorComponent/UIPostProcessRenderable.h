// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "UIBaseRenderable.h"
#include "Core/HudRender/LGUIHudVertex.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "UIPostProcessRenderable.generated.h"

class FUIPostProcessRenderProxy;
struct FLGUIPostProcessVertex;
/** 
 * UI element that can do post processing effect on screen space.
 * Only valid on ScreenSpaceUI.
 */
UCLASS(Abstract, NotBlueprintable)
class LGUI_API UUIPostProcessRenderable : public UUIBaseRenderable
{
	GENERATED_BODY()

public:	
	UUIPostProcessRenderable(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	virtual void OnUnregister()override;
	TSharedPtr<UIGeometry> geometry = nullptr;
	virtual void UpdateGeometry()override final;

	virtual void OnAnchorChange(bool InPivotChange, bool InSizeChange, bool InDiscardCache = true)override;

	virtual void MarkAllDirty()override;

protected:
	/** Use maskTexture's red channel to mask out blur result. */
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayThumbnail = "false"))
		UTexture2D* maskTexture;
	void SendMaskTextureToRenderProxy();
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		UTexture2D* GetMaskTexture()const { return maskTexture; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetMaskTexture(UTexture2D* newValue);
public:
	void MarkVertexPositionDirty();
	void MarkUVDirty();
	UIGeometry* GetGeometry() { return geometry.Get(); }
public:
	virtual TSharedPtr<FUIPostProcessRenderProxy> GetRenderProxy()PURE_VIRTUAL(UUIPostProcessRenderable::GetRenderProxy, return 0;);
	virtual bool IsRenderProxyValid()const;
	virtual void SetClipType(ELGUICanvasClipType clipType);
	virtual void SetRectClipParameter(const FVector4& OffsetAndSize, const FVector4& Feather);
	virtual void SetTextureClipParameter(UTexture* ClipTex, const FVector4& OffsetAndSize);

	virtual bool LineTraceUI(FHitResult& OutHit, const FVector& Start, const FVector& End)override;
private:
	/** local vertex position changed */
	uint8 bLocalVertexPositionChanged : 1;
	/** vertex's uv change */
	uint8 bUVChanged : 1;
protected:
	TSharedPtr<FUIPostProcessRenderProxy> RenderProxy = nullptr;
	/** update ui geometry */
	virtual void OnUpdateGeometry(UIGeometry* InGeo, bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged);
	/** update region vertex data */
	virtual void UpdateRegionVertex();
	TArray<FLGUIPostProcessCopyMeshRegionVertex> renderScreenToMeshRegionVertexArray;
	TArray<FLGUIPostProcessVertex> renderMeshRegionToScreenVertexArray;

	virtual void SendRegionVertexDataToRenderProxy();
};
