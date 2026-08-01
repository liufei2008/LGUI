// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Event/LexPointerEventData.h"
#include "LexPointerDragInterface.generated.h"


UINTERFACE(Blueprintable, MinimalAPI)
class ULexPointerDragInterface : public UInterface
{
	GENERATED_BODY()
};
/**
 * Interface for handling LexUI beginDrag drag endDrag event
 */
class LGUI_API ILexPointerDragInterface
{
	GENERATED_BODY()
public:
	/**
	 * Called when drag this object begin.
	 * @return Allow event bubble up? If all interfaces of widget's components return true, then the event can bubble up.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI)
		bool OnPointerBeginDrag(ULexPointerEventData* EventData);
	/**
	 * Called when dragging this object.
	 * @return Allow event bubble up? If all interfaces of widget's components return true, then the event can bubble up.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI)
		bool OnPointerDrag(ULexPointerEventData* EventData);
	/**
	 * Called when drag this object end.
	 * @return Allow event bubble up? If all interfaces of widget's components return true, then the event can bubble up.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI)
		bool OnPointerEndDrag(ULexPointerEventData* EventData);
};