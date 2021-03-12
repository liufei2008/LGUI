// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "UIPostProcess.h"
#include "Core/HudRender/LGUIHudVertex.h"
#include "UIBackgroundPixelate.generated.h"

/** 
 * UI element that can make the background look pixelated
 * Use it on ScreenSpaceUI.
 * May have issue when MSAA is on.
 */
UCLASS(ClassGroup = (LGUI), NotBlueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIBackgroundPixelate : public UUIPostProcess
{
	GENERATED_BODY()

public:	
	UUIBackgroundPixelate(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	/** Pixelate effect strength. */
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (ClampMin = "0.0", ClampMax = 100.0f))
		float pixelateStrength = 10.0f;
	/** Will alpha affect pixelate strength? If true, then 0 alpha means 0 pixelate strength, and 1 alpha means full pixelate strength. */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool applyAlphaToStrength = true;
public:
	UFUNCTION(BlueprintCallable, Category="LGUI")
		float GetPixelateStrength() const { return pixelateStrength; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetApplyAlphaToStrength()const { return applyAlphaToStrength; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetPixelateStrength(float newValue);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetApplyAlphaToStrength(bool newValue);

	virtual TWeakPtr<FUIPostProcessRenderProxy> GetRenderProxy()override;
protected:
	FORCEINLINE float GetStrengthInternal();
protected:
	virtual void SendRegionVertexDataToRenderProxy()override;
	void SendOthersDataToRenderProxy();
};
