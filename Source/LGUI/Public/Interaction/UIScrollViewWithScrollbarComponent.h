// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "Core/ActorComponent/UISprite.h"
#include "Event/LGUIDrawableEvent.h"
#include "UIScrollViewComponent.h"
#include "UIScrollViewWithScrollbarComponent.generated.h"

UENUM(BlueprintType)
enum class EScrollViewScrollbarVisibility :uint8
{
	Permanent,
	AutoHide,
	AutoHideAndExpandViewport,
};

//ScrollView with scrollbars
UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIScrollViewWithScrollbarComponent : public UUIScrollViewComponent
{
	GENERATED_BODY()

protected:
	virtual void OnUIDimensionsChanged(bool positionChanged, bool sizeChanged)override;
protected:
	//For scrollbars to expand or shrink viewport
	UPROPERTY(EditAnywhere, Category = "LGUI-ScrollViewWithScrollbar")
		AUIBaseActor* Viewport;
	UPROPERTY(EditAnywhere, Category = "LGUI-ScrollViewWithScrollbar")
		AUIBaseActor* HorizontalScrollbar;
	UPROPERTY(EditAnywhere, Category = "LGUI-ScrollViewWithScrollbar")
		EScrollViewScrollbarVisibility HorizontalScrollbarVisibility = EScrollViewScrollbarVisibility::AutoHideAndExpandViewport;
	UPROPERTY(EditAnywhere, Category = "LGUI-ScrollViewWithScrollbar")
		AUIBaseActor* VerticalScrollbar;
	UPROPERTY(EditAnywhere, Category = "LGUI-ScrollViewWithScrollbar")
		EScrollViewScrollbarVisibility VerticalScrollbarVisibility = EScrollViewScrollbarVisibility::AutoHideAndExpandViewport;

	virtual void CalculateHorizontalRange()override;
	virtual void CalculateVerticalRange()override;
	virtual bool CheckValidHit(USceneComponent* InHitComp)override;
	virtual void UpdateProgress(bool InFireEvent = true)override;
	virtual bool OnPointerDrag_Implementation(ULGUIPointerEventData* eventData)override;
	virtual bool OnPointerScroll_Implementation(ULGUIPointerEventData* eventData)override;
	UPROPERTY(Transient)class UUIScrollbarComponent* HorizontalScrollbarComp = nullptr;
	UPROPERTY(Transient)class UUIScrollbarComponent* VerticalScrollbarComp = nullptr;
	bool CheckScrollbarParameter();
	void OnHorizontalScrollbar(float InScrollValue);
	void OnVerticalScrollbar(float InScrollValue);
	bool ValueIsSetFromHorizontalScrollbar = false;
	bool ValueIsSetFromVerticalScrollbar = false;
public:
};
