// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Event/LGUIPointerEventData.h"
#include "LGUIPointerDownUpInterface.generated.h"


UINTERFACE(Blueprintable, MinimalAPI)
class ULGUIPointerDownUpInterface : public UInterface
{
	GENERATED_BODY()
};
/**
 * Interface for handling LGUI trigger press or release event
 */
class LGUI_API ILGUIPointerDownUpInterface
{
	GENERATED_BODY()
public:
	/**
	 * Called when a pointer press event occurs.
	 * @return Allow event bubble up? If all interface of same actor's components return true, then the event can bubble up.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI)
		bool OnPointerDown(ULGUIPointerEventData* eventData);
	/**
	 * Called when a pointer release event occurs.
	 * @return Allow event bubble up? If all interface of same actor's components return true, then the event can bubble up.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI)
		bool OnPointerUp(ULGUIPointerEventData* eventData);
};