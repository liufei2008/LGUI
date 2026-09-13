// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/Components/LexVisualBatchMesh.h"
#include "Core/Components/LexText.h"
#include "LexPostProcessRenderElement_Text.generated.h"

class ULexVisualPostProcess;
/**
 * This component will grab post-process result image and display here.
 * NOTE!!! This only valid when target PostProcess RenderType is set to RenderTarget.
 * UV channel:
 *		UV0 ~ UV2: Check LexText
 *		UV3: TextureCoordinate for sampling PostProcess RenderTarget
 */
UCLASS(ClassGroup = (LGUI), Blueprintable)
class LGUI_API ULexPostProcessRenderElement_Text : public ULexText
{
	GENERATED_BODY()
protected:
	virtual void BeginPlay()override;
	virtual void EndPlay() override;
#if WITH_EDITOR
	virtual void PreEditChange(FProperty* PropertyAboutToChange) override;
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	UPROPERTY(EditAnywhere, Category = "LGUI")
	TWeakObjectPtr<ULexVisualPostProcess> PostProcess;
	UPROPERTY(VisibleAnywhere, Category = "LGUI", Transient)
	TObjectPtr<UMaterialInstanceDynamic> MaterialInstanceDynamic;

	bool bHasRegisterPostProcessChangedEvent = false;
	void RegisterPostProcessChangedEvent();
	void UnregisterPostProcessChangedEvent();
	void SetMaterialParameter();
	void CheckMaterialInstanceDynamic();

	static FName LexUI_PostProcessTexture;

	virtual void OnDimensionChanged(bool InPivotChange, bool InWidthChange, bool InHeightChange) override;
	virtual void OnTransformChanged(bool InPositionChanged, bool InScaleChanged) override;
	
	virtual UTexture* GetTextureToCreateGeometry()override;
	virtual UMaterialInterface* GetMaterialToCreateGeometry() override;
	virtual void OnBeforeCreateOrUpdateGeometry() override;
	virtual void OnUpdateGeometry(FLexUIGeometry& InGeo, bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged) override;
};
