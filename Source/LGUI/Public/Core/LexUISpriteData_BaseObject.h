// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/Texture2D.h"
#include "LexUISpriteInfo.h"
#include "LexUISpriteData_BaseObject.generated.h"

class ULexSpriteBase;
class ILexUISpriteRenderInterface;

/**
 * Base class for Sprite data.
 * A Sprite is a small area rendered in a big atlas texture.
 */
UCLASS(Abstract, BlueprintType)
class LGUI_API ULexUISpriteData_BaseObject :public UObject
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		virtual UTexture2D* GetAtlasTexture()PURE_VIRTUAL(ULGUISpriteData_BaseObject::GetAtlasTexture, return nullptr;);
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		virtual const FLexUISpriteInfo& GetSpriteInfo()PURE_VIRTUAL(ULGUISpriteData_BaseObject::GetSpriteInfo, static FLexUISpriteInfo ForReturn; return ForReturn;);
	/** This Sprite-data is a individual one? Means it will not pack into any atlas texture. */
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		virtual bool IsIndividual()const PURE_VIRTUAL(ULGUISpriteData_BaseObject::IsIndividual, return false;);
	/**
	 * Read pixel value from packed atlas texture.
	 * @param InUV uv coordinate in atlas texture.
	 * @param OutPixel result pixel value.
	 * @return true- successfully read pixel, false- not support read pixel.
	 */
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		virtual bool ReadPixel(const FVector2D& InUV, FColor& OutPixel)const PURE_VIRTUAL(ULGUISpriteData_BaseObject::ReadPixel, return false;);
	/**
	 * Can we read texture's pixel from this Sprite object?
	 */
	virtual bool SupportReadPixel()const PURE_VIRTUAL(ULGUISpriteData_BaseObject::SupportReadPixel, return false;);

	virtual void AddUISprite(TScriptInterface<ILexUISpriteRenderInterface> InUISprite) {};
	virtual void RemoveUISprite(TScriptInterface<ILexUISpriteRenderInterface> InUISprite) {};

//#if WITH_EDITOR
//	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)override;
//#endif
};
