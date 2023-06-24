// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "UIBaseActor.h"
#include "Core/ActorComponent/UIBackgroundPixelate.h"
#include "UIBackgroundPixelateActor.generated.h"

UCLASS(ClassGroup = LGUI)
class LGUI_API AUIBackgroundPixelateActor : public AUIBasePostProcessActor
{
	GENERATED_BODY()
	
public:	
	AUIBackgroundPixelateActor();

	virtual UUIItem* GetUIItem()const override { return UIBackgroundPixelate; }
	virtual class UUIPostProcessRenderable* GetUIPostProcessRenderable()const override { return UIBackgroundPixelate; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		UUIBackgroundPixelate* GetUIBackgroundPixelate()const { return UIBackgroundPixelate; }
private:
	UPROPERTY(Category = "LGUI", VisibleAnywhere, BlueprintReadOnly, Transient, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<class UUIBackgroundPixelate> UIBackgroundPixelate;
	
};
