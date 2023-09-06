// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "LGUIBaseEventData.h"
#include "LGUIPointerEventData.generated.h"

class ULGUIBaseRaycaster;

UENUM(BlueprintType, Category = LGUI)
enum class ELGUINavigationDirection :uint8
{
	None,
	Left,
	Right,
	Up,
	Down,
	Next,
	Prev,
};
UENUM(BlueprintType, Category = LGUI)
enum class ELGUIPointerInputType :uint8
{
	Pointer,
	Navigation,
};

UCLASS(BlueprintType, classGroup = LGUI)
class LGUI_API ULGUIPointerEventData: public ULGUIBaseEventData
{
	GENERATED_BODY()
public:
	/**
	 * pointer or navigation input?
	 * note some data is not valid when in navigation input.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LGUI")
		ELGUIPointerInputType inputType;

	/** id of the pointer (touch id) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LGUI")
		int pointerID = 0;
	/** current pointer position (mouse position or touch point position in screen space. X&Y for mouse position) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LGUI")
		FVector pointerPosition = FVector::ZeroVector;
	/** pointer position when press (mouse position or touch point position in screen space. X&Y for mouse position) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LGUI")
		FVector pressPointerPosition = FVector::ZeroVector;

	/** enterred component */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LGUI")
		USceneComponent* enterComponent = nullptr;
	/** a stack list for store enterred component. the latest enter one stay at num-1, first stay at 0. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LGUI")
		TArray<USceneComponent*> enterComponentStack;
	/** a collection that current pointer hoverring objects. the top most one stay at index 0 in array. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LGUI")
		TArray<USceneComponent*> hoverComponentArray;
	/** current world space hit point */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LGUI")
		FVector worldPoint = FVector(0, 0, 0);
	/** current world space hit normal */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LGUI")
		FVector worldNormal = FVector(0, 0, 1);
	/**
	 * current hit object's triangle face index.
	 * only valid when raycast hit world space mesh object and LGUIWorldSpaceRaycaster->bRequireFaceIndex is true.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LGUI")
		int32 faceIndex = -1;

	/** pointer scroll event. X for horizontal, Y for vertical */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LGUI")
		FVector2D scrollAxisValue = FVector2D::ZeroVector;
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
		double clickTime;
	/** the last time when trigger release(get time from GetWorld()->TimeSeconds). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LGUI")
		double releaseTime;
	/** the last time when trigger press(get time from GetWorld()->TimeSeconds). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LGUI")
		double pressTime;

	/** is dragging? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LGUI")
		bool isDragging = false;
	/** current dragging component */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LGUI")
		USceneComponent* dragComponent = nullptr;

	ELGUIEventFireType enterComponentEventFireType = ELGUIEventFireType::TargetActorAndAllItsComponents;
	ELGUIEventFireType pressComponentEventFireType = ELGUIEventFireType::TargetActorAndAllItsComponents;
	ELGUIEventFireType dragComponentEventFireType = ELGUIEventFireType::TargetActorAndAllItsComponents;

	bool isUpFiredAtCurrentFrame = false;//PointerUp event is called at current frame?
	bool isExitFiredAtCurrentFrame = false;//PointerExit event is called at current frame?
	bool isEndDragFiredAtCurrentFrame = false;//EndDrag event is called at current frame?

	bool nowIsTriggerPressed = false;
	bool prevIsTriggerPressed = false;

	TWeakObjectPtr<USceneComponent> highlightComponentForNavigation = nullptr;
	float navigateTickTime = 0;
	ELGUINavigationDirection navigateDirection = ELGUINavigationDirection::None;
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetHighlightedComponentForNavigation(USceneComponent* InComp);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		USceneComponent* GetHighlightedComponentForNavigation()const { return highlightComponentForNavigation.Get(); }

	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool IsPointerOverUI();

	virtual FString ToString()const override;
	/** Use a line-plane intersection to get world point. The plane is pressComponent's x-axis plane. */
	UFUNCTION(BlueprintCallable, Category = "LGUI") 
		FVector GetWorldPointInPlane()const;
	/** Use a line-plane intersection to get world point, and convert to pressComponent's local space. The plane is pressComponent's x-axis plane.  */
	UFUNCTION(BlueprintCallable, Category = "LGUI") 
		FVector GetLocalPointInPlane()const;
	/** Use (ray direction) * (press line distance) + (ray origin) to calculated world point, so the result is a sphere with (ray origin) as center point. */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FVector GetWorldPointSpherical()const;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FVector GetDragRayOrigin()const;
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FVector GetDragRayDirection()const;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FVector GetCumulativeMoveDelta()const;
};

struct FLGUIHitResult
{
	FHitResult hitResult;
	ELGUIEventFireType eventFireType = ELGUIEventFireType::TargetActorAndAllItsComponents;

	FVector rayOrigin = FVector(0, 0, 0), rayDirection = FVector(1, 0, 0), rayEnd = FVector(1, 0, 0);

	ULGUIBaseRaycaster* raycaster = nullptr;

	TArray<USceneComponent*> hoverArray;
};
