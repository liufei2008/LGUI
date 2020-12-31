// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Event/LGUIPointerEventData.h"
#include "LGUIPointerDragEnterExitInterface.generated.h"


UINTERFACE(Blueprintable, MinimalAPI)
class ULGUIPointerDragEnterExitInterface : public UInterface
{
	GENERATED_BODY()
};
/**
 * Interface for handling LGUI drag->drop enter/exist event
 */
class LGUI_API ILGUIPointerDragEnterExitInterface
{
	GENERATED_BODY()
public:
	/**
	 * Called when dragging another object and pointer enter this object.
	 * @return Allow event bubble up? If all interface of same actor's components return true, then the event can bubble up.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI)
		bool OnPointerDragEnter(ULGUIPointerEventData* eventData);
	/**
	 * Called when dragging another object and pointer exit this object.
	 * @return Allow event bubble up? If all interface of same actor's components return true, then the event can bubble up.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI)
		bool OnPointerDragExit(ULGUIPointerEventData* eventData);
};