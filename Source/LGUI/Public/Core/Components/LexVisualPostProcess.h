// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "LexVisual.h"
#include "LexVisualBackBufferReader.h"
#include "Core/LexUIRender/LexUIPostProcessVertex.h"
#include "LexVisualPostProcess.generated.h"

class FLexVisualBackBufferRenderProxy;
struct FLexUIPostProcessVertex;

/** 
 * UI element that can do post-processing effect on screen space.
 * Only valid on LexUIRenderer (ScreenSpaceUI or WorldSpace-LexUIRenderer).
 */
UCLASS(Abstract, NotBlueprintable)
class LGUI_API ULexVisualPostProcess : public ULexVisualBackBufferReader
{
	GENERATED_BODY()

public:
	ULexVisualPostProcess(const FObjectInitializer& ObjectInitializer);

protected:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif

protected:
	friend class FLexVisualPostProcessCustomization;
	/** Use maskTexture's red channel to mask out effect result. */
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayThumbnail = "false"))
	TObjectPtr<UTexture2D> MaskTexture;
	/** MaskTexture UV offset and scale info. Only get good result when MaskTextureType is Simple */
	UPROPERTY(EditAnywhere, Category = "LGUI")
	FVector4 MaskTextureUVRect = FVector4(0, 0, 1, 1);
	
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	UTexture2D* GetMaskTexture()const { return MaskTexture; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	const FVector4& GetMaskTextureUVRect()const { return MaskTextureUVRect; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
	void SetMaskTexture(UTexture2D* Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	void SetMaskTextureUVRect(const FVector4& Value);
protected:
	void SendMaskTextureToRenderProxy();
	virtual void SendRegionVertexDataToRenderProxy() override;
};
