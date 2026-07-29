// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Event/LexPointerEventData.h"
#include "LexPointerEnterExitInterface.generated.h"


UINTERFACE(Blueprintable, MinimalAPI)
class ULexPointerEnterExitInterface : public UInterface
{
	GENERATED_BODY()
};
/**
 * Interface for handling LexUI pointer enter/exist event
 */
class LGUI_API ILexPointerEnterExitInterface
{
	GENERATED_BODY()
public:
	/**
	 * Called when pointer enter this object.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI)
		void OnPointerEnter(ULexPointerEventData* EventData);
	/**
	 * Called when pointer exit this object.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI)
		void OnPointerExit(ULexPointerEventData* EventData);
};