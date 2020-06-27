// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Event/LGUIPointerEventData.h"
#include "LGUIPointerClickInterface.generated.h"


UINTERFACE(Blueprintable, MinimalAPI)
class ULGUIPointerClickInterface : public UInterface
{
	GENERATED_BODY()
};
//Called when click
class LGUI_API ILGUIPointerClickInterface
{
	GENERATED_BODY()
public:
	//Called when a click event occurs
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI)
		bool OnPointerClick(ULGUIPointerEventData* eventData);
};