// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LexBaseInputModule.generated.h"

class ULexEventSystem;

/**
 * This is the place for handling inputs.
 * Call RegisterInputModuleToEventSystem to make this work. Only one InputModule is valid in the same time.
 */
UCLASS(Abstract)
class LGUI_API ULexBaseInputModule : public UActorComponent
{
	GENERATED_BODY()

public:
	ULexBaseInputModule();

	virtual void ProcessInput() PURE_VIRTUAL(, );
	virtual void ClearEvent() PURE_VIRTUAL(, );

	/**
	 * Register this InputModule to a EventSystem. Only one InputModule is valid in the same time.
	 */
	UFUNCTION(BlueprintCallable, Category = "LexUI")
	void RegisterInputModuleToEventSystem(ULexEventSystem* TargetEventSystem);
	UFUNCTION(BlueprintCallable, Category = "LexUI")
	void UnregisterInputModuleFromEventSystem();
protected:
	UPROPERTY(Transient)TWeakObjectPtr<ULexEventSystem> EventSystem = nullptr;
};