// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "Core/ActorComponent/UISprite.h"
#include "Event/LGUIDelegateDeclaration.h"
#include "Event/LGUIDrawableEvent.h"
#include "Event/LGUIDelegateHandleWrapper.h"
#include "Event/LGUIPointerDragInterface.h"
#include "Event/LGUIPointerScrollInterface.h"
#include "Event/LGUIPointerDownUpInterface.h"
#include "Core/UIComponentBase.h"
#include "Core/Actor/UIBaseActor.h"
#include "UIScrollViewComponent.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIScrollViewDynamicDelegate, FVector2D, InVector2);

UCLASS(ClassGroup=(LGUI), Transient)
class LGUI_API UUIScrollViewHelper :public UUIComponentBase
{
	GENERATED_BODY()
private:
	virtual void OnUIDimensionsChanged(bool positionChanged, bool sizeChanged)override;
	virtual void OnUIChildDimensionsChanged(UUIItem* child, bool positionChanged, bool sizeChanged)override;
	friend class UUIScrollViewComponent;
	class UUIScrollViewComponent* TargetComp;
};
//ScrollView
UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIScrollViewComponent : public UUIComponentBase, public ILGUIPointerDragInterface, public ILGUIPointerScrollInterface, public ILGUIPointerDownUpInterface
{
	GENERATED_BODY()
	
public:	
	UUIScrollViewComponent();
protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	virtual void OnUIActiveInHierachy(bool ativeOrInactive)override;
	virtual void OnUIDimensionsChanged(bool positionChanged, bool sizeChanged)override;
	virtual void RecalculateRange();
protected:
	friend class UUIScrollViewHelper;
	//Content can move inside it's parent area
	UPROPERTY(EditAnywhere, Category = "LGUI-ScrollView")
		AUIBaseActor* Content;
	UPROPERTY(EditAnywhere, Category = "LGUI-ScrollView")
		bool Horizontal = true;
	UPROPERTY(EditAnywhere, Category = "LGUI-ScrollView")
		bool Vertical = true;
	//If Horizontal and Vertical, only allow one direction drag at the same time. Not valid when use scroll(Mouse wheel), because scroll only have one direction
	UPROPERTY(EditAnywhere, Category = "LGUI-ScrollView")
		bool OnlyOneDirection = true;
	//Sensitivity when use scroll(Mouse wheel)
	UPROPERTY(EditAnywhere, Category = "LGUI-ScrollView")
		float ScrollSensitivity = 1.0f;

	UPROPERTY(EditAnywhere, Category = "LGUI-ScrollView")
		bool AllowEventBubbleUp = false;

	bool AllowHorizontalScroll, AllowVerticalScroll;
	virtual void CalculateHorizontalRange();
	virtual void CalculateVerticalRange();
	bool CheckParameters();
	virtual bool CheckValidHit(USceneComponent* InHitComp);
	UPROPERTY(Transient)UUIItem* ContentUIItem = nullptr;//drag or scroll Content
	UPROPERTY(Transient)UUIItem* ContentParentUIItem = nullptr;//Content's parent
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

	void RegisterScrollEvent(const FLGUIVector2Delegate& InDelegate);
	void UnregisterScrollEvent(const FLGUIVector2Delegate& InDelegate);

	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollView")
		FLGUIDelegateHandleWrapper RegisterScrollEvent(const FLGUIScrollViewDynamicDelegate& InDelegate);
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollView")
		void UnregisterScrollEvent(const FLGUIDelegateHandleWrapper& InDelegateHandle);

	virtual bool OnPointerDown_Implementation(const FLGUIPointerEventData& eventData)override;
	virtual bool OnPointerUp_Implementation(const FLGUIPointerEventData& eventData)override;

	virtual bool OnPointerBeginDrag_Implementation(const FLGUIPointerEventData& eventData)override;
	virtual bool OnPointerDrag_Implementation(const FLGUIPointerEventData& eventData)override;
	virtual bool OnPointerEndDrag_Implementation(const FLGUIPointerEventData& eventData)override;

	virtual bool OnPointerScroll_Implementation(const FLGUIPointerEventData& eventData)override;
};
