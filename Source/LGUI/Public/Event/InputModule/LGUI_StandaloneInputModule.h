// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Event/InputModule/LGUI_PointerInputModule.h"
#include "Event/LGUIPointerEventData.h"
#include "LGUI_StandaloneInputModule.generated.h"

//only one InputModule is valid in the same time. so if multiple InputModule is activated, then the latest one is valid.
UCLASS(ClassGroup = LGUI, meta = (BlueprintSpawnableComponent), Blueprintable)
class LGUI_API ULGUI_StandaloneInputModule : public ULGUI_PointerInputModule
{
	GENERATED_BODY()

public:
	virtual void ProcessInput()override;
	//TriggerInput, need mannually setup
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (AdvancedDisplay = "inMouseButtonType"))
		void InputTrigger(bool inTriggerPress, EMouseButtonType inMouseButtonType = EMouseButtonType::Left);
	//ScrollInput, need mannually setup
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void InputScroll(const float& inAxisValue);
};