// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Raycaster/LGUI_UIRaycaster.h"
#include "LGUIWorldSpaceInteraction.generated.h"

/**
 * The interaction source for world space UI, actually the ray emitter.
 */
UENUM(BlueprintType, Category = LGUI)
enum class ELGUIWorldSpaceInteractionSource :uint8
{
	/** Sends traces from the world location and orientation of the interaction component. */
	World,
	/** Sends traces from the mouse location of the first local player controller. */
	Mouse,
	/** Sends trace from the center of the first local player's screen. */
	CenterScreen,
	/**
	 * Sends traces from a custom location determined by the user.  Will use whatever
	 * FHitResult is set by the call to SetCustomHitResult.
	 */
	//Custom
};
/**
 * Perform a raycaster interaction for WorldSpaceUI.
 * When hit play, a Rayemitter will be created depend on interactionSource.
 */
UCLASS(ClassGroup = LGUI, meta = (BlueprintSpawnableComponent), Blueprintable)
class LGUI_API ULGUIWorldSpaceInteraction : public ULGUI_UIRaycaster
{
	GENERATED_BODY()
	
public:	
	ULGUIWorldSpaceInteraction();
protected:
	/** click/drag threshold, calculated in target's local space */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = LGUI)
		float clickThreshold = 5;
	/** hold press for a little while to entering drag mode */
	UPROPERTY(EditAnywhere, Category = LGUI)
		bool holdToDrag = false;
	/** hold press for "holdToDragTime" to entering drag mode */
	UPROPERTY(EditAnywhere, Category = LGUI)
		float holdToDragTime = 0.5f;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		ELGUIWorldSpaceInteractionSource interactionSource = ELGUIWorldSpaceInteractionSource::Mouse;
	void CheckRayemitter();
	virtual bool ShouldSkipUIItem(class UUIItem* UIItem)override;
public:
	virtual bool Raycast(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, FHitResult& OutHitResult, TArray<USceneComponent*>& OutHoverArray)override;
	UFUNCTION(BlueprintCallable, Category = LGUI)
		float GetClickThreshold()const { return clickThreshold; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool GetHoldToDrag()const { return holdToDrag; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		float GetHoldToDragTime()const { return holdToDragTime; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		ELGUIWorldSpaceInteractionSource GetInteractionSource()const { return interactionSource; }

	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetClickThreshold(float value);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetHoldToDrag(bool value);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetHoldToDragTime(float value);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetInteractionSource(ELGUIWorldSpaceInteractionSource value);
};
