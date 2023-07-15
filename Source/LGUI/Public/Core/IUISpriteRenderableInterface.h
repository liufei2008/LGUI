// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IUISpriteRenderableInterface.generated.h"

class ULGUISpriteData_BaseObject;

UINTERFACE(Blueprintable, MinimalAPI)
class UUISpriteRenderableInterface : public UInterface
{
	GENERATED_BODY()
};
class LGUI_API IUISpriteRenderableInterface
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "LGUI")
		ULGUISpriteData_BaseObject* GetSprite()const;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "LGUI")
		void ApplyAtlasTextureChange();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "LGUI")
		void ApplyAtlasTextureScaleUp();
};
