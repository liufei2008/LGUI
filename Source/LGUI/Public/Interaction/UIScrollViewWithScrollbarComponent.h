// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "Core/ActorComponent/UISprite.h"
#include "Event/LGUIEventDelegate.h"
#include "UIScrollViewComponent.h"
#include "Layout/ILGUILayoutInterface.h"
#include "UIScrollViewWithScrollbarComponent.generated.h"

UENUM(BlueprintType, Category = LGUI)
enum class EScrollViewScrollbarVisibility :uint8
{
	//Always visible.
	Permanent,
	//Auto hide scrollbar when content's size less than viewport's size.
	AutoHide,
	//Same like AutoHide, but also expand viewport size when hide scrollbar.
	//For this mode, viewport and scrollbar must directly attach to ScrollViewWithScrollBar.
	AutoHideAndExpandViewport,
};

//ScrollView with scrollbars
UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIScrollViewWithScrollbarComponent : public UUIScrollViewComponent, public ILGUILayoutInterface
{
	GENERATED_BODY()

public:
	UUIScrollViewWithScrollbarComponent();
protected:
	virtual void OnUIDimensionsChanged(bool positionChanged, bool sizeChanged)override;
	virtual void OnRegister()override;
	virtual void OnUnregister()override;
protected:
	friend class FUIScrollViewWithScrollBarCustomization;
	//For scrollbars to expand or shrink viewport
	UPROPERTY(EditAnywhere, Category = "LGUI-ScrollViewWithScrollbar")
		TWeakObjectPtr<AUIBaseActor> Viewport;
	UPROPERTY(EditAnywhere, Category = "LGUI-ScrollViewWithScrollbar")
		TWeakObjectPtr<AUIBaseActor> HorizontalScrollbar;
	UPROPERTY(EditAnywhere, Category = "LGUI-ScrollViewWithScrollbar")
		EScrollViewScrollbarVisibility HorizontalScrollbarVisibility = EScrollViewScrollbarVisibility::AutoHideAndExpandViewport;
	UPROPERTY(EditAnywhere, Category = "LGUI-ScrollViewWithScrollbar")
		TWeakObjectPtr<AUIBaseActor> VerticalScrollbar;
	UPROPERTY(EditAnywhere, Category = "LGUI-ScrollViewWithScrollbar")
		EScrollViewScrollbarVisibility VerticalScrollbarVisibility = EScrollViewScrollbarVisibility::AutoHideAndExpandViewport;

	virtual void CalculateHorizontalRange()override;
	virtual void CalculateVerticalRange()override;
	virtual bool CheckValidHit(USceneComponent* InHitComp)override;
	virtual void UpdateProgress(bool InFireEvent = true)override;
	virtual bool OnPointerDrag_Implementation(ULGUIPointerEventData* eventData)override;
	virtual bool OnPointerScroll_Implementation(ULGUIPointerEventData* eventData)override;
	UPROPERTY(Transient)TWeakObjectPtr<class UUIScrollbarComponent> HorizontalScrollbarComp = nullptr;
	UPROPERTY(Transient)TWeakObjectPtr<class UUIScrollbarComponent> VerticalScrollbarComp = nullptr;
	bool CheckScrollbarParameter();
	void OnHorizontalScrollbar(float InScrollValue);
	void OnVerticalScrollbar(float InScrollValue);
	uint8 bLayoutDirty : 1;
	enum class EScrollbarLayoutAction :uint8
	{
		None,
		NeedToShow,
		NeedToHide,
	};
	EScrollbarLayoutAction HorizontalScrollbarLayoutActionType = EScrollbarLayoutAction::None;
	EScrollbarLayoutAction VerticalScrollbarLayoutActionType = EScrollbarLayoutAction::None;

	virtual void OnUIChildHierarchyIndexChanged(UUIItem* child)override;
	virtual void OnUIChildAttachmentChanged(UUIItem* child, bool attachOrDetach)override;
	// Begin LGUILayout interface
	virtual void OnUpdateLayout_Implementation()override;
	virtual bool GetCanLayoutControlAnchor_Implementation(class UUIItem* InUIItem, FLGUICanLayoutControlAnchor& OutResult)const override;
	virtual void MarkRebuildLayout_Implementation()override { MarkLayoutDirty(); }
	// End LGUILayout interface
	void MarkLayoutDirty();
public:

	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollViewWithScrollbar")
		AUIBaseActor* GetViewport()const { return Viewport.Get(); }
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollViewWithScrollbar")
		AUIBaseActor* GetHorizontalScrollbar()const { return HorizontalScrollbar.Get(); }
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollViewWithScrollbar")
		EScrollViewScrollbarVisibility GetHorizontalScrollbarVisibility()const { return HorizontalScrollbarVisibility; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollViewWithScrollbar")
		AUIBaseActor* GetVerticalScrollbar()const { return VerticalScrollbar.Get(); }
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollViewWithScrollbar")
		EScrollViewScrollbarVisibility GetVerticalScrollbarVisibility()const { return VerticalScrollbarVisibility; }

	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollViewWithScrollbar")
		void SetHorizontalScrollbarVisibility(EScrollViewScrollbarVisibility value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollViewWithScrollbar")
		void SetVerticalScrollbarVisibility(EScrollViewScrollbarVisibility value);
};
