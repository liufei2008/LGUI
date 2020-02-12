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
USTRUCT(BlueprintType)
struct FLGUIPointerEventData
{
	GENERATED_BODY()

	//current event target component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LGUI")
		USceneComponent* currentComponent = nullptr;
	//world space hit point
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LGUI")
		FVector worldPoint = FVector(0, 0, 0);
	//world space hit normal
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LGUI")
		FVector worldNormal = FVector(0, 0, 1);
	//world space hit point delta
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LGUI")
		FVector moveDelta = FVector(0, 0, 0);
	//world space hit point cumulative delta
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LGUI")
		FVector cumulativeMoveDelta = FVector(0, 0, 0);
	//current dragging component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LGUI")
		USceneComponent* dragComponent = nullptr;
	//drag event ray emitter's ray origin
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LGUI")
		FVector dragRayOrigin;
	//drag event ray emitter's ray direction
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LGUI")
		FVector dragRayDirection;
	//pointer scroll event
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LGUI")
		float scrollAxisValue = 0;
	//current ray origin
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LGUI")
		FVector rayOrigin;
	//current ray direction
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LGUI")
		FVector rayDirection;
	//current raycaster
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LGUI")
		ULGUIBaseRaycaster* raycaster;
	//mouse input type
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LGUI")
		EMouseButtonType mouseButtonType = EMouseButtonType::Left;
	//event type
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LGUI")
		EPointerEventType eventType = EPointerEventType::Click;



	//hit component when press
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LGUI")
		USceneComponent* pressComponent = nullptr;
	//world space hit point when press and hit something
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LGUI")
		FVector pressWorldPoint = FVector(0, 0, 0);
	//world space normal direction when press and hit something
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LGUI")
		FVector pressWorldNormal = FVector(0, 0, 1);
	//ray distance when press and hit something
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LGUI")
		float pressDistance = 0;
	//ray origin when press and hit something
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LGUI")
		FVector pressRayOrigin;
	//ray direction when press and hit something
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LGUI")
		FVector pressRayDirection;
	//world to press component's local transform when trigger press, usefull to calculate local space point/normal/delta
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LGUI")
		FTransform pressWorldToLocalTransform;
	//raycaster when press
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LGUI")
		ULGUIBaseRaycaster* pressRaycaster;
	//the last time when trigger click(get time from GetWorld()->TimeSeconds), can be used to tell double click
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LGUI")
		float clickTime;



	FORCEINLINE FString ToString()const;
	//use a line-plane intersection to get world point
	FORCEINLINE FVector GetWorldPointInPlane()const;
	//use a line-plane intersection to get world point, and convert to press component's local space
	FORCEINLINE FVector GetLocalPointInPlane()const;
};

FVector FLGUIPointerEventData::GetWorldPointInPlane()const
{
	switch (eventType)
	{
	default:
		return worldPoint;
		break;
	case EPointerEventType::Down:
		return pressWorldPoint;
		break;
	case EPointerEventType::Up:
	case EPointerEventType::Click:
	case EPointerEventType::BeginDrag:
	case EPointerEventType::Drag:
	case EPointerEventType::EndDrag:
		return FMath::LinePlaneIntersection(dragRayOrigin, dragRayOrigin + dragRayDirection * pressRaycaster->rayLength, pressWorldPoint, pressWorldNormal);
		break;
	}
	
}
FVector FLGUIPointerEventData::GetLocalPointInPlane()const
{
	return pressWorldToLocalTransform.TransformPosition(GetWorldPointInPlane());
}

