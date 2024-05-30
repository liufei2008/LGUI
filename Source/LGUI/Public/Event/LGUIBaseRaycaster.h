// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "CollisionQueryParams.h"
#include "Event/LGUIPointerEventData.h"
#include "LGUIBaseRaycaster.generated.h"

enum class ELGUIRenderMode :uint8;

/** 
 * Base interaction component that perform a raycast hit test
 */
UCLASS(Abstract)
class LGUI_API ULGUIBaseRaycaster : public USceneComponent
{
	GENERATED_BODY()
	
public:	
	ULGUIBaseRaycaster();
protected:
	virtual void BeginPlay()override;
	virtual void Activate(bool bReset = false)override;
	virtual void Deactivate()override;
	virtual void OnUnregister()override;

	friend class FUIBaseRaycasterCustomization;

	/** temp array, hit result */
	TArray<FHitResult> multiHitResult;
protected:
	/**
	 * Link pointerID, limit this raycaster to work on specific pointer. This is useful when multiple pointer interact in same level.
	 * Default is -1, means this raycaster will work on all pointers.
	 */
	UPROPERTY(EditAnywhere, Category = LGUI)
		int32 pointerID = INDEX_NONE;
	/** 
	 * For LGUIBaseRaycasters with same depth, LGUI will line trace them all and sort result on hit distance.
	 * For LGUIBaseRaycasters with different depth, LGUI will sort raycasters on depth, and line trace from highest depth to lowest, if hit anything then stop line trace.
	 */
	UPROPERTY(EditAnywhere, Category = LGUI)
		int32 depth = 0;
	/** line trace ray emit length */
	UPROPERTY(EditAnywhere, Category = LGUI)
		float rayLength = 10000;
	UPROPERTY(EditAnywhere, Category = LGUI)
		TEnumAsByte<ETraceTypeQuery> traceChannel;
	UPROPERTY(EditAnywhere, Category = LGUI)
		ELGUIEventFireType eventFireType = ELGUIEventFireType::TargetActorAndAllItsComponents;
	/** click/drag threshold, calculated in target's local space */
	UPROPERTY(EditAnywhere, Category = LGUI)
		float clickThreshold = 5;
	/** hold press for a little while to entering drag mode */
	UPROPERTY(EditAnywhere, Category = LGUI)
		bool holdToDrag = false;
	/** hold press for "holdToDragTime" to entering drag mode */
	UPROPERTY(EditAnywhere, Category = LGUI)
		float holdToDragTime = 0.5f;
	float clickThresholdSquare = 0;
	FVector CurrentRayOrigin = FVector::ZeroVector, CurrentRayDirection = FVector(1, 0, 0);
public:
	/** Called by raycaster to get ray */
	virtual bool GenerateRay(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection) PURE_VIRTUAL(ULGUIBaseRaycaster::GenerateRay, return false;);
	/** Called by InputModule to raycast hit test */
	virtual bool Raycast(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, FHitResult& OutHitResult, TArray<USceneComponent*>& OutHoverArray) PURE_VIRTUAL(ULGUIBaseRaycaster::Raycast, return false;);
	/** Called by InputModule to decide if current trigger press need to convert to drag */
	virtual bool ShouldStartDrag(ULGUIPointerEventData* InPointerEventData) PURE_VIRTUAL(ULGUIBaseRaycaster::ShouldStartDrag, return false;);

	UFUNCTION(BlueprintCallable, Category = LGUI)virtual void ActivateRaycaster();
	UFUNCTION(BlueprintCallable, Category = LGUI)virtual void DeactivateRaycaster();

	UFUNCTION(BlueprintCallable, Category = LGUI)
		int32 GetPointerID()const { return pointerID; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		int32 GetDepth()const { return depth; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		float GetRayLength()const { return rayLength; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		TEnumAsByte<ETraceTypeQuery> GetTraceChannel()const { return traceChannel; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		float GetClickThreshold()const { return clickThreshold; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		ELGUIEventFireType GetEventFireType()const { return eventFireType; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool GetHoldToDrag()const { return holdToDrag; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		float GetHoldToDragTime()const { return holdToDragTime; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		virtual bool GetAffectByGamePause()const { return true; }
	float GetClickThresholdSquare()const { return clickThresholdSquare; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		FVector GetRayOrigin()const { return CurrentRayOrigin; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		FVector GetRayDirection()const { return CurrentRayDirection; }

	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetClickThreshold(float value);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetHoldToDrag(bool value);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetHoldToDragTime(float value);
protected:
	bool ShouldStartDrag_HoldToDrag(ULGUIPointerEventData* InPointerEventData);
	virtual bool ShouldSkipCanvas(class ULGUICanvas* UICanvas) { return false; }
	TArray<FHitResult> multiUIHitResult;
	bool IsHitVisibleUI(class UUIItem* HitUI, const FVector& HitPoint);

	bool RaycastUI(ULGUIPointerEventData* InPointerEventData, const TArray<ELGUIRenderMode>& InRenderModeArray, FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, FHitResult& OutHitResult, TArray<USceneComponent*>& OutHoverArray);
	bool RaycastWorld(bool InRequireFaceIndex, ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, FHitResult& OutHitResult, TArray<USceneComponent*>& OutHoverArray);
};
