// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Event/InputModule/LGUI_PointerInputModule.h"
#include "Event/LGUIPointerEventData.h"
#include "LGUI_StandaloneInputModule.generated.h"

//Common standalone platform input, or mouse input
UCLASS(ClassGroup = LGUI, meta = (BlueprintSpawnableComponent), Blueprintable)
class LGUI_API ULGUI_StandaloneInputModule : public ULGUI_PointerInputModule
{
	GENERATED_BODY()

public:
	virtual void ProcessInput()override;
	//input for mouse press and release
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (AdvancedDisplay = "inMouseButtonType"))
		void InputTrigger(bool inTriggerPress, EMouseButtonType inMouseButtonType = EMouseButtonType::Left);
	//input for scroll
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void InputScroll(const float& inAxisValue);
};