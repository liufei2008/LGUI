// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "LexVisualBackBufferReader.h"
#include "LexBackBufferCopy.generated.h"

struct FLexBackBufferCopyFilterRenderProxy
{
	virtual~FLexBackBufferCopyFilterRenderProxy(){}
	virtual void DoFilter(FRDGBuilder& GraphBuilder, FGlobalShaderMap* GlobalShaderMap, FTextureRHIRef RenderTargetRHITexture) = 0;
};

UCLASS(BlueprintType, Abstract, DefaultToInstanced, EditInlineNew)
class LGUI_API ULexBackBufferCopyFilter : public UObject
{
	GENERATED_BODY()
public:
	virtual void BeginDestroy()override;
	virtual FLexBackBufferCopyFilterRenderProxy* GetRenderProxy()PURE_VIRTUAL(ULexBackBufferCopyFilter::GetRenderProxy, return 0;);
protected:
	FLexBackBufferCopyFilterRenderProxy* RenderProxy = nullptr;
};

UENUM(BlueprintType)
enum class ELexBackBufferCopyRenderMode : uint8
{
	/** Copy back-buffer to a RenderTarget, then we can use it in our material */
	RenderToTarget,
	/** Render back-buffer back to viewport */
	RenderToViewport,
};

/** 
 * UI element that can copy a back-buffer and do a filter effect, then we can use it in our material or render back to viewport.
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

protected:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	virtual void SendRegionVertexDataToRenderProxy() override;

	/**
	 * Copy screen content to this RenderTarget.
	 * Will create one if not specified.
	 */
	UPROPERTY(EditAnywhere, Category = "LGUI", BlueprintReadWrite, Getter, Setter, meta=(AllowPrivateAccess=true))
	TObjectPtr<UTextureRenderTarget2D> RenderTarget = nullptr;
	FRenderTargetChangedEvent OnRenderTargetChanged;
	//Render filtered BackBuffer image directly to viewport
	UPROPERTY(EditAnywhere, Category = "LGUI", BlueprintReadWrite, Getter, Setter, meta=(AllowPrivateAccess=true))
	ELexBackBufferCopyRenderMode RenderMode = ELexBackBufferCopyRenderMode::RenderToTarget;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LGUI", Instanced, Getter, meta=(AllowPrivateAccess = true))
	TObjectPtr<ULexBackBufferCopyFilter> BackBufferCopyFilter;
public:
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	virtual void PostUpdateDrawCall() override;
	virtual void MarkAllDirty()override;
	virtual FLexVisualBackBufferRenderProxy* GetRenderProxy()override;
	
	void ClearMaterialsUsingThisBackBuffer();
	void RegisterMaterialsUsingThisBackBuffer(UMaterialInstanceDynamic* InMaterialInstanceDynamic);
	
	FRenderTargetChangedEvent& GetRenderTargetChangedEvent(){return OnRenderTargetChanged;}
	UFUNCTION()
	UTextureRenderTarget2D* GetRenderTarget()const { return RenderTarget; }
	UFUNCTION()
	void SetRenderTarget(UTextureRenderTarget2D* InRenderTarget);
	UFUNCTION()
	ELexBackBufferCopyRenderMode GetRenderMode()const { return RenderMode; }
	UFUNCTION()
	void SetRenderMode(ELexBackBufferCopyRenderMode Value);
	UFUNCTION()
	ULexBackBufferCopyFilter* GetBackBufferCopyFilter()const{return BackBufferCopyFilter;}
private:
	void UpdateRenderTarget();
	void SendRenderTargetToRenderProxy();
	void SendFilterToRenderProxy();
	void SendOthersToRenderProxy();
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
	TArray<TWeakObjectPtr<UMaterialInstanceDynamic>> MaterialsUsingThisBackBuffer;
};


/** High quality GaussianBlur with continuously blur strength */
UCLASS(ClassGroup = (LGUI), NotBlueprintable)
class LGUI_API ULexBackBufferCopyFilter_GaussianBlur : public ULexBackBufferCopyFilter
{
	GENERATED_BODY()
private:
	/** Blur effect strength. */
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (ClampMin = 0.0, ClampMax = 1.0f))
	float BlurStrength = 0.1f;
	const int MaxDownSampleLevel = 7;
	void SendDataToRenderProxy();
public:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	virtual FLexBackBufferCopyFilterRenderProxy* GetRenderProxy() override;
};
/** Fast & efficient blur, but not continuously blur strength */
UCLASS(ClassGroup = (LGUI), NotBlueprintable)
class LGUI_API ULexBackBufferCopyFilter_DualKawaseBlur : public ULexBackBufferCopyFilter
{
	GENERATED_BODY()
private:
	/** Blur effect strength. */
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (ClampMin = 0, ClampMax = 10))
	int DownSampleLevel = 2;
	void SendDataToRenderProxy();
public:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	virtual FLexBackBufferCopyFilterRenderProxy* GetRenderProxy() override;
};