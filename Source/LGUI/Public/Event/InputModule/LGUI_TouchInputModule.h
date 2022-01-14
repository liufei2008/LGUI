// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Event/InputModule/LGUI_PointerInputModule.h"
#include "LGUI_TouchInputModule.generated.h"

/** 
 * for mobile multi touch input
 */
UCLASS(ClassGroup = LGUI, meta = (BlueprintSpawnableComponent), Blueprintable)
class LGUI_API ULGUI_TouchInputModule : public ULGUI_PointerInputModule
{
	GENERATED_BODY()

public:
	virtual void ProcessInput()override;
	/** input for touch press and release */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void InputTouchTrigger(bool inTouchPress, int inTouchID, const FVector& inTouchPointPosition);
	/** input for touch point moved */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void InputTouchMoved(int inTouchID, const FVector& inTouchPointPosition);
	/** input for scroll */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void InputScroll(const FVector2D& inAxisValue);
};