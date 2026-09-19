// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "LexVisualPostProcess.h"
#include "LexBackgroundPixelate.generated.h"

/** 
 * UI element that can make the background look pixelated
 * Use it in ScreenSpace or WorldSpace-LexUIRenderer.
 * If android OpenGL ES3.1, need to enable "ProjectSettings/Platforms/Android/Build/Support Backbuffer Sampling on OpenGL".
 */
UCLASS(ClassGroup = (LGUI), NotBlueprintable)
class LGUI_API ULexBackgroundPixelate : public ULexVisualPostProcess
{
	GENERATED_BODY()

public:	
	ULexBackgroundPixelate(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	/** Pixelate effect strength. */
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (ClampMin = "0.0", ClampMax = 100.0f))
		float PixelateStrength = 10.0f;
	/** Will alpha affect pixelate strength? If true, then 0 alpha means 0 pixelate strength, and 1 alpha means full pixelate strength. */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool ApplyAlphaToStrength = true;
public:
	UFUNCTION(BlueprintCallable, Category="LGUI")
		float GetPixelateStrength() const { return PixelateStrength; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetApplyAlphaToStrength()const { return ApplyAlphaToStrength; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetPixelateStrength(float Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetApplyAlphaToStrength(bool Value);

	virtual FLexVisualBackBufferRenderProxy* GetRenderProxy()override;
	virtual void MarkAllDirty()override;
protected:
	FORCEINLINE float GetStrengthInternal();
protected:
	virtual void SendRegionVertexDataToRenderProxy()override;
	void SendOthersDataToRenderProxy();
};
