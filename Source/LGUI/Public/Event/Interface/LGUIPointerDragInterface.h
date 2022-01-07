// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Event/LGUIPointerEventData.h"
#include "LGUIPointerDragInterface.generated.h"


UINTERFACE(Blueprintable, MinimalAPI)
class ULGUIPointerDragInterface : public UInterface
{
	GENERATED_BODY()
};
/**
 * Interface for handling LGUI drag beginDrag endDrag event
 */
class LGUI_API ILGUIPointerDragInterface
{
	GENERATED_BODY()
public:
	/**
	 * Called when drag this object begin.
	 * @return Allow event bubble up? If all interface of same actor's components return true, then the event can bubble up.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI)
		bool OnPointerBeginDrag(ULGUIPointerEventData* eventData);
	/**
	 * Called when dragging this object.
	 * @return Allow event bubble up? If all interface of same actor's components return true, then the event can bubble up.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI)
		bool OnPointerDrag(ULGUIPointerEventData* eventData);
	/**
	 * Called when drag this object end.
	 * @return Allow event bubble up? If all interface of same actor's components return true, then the event can bubble up.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI)
		bool OnPointerEndDrag(ULGUIPointerEventData* eventData);
};