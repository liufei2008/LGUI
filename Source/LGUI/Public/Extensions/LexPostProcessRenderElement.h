// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/Components/LexVisualBatchMesh.h"
#include "LexPostProcessRenderElement.generated.h"

class ULexVisualPostProcess;
/**
 * This component will grab post-process result image and display here.
 * NOTE!!! This only valid when target PostProcess RenderType is set to RenderTarget.
 * UV channel:
 *		UV0: Rect UV from (0,0) to (1,1)
 *		UV1: Check LexCanvas
 */
UCLASS(ClassGroup = (LGUI), Blueprintable)
class LGUI_API ULexPostProcessRenderElement : public ULexVisualBatchMesh
{
	GENERATED_BODY()
protected:
	virtual void BeginPlay()override;
	virtual void EndPlay() override;
#if WITH_EDITOR
	virtual void PreEditChange(FProperty* PropertyAboutToChange) override;
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	virtual void OnRegister() override;

	UPROPERTY(EditAnywhere, Category = "LGUI")
	TWeakObjectPtr<ULexVisualPostProcess> PostProcess;
	UPROPERTY(EditAnywhere, Category = "LGUI")
	TObjectPtr<UMaterialInterface> Material;
	UPROPERTY(VisibleAnywhere, Category = "LGUI", Transient)
	TObjectPtr<UMaterialInstanceDynamic> MaterialInstanceDynamic;

	bool bHasRegisterPostProcessChangedEvent = false;
	void RegisterPostProcessChangedEvent();
	void UnregisterPostProcessChangedEvent();
	void SetMaterialParameter();
	void CheckMaterialInstanceDynamic();

	virtual void OnDimensionChanged(bool InPivotChange, bool InWidthChange, bool InHeightChange) override;
	virtual void OnTransformChanged(bool InPositionChanged, bool InScaleChanged) override;
	
	virtual UTexture* GetTextureToCreateGeometry()override;
	virtual UMaterialInterface* GetMaterialToCreateGeometry() override;
	virtual void OnBeforeCreateOrUpdateGeometry() override;
	virtual void OnUpdateGeometry(FLexUIGeometry& InGeo, bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged) override;
public:
	static FName LexUI_World2PostProcess_Row1;
	static FName LexUI_World2PostProcess_Row2;
	static FName LexUI_World2PostProcess_Row3;
	static FName LexUI_World2PostProcess_Row4;
	static void SetMaterialMatrixProperty(ULexVisualPostProcess* PostProcess, UMaterialInstanceDynamic* MID);
};
