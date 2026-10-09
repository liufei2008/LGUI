// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LexBaseRaycaster.h"
#include "LexScreenSpaceRaycaster.generated.h"

class ULexCanvas;
enum class ELexRenderMode :uint8;

/**
 * Perform a raycaster interaction for ScreenSpaceUI.
 * This component should be placed on a actor which have a LexCanvas, and RenderMode should set to ScreenSpaceOverlay.
 */
UCLASS(ClassGroup = LGUI, meta = (BlueprintSpawnableComponent), Blueprintable)
class LGUI_API ULexScreenSpaceRaycaster : public ULexBaseRaycaster
{
	GENERATED_BODY()
	
public:	
	ULexScreenSpaceRaycaster();
	virtual void BeginPlay()override;
protected:
	/** ray length for line trace hit */
	UPROPERTY(EditAnywhere, Category = LGUI)
	float RayLength = 100000;
	/** drag threshold, calculated in target's local space */
	UPROPERTY(EditAnywhere, Category = LGUI)
	float DragThreshold = 5;
	/** hold press for a little while to entering drag mode */
	UPROPERTY(EditAnywhere, Category = LGUI)
	bool bHoldToDrag = false;
	/** hold press for "holdToDragTime" to entering drag mode */
	UPROPERTY(EditAnywhere, Category = LGUI, meta = (EditCondition = "bHoldToDrag"))
	float HoldToDragTime = 0.5f;
	float DragThresholdSquare = 0;
	
	TWeakObjectPtr<ULexCanvas> RootCanvas = nullptr;
	bool CheckRootCanvas();
public:
	virtual bool GetAffectByGamePause()const override;
	virtual bool ShouldStartDrag(ULexPointerEventData* InPointerEventData)override;
	virtual bool GenerateRay(ULexPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, float& OutRayLength)override;
	virtual void Raycast(ULexPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, TArray<FLexUIHitResult>& OutHitResult)override;

	static void DeprojectViewPointToWorld(const FMatrix& InViewProjectionMatrix, const FVector2D& InViewPoint01, FVector& OutWorldLocation, FVector& OutWorldDirection);

	virtual float GetRayLength()const override { return RayLength; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
	float GetDragThreshold()const { return DragThreshold; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
	bool GetHoldToDrag()const { return bHoldToDrag; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
	float GetHoldToDragTime()const { return HoldToDragTime; }
	float GetDragThresholdSquare()const { return DragThresholdSquare; }

	UFUNCTION(BlueprintCallable, Category = LGUI)
	void SetRayLength(float Value);
	UFUNCTION(BlueprintCallable, Category = LGUI)
	void SetDragThreshold(float Value);
	UFUNCTION(BlueprintCallable, Category = LGUI)
	void SetHoldToDrag(bool Value);
	UFUNCTION(BlueprintCallable, Category = LGUI)
	void SetHoldToDragTime(float Value);
};
