// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Event/LexPointerEventData.h"
#include "LexPointerScrollInterface.generated.h"


UINTERFACE(Blueprintable, MinimalAPI)
class ULexPointerScrollInterface : public UInterface
{
	GENERATED_BODY()
};
/**
 * Interface for handling LexUI pointer scroll event
 */
class LGUI_API ILexPointerScrollInterface
{
	GENERATED_BODY()
public:
	/**
	 * Called when pointer inside this object and scroll(mouse wheel).
	 * @return Allow event bubble up? If all interfaces of widget's components return true, then the event can bubble up.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI)
		bool OnPointerScroll(ULexPointerEventData* EventData);
};