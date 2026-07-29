// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Event/LexPointerEventData.h"
#include "LexNavigationInterface.generated.h"


UINTERFACE(Blueprintable, MinimalAPI)
class ULexNavigationInterface : public UInterface
{
	GENERATED_BODY()
};
/**
 * Interface for handling keyboard/gamepad navigation event. Only allowed on ActorComponent
 */ 
class LGUI_API ILexNavigationInterface
{
	GENERATED_BODY()
public:
	/**
	 * @return true if we can navigate from other to this, false otherwise
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI)
	bool CanNavigateHere()const;
	/**
	 * Called when a navigation event occurs.
	 * @param Direction navigation direction
	 * @param Result navigate next object
	 * @return true if this action can navigation to next, false if no need to navigate to next
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI)
	bool OnNavigate(ELexUINavigationDirection Direction, TScriptInterface<ILexNavigationInterface>& Result);
};