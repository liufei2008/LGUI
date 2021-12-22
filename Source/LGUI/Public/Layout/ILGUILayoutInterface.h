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

#if WITH_EDITOR
	virtual bool CanControlChildAnchor() { return false; };
	virtual bool CanControlChildAnchorOffsetX() { return false; }
	virtual bool CanControlChildAnchorOffsetY() { return false; }
	virtual bool CanControlChildWidth() { return false; }
	virtual bool CanControlChildHeight() { return false; }
	virtual bool CanControlSelfHorizontalAnchor() { return false; }
	virtual bool CanControlSelfVerticalAnchor() { return false; }
	virtual bool CanControlSelfAnchorOffsetX() { return false; }
	virtual bool CanControlSelfAnchorOffsetY() { return false; }
	virtual bool CanControlSelfWidth() { return false; }
	virtual bool CanControlSelfHeight() { return false; }
	virtual bool CanControlSelfStrengthLeft() { return false; }
	virtual bool CanControlSelfStrengthRight() { return false; }
	virtual bool CanControlSelfStrengthTop() { return false; }
	virtual bool CanControlSelfStrengthBottom() { return false; }
#endif
};