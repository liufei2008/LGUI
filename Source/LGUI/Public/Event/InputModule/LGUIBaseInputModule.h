// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LGUIBaseInputModule.generated.h"

/**
 * This is the place for handling inputs.
 * Only one InputModule is valid in the same time. so if multiple InputModule is activate, then the last one will be deactivate.
 */
UCLASS(Abstract)
class LGUI_API ULGUIBaseInputModule : public UActorComponent
{
	GENERATED_BODY()

public:
	ULGUIBaseInputModule();

	virtual void ProcessInput() PURE_VIRTUAL(, );

	virtual void Activate(bool bReset = false)override;
	virtual void Deactivate()override;
	UFUNCTION(BlueprintCallable, Category = LGUI)
	virtual void ClearEvent() PURE_VIRTUAL(, );
protected:
	void ActivateInputModule();
	void DeactivateInputModule();
};