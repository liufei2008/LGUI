// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/Texture2D.h"
#include "LGUISpriteInfo.h"
#include "LGUISpriteData_BaseObject.generated.h"

class UUISpriteBase;

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

	virtual void AddUISprite(UUISpriteBase* InUISprite) {};
	virtual void RemoveUISprite(UUISpriteBase* InUISprite) {};

//#if WITH_EDITOR
//	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)override;
//#endif
};