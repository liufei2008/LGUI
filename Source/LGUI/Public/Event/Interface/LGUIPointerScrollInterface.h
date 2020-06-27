// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Event/LGUIPointerEventData.h"
#include "LGUIPointerScrollInterface.generated.h"


UINTERFACE(Blueprintable, MinimalAPI)
class ULGUIPointerScrollInterface : public UInterface
{
	GENERATED_BODY()
};
//Called when mouse scroll wheel 
class LGUI_API ILGUIPointerScrollInterface
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI)
		bool OnPointerScroll(ULGUIPointerEventData* eventData);
};