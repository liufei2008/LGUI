// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/PrimitiveComponent.h"
#include "LGUIPointerEventData.generated.h"

class ULGUIBaseRaycaster;

/** event execute type */
UENUM(BlueprintType)
enum class ELGUIEventFireType :uint8
{
	/** event will call on trace target actor and all component of the actor */
	TargetActorAndAllItsComponents,
	/** event will call only on trace target */
	OnlyTargetComponent,
	/** event will call only on trace target actor */
	OnlyTargetActor,
};

UENUM(BlueprintType)
enum class EPointerEventType :uint8
{
	Click = 0,
	Enter = 1,
	Exit = 2,
	Down = 3,
	Up = 4,
	BeginDrag = 5,
	Drag = 6,
	EndDrag = 7,
	Scroll = 8,
	/* @deprecated This was removed in LGUI version 2.10.2*/
	DragEnter_DEPRECATED = 9,
	/* @deprecated This was removed in LGUI version 2.10.2*/
	DragExit_DEPRECATED = 10,
	DragDrop = 11,
	Select = 12,
	Deselect = 13,
};
UENUM(BlueprintType)
enum class EMouseButtonType :uint8
{
	Left,Middle,Right,
	/** UserDefinedX is for custom defined input buttun type */
	UserDefined1,
	UserDefined2,
	UserDefined3,
	UserDefined4,
	UserDefined5,
	UserDefined6,
	UserDefined7,
	UserDefined8,
};
UCLASS(BlueprintType)
class LGUI_API ULGUIBaseEventData :public UObject
{
	GENERATED_BODY()
public:
	/** current selected component. when call Deselect interface, this is also the new selected component*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		USceneComponent* selectedComponent = nullptr;
	/** event type*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		EPointerEventType eventType = EPointerEventType::Click;

	ELGUIEventFireType selectedComponentEventFireType = ELGUIEventFireType::TargetActorAndAllItsComponents;

	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToString (LGUIEventData)", CompactNodeTitle = ".", BlueprintAutocast), Category = "LGUI") virtual FString ToString()const 
	{
		return TEXT("");
	};
};

#ifndef FLGUIPointerEventData
#define FLGUIPointerEventData DEPRECATED_MACRO(4.23, "FLGUIPointerEventData has been changed to ULGUIPointerEventData which is inherited from UObject.") ULGUIPointerEventData
#endif

UCLASS(BlueprintType)
class LGUI_API ULGUIPointerEventData: public ULGUIBaseEventData
{
	GENERATED_BODY()
public:
	/** id of the pointer (touch id) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LGUI")
		int pointerID = 0;
	/** current pointer position (mouse positin or touch point position in screen space) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LGUI")
		FVector pointerPosition;

	UE_DEPRECATED(4.23, "currentComponent not valid anymore, use enterComponent instead.")
	UPROPERTY()
		USceneComponent* currentComponent_DEPRECATED = nullptr;

	/** enterred component */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LGUI")
		USceneComponent* enterComponent = nullptr;
	/** a stack list for store enterred component. the latest enter actor stay at num-1, first stay at 0 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LGUI")
		TArray<USceneComponent*> enterComponentStack;
	/** a collection that current pointer hoverring objects */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LGUI")
		TArray<USceneComponent*> hoverComponentArray;
	/** current world space hit point */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LGUI")
		FVector worldPoint = FVector(0, 0, 0);
	/** current world space hit normal */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LGUI")
		FVector worldNormal = FVector(0, 0, 1);

	/** current world space hit point delta when drag */
	UPROPERTY(meta = (DeprecatedProperty, DeprecationMessage = "This property is deprecated. Use GetWorldPointSpherical() to get cumulative point and minus prev one to get delta."))
		FVector moveDelta_DEPRECATED = FVector(0, 0, 0);
	/** current world space hit point cumulative delta when drag */
	UPROPERTY(meta = (DeprecatedProperty, DeprecationMessage = "This property is deprecated. Use GetCumulativeMoveDelta() instead."))
		FVector cumulativeMoveDelta_DEPRECATED = FVector(0, 0, 0);

