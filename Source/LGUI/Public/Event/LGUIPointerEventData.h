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
//change from ustruct to uclass for something big...
UCLASS(BlueprintType)
class LGUI_API ULGUIPointerEventData: public UObject
{
	GENERATED_BODY()
public:
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

	//current dragging component
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		USceneComponent* dragComponent = nullptr;
	//drag event ray emitter's ray origin
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		FVector dragRayOrigin;
	//drag event ray emitter's ray direction
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		FVector dragRayDirection;

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
	//event type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		EPointerEventType eventType = EPointerEventType::Click;

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



	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToString (LGUIPointerEventData)", CompactNodeTitle = ".", BlueprintAutocast), Category = "LGUI") FString ToString()const;
	//use a line-plane intersection to get world point
	UFUNCTION(BlueprintCallable, Category = "LGUI") FVector GetWorldPointInPlane()const;
	//use a line-plane intersection to get world point, and convert to press component's local space
	UFUNCTION(BlueprintCallable, Category = "LGUI") FVector GetLocalPointInPlane()const;
};
