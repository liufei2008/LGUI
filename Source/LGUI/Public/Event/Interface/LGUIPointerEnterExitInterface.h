// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Event/LGUIPointerEventData.h"
#include "LGUIPointerEnterExitInterface.generated.h"


UINTERFACE(Blueprintable, MinimalAPI)
class ULGUIPointerEnterExitInterface : public UInterface
{
	GENERATED_BODY()
};
/**
 * Interface for handling LGUI pointer enter/exist event
 */
class LGUI_API ILGUIPointerEnterExitInterface
{
	GENERATED_BODY()
public:
	/**
	 * Called when pointer enter this object.
	 * @return Allow event bubble up? If all interface of same actor's components return true, then the event can bubble up.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI)
		bool OnPointerEnter(ULGUIPointerEventData* eventData);
	/**
	 * Called when pointer exit this object.
	 * @return Allow event bubble up? If all interface of same actor's components return true, then the event can bubble up.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI)
		bool OnPointerExit(ULGUIPointerEventData* eventData);
};