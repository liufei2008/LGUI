// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "LexVisualBatchMesh.h"
#include "Engine/Texture.h"
#include "LexTextureBase.generated.h"

/** 
 * This is base class for create custom mesh based on UITexture. Just override OnCreateGeometry() and OnUpdateGeometry(...) to create or update your own geometry
 */
UCLASS(ClassGroup = (LGUI), Abstract, Blueprintable)
class LGUI_API ULexTextureBase : public ULexVisualBatchMesh
{
	GENERATED_BODY()

public:	
	ULexTextureBase(const FObjectInitializer& ObjectInitializer);

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	void CheckTexture();
#endif
protected:
	virtual void BeginPlay()override;
	friend class FLexTextureBaseCustomization;
	UPROPERTY(EditAnywhere, Category = "LexUI", meta = (DisplayThumbnail = "false"))
	TObjectPtr<UTexture> Texture = nullptr;
	/** Use a custom material to render this texture */
	UPROPERTY(EditAnywhere, Category = "LexUI")
	UMaterialInterface* OverrideMaterial = nullptr;

	virtual UTexture* GetTextureToCreateGeometry()override;
	virtual UMaterialInterface* GetMaterialToCreateGeometry() override;

	virtual bool ReadPixelFromMainTexture(const FVector2D& InUV, FColor& OutPixel)const override;
public:
	UFUNCTION(BlueprintCallable, Category = "LexUI") UTexture* GetTexture()const { return Texture; }
	UFUNCTION(BlueprintCallable, Category = "LexUI") UMaterialInterface* GetOverrideMaterial()const{return OverrideMaterial;}

	UFUNCTION(BlueprintCallable, Category = "LexUI")
		virtual void SetTexture(UTexture* Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		void SetSizeFromTexture();
	UFUNCTION(BlueprintCallable, Category = "LexUI")
	void SetOverrideMaterial(UMaterialInterface* Value);
};