	/** pointer scroll event */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LGUI")
		float scrollAxisValue = 0;
	/** current ray origin */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LGUI")
		FVector rayOrigin;
	/** current ray direction */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LGUI")
		FVector rayDirection;
	/** current raycaster */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LGUI")
		ULGUIBaseRaycaster* raycaster;
	/** mouse input type */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LGUI")
		EMouseButtonType mouseButtonType = EMouseButtonType::Left;

	/** hit component when press */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LGUI")
		USceneComponent* pressComponent = nullptr;
	/** world space hit point when press and hit something */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LGUI")
		FVector pressWorldPoint = FVector(0, 0, 0);
	/** world space normal direction when press and hit something */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LGUI")
		FVector pressWorldNormal = FVector(0, 0, 1);
	/** ray distance when press and hit something */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LGUI")
		float pressDistance = 0;
	/** ray origin when press and hit something */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LGUI")
		FVector pressRayOrigin;
	/** ray direction when press and hit something */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LGUI")
		FVector pressRayDirection;
	/** world to press component's local transform when trigger press, usefull to calculate local space point/normal/delta */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LGUI")
		FTransform pressWorldToLocalTransform;
	/** raycaster when press */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LGUI")
		ULGUIBaseRaycaster* pressRaycaster;
	/** the last time when trigger click(get time from GetWorld()->TimeSeconds), can be used to tell double click */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LGUI")
		float clickTime;
	/** the last time when trigger press. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LGUI")
		float pressTime;

	/** is dragging? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LGUI")
		bool isDragging = false;
	/** current dragging component */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LGUI")
		USceneComponent* dragComponent = nullptr;
	/** drag event ray emitter's ray origin */
	UPROPERTY(meta = (DeprecatedProperty, DeprecationMessage = "This property is deprecated. Use GetDragRayOrigin() function instead."))
		FVector dragRayOrigin_DEPRECATED;
	/** drag event ray emitter's ray direction */
	UPROPERTY(meta = (DeprecatedProperty, DeprecationMessage = "This property is deprecated. Use GetDragRayDirection() function instead."))
		FVector dragRayDirection_DEPRECATED;

	/** enter a component when drag anything */
	UE_DEPRECATED(4.23, "dragEnterComponent not valid anymore, use isDragging and enterComponent to get drag and enter component.")
		USceneComponent* dragEnterComponent = nullptr;

	ELGUIEventFireType enterComponentEventFireType = ELGUIEventFireType::TargetActorAndAllItsComponents;
	ELGUIEventFireType pressComponentEventFireType = ELGUIEventFireType::TargetActorAndAllItsComponents;
	ELGUIEventFireType dragComponentEventFireType = ELGUIEventFireType::TargetActorAndAllItsComponents;

	bool isUpFiredAtCurrentFrame = false;//is PointerUp event in current frame is called?
	bool isExitFiredAtCurrentFrame = false;//is PointerExit event in current frame is called?
	bool isEndDragFiredAtCurrentFrame = false;//is EndDrag event in current frame is called?

	bool nowIsTriggerPressed = false;
	bool prevIsTriggerPressed = false;
	EMouseButtonType prevPressTriggerType = EMouseButtonType::Left;

	virtual FString ToString()const override;
	/** Use a line-plane intersection to get world point. The plane is pressComponent's z-axis plane. */
	UFUNCTION(BlueprintCallable, Category = "LGUI") 
		FVector GetWorldPointInPlane()const;
	/** Use a line-plane intersection to get world point, and convert to pressComponent's local space. The plane is pressComponent's z-axis plane.  */
	UFUNCTION(BlueprintCallable, Category = "LGUI") 
		FVector GetLocalPointInPlane()const;
	/** Use (ray direction) * (press line distance) + (ray origin) to calculated world point, so the result is a sphere with (ray origin) as center point. */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FVector GetWorldPointSpherical()const;
	/** Get ray origin of drag */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FVector GetDragRayOrigin()const;
	/** Get ray direction of drag */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FVector GetDragRayDirection()const;
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FVector GetCumulativeMoveDelta()const;
};
