// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "UIBaseRenderable.h"
#include "Core/HudRender/LGUIHudVertex.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "UIPostProcess.generated.h"

class FUIPostProcessRenderProxy;
struct FLGUIPostProcessVertex;
/** 
 * UI element that can add post processing effect
 * Only valid on ScreenSpaceUI
 */
UCLASS(Abstract, NotBlueprintable)
class LGUI_API UUIPostProcess : public UUIBaseRenderable
{
	GENERATED_BODY()

public:	
	UUIPostProcess(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	virtual void OnUnregister()override;
	virtual void ApplyUIActiveState() override;
	TSharedPtr<UIGeometry> geometry = nullptr;
	virtual void UpdateGeometry(const bool& parentLayoutChanged)override final;

	virtual void OnRenderCanvasChanged(ULGUICanvas* OldCanvas, ULGUICanvas* NewCanvas)override;
	virtual void WidthChanged()override;
	virtual void HeightChanged()override;
	virtual void PivotChanged()override;

	virtual void UpdateBasePrevData()override;
	virtual void UpdateCachedData()override;
	virtual void UpdateCachedDataBeforeGeometry()override;
	virtual void MarkAllDirtyRecursive()override;

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
	TSharedPtr<UIGeometry> GetGeometry() { return geometry; }
public:
	virtual TWeakPtr<FUIPostProcessRenderProxy> GetRenderProxy()PURE_VIRTUAL(UUIPostProcess::GetRenderProxy, return 0;);
	void SetClipType(ELGUICanvasClipType clipType);
	void SetRectClipParameter(const FVector4& OffsetAndSize, const FVector4& Feather);
	void SetTextureClipParameter(UTexture* ClipTex, const FVector4& OffsetAndSize);
private:
	/** local vertex position changed */
	uint8 bLocalVertexPositionChanged : 1;
	/** vertex's uv change */
	uint8 bUVChanged : 1;

	uint8 cacheForThisUpdate_LocalVertexPositionChanged : 1, cacheForThisUpdate_UVChanged : 1;
protected:
	TSharedPtr<FUIPostProcessRenderProxy> RenderProxy = nullptr;
	/** create ui geometry */
	virtual void OnCreateGeometry();
	/** update ui geometry */
	virtual void OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged);
	/** update region vertex data */
	virtual void UpdateRegionVertex();
	TArray<FLGUIPostProcessCopyMeshRegionVertex> renderScreenToMeshRegionVertexArray;
	TArray<FLGUIPostProcessVertex> renderMeshRegionToScreenVertexArray;

	virtual void SendRegionVertexDataToRenderProxy(const FMatrix& InModelViewProjectionMatrix);
private:
	void CreateGeometry();
};
