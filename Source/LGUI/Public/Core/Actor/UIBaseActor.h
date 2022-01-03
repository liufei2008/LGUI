// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "UIBaseActor.generated.h"

UCLASS(Abstract)
class LGUI_API AUIBaseActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AUIBaseActor();

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		virtual class UUIItem* GetUIItem()const PURE_VIRTUAL(AUIBaseActor::GetUIItem, return nullptr;);
};

UCLASS(Abstract)
class LGUI_API AUIBaseRenderableActor : public AUIBaseActor
{
	GENERATED_BODY()

public:
	AUIBaseRenderableActor();
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		virtual class UUIBaseRenderable* GetUIRenderable()const PURE_VIRTUAL(AUIBaseRenderableActor::GetUIRenderable, return nullptr;);
};

UCLASS(Abstract)
class LGUI_API AUIBasePostProcessActor : public AUIBaseRenderableActor
{
	GENERATED_BODY()

public:
	AUIBasePostProcessActor();
};
