// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "UIBaseRenderable.h"
#include "Core/LGUIRender/LGUIVertex.h"
#include "Core/LGUISpriteInfo.h"
#include "UIPostProcessRenderable.generated.h"

class FUIPostProcessRenderProxy;
struct FLGUIPostProcessVertex;
enum class ELGUICanvasClipType :uint8;

UENUM(BlueprintType)
enum class EUIPostProcessMaskTextureType :uint8
{
	Simple, 
	Sliced,
};

/** 
 * UI element that can do post processing effect on screen space.
 * Only valid on LGUIRenderer (ScreenSpaceUI or WorldSpace-LGUIRenderer).
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
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif
	virtual void OnUnregister()override;
	TSharedPtr<UIGeometry> geometry_Simple = nullptr;
	TSharedPtr<UIGeometry> geometry_Sliced = nullptr;
	virtual void UpdateGeometry()override final;

	virtual void OnAnchorChange(bool InPivotChange, bool InWidthChange, bool InHeightChange, bool InDiscardCache = true)override;

	virtual void MarkAllDirty()override;

protected:
	friend class FUIPostProcessRenderableCustomization;
	/** Use maskTexture's red channel to mask out effect result. */
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayThumbnail = "false"))
		TObjectPtr<UTexture2D> maskTexture;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		EUIPostProcessMaskTextureType MaskTextureType = EUIPostProcessMaskTextureType::Simple;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FLGUISpriteInfo MaskTextureSpriteInfo;
	/** MaskTexture UV offset and scale info. Only get good result when MaskTextureType is Simple */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FVector4 MaskTextureUVRect = FVector4(0, 0, 1, 1);
	void SendMaskTextureToRenderProxy();
	void CheckSpriteData();
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		UTexture2D* GetMaskTexture()const { return maskTexture; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		EUIPostProcessMaskTextureType GetMaskTextureType()const { return MaskTextureType; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		const FLGUISpriteInfo& GetMaskTextureSpriteInfo()const { return MaskTextureSpriteInfo; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		const FVector4& GetMaskTextureUVRect()const { return MaskTextureUVRect; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetMaskTexture(UTexture2D* newValue);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetMaskTextureType(EUIPostProcessMaskTextureType value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetMaskTextureSpriteInfo(const FLGUISpriteInfo& value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetMaskTextureUVRect(const FVector4& value);
public:
	void MarkVertexPositionDirty();
	void MarkUVDirty();
public:
	virtual TSharedPtr<FUIPostProcessRenderProxy> GetRenderProxy()PURE_VIRTUAL(UUIPostProcessRenderable::GetRenderProxy, return 0;);
	virtual bool IsRenderProxyValid()const;
	virtual void SetClipType(ELGUICanvasClipType clipType);
	virtual void SetRectClipParameter(const FVector4& OffsetAndSize, const FVector4& Feather);
	virtual void SetTextureClipParameter(UTexture* ClipTex, const FVector4& OffsetAndSize);
	virtual bool HaveValidData()const;

	virtual bool LineTraceUI(FHitResult& OutHit, const FVector& Start, const FVector& End)override;
private:
	/** local vertex position changed */
	uint8 bLocalVertexPositionChanged : 1;
	/** vertex's uv change */
	uint8 bUVChanged : 1;
protected:
	TSharedPtr<FUIPostProcessRenderProxy> RenderProxy = nullptr;
	/** update ui geometry */
	virtual void OnUpdateGeometry(bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged);
	/** update region vertex data */
	virtual void UpdateRegionVertex();
	TArray<FLGUIPostProcessCopyMeshRegionVertex> renderScreenToMeshRegionVertexArray;
	TArray<FLGUIPostProcessVertex> renderMeshRegionToScreenVertexArray;

	virtual void SendRegionVertexDataToRenderProxy();
};
