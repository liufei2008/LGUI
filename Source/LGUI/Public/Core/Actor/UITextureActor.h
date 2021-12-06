// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "UIBaseActor.h"
#include "Core/ActorComponent/UITexture.h"
#include "UITextureActor.generated.h"

UCLASS()
class LGUI_API AUITextureActor : public AUIBaseRenderableActor
{
	GENERATED_BODY()
	
public:	
	AUITextureActor();

	virtual UUIItem* GetUIItem()const override { return UITexture; }
	virtual class UUIBaseRenderable* GetUIRenderable()const override { return UITexture; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		UUITexture* GetUITexture()const { return UITexture; }
private:
	UPROPERTY(Category = "LGUI", VisibleAnywhere, Transient)
		class UUITexture* UITexture;
	
};
