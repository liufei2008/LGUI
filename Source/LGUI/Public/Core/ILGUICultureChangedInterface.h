// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ILGUICultureChangedInterface.generated.h"


/**
 * Interface for handling LGUI's culture changed.
 * Need to register UObject with RegisterLGUICultureChangedEvent, check UIText for reference
 */
UINTERFACE(Blueprintable, MinimalAPI)
class ULGUICultureChangedInterface : public UInterface
{
	GENERATED_BODY()
};
/**
 * Interface for handling LGUI's culture changed.
 * Need to register UObject with RegisterLGUICultureChangedEvent, check UIText for reference
 */ 
class LGUI_API ILGUICultureChangedInterface
{
	GENERATED_BODY()
public:
	/**
	 * Called when current culture changed and LGUI need to update culture.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI)
		void OnCultureChanged();
};