// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "UIBaseActor.generated.h"

class UUIItem;
UCLASS(Abstract, ClassGroup = LGUI)
class LGUI_API AUIBaseActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AUIBaseActor();

#if WITH_EDITOR
	virtual void SetIsTemporarilyHiddenInEditor(bool bIsHidden) override;
#endif
	UFUNCTION(BlueprintNativeEvent, Category = "LGUI")
		UUIItem* GetUIItem_BP()const;
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		virtual UUIItem* GetUIItem()const PURE_VIRTUAL(AUIBaseActor::GetUIItem, return nullptr;);
private:
	UUIItem* GetUIItem_BP_Implementation()const { return GetUIItem(); }
};

class UUIBaseRenderable;
UCLASS(Abstract, ClassGroup = LGUI)
class LGUI_API AUIBaseRenderableActor : public AUIBaseActor
{
	GENERATED_BODY()

public:
	AUIBaseRenderableActor();
	UFUNCTION(BlueprintNativeEvent, Category = "LGUI")
		UUIBaseRenderable* GetUIRenderable_BP()const;
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		virtual UUIBaseRenderable* GetUIRenderable()const PURE_VIRTUAL(AUIBaseRenderableActor::GetUIRenderable, return nullptr;);
	virtual UUIItem* GetUIItem()const override;
private:
	UUIBaseRenderable* GetUIRenderable_BP_Implementation()const { return GetUIRenderable(); }
};

class UUIPostProcessRenderable;
UCLASS(Abstract, ClassGroup = LGUI)
class LGUI_API AUIBasePostProcessActor : public AUIBaseRenderableActor
{
	GENERATED_BODY()

public:
	AUIBasePostProcessActor();
	UFUNCTION(BlueprintNativeEvent, Category = "LGUI")
		UUIPostProcessRenderable* GetUIPostProcessRenderable_BP()const;
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		virtual UUIPostProcessRenderable* GetUIPostProcessRenderable()const PURE_VIRTUAL(AUIBasePostProcessActor::GetUIPostProcessRenderable, return nullptr;);
	virtual UUIBaseRenderable* GetUIRenderable()const override;
private:
	UUIPostProcessRenderable* GetUIPostProcessRenderable_BP_Implementation()const { return GetUIPostProcessRenderable(); }
};
