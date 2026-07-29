// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Event/LexPointerEventData.h"
#include "LexPointerDownUpInterface.generated.h"


UINTERFACE(Blueprintable, MinimalAPI)
class ULexPointerDownUpInterface : public UInterface
{
	GENERATED_BODY()
};
/**
 * Interface for handling LexUI trigger press or release event
 */
class LGUI_API ILexPointerDownUpInterface
{
	GENERATED_BODY()
public:
	/**
	 * Called when a pointer press event occurs.
	 * @return Allow event bubble up? If all interface of same widget's components return true, then the event can bubble up.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI)
		bool OnPointerDown(ULexPointerEventData* EventData);
	/**
	 * Called when a pointer release event occurs.
	 * @return Allow event bubble up? If all interface of same widget's components return true, then the event can bubble up.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI)
		bool OnPointerUp(ULexPointerEventData* EventData);
};