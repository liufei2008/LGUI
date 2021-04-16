// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "Core/ActorComponent/UISprite.h"
#include "Event/LGUIDelegateDeclaration.h"
#include "Event/LGUIDrawableEvent.h"
#include "Event/LGUIDelegateHandleWrapper.h"
#include "Event/Interface/LGUIPointerDragInterface.h"
#include "Event/Interface/LGUIPointerScrollInterface.h"
#include "Event/Interface/LGUIPointerDownUpInterface.h"
#include "Core/LGUIBehaviour.h"
#include "Core/Actor/UIBaseActor.h"
#include "UIScrollViewComponent.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIScrollViewDynamicDelegate, FVector2D, InVector2);

UCLASS(ClassGroup=(LGUI), Transient)
class LGUI_API UUIScrollViewHelper :public ULGUIBehaviour
{
	GENERATED_BODY()
private:
	virtual void OnUIDimensionsChanged(bool positionChanged, bool sizeChanged)override;
	virtual void OnUIChildDimensionsChanged(UUIItem* child, bool positionChanged, bool sizeChanged)override;
	friend class UUIScrollViewComponent;
	UPROPERTY(Transient)
		TWeakObjectPtr<class UUIScrollViewComponent> TargetComp;
};
//ScrollView
UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIScrollViewComponent : public ULGUIBehaviour, public ILGUIPointerDragInterface, public ILGUIPointerScrollInterface, public ILGUIPointerDownUpInterface
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
	/** If Horizontal and Vertical, only allow one direction drag at the same time. Not valid when use scroll(Mouse wheel), because scroll only have one direction. */ 
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

	bool AllowHorizontalScroll, AllowVerticalScroll;
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
	bool CanUpdateAfterDrag = false;
	FVector PrevWorldPoint;
private:
	void UpdateAfterDrag(float deltaTime);

	FLGUIMulticastVector2Delegate OnScrollCPP;
	UPROPERTY(EditAnywhere, Category = "LGUI-ScrollView")
		FLGUIDrawableEvent OnScroll = FLGUIDrawableEvent(LGUIDrawableEventParameterType::Vector2);

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
};
