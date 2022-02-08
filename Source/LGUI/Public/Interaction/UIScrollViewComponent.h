// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "Core/ActorComponent/UISprite.h"
#include "Event/LGUIDelegateDeclaration.h"
#include "Event/LGUIEventDelegate.h"
#include "Event/LGUIDelegateHandleWrapper.h"
#include "Event/Interface/LGUIPointerDragInterface.h"
#include "Event/Interface/LGUIPointerScrollInterface.h"
#include "Event/Interface/LGUIPointerDownUpInterface.h"
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
	virtual void OnUIDimensionsChanged(bool positionChanged, bool sizeChanged)override;
	virtual void OnUIChildDimensionsChanged(UUIItem* child, bool positionChanged, bool sizeChanged)override;
	friend class UUIScrollViewComponent;
	UPROPERTY(Transient)
		TWeakObjectPtr<class UUIScrollViewComponent> TargetComp;
};
//ScrollView
UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIScrollViewComponent : public ULGUILifeCycleUIBehaviour, public ILGUIPointerDragInterface, public ILGUIPointerScrollInterface, public ILGUIPointerDownUpInterface
{
	GENERATED_BODY()
	
protected:
	virtual void Awake() override;
	virtual void Update(float DeltaTime) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	virtual void OnUIActiveInHierachy(bool ativeOrInactive)override;
	virtual void OnUIDimensionsChanged(bool positionChanged, bool sizeChanged)override;
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
	/** When Content size is smaller than Content's parent size, can we still scroll? */
	UPROPERTY(EditAnywhere, Category = "LGUI-ScrollView")
		bool CanScrollInSmallSize = true;

	/** inherited events of this component can bubble up? */
	UPROPERTY(EditAnywhere, Category = "LGUI-ScrollView")
		bool AllowEventBubbleUp = false;

	uint8 AllowHorizontalScroll: 1, AllowVerticalScroll: 1;
	uint8 CanUpdateAfterDrag: 1;
	uint8 bRangeCalculated: 1;

	virtual void CalculateHorizontalRange();
	virtual void CalculateVerticalRange();
	bool CheckParameters();
	virtual bool CheckValidHit(USceneComponent* InHitComp);
	UPROPERTY(Transient)TWeakObjectPtr<UUIItem> ContentUIItem = nullptr;//drag or scroll Content
	UPROPERTY(Transient)TWeakObjectPtr<UUIItem> ContentParentUIItem = nullptr;//Content's parent
	FVector2D Progress = FVector2D(0, 0);//progress, 0--1, x for horizontal, y for vertical
	virtual void UpdateProgress(bool InFireEvent = true);
	FVector Position = FVector(0, 0, 0);//content position in parent space
	FVector2D DragSpeed = FVector2D(0, 0);//drag speed
	FVector2D HorizontalRange;//horizontal scroll range, x--min, y--max
	FVector2D VerticalRange;//vertical scroll range, x--min, y--max
	FVector PrevWorldPoint;
private:
	void UpdateAfterDrag(float deltaTime);

	FLGUIMulticastVector2Delegate OnScrollCPP;
	UPROPERTY(EditAnywhere, Category = "LGUI-ScrollView")
		FLGUIEventDelegate OnScroll = FLGUIEventDelegate(LGUIEventDelegateParameterType::Vector2);

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

	virtual bool OnPointerDown_Implementation(ULGUIPointerEventData* eventData)override;
	virtual bool OnPointerUp_Implementation(ULGUIPointerEventData* eventData)override;

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
		void SetScrollSensitivity(float value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollView")
		void SetHorizontal(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollView")
		void SetVertical(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollView")
		void SetOnlyOneDirection(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollView")
		void SetCanScrollInSmallSize(bool value);

	/** Mannually scroll it with delta value. */
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollView")
		void SetScrollDelta(FVector2D value);
	/** Mannually scroll it with absolute value. The value will be applyed to Content's relative location. */
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollView")
		void SetScrollValue(FVector2D value);
	/** Mannually scroll it with progress value (from 0 to 1). The value will be applyed to Content's relative location. */
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollView")
		void SetScrollProgress(FVector2D value);
};
