// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "UIBaseActor.h"
#include "Core/ActorComponent/UIText.h"
#include "UITextActor.generated.h"

UCLASS(ClassGroup = LGUI)
class LGUI_API AUITextActor : public AUIBaseRenderableActor
{
	GENERATED_BODY()
	
public:	
	AUITextActor();

	virtual UUIItem* GetUIItem()const override { return UIText; }
	virtual class UUIBaseRenderable* GetUIRenderable()const override { return UIText; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		UUIText* GetUIText()const { return UIText; }
private:
	UPROPERTY(Category = "LGUI", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UUIText* UIText;
};
