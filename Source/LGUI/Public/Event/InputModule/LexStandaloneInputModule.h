// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Event/InputModule/LexPointerInputModule.h"
#include "Event/LexPointerEventData.h"
#include "LexStandaloneInputModule.generated.h"

/**
 * Common standalone platform input, or mouse input
 */
UCLASS(ClassGroup = LGUI, meta = (BlueprintSpawnableComponent), Blueprintable)
class LGUI_API ULexStandaloneInputModule : public ULexPointerInputModule
{
	GENERATED_BODY()

public:
	virtual void ProcessInput()override;
	/** input for mouse press and release */
	UFUNCTION(BlueprintCallable, Category = "LexUI", meta = (AdvancedDisplay = "inMouseButtonType"))
		void InputTrigger(const FVector& InMousePosition, bool InTriggerPress, ELexUIMouseButtonType InMouseButtonType = ELexUIMouseButtonType::Left);
	/**
	 * input for scroll
	 * @param	InAxisValue		Use a 2d vector for scroll value. For mouse scroll just fill X&Y with mouse scroll value; For touchpad input use X for horizontal and Y for vertical.
	 */
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		void InputScroll(const FVector2D& InAxisValue);
	/**
	 * see "bOverrideMousePosition" property
	 */
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		void InputMouseMove(const FVector& InMousePosition);
	/** input for touch press and release */
	UFUNCTION(BlueprintCallable, Category = "LexUI")
	void InputTouchTrigger(bool InTouchPress, int InTouchID, const FVector& InTouchPointPosition);
	/** input for touch point moved */
	UFUNCTION(BlueprintCallable, Category = "LexUI")
	void InputTouchMoved(int InTouchID, const FVector& InTouchPointPosition);
	/** get current mouse position, return (0,0) if mouse position is not valid */
	UFUNCTION(BlueprintCallable, Category = "LexUI")
	void GetMousePosition(FVector2D& OutMousePos)const;

	/** input for gamepad or keyboard navigation */
	UFUNCTION(BlueprintCallable, Category = "LexUI")
	void InputNavigation(ELexUINavigationDirection InDirection, bool InPressOrRelease, int InPointerID);
	/** input for gamepad or keyboard press and release */
	UFUNCTION(BlueprintCallable, Category = "LexUI")
	void InputTriggerForNavigation(bool InTriggerPress, int InPointerID);
protected:
	void CommonInputTrigger(const FVector& InPointerPosition, bool InTriggerPress, int InPointerID, ELexUIMouseButtonType InMouseButtonType = ELexUIMouseButtonType::Left);
	struct StandaloneInputData
	{
		bool bTriggerPress;
		float PressTime;
		float ReleaseTime;
		int PointerID;
		ELexUIMouseButtonType MouseButtonType;
		FVector PointerPosition;
	};
	TArray<StandaloneInputData> StandaloneInputDataArray;//collect input data into array in input event, and process these input data in ProcessInput. This can solve the condition: multiple mouse button input in one frame
};