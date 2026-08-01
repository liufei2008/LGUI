// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Event/LexPointerEventData.h"
#include "LexPointerClickInterface.generated.h"


UINTERFACE(Blueprintable, MinimalAPI)
class ULexPointerClickInterface : public UInterface
{
	GENERATED_BODY()
};
/**
 * Interface for handling LexUI click event
 */ 
class LGUI_API ILexPointerClickInterface
{
	GENERATED_BODY()
public:
	/**
	 * Called when a click event occurs.
	 * @return Allow event bubble up? If all interfaces of widget's components return true, then the event can bubble up.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI)
		bool OnPointerClick(ULexPointerEventData* EventData);
};