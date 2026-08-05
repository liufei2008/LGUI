// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "Event/LexDelegateDeclaration.h"
#include "Event/LexUIEventDelegate.h"
#include "Event/Interface/LexPointerDragInterface.h"
#include "Event/Interface/LexPointerScrollInterface.h"
#include "Core/LexUIBehaviour.h"
#include "UIScrollView.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUIScrollViewValueChangedEvent, FVector2D, InVector2);

UCLASS(ClassGroup=(LGUI), Transient)
class LGUI_API UUIScrollViewHelper :public ULexUIBehaviour
{
	GENERATED_BODY()
	UUIScrollViewHelper();
private:
	virtual void Awake()override;
	virtual void OnDimensionsChanged(bool PivotChanged, bool WidthChanged, bool HeightChanged)override;
	virtual void OnChildDimensionsChanged(ULexWidget* Child, bool PivotChanged, bool WidthChanged, bool HeightChanged)override;
	friend class UUIScrollView;
	UPROPERTY(Transient)
		TWeakObjectPtr<class UUIScrollView> TargetComp;
};
//ScrollView
UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIScrollView : public ULexUIBehaviour, public ILexPointerDragInterface, public ILexPointerScrollInterface
{
	GENERATED_BODY()
	
protected:
	virtual void Awake() override;
	virtual void Tick(float DeltaTime) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	virtual void OnEnable() override;
	virtual void OnTransformChanged() override;
	virtual void OnDimensionsChanged(bool PivotChanged, bool WidthChanged, bool HeightChanged)override;
	virtual void RecalculateRange();
protected:
	friend class UUIScrollViewHelper;
	/** Content can move inside it's parent area. */ 
	UPROPERTY(EditAnywhere, Category = "LGUI-ScrollView")
		TWeakObjectPtr<ULexWidget> Content;
	UPROPERTY(EditAnywhere, Category = "LGUI-ScrollView")
		bool Horizontal = true;
	UPROPERTY(EditAnywhere, Category = "LGUI-ScrollView")
		bool Vertical = true;
	/** If allow Horizontal and Vertical both, then only allow one direction drag at the same time. */ 
	UPROPERTY(EditAnywhere, Category = "LGUI-ScrollView")
		bool OnlyOneDirection = true;
	/** Sensitivity when use mouse scroll wheel input */ 
	UPROPERTY(EditAnywhere, Category = "LGUI-ScrollView")
		float ScrollSensitivity = 1.0f;
	/** When Content size is smaller than Content's parent size, can we still scroll it? */
	UPROPERTY(EditAnywhere, Category = "LGUI-ScrollView")
		bool CanScrollInSmallSize = true;
	/** When Content size is smaller than Content's parent size, flip content's scroll direction and position. */
	UPROPERTY(EditAnywhere, Category = "LGUI-ScrollView")
		bool FlipDirectionInSmallSize = false;
	/** Determines how quickly the contents stop moving. A value of 0 means the movement will never slow down, larger value will stop the movement faster. */
	UPROPERTY(EditAnywhere, Category = "LGUI-ScrollView", meta = (ClampMin = "0.0"))
		float DecelerateRate = 0.135f;
	/** Limit Content inside Viewport's rect area, if out-of-range then move it back. */
	UPROPERTY(EditAnywhere, Category = "LGUI-ScrollView", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		bool RestrictRectArea = true;
	/** Decrease movement value when drag content out of range. A value of 0 means not allowed out of range. A value of 1 means no damp effect. */
	UPROPERTY(EditAnywhere, Category = "LGUI-ScrollView", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float OutOfRangeDamper = 0.5f;

	/** inherited events of this component can bubble up? */
	UPROPERTY(EditAnywhere, Category = "LGUI-ScrollView")
		bool AllowEventBubbleUp = false;

	/**
	 * Keep progress value when content position and size change.
	 * true- keep progress value and change content's position and size to fit progress.
	 * false- change progress value to fit content's position and size.
	 */
	UPROPERTY(EditAnywhere, Category = "LGUI-ScrollView")
		bool KeepProgress = false;
	//progress, 0--1, x for horizontal, y for vertical
	UPROPERTY(EditAnywhere, Category = "LGUI-ScrollView", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition="KeepProgress"))
		FVector2D Progress = FVector2D(0, 0);

	uint8 bAllowHorizontalScroll: 1, bAllowVerticalScroll: 1;
	uint8 bCanUpdateAfterDrag: 1;
	uint8 bRangeCalculated: 1;

	virtual void CalculateHorizontalRange();
	virtual void CalculateVerticalRange();
	bool CheckParameters();
	virtual bool CheckValidHit(ULexWidget* InHitComp);
	UPROPERTY(Transient)TWeakObjectPtr<ULexWidget> ContentParent = nullptr;//Content's parent
	virtual void UpdateProgress(bool InFireEvent = true);
	FVector2D Velocity = FVector2D(0, 0);//drag speed
	FVector2D HorizontalRange;//horizontal scroll range, x--min, y--max
	FVector2D VerticalRange;//vertical scroll range, x--min, y--max
	FVector PrevPointerPosition;//prev frame pointer hit position in world

	void UpdateAfterDrag(float deltaTime);
	virtual void ApplyContentPositionWithProgress();

	FLexUIMulticastDelegateVector2 OnValueChangedCPP;
	UPROPERTY(BlueprintAssignable, Category = "LGUI-ScrollView")
	FUIScrollViewValueChangedEvent OnValueChanged;
	UPROPERTY(EditAnywhere, Category = "LGUI-ScrollView", DisplayName="OnValueChangedED")
	FLexUIEventDelegate OnValueChangedED = FLexUIEventDelegate(ELexUIEventDelegateParameterType::Vector2);
public:
	FLexUIMulticastDelegateVector2& GetOnValueChangedEvent(){return OnValueChangedCPP;}
	
	//scroll range change(eg content or content's parent size change), use this to recalculate range
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollView")
		void RectRangeChanged();
	
	virtual bool OnPointerBeginDrag_Implementation(ULexPointerEventData* EventData)override;
	virtual bool OnPointerDrag_Implementation(ULexPointerEventData* EventData)override;
	virtual bool OnPointerEndDrag_Implementation(ULexPointerEventData* EventData)override;

	virtual bool OnPointerScroll_Implementation(ULexPointerEventData* EventData)override;

	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollView")
		ULexWidget* GetContent()const { return Content.Get(); }
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollView")
		bool GetHorizontal()const { return Horizontal; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollView")
		bool GetVertical()const { return Vertical; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollView")
		bool GetOnlyOneDirection()const { return OnlyOneDirection; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollView")
		float GetScrollSensitivity()const { return ScrollSensitivity; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollView")
		bool GetCanScrollInSmallSize()const { return CanScrollInSmallSize; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollView")
		FVector2D GetVelocity()const { return Velocity; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollView")
		float GetDecelerateRate()const { return DecelerateRate; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollView")
		bool GetRestrictRectArea()const { return RestrictRectArea; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollView")
		float GetOutOfRangeDamper()const { return OutOfRangeDamper; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollView")
		FVector2D GetScrollProgress()const { return Progress; }
	/** Get Content's position range in horizontal. */
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollView")
		FVector2D GetHorizontalRange()const { return HorizontalRange; }
	/** Get Content's position range in vertical. */
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollView")
		FVector2D GetVerticalRange()const { return VerticalRange; }

	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollView")
		void SetScrollSensitivity(float value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollView")
		void SetHorizontal(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollView")
		void SetVertical(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollView")
		void SetOnlyOneDirection(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollView")
		void SetCanScrollInSmallSize(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollView")
		void SetVelocity(const FVector2D& value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollView")
		void SetDecelerateRate(float value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollView")
		void SetRestrictRectArea(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollView")
		void SetOutOfRangeDamper(float value);

	/** Manually scroll it with delta value. */
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollView")
		void SetScrollDelta(FVector2D value);
	/** Manually scroll it with absolute value. The value will be applyed to Content's relative location. */
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollView")
		void SetScrollValue(FVector2D value);
	/** Manually scroll it with progress value (from 0 to 1). */
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollView")
		void SetScrollProgress(FVector2D value);

	/**
	 * Try to scroll the scrollview so the child can sit at center. Will clamp it in valid range.
	 * @param InChild Target child actor.
	 * @param InEaseAnimation true-use tween animation to make smooth scroll, false-immediate set.
	 * @param InAnimationDuration Animation duration if InEaseAnimation = true.
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollView")
		void ScrollTo(ULexWidget* InChild, bool InEaseAnimation = true, float InAnimationDuration = 0.5f);
};


