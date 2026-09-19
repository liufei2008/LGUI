// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "LexVisualPostProcess.h"
#include "LexBackBufferCopy.generated.h"

/** 
 * UI element that can copy a back-buffer to a texture, so we can use it in our material.
 * Use it in ScreenSpace or WorldSpace-LexUIRenderer.
 * If android OpenGL ES3.1, need to enable "ProjectSettings/Platforms/Android/Build/Support Backbuffer Sampling on OpenGL".
 */
UCLASS(ClassGroup = (LGUI), NotBlueprintable)
class LGUI_API ULexBackBufferCopy : public ULexVisualBackBufferReader
{
	GENERATED_BODY()

public:	
	DECLARE_EVENT_OneParam(ULexBackBufferCopy, FRenderTargetChangedEvent, UTextureRenderTarget2D*);
	ULexBackBufferCopy(const FObjectInitializer& ObjectInitializer);

private:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	/**
	 * Blur result will output to this RenderTarget.
	 * Will create one if not specified.
	 */
	UPROPERTY(EditAnywhere, Category = "LGUI")
	TObjectPtr<UTextureRenderTarget2D> OutputRenderTarget = nullptr;
	FRenderTargetChangedEvent OnRenderTargetChanged;
protected:
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	virtual void PostUpdateDrawCall() override;
	virtual void MarkAllDirty()override;
public:
	virtual FLexVisualBackBufferRenderProxy* GetRenderProxy()override;
	void ClearMaterialsUsingThisBackBuffer();
	void RegisterMaterialsUsingThisBackBuffer(UMaterialInstanceDynamic* InMaterialInstanceDynamic);
	
	FRenderTargetChangedEvent& GetRenderTargetChangedEvent(){return OnRenderTargetChanged;}
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	UTextureRenderTarget2D* GetOutputRenderTarget()const { return OutputRenderTarget; }
private:
	void UpdateRenderTarget();
	void SendRenderTargetToRenderProxy();
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
	TArray<TWeakObjectPtr<UMaterialInstanceDynamic>> MaterialsUsingThisBackBuffer;
};
