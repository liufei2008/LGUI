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
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "LGUI", meta = (DisplayName="GetUIItem"))
		UUIItem* GetUIItem_BP()const;
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
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "LGUI", meta = (DisplayName = "GetUIRenderable"))
		UUIBaseRenderable* GetUIRenderable_BP()const;
	virtual UUIBaseRenderable* GetUIRenderable()const PURE_VIRTUAL(AUIBaseRenderableActor::GetUIRenderable, return nullptr;);
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
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "LGUI", meta = (DisplayName = "GetUIPostProcessRenderable"))
		UUIPostProcessRenderable* GetUIPostProcessRenderable_BP()const;
	virtual UUIPostProcessRenderable* GetUIPostProcessRenderable()const PURE_VIRTUAL(AUIBasePostProcessActor::GetUIPostProcessRenderable, return nullptr;);
private:
	UUIPostProcessRenderable* GetUIPostProcessRenderable_BP_Implementation()const { return GetUIPostProcessRenderable(); }
};
