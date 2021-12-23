// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ILGUILayoutInterface.generated.h"


/**
 * Interface for handling LGUI's layout update.
 * Need to register UObject with RegisterLGUILayout, check UIBaseLayout for reference
 */
UINTERFACE(Blueprintable, MinimalAPI)
class ULGUILayoutInterface : public UInterface
{
	GENERATED_BODY()
};
/**
 * Interface for handling LGUI's culture changed.
 * Need to register UObject with RegisterLGUILayout, check UIBaseLayout for reference
 */ 
class LGUI_API ILGUILayoutInterface
{
	GENERATED_BODY()
public:
	/**
	 * Called when need to update layout.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI)
		void OnUpdateLayout();
};