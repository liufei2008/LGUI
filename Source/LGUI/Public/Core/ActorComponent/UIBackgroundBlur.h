// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "UIPostProcessRenderable.h"
#include "UIBackgroundBlur.generated.h"

/** 
 * UI element that can add blur effect on background renderred image, just like UMG's BackgroundBlur.
 * Use it on ScreenSpace or WorldSpace-LGUIRenderer.
 * If android OpenGL ES3.1, need to enable "ProjectSettings/Platforms/Android/Build/Support Backbuffer Sampling on OpenGL".
 */
UCLASS(ClassGroup = (LGUI), NotBlueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIBackgroundBlur : public UUIPostProcessRenderable
{
	GENERATED_BODY()

public:	
	UUIBackgroundBlur(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	/** Blur effect strength. */
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (ClampMin = "0.0", ClampMax = 100.0f))
		float blurStrength = 10.0f;
	/** Will alpha affect blur strength? If true, then 0 alpha means 0 blur strength, and 1 alpha means full blur strength. */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool applyAlphaToBlur = true;
	/** No need to change this because default value can give you good result. */
	UPROPERTY(EditAnywhere, Category = "LGUI", AdvancedDisplay) 
		int maxDownSampleLevel = 6;
	/** Use strengthTexture's red channel to control blur strength, 0-no blur, 1-full blur. */
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayThumbnail = "false"))
		UTexture2D* strengthTexture;
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetBlurStrength() const { return blurStrength; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		int GetMaxDownSampleLevel() const { return maxDownSampleLevel; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetApplyAlphaToBlur()const { return applyAlphaToBlur; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		UTexture2D* GetStrengthTexture()const { return strengthTexture; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetBlurStrength(float newValue);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetMaxDownSampleLevel(int newValue);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetApplyAlphaToBlur(bool newValue);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetStrengthTexture(UTexture2D* newValue);

	virtual TSharedPtr<FUIPostProcessRenderProxy> GetRenderProxy()override;
	virtual void MarkAllDirty()override;
protected:
	float inv_SampleLevelInterval = 1.0f;
	FORCEINLINE float GetBlurStrengthInternal();
protected:
	virtual void SendRegionVertexDataToRenderProxy()override;
	void SendOthersDataToRenderProxy();
	void SendStrengthTextureToRenderProxy();
};
