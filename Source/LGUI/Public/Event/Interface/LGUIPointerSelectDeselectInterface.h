// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Event/LGUIPointerEventData.h"
#include "LGUIPointerSelectDeselectInterface.generated.h"


UINTERFACE(Blueprintable, MinimalAPI)
class ULGUIPointerSelectDeselectInterface : public UInterface
{
	GENERATED_BODY()
};
/**
 * Interface for handling LGUI select/deselect event
 */
class LGUI_API ILGUIPointerSelectDeselectInterface
{
	GENERATED_BODY()
public:
	/**
	 * Called when LGUI EventSystem select this object.
	 * @return Allow event bubble up? If all interface of same actor's components return true, then the event can bubble up.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI)
		bool OnPointerSelect(ULGUIBaseEventData* eventData);
	/**
	 * Called when LGUI EventSystem deselect this object.
	 * @return Allow event bubble up? If all interface of same actor's components return true, then the event can bubble up.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI)
		bool OnPointerDeselect(ULGUIBaseEventData* eventData);
};