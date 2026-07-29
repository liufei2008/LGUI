// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Event/LexPointerEventData.h"
#include "LexPointerDragDropInterface.generated.h"


UINTERFACE(Blueprintable, MinimalAPI)
class ULexPointerDragDropInterface : public UInterface
{
	GENERATED_BODY()
};
/**
 * Interface for handling LexUI drag->drop event
 */
class LGUI_API ILexPointerDragDropInterface
{
	GENERATED_BODY()
public:
	/**
	 * Called when dragging another object and drop on this object.
	 * @return Allow event bubble up? If all interface of same widget's components return true, then the event can bubble up.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI)
		bool OnPointerDragDrop(ULexPointerEventData* EventData);
};