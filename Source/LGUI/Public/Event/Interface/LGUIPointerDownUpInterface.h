// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Event/LGUIPointerEventData.h"
#include "LGUIPointerDownUpInterface.generated.h"


UINTERFACE(Blueprintable, MinimalAPI)
class ULGUIPointerDownUpInterface : public UInterface
{
	GENERATED_BODY()
};
//Called when press or release
class LGUI_API ILGUIPointerDownUpInterface
{
	GENERATED_BODY()
public:
	//Called when press
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI)
		bool OnPointerDown(ULGUIPointerEventData* eventData);
	//Called when release
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI)
		bool OnPointerUp(ULGUIPointerEventData* eventData);
};