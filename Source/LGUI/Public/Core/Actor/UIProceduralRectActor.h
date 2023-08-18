// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "UIBaseActor.h"
#include "Core/ActorComponent/UIProceduralRect.h"
#include "UIProceduralRectActor.generated.h"

UCLASS(ClassGroup = LGUI)
class LGUI_API AUIProceduralRectActor : public AUIBaseRenderableActor
{
	GENERATED_BODY()
	
public:	
	AUIProceduralRectActor();

	virtual UUIItem* GetUIItem()const override { return UIProceduralRect; }
	virtual class UUIBaseRenderable* GetUIRenderable()const override { return UIProceduralRect; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		UUIProceduralRect* GetUIProceduralRect()const { return UIProceduralRect; }
private:
	UPROPERTY(Category = "LGUI", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UUIProceduralRect> UIProceduralRect;
	
};
