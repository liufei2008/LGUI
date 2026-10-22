// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ILexUICultureChangedInterface.generated.h"


/**
 * Interface for LGUI Widget to handle culture change event.
 * Need to register UObject with RegisterLGUICultureChangedEvent, check UIText for reference
 */
UINTERFACE(Blueprintable, MinimalAPI)
class ULexUICultureChangedInterface : public UInterface
{
	GENERATED_BODY()
};
/**
 * Interface for LGUI Widget to handle culture change event.
 * Need to register UObject with RegisterLGUICultureChangedEvent, check UIText for reference
 */
class LGUI_API ILexUICultureChangedInterface
{
	GENERATED_BODY()
public:
	/**
	 * Called when current culture changed and LGUI need to update culture.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "LexUI")
		void OnCultureChanged();
};