// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/Texture2D.h"
#include "LGUISpriteInfo.h"
#include "LGUISpriteData_BaseObject.generated.h"

class UUISpriteBase;
class IUISpriteRenderableInterface;

/**
 * Base class for sprite data.
 * A sprite is a small area renderred in a big atlas texture.
 */
UCLASS(Abstract, BlueprintType)
class LGUI_API ULGUISpriteData_BaseObject :public UObject
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		virtual UTexture2D* GetAtlasTexture()PURE_VIRTUAL(ULGUISpriteData_BaseObject::GetAtlasTexture, return nullptr;);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		virtual const FLGUISpriteInfo& GetSpriteInfo()PURE_VIRTUAL(ULGUISpriteData_BaseObject::GetSpriteInfo, static FLGUISpriteInfo ForReturn; return ForReturn;);
	/** This sprite-data is a individal one? Means it will not pack into any atlas texture. */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		virtual bool IsIndividual()const PURE_VIRTUAL(ULGUISpriteData_BaseObject::IsIndividual, return false;);
	/**
	 * Read pixel value from packed atlas texture.
	 * @param InUV uv coordinate in atlas texture.
	 * @param OutPixel result pixel value.
	 * @return true- successfully read pixel, false- not support read pixel.
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		virtual bool ReadPixel(const FVector2D& InUV, FColor& OutPixel)const PURE_VIRTUAL(ULGUISpriteData_BaseObject::ReadPixel, return false;);
	/**
	 * Can we read texture's pixel from this sprite object?
	 */
	virtual bool SupportReadPixel()const PURE_VIRTUAL(ULGUISpriteData_BaseObject::SupportReadPixel, return false;);

	virtual void AddUISprite(TScriptInterface<IUISpriteRenderableInterface> InUISprite) {};
	virtual void RemoveUISprite(TScriptInterface<IUISpriteRenderableInterface> InUISprite) {};

//#if WITH_EDITOR
//	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)override;
//#endif
};
