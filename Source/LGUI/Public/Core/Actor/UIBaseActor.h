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
	static AActor* FirstTemporarilyHiddenActor;
	virtual void SetIsTemporarilyHiddenInEditor(bool bIsHidden) override;
#endif
	/** This is for blueprint actor which inherit UIBaseActor, Implement it and return UIItem. */
	UFUNCTION(BlueprintNativeEvent, Category = "LGUI")
		UUIItem* GetUIItem_Impl()const;
	UE_DEPRECATED(4.26, "Use GetUIItem instead.")
	UFUNCTION(BlueprintCallable, Category = "LGUI", meta = (DeprecatedFunction, DeprecationMessage = "Use GetUIItem instead."))
		UUIItem* GetUIItem_BP()const { return GetUIItem(); }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		virtual UUIItem* GetUIItem()const PURE_VIRTUAL(AUIBaseActor::GetUIItem, return nullptr;);
private:
	UUIItem* GetUIItem_Impl_Implementation()const { return GetUIItem(); }
};

class UUIBaseRenderable;
UCLASS(Abstract, ClassGroup = LGUI)
class LGUI_API AUIBaseRenderableActor : public AUIBaseActor
{
	GENERATED_BODY()

public:
	AUIBaseRenderableActor();
	/** This is for blueprint actor which inherit UIBaseRenderableActor, Implement it and return UIBaseRenderable. */
	UFUNCTION(BlueprintNativeEvent, Category = "LGUI")
		UUIBaseRenderable* GetUIRenderable_Impl()const;
	UE_DEPRECATED(4.26, "Use GetUIRenderable instead.")
	UFUNCTION(BlueprintCallable, Category = "LGUI", meta = (DeprecatedFunction, DeprecationMessage = "Use GetUIRenderable instead."))
		UUIBaseRenderable* GetUIRenderable_BP()const { return GetUIRenderable(); }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		virtual UUIBaseRenderable* GetUIRenderable()const PURE_VIRTUAL(AUIBaseRenderableActor::GetUIRenderable, return nullptr;);
	virtual UUIItem* GetUIItem()const override;
private:
	UUIBaseRenderable* GetUIRenderable_Impl_Implementation()const { return GetUIRenderable(); }
};

class UUIPostProcessRenderable;
UCLASS(Abstract, ClassGroup = LGUI)
class LGUI_API AUIBasePostProcessActor : public AUIBaseRenderableActor
{
	GENERATED_BODY()

public:
	AUIBasePostProcessActor();
	/** This is for blueprint actor which inherit UIBasePostProcessActor, Implement it and return UIPostProcessRenderable. */
	UFUNCTION(BlueprintNativeEvent, Category = "LGUI")
		UUIPostProcessRenderable* GetUIPostProcessRenderable_Impl()const;
	UE_DEPRECATED(4.26, "Use GetUIPostProcessRenderable instead.")
	UFUNCTION(BlueprintCallable, Category = "LGUI", meta = (DeprecatedFunction, DeprecationMessage = "Use GetUIPostProcessRenderable instead."))
		UUIPostProcessRenderable* GetUIPostProcessRenderable_BP()const { return GetUIPostProcessRenderable(); }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		virtual UUIPostProcessRenderable* GetUIPostProcessRenderable()const PURE_VIRTUAL(AUIBasePostProcessActor::GetUIPostProcessRenderable, return nullptr;);
	virtual UUIBaseRenderable* GetUIRenderable()const override;
private:
	UUIPostProcessRenderable* GetUIPostProcessRenderable_Impl_Implementation()const { return GetUIPostProcessRenderable(); }
};
