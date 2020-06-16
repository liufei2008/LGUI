// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LGUIPointerEventData.h"
#include "LGUIPointerEnterExitInterface.generated.h"


UINTERFACE(Blueprintable, MinimalAPI)
class ULGUIPointerEnterExitInterface : public UInterface
{
	GENERATED_BODY()
};
//Called when pointer enter or exit a object
class LGUI_API ILGUIPointerEnterExitInterface
{
	GENERATED_BODY()
public:
	//Called when pointer enter a object
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI)
		bool OnPointerEnter(ULGUIPointerEventData* eventData);
	//Called when pointer exit a object
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI)
		bool OnPointerExit(ULGUIPointerEventData* eventData);
};