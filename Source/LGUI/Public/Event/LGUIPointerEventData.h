// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/PrimitiveComponent.h"
#include "Raycaster/LGUIBaseRaycaster.h"
#include "LGUIPointerEventData.generated.h"

UENUM(BlueprintType)
enum class EPointerEventType:uint8
{
	Click,
	Enter,
	Exit,
	Down,
	Up,
	BeginDrag,
	Drag,
	EndDrag,
	Scroll,
	DragEnter,
	DragExit,
	DragDrop,
	Select,
	Deselect,
};
UENUM(BlueprintType)
enum class EMouseButtonType :uint8
{
	Left,Middle,Right,
	///UserDefinedX is for custom defined input buttun type
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
	//current selected component
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		USceneComponent* selectedComponent = nullptr;
	//event type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		EPointerEventType eventType = EPointerEventType::Click;

	bool selectedComponentEventFireOnAllOrOnlyTarget = false;

	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToString (LGUIEventData)", CompactNodeTitle = ".", BlueprintAutocast), Category = "LGUI") virtual FString ToString()const 
	{
		return TEXT("");
	};
};

#ifndef FLGUIPointerEventData
#define FLGUIPointerEventData DEPRECATED_MACRO(4.23, "FLGUIPointerEventData has been changed to ULGUIPointerEventData which is inherited from UObject.") ULGUIPointerEventData
#endif
//change from ustruct to uclass for something big...
UCLASS(BlueprintType)
class LGUI_API ULGUIPointerEventData: public ULGUIBaseEventData
{
	GENERATED_BODY()
public:
	//id of the pointer (touch id)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		int pointerID = 0;

	UE_DEPRECATED(4.23, "hitComponent not valid anymore, use enterComponent instead.")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		USceneComponent* hitComponent = nullptr;

	//enterred component
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		USceneComponent* enterComponent = nullptr;
	//current world space hit point
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		FVector worldPoint = FVector(0, 0, 0);
	//current world space hit normal
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		FVector worldNormal = FVector(0, 0, 1);

	//current world space hit point delta when drag
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		FVector moveDelta = FVector(0, 0, 0);
	//current world space hit point cumulative delta when drag
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		FVector cumulativeMoveDelta = FVector(0, 0, 0);

	//pointer scroll event
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		float scrollAxisValue = 0;
	//current ray origin
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		FVector rayOrigin;
	//current ray direction
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		FVector rayDirection;
	//current raycaster
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		ULGUIBaseRaycaster* raycaster;
	//mouse input type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		EMouseButtonType mouseButtonType = EMouseButtonType::Left;

	//hit component when press
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		USceneComponent* pressComponent = nullptr;
	//world space hit point when press and hit something
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		FVector pressWorldPoint = FVector(0, 0, 0);
	//world space normal direction when press and hit something
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		FVector pressWorldNormal = FVector(0, 0, 1);
	//ray distance when press and hit something
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		float pressDistance = 0;
	//ray origin when press and hit something
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		FVector pressRayOrigin;
	//ray direction when press and hit something
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		FVector pressRayDirection;
	//world to press component's local transform when trigger press, usefull to calculate local space point/normal/delta
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		FTransform pressWorldToLocalTransform;
	//raycaster when press
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		ULGUIBaseRaycaster* pressRaycaster;
	//the last time when trigger click(get time from GetWorld()->TimeSeconds), can be used to tell double click
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		float clickTime;
	//the last time when trigger press.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		float pressTime;

	//current dragging component
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		USceneComponent* dragComponent = nullptr;
	//drag event ray emitter's ray origin
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		FVector dragRayOrigin;
	//drag event ray emitter's ray direction
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		FVector dragRayDirection;

	//enter a component when drag anything
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		USceneComponent* dragEnterComponent = nullptr;


	bool enterComponentEventFireOnAllOrOnlyTarget = false;
	bool dragComponentEventFireOnAllOrOnlyTarget = false;
	bool dragEnterComponentEventFireOnAllOrOnlyTarget = false;

	bool isUpFiredAtCurrentFrame = false;//is PointerUp event in current frame is called?
	bool isExitFiredAtCurrentFrame = false;//is PointerExit event in current frame is called?
	bool isEndDragFiredAtCurrentFrame = false;//is EndDrag event in current frame is called?
	bool isDragExitFiredAtCurrentFrame = false;//is EndDrag event in current frame is called?

	bool dragging = false;//is dragging anything?

	bool nowIsTriggerPressed = false;
	bool prevIsTriggerPressed = false;
	EMouseButtonType prevPressTriggerType = EMouseButtonType::Left;

	virtual FString ToString()const override;
	//use a line-plane intersection to get world point
	UFUNCTION(BlueprintCallable, Category = "LGUI") FVector GetWorldPointInPlane()const;
	//use a line-plane intersection to get world point, and convert to press component's local space
	UFUNCTION(BlueprintCallable, Category = "LGUI") FVector GetLocalPointInPlane()const;
};
