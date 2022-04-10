// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "UIPostProcessRenderable.h"
#include "Core/HudRender/LGUIHudVertex.h"
#include "UICustomDepthStencilMask.generated.h"

UENUM(BlueprintType)
enum class EUICustomDepthStencilMaskSourceType :uint8
{
	/** Use custom depth as mask */
	CustomDepth,
	/** Use custom stencil as mask, only support full-screen mode */
	CustomStencil,
};

/** 
 * Use CustomDepth or CustomDepthStencil as mask, so specified object can render on top of screen-space-UI.
 * Use it on ScreenSpace or WorldSpace-LGUIRenderer.
 * If android OpenGL ES3.1, need to enable "ProjectSettings/Platforms/Android/Build/Support Backbuffer Sampling on OpenGL".
 * Android: CustomDepth mode not supported yet.
 */
UCLASS(ClassGroup = (LGUI), NotBlueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUICustomDepthStencilMask : public UUIPostProcessRenderable
{
	GENERATED_BODY()

public:	
	UUICustomDepthStencilMask(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	/** Ignore UI item's size and do full screen effect. If SourceType==CustomStencil then this will always be true. */
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (EditCondition="sourceType==EUICustomDepthStencilMaskSourceType::CustomDepth"))
		bool bFullScreen = true;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (ClampMin = "0.0", ClampMax = 1.0f))
		EUICustomDepthStencilMaskSourceType sourceType = EUICustomDepthStencilMaskSourceType::CustomStencil;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (EditCondition="sourceType==EUICustomDepthStencilMaskSourceType::CustomStencil"))
		int stencilValue = 0;
public:
	virtual TSharedPtr<FUIPostProcessRenderProxy> GetRenderProxy()override;
	virtual void MarkAllDirty()override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetFullScreen()const { return bFullScreen; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		EUICustomDepthStencilMaskSourceType GetSourceType()const { return sourceType; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		int GetStencilValue()const { return stencilValue; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetFullScreen(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetSourceType(EUICustomDepthStencilMaskSourceType value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetStencilValue(int value);
protected:
	virtual void SendRegionVertexDataToRenderProxy()override;
	void SendOthersDataToRenderProxy();
};
