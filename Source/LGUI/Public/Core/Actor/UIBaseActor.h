// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "UIBaseActor.generated.h"

UCLASS(Abstract, HideCategories = (Actor, Rendering))
class LGUI_API AUIBaseActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AUIBaseActor();

	UFUNCTION(BlueprintCallable, Category = "LGUI")
	virtual class UUIItem* GetUIItem()const;
};
