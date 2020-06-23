// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LGUIBaseInputModule.generated.h"

//only one InputModule is valid in the same time. so if multiple InputModule is activated, then the latest one is valid.
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