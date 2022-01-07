// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Event/LGUIPointerEventData.h"
#include "LGUIPointerClickInterface.generated.h"


UINTERFACE(Blueprintable, MinimalAPI)
class ULGUIPointerClickInterface : public UInterface
{
	GENERATED_BODY()
};
/**
 * Interface for handling LGUI click event
 */ 
class LGUI_API ILGUIPointerClickInterface
{
	GENERATED_BODY()
public:
	/**
	 * Called when a click event occurs.
	 * @return Allow event bubble up? If all interface of same actor's components return true, then the event can bubble up.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI)
		bool OnPointerClick(ULGUIPointerEventData* eventData);
};