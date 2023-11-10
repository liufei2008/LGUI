// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "Core/ActorComponent/UISprite.h"
#include "Event/LGUIDelegateDeclaration.h"
#include "Event/LGUIEventDelegate.h"
#include "LGUIDelegateHandleWrapper.h"
#include "Event/Interface/LGUIPointerDragInterface.h"
#include "Event/Interface/LGUIPointerScrollInterface.h"
#include "Core/LGUILifeCycleUIBehaviour.h"
#include "Core/Actor/UIBaseActor.h"
#include "UIScrollViewComponent.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIScrollViewDynamicDelegate, FVector2D, InVector2);

UCLASS(ClassGroup=(LGUI), Transient)
class LGUI_API UUIScrollViewHelper :public ULGUILifeCycleUIBehaviour
{
	GENERATED_BODY()
private:
	virtual void Awake()override;
	virtual void OnUIDimensionsChanged(bool horizontalPositionChanged, bool verticalPositionChanged, bool widthChanged, bool heightChanged)override;
	virtual void OnUIChildDimensionsChanged(UUIItem* child, bool horizontalPositionChanged, bool verticalPositionChanged, bool widthChanged, bool heightChanged)override;
	friend class UUIScrollViewComponent;
	UPROPERTY(Transient)
		TWeakObjectPtr<class UUIScrollViewComponent> TargetComp;
};
//ScrollView
UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIScrollViewComponent : public ULGUILifeCycleUIBehaviour, public ILGUIPointerDragInterface, public ILGUIPointerScrollInterface
{
	GENERATED_BODY()
	
protected:
	virtual void Awake() override;
	virtual void Update(float DeltaTime) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	virtual void OnUIActiveInHierachy(bool ativeOrInactive)override;
	virtual void OnUIDimensionsChanged(bool horizontalPositionChanged, bool verticalPositionChanged, bool widthChanged, bool heightChanged)override;
	virtual void RecalculateRange();
protected:
	friend class UUIScrollViewHelper;
	/** Content can move inside it's parent area. */ 
	UPROPERTY(EditAnywhere, Category = "LGUI-ScrollView")
		TWeakObjectPtr<AUIBaseActor> Content;
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
	virtual bool CheckValidHit(USceneComponent* InHitComp);
	UPROPERTY(Transient)TWeakObjectPtr<UUIItem> ContentUIItem = nullptr;//drag or scroll Content
	UPROPERTY(Transient)TWeakObjectPtr<UUIItem> ContentParentUIItem = nullptr;//Content's parent
	virtual void UpdateProgress(bool InFireEvent = true);
	FVector2D Velocity = FVector2D(0, 0);//drag speed
	FVector2D HorizontalRange;//horizontal scroll range, x--min, y--max
	FVector2D VerticalRange;//vertical scroll range, x--min, y--max
	FVector PrevPointerPosition;//prev frame pointer hit position in world

	void UpdateAfterDrag(float deltaTime);
	virtual void ApplyContentPositionWithProgress();

	FLGUIMulticastVector2Delegate OnScrollCPP;
	UPROPERTY(EditAnywhere, Category = "LGUI-ScrollView")
		FLGUIEventDelegate OnScroll = FLGUIEventDelegate(ELGUIEventDelegateParameterType::Vector2);

public:
	//scroll range change(eg content or content's parent size change), use this to recalculate range
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollView")
		void RectRangeChanged();

	FDelegateHandle RegisterScrollEvent(const FLGUIVector2Delegate& InDelegate);
	FDelegateHandle RegisterScrollEvent(const TFunction<void(FVector2D)>& InFunction);
	void UnregisterScrollEvent(const FDelegateHandle& InHandle);

	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollView")
		FLGUIDelegateHandleWrapper RegisterScrollEvent(const FLGUIScrollViewDynamicDelegate& InDelegate);
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollView")
		void UnregisterScrollEvent(const FLGUIDelegateHandleWrapper& InDelegateHandle);

	virtual bool OnPointerBeginDrag_Implementation(ULGUIPointerEventData* eventData)override;
	virtual bool OnPointerDrag_Implementation(ULGUIPointerEventData* eventData)override;
	virtual bool OnPointerEndDrag_Implementation(ULGUIPointerEventData* eventData)override;

	virtual bool OnPointerScroll_Implementation(ULGUIPointerEventData* eventData)override;

	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollView")
		AUIBaseActor* GetContent()const { return Content.Get(); }
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
		void SetOutOfRangeDamper(float value);

	/** Mannually scroll it with delta value. */
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollView")
		void SetScrollDelta(FVector2D value);
	/** Mannually scroll it with absolute value. The value will be applyed to Content's relative location. */
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollView")
		void SetScrollValue(FVector2D value);
	/** Mannually scroll it with progress value (from 0 to 1). */
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollView")
		void SetScrollProgress(FVector2D value);

	/**
	 * Try to scroll the scrollview so the child can sit at center. Will clamp it in valid range.
	 * @param InChild Target child actor. Can also work on not child actor.
	 * @param InEaseAnimation true-use tween animation to make smooth scroll, false-immediate set.
	 * @param InAnimationDuration Animation duration if InEaseAnimation = true.
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollView")
		void ScrollTo(AUIBaseActor* InChild, bool InEaseAnimation = true, float InAnimationDuration = 0.5f);
};


