// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Event/InputModule/LGUI_PointerInputModule.h"
#include "Event/LGUIPointerEventData.h"
#include "LGUI_TouchInputModule.generated.h"

//for mobile multi touch input
UCLASS(ClassGroup = LGUI, meta = (BlueprintSpawnableComponent), Blueprintable, Experimental)
class LGUI_API ULGUI_TouchInputModule : public ULGUI_PointerInputModule
{
	GENERATED_BODY()

public:
	virtual void ProcessInput()override;
	//TriggerInput, need mannually setup
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void InputTouch(bool inTouchPress, int inTouchID, const FVector& inTouchPointPosition);
	//ScrollInput, need mannually setup
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void InputScroll(const float& inAxisValue);
};