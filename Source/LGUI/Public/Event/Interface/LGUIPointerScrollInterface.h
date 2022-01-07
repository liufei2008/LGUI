// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Event/LGUIPointerEventData.h"
#include "LGUIPointerScrollInterface.generated.h"


UINTERFACE(Blueprintable, MinimalAPI)
class ULGUIPointerScrollInterface : public UInterface
{
	GENERATED_BODY()
};
/**
 * Interface for handling LGUI pointer scroll event
 */
class LGUI_API ILGUIPointerScrollInterface
{
	GENERATED_BODY()
public:
	/**
	 * Called when pointer inside this object and scroll(mouse wheel).
	 * @return Allow event bubble up? If all interface of same actor's components return true, then the event can bubble up.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI)
		bool OnPointerScroll(ULGUIPointerEventData* eventData);
};