// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Event/LexPointerEventData.h"
#include "LexPointerSelectDeselectInterface.generated.h"


UINTERFACE(Blueprintable, MinimalAPI)
class ULexPointerSelectDeselectInterface : public UInterface
{
	GENERATED_BODY()
};
/**
 * Interface for handling LexUI select/deselect event
 */
class LGUI_API ILexPointerSelectDeselectInterface
{
	GENERATED_BODY()
public:
	/**
	 * Called when LexUI EventSystem select this object.
	 * @return Allow event bubble up? If all interface of same widget's components return true, then the event can bubble up.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI)
		bool OnPointerSelect(ULexBaseEventData* EventData);
	/**
	 * Called when LexUI EventSystem deselect this object.
	 * @return Allow event bubble up? If all interface of same widget's components return true, then the event can bubble up.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI)
		bool OnPointerDeselect(ULexBaseEventData* EventData);
};