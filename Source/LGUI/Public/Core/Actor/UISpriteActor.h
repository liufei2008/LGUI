// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "UIBaseActor.h"
#include "Core/ActorComponent/UISprite.h"
#include "UISpriteActor.generated.h"

UCLASS()
class LGUI_API AUISpriteActor : public AUIBaseRenderableActor
{
	GENERATED_BODY()
	
public:	
	AUISpriteActor();

	virtual UUIItem* GetUIItem()const override { return UISprite; }
	virtual class UUIBaseRenderable* GetUIRenderable()const override { return UISprite; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		UUISprite* GetUISprite()const { return UISprite; }
private:
	UPROPERTY(Category = "LGUI", VisibleAnywhere, Transient)
		class UUISprite* UISprite;
	
};
