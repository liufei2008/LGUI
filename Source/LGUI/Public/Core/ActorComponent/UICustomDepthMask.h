// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "UIPostProcess.h"
#include "Core/HudRender/LGUIHudVertex.h"
#include "UICustomDepthMask.generated.h"

UENUM(BlueprintType)
enum class EUICustomDepthMaskSourceType :uint8
{
	/** Use custom depth as mask */
	CustomDepth,
	/** Use custom stencil as mask, only support full-screen mode */
	CustomStencil,
};

/** 
 * Use CustomDepth or CustomDepthStencil as mask, so specified object can render on top of screen-space-UI.
 * Android: CustomDepth mode not supported yet.
 */
UCLASS(ClassGroup = (LGUI), NotBlueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUICustomDepthMask : public UUIPostProcess
{
	GENERATED_BODY()

public:	
	UUICustomDepthMask(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	/** Ignore UI item's size and do full screen effect. If SourceType==CustomStencil then this will always be true. */
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (EditCondition="sourceType==EUICustomDepthMaskSourceType::CustomDepth"))
		bool bFullScreen = true;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (ClampMin = "0.0", ClampMax = 1.0f))
		EUICustomDepthMaskSourceType sourceType = EUICustomDepthMaskSourceType::CustomStencil;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (EditCondition="sourceType==EUICustomDepthMaskSourceType::CustomStencil"))
		int stencilValue = 0;
public:
	virtual TWeakPtr<FUIPostProcessRenderProxy> GetRenderProxy()override;
	virtual void MarkAllDirtyRecursive()override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetFullScreen()const { return bFullScreen; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		EUICustomDepthMaskSourceType GetSourceType()const { return sourceType; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		int GetStencilValue()const { return stencilValue; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetFullScreen(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetSourceType(EUICustomDepthMaskSourceType value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetStencilValue(int value);
protected:
	virtual void SendRegionVertexDataToRenderProxy(const FMatrix& InModelViewProjectionMatrix)override;
	void SendOthersDataToRenderProxy();
};
