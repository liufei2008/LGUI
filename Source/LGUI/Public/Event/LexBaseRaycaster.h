// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "CollisionQueryParams.h"
#include "Core/Components/LexCanvas.h"
#include "Event/LexPointerEventData.h"
#include "Engine/HitResult.h"
#include "LexBaseRaycaster.generated.h"

/** 
 * Base interaction component that perform a raycast hit test
 */
UCLASS(Abstract)
class LGUI_API ULexBaseRaycaster : public UActorComponent
{
	GENERATED_BODY()
	
public:	
	ULexBaseRaycaster();
	virtual void BeginPlay()override;
	virtual void Activate(bool bReset = false)override;
	virtual void Deactivate()override;
protected:
	virtual void OnUnregister()override;

	friend class FUIBaseRaycasterCustomization;

protected:
	UPROPERTY(EditAnywhere, Category = LGUI)
	int UserIndex = 0;
	/**
	 * Link PointerID, limit this raycaster to work on specific pointer. This is useful when multiple pointer interact in same level.
	 * Default is -1, means this raycaster will work on all pointers.
	 */
	UPROPERTY(EditAnywhere, Category = LGUI)
		int32 PointerID = INDEX_NONE;
	
	FVector CurrentRayOrigin = FVector::ZeroVector, CurrentRayDirection = FVector(1, 0, 0);
	float CurrentRayLength = 0.0f;
public:
	/** Called by raycaster to get ray */
	virtual bool GenerateRay(ULexPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, float& OutRayLength) PURE_VIRTUAL(ULGUIBaseRaycaster::GenerateRay, return false;);
	/** Called by InputModule to raycast hit test */
	virtual void Raycast(ULexPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, TArray<FLexUIHitResult>& OutHitResultArray) PURE_VIRTUAL(ULGUIBaseRaycaster::Raycast, );
	/** Called by InputModule to decide if current trigger press need to convert to drag */
	virtual bool ShouldStartDrag(ULexPointerEventData* InPointerEventData) PURE_VIRTUAL(ULexBaseRaycaster::ShouldStartDrag, return false;);

	UFUNCTION(BlueprintCallable, Category = LGUI)virtual void ActivateRaycaster();
	UFUNCTION(BlueprintCallable, Category = LGUI)virtual void DeactivateRaycaster();

	UFUNCTION(BlueprintCallable, Category = LGUI)
	int GetUserIndex()const{return UserIndex;}

	UFUNCTION(BlueprintCallable, Category = LGUI)
	int32 GetPointerID()const { return PointerID; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
	virtual bool GetAffectByGamePause()const { return true; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
	FVector GetRayOrigin()const { return CurrentRayOrigin; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
	FVector GetRayDirection()const { return CurrentRayDirection; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
	virtual float GetRayLength()const { return CurrentRayLength; }

	UFUNCTION(BlueprintCallable, Category = LGUI)
	void SetPointerID(int32 Value);
protected:
	void RaycastUI(ULexPointerEventData* InPointerEventData, ULexCanvas* InRootCanvas, FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, TArray<FLexUIHitResult>& OutHitResultArray);
	void RaycastWorld(ULexPointerEventData* InPointerEventData, bool InRequireFaceIndex, ETraceTypeQuery InTraceChannel, FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, TArray<FLexUIHitResult>& OutHitResultArray);
};
