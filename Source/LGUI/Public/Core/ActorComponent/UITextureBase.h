﻿// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "UIBatchGeometryRenderable.h"
#include "Engine/Texture.h"
#include "UITextureBase.generated.h"

/** 
 * This is base class for create custom mesh based on UITexture. Just override OnCreateGeometry() and OnUpdateGeometry(...) to create or update your own geometry
 */
UCLASS(ClassGroup = (LGUI), Abstract, Blueprintable)
class LGUI_API UUITextureBase : public UUIBatchGeometryRenderable
{
	GENERATED_BODY()

public:	
	UUITextureBase(const FObjectInitializer& ObjectInitializer);

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	void CheckTexture();
#endif
	static UTexture* GetDefaultWhiteTexture();
protected:
	virtual void BeginPlay()override;
	friend class FUITextureBaseCustomization;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayThumbnail = "false"))
		UTexture* texture = nullptr;

	virtual void OnBeforeCreateOrUpdateGeometry()override {}
	virtual UTexture* GetTextureToCreateGeometry()override;
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI") UTexture* GetTexture()const { return texture; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		virtual void SetTexture(UTexture* newTexture);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetSizeFromTexture();
};
