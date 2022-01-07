// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Event/LGUIPointerEventData.h"
#include "LGUIPointerDragDropInterface.generated.h"


UINTERFACE(Blueprintable, MinimalAPI)
class ULGUIPointerDragDropInterface : public UInterface
{
	GENERATED_BODY()
};
/**
 * Interface for handling LGUI drag->drop event
 */
class LGUI_API ILGUIPointerDragDropInterface
{
	GENERATED_BODY()
public:
	/**
	 * Called when dragging another object and drop on this object.
	 * @return Allow event bubble up? If all interface of same actor's components return true, then the event can bubble up.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI)
		bool OnPointerDragDrop(ULGUIPointerEventData* eventData);
};