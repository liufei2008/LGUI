// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Event/LGUIPointerEventData.h"
#include "LGUIPointerDragEnterExitInterface.generated.h"

/**
 * This interface is deprecated, instead use isDragging and enterComponent from eventData.
 */
UINTERFACE(Blueprintable, MinimalAPI)
class ULGUIPointerDragEnterExitInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * This interface is deprecated, instead use isDragging and enterComponent from eventData.
 * Interface for handling LGUI drag->drop enter/exist event
 */
class LGUI_API ILGUIPointerDragEnterExitInterface
{
	GENERATED_BODY()
public:
	/**
	 * This interface is deprecated, instead use isDragging and enterComponent from eventData.
	 * Called when dragging another object and pointer enter this object.
	 * @return Allow event bubble up? If all interface of same actor's components return true, then the event can bubble up.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI, meta = (DeprecatedFunction, DeprecationMessage = "This interface is deprecated, instead use isDragging and enterComponent from eventData."))
		bool OnPointerDragEnter(ULGUIPointerEventData* eventData);
	/**
	 * This interface is deprecated, instead use isDragging and enterComponent from eventData.
	 * Called when dragging another object and pointer exit this object.
	 * @return Allow event bubble up? If all interface of same actor's components return true, then the event can bubble up.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI, meta = (DeprecatedFunction, DeprecationMessage = "This interface is deprecated, instead use isDragging and enterComponent from eventData."))
		bool OnPointerDragExit(ULGUIPointerEventData* eventData);
};