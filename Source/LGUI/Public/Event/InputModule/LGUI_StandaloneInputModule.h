// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Event/InputModule/LGUI_PointerInputModule.h"
#include "Event/LGUIPointerEventData.h"
#include "LGUI_StandaloneInputModule.generated.h"

/**
 * Common standalone platform input, or mouse input
 */
UCLASS(ClassGroup = LGUI, meta = (BlueprintSpawnableComponent), Blueprintable)
class LGUI_API ULGUI_StandaloneInputModule : public ULGUI_PointerInputModule
{
	GENERATED_BODY()

public:
	virtual void ProcessInput()override;
	/** input for mouse press and release */
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (AdvancedDisplay = "inMouseButtonType"))
		void InputTrigger(bool inTriggerPress, EMouseButtonType inMouseButtonType = EMouseButtonType::Left);
	/**
	 * input for scroll
	 * @param	inAxisValue		Use a 2d vector for scroll value. For mouse scroll just fill X&Y with mouse scroll value; For touchpad input use X for horizontal and Y for vertical.
	 */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void InputScroll(const FVector2D& inAxisValue);
	/**
	 * see "bOverrideMousePosition" property
	 */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void InputOverrideMousePosition(const FVector2D& inMousePosition);
	/**
	 * should we use custom mouse position?
	 * if set to true, then you should call InputOverrideMousePosition to set mouse position;
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LGUI)
		bool bOverrideMousePosition = false;
protected:
	FVector2D overrideMousePosition;
	struct StandaloneInputData
	{
		bool triggerPress;
		float pressTime;
		float releaseTime;
		EMouseButtonType mouseButtonType;
		FVector2D mousePosition;
	};
	TArray<StandaloneInputData> standaloneInputDataArray;//collect input data into array in input event, and process these input data in ProcessInput. This can solve the condition: multiple mouse button input in one frame
	FORCEINLINE bool GetMousePosition(FVector2D& OutMousePos);
};