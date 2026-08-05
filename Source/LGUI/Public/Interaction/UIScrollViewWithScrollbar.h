// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "UIScrollView.h"
#include "UIScrollViewWithScrollbar.generated.h"

class UUIScrollbar;

UENUM(BlueprintType, Category = LGUI)
enum class ELexUIScrollViewScrollbarVisibility :uint8
{
	//Not control scrollbar's visibility
	None,
	//Always visible.
	Permanent,
	//Auto hide scrollbar when content's size less than viewport's size.
	AutoHide,
};

//ScrollView with scrollbars
UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIScrollViewWithScrollbar : public UUIScrollView
{
	GENERATED_BODY()

public:
	UUIScrollViewWithScrollbar();

#if WITH_EDITOR
	virtual void PreEditChange(FProperty* PropertyAboutToChange) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
protected:
	virtual void OnDimensionsChanged(bool PivotChanged, bool WidthChanged, bool HeightChanged)override;
private:
	friend class FUIScrollViewWithScrollBarCustomization;
	//For scrollbars to expand or shrink viewport
	UPROPERTY(EditAnywhere, Category = "LGUI-ScrollViewWithScrollbar")
		TWeakObjectPtr<ULexWidget> Viewport;
	UPROPERTY(EditAnywhere, Category = "LGUI-ScrollViewWithScrollbar")
		TWeakObjectPtr<UUIScrollbar> HorizontalScrollbar;
	UPROPERTY(EditAnywhere, Category = "LGUI-ScrollViewWithScrollbar")
		ELexUIScrollViewScrollbarVisibility HorizontalScrollbarVisibility = ELexUIScrollViewScrollbarVisibility::AutoHide;
	UPROPERTY(EditAnywhere, Category = "LGUI-ScrollViewWithScrollbar")
		TWeakObjectPtr<UUIScrollbar> VerticalScrollbar;
	UPROPERTY(EditAnywhere, Category = "LGUI-ScrollViewWithScrollbar")
		ELexUIScrollViewScrollbarVisibility VerticalScrollbarVisibility = ELexUIScrollViewScrollbarVisibility::AutoHide;

	virtual void CalculateHorizontalRange()override;
	virtual void CalculateVerticalRange()override;
	virtual bool CheckValidHit(ULexWidget* InHitComp)override;
	virtual void UpdateProgress(bool InFireEvent = true)override;
	virtual bool OnPointerDrag_Implementation(ULexPointerEventData* EventData)override;
	virtual bool OnPointerScroll_Implementation(ULexPointerEventData* EventData)override;
	UPROPERTY(Transient)TWeakObjectPtr<ULexWidget> HorizontalScrollbarWidget;
	UPROPERTY(Transient)TWeakObjectPtr<ULexWidget> VerticalScrollbarWidget;
	bool CheckScrollbarParameter();
	void OnHorizontalScrollbar(float InScrollValue);
	void OnVerticalScrollbar(float InScrollValue);
	FDelegateHandle HorizontalScrollbarDelegateHandle;
	FDelegateHandle VerticalScrollbarDelegateHandle;

public:

	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollViewWithScrollbar")
		ULexWidget* GetViewport()const { return Viewport.Get(); }
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollViewWithScrollbar")
		UUIScrollbar* GetHorizontalScrollbar()const { return HorizontalScrollbar.Get(); }
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollViewWithScrollbar")
		ELexUIScrollViewScrollbarVisibility GetHorizontalScrollbarVisibility()const { return HorizontalScrollbarVisibility; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollViewWithScrollbar")
		UUIScrollbar* GetVerticalScrollbar()const { return VerticalScrollbar.Get(); }
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollViewWithScrollbar")
		ELexUIScrollViewScrollbarVisibility GetVerticalScrollbarVisibility()const { return VerticalScrollbarVisibility; }

	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollViewWithScrollbar")
		void SetHorizontalScrollbarVisibility(ELexUIScrollViewScrollbarVisibility value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollViewWithScrollbar")
		void SetVerticalScrollbarVisibility(ELexUIScrollViewScrollbarVisibility value);
};
