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
	UFUNCTION(BlueprintNativeEvent, Category = "LGUI")
		ULGUISpriteData_BaseObject* SpriteRenderableGetSprite()const;
	UFUNCTION(BlueprintNativeEvent, Category = "LGUI")
		void ApplyAtlasTextureChange();
	UFUNCTION(BlueprintNativeEvent, Category = "LGUI")
		void ApplyAtlasTextureScaleUp();
};
