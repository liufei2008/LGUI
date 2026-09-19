// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "LexVisualPostProcess.h"
#include "LexBackgroundBlur.generated.h"

/** 
 * UI element that can add blur effect on background image, just like UMG's BackgroundBlur.
 * Use it in ScreenSpace or WorldSpace-LexUIRenderer.
 * If android OpenGL ES3.1, need to enable "ProjectSettings/Platforms/Android/Build/Support Backbuffer Sampling on OpenGL".
 */
UCLASS(ClassGroup = (LGUI), NotBlueprintable)
class LGUI_API ULexBackgroundBlur : public ULexVisualPostProcess
{
	GENERATED_BODY()

public:	
	ULexBackgroundBlur(const FObjectInitializer& ObjectInitializer);

private:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	/** Blur effect strength. */
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (ClampMin = 0.0, ClampMax = 1.0f))
		float BlurStrength = 0.1f;
	/** Will alpha affect blur strength? If true, then 0 alpha means 0 blur strength, and 1 alpha means full blur strength. */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool ApplyAlphaToBlur = true;
	
	UPROPERTY(EditAnywhere, Category = "LGUI", AdvancedDisplay)
		int MaxDownSampleLevel = 7;
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	float GetBlurStrength() const { return BlurStrength; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	int GetMaxDownSampleLevel() const { return MaxDownSampleLevel; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	bool GetApplyAlphaToBlur()const { return ApplyAlphaToBlur; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	void SetBlurStrength(float Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	void SetMaxDownSampleLevel(int Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	void SetApplyAlphaToBlur(bool Value);

	virtual FLexVisualBackBufferRenderProxy* GetRenderProxy()override;
	virtual void MarkAllDirty()override;
private:
	FORCEINLINE float GetBlurStrengthInternal();
	virtual void SendRegionVertexDataToRenderProxy()override;
	void SendOthersDataToRenderProxy();
};
