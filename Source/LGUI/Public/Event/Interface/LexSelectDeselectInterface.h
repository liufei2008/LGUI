// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Event/LexPointerEventData.h"
#include "LexSelectDeselectInterface.generated.h"


UINTERFACE(Blueprintable, MinimalAPI)
class ULexSelectDeselectInterface : public UInterface
{
	GENERATED_BODY()
};
/**
 * Interface for handling LexUI select/deselect event
 */
class LGUI_API ILexSelectDeselectInterface
{
	GENERATED_BODY()
public:
	/**
	 * Called when LexUI EventSystem select or focus on this object.
	 * @return Allow event bubble up? If all interfaces of widget's components return true, then the event can bubble up.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI)
		bool OnSelect(ULexBaseEventData* EventData);
	/**
	 * Called when LexUI EventSystem deselect or defocus this object.
	 * @return Allow event bubble up? If all interfaces of widget's components return true, then the event can bubble up.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI)
		bool OnDeselect(ULexBaseEventData* EventData);
};