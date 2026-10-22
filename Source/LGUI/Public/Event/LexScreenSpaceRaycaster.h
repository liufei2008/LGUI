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
	UPROPERTY(EditAnywhere, Category = "LexUI")
	float RayLength = 100000;
	/** drag threshold, calculated in target's local space */
	UPROPERTY(EditAnywhere, Category = "LexUI")
	float DragThreshold = 5;
	/** hold press for a little while to entering drag mode */
	UPROPERTY(EditAnywhere, Category = "LexUI")
	bool bHoldToDrag = false;
	/** hold press for "holdToDragTime" to entering drag mode */
	UPROPERTY(EditAnywhere, Category = "LexUI", meta = (EditCondition = "bHoldToDrag"))
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
	UFUNCTION(BlueprintCallable, Category = "LexUI")
	float GetDragThreshold()const { return DragThreshold; }
	UFUNCTION(BlueprintCallable, Category = "LexUI")
	bool GetHoldToDrag()const { return bHoldToDrag; }
	UFUNCTION(BlueprintCallable, Category = "LexUI")
	float GetHoldToDragTime()const { return HoldToDragTime; }
	float GetDragThresholdSquare()const { return DragThresholdSquare; }

	UFUNCTION(BlueprintCallable, Category = "LexUI")
	void SetRayLength(float Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI")
	void SetDragThreshold(float Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI")
	void SetHoldToDrag(bool Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI")
	void SetHoldToDragTime(float Value);
};