FString FLGUIPointerEventData::ToString()const
{
	FString result;
	if (IsValid(currentComponent))
	{
		result += FString::Printf(TEXT("\n		hitComponent actor:%s, comp:%s"),
#if WITH_EDITOR
			*(currentComponent->GetOwner()->GetActorLabel()),
#else
			*(currentComponent->GetOwner()->GetName()),
#endif
			*(currentComponent->GetPathName()));
	}
	else
	{
		result += TEXT("\n		hitComponent is null");
	}
	if (IsValid(dragComponent))
	{
		result += FString::Printf(TEXT("\n		dragComponent actor:%s, comp:%s"),
#if WITH_EDITOR
			*(dragComponent->GetOwner()->GetActorLabel()),
#else
			*(dragComponent->GetOwner()->GetName()),
#endif
			*(dragComponent->GetPathName()));
	}
	else
	{
		result += TEXT("\n		dragComponent is null");
	}
	result += FString::Printf(TEXT("\n		dragRayOrigin:%s"), *(dragRayOrigin.ToString()));
	result += FString::Printf(TEXT("\n		dragRayDirection:%s"), *(dragRayDirection.ToString()));

	result += FString::Printf(TEXT("\n		worldPoint:%s"), *(worldPoint.ToString()));
	result += FString::Printf(TEXT("\n		moveDelta:%s"), *(moveDelta.ToString()));
	result += FString::Printf(TEXT("\n		cumulativeMoveDelta:%s"), *(cumulativeMoveDelta.ToString()));

	result += FString::Printf(TEXT("\n		scrollAxisValue:%f"), scrollAxisValue);

	result += FString::Printf(TEXT("\n		rayOrigin:%s"), *(rayOrigin.ToString()));
	result += FString::Printf(TEXT("\n		rayDirection:%s"), *(rayDirection.ToString()));

	result += FString::Printf(TEXT("\n		raycaster:%s"), *(IsValid(raycaster) ? raycaster->GetName() : TEXT("null")));

	switch (mouseButtonType)
	{
	case EMouseButtonType::Left:
		result += TEXT("\n		mouseButtonType:Left");
		break;
	case EMouseButtonType::Middle:
		result += TEXT("\n		mouseButtonType:Middle");
		break;
	case EMouseButtonType::Right:
		result += TEXT("\n		mouseButtonType:Right");
		break;
	}
	switch (eventType)
	{
	case EPointerEventType::Click:
		result += TEXT("\n		eventType:Click");
		break;
	case EPointerEventType::Enter:
		result += TEXT("\n		eventType:Enter");
		break;
	case EPointerEventType::Exit:
		result += TEXT("\n		eventType:Exit");
		break;
	case EPointerEventType::Down:
		result += TEXT("\n		eventType:Down");
		break;
	case EPointerEventType::Up:
		result += TEXT("\n		eventType:Up");
		break;
	case EPointerEventType::BeginDrag:
		result += TEXT("\n		eventType:BeginDrag");
		break;
	case EPointerEventType::Drag:
		result += TEXT("\n		eventType:Drag");
		break;
	case EPointerEventType::EndDrag:
		result += TEXT("\n		eventType:EndDrag");
		break;
	case EPointerEventType::Scroll:
		result += TEXT("\n		eventType:Scroll");
		break;
	case EPointerEventType::DragDrop:
		result += TEXT("\n		eventType:DragDrop");
		break;
	case EPointerEventType::Select:
		result += TEXT("\n		eventType:Select");
		break;
	case EPointerEventType::Deselect:
		result += TEXT("\n		eventType:Deselect");
		break;
	}



	if (IsValid(pressComponent))
	{
		result += FString::Printf(TEXT("\n		pressHitComponent actor:%s, comp:%s"),
#if WITH_EDITOR
			*(pressComponent->GetOwner()->GetActorLabel()),
#else
			*(pressComponent->GetOwner()->GetName()),
#endif
			*(pressComponent->GetPathName()));
	}
	else
	{
		result += TEXT("\n		pressHitComponent is null");
	}
	result += FString::Printf(TEXT("\n		pressWorldPoint:%s"), *(pressWorldPoint.ToString()));
	result += FString::Printf(TEXT("\n		pressWorldNormal:%s"), *(pressWorldNormal.ToString()));
	result += FString::Printf(TEXT("\n		pressDistance:%f"), pressDistance);
	result += FString::Printf(TEXT("\n		pressRayOrigin:%s"), *(pressRayOrigin.ToString()));
	result += FString::Printf(TEXT("\n		pressRayDirection:%s"), *(pressRayDirection.ToString()));
	result += FString::Printf(TEXT("\n		pressRaycaster:%s"), *(IsValid(pressRaycaster) ? pressRaycaster->GetName() : TEXT("null")));
	result += FString::Printf(TEXT("\n		pressTime:%f"), clickTime);

	return result;
}
