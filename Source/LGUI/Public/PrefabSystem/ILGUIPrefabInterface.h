// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ILGUIPrefabInterface.generated.h"


/**
 * Interface for Actor or ActorComponent that loaded from LGUIPrefab
 */
UINTERFACE(Blueprintable, MinimalAPI)
class ULGUIPrefabInterface : public UInterface
{
	GENERATED_BODY()
};
/**
 * Interface for Actor or ActorComponent that loaded from LGUIPrefab
 */ 
class LGUI_API ILGUIPrefabInterface
{
	GENERATED_BODY()
public:
	/**
	 * Called when LGUIPrefab finish load. This is called late than BeginPlay.
	 *		Awake execute order in prefab: higher in hierarchy will execute earlier, so scripts on root actor will execute the first. Actor execute first, then execute on component.
	 *		And this Awake is execute later than LGUILifeCycleBehaviour's Awake when in same prefab.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI)
		void Awake();
	/**
	 * Same as Awake but only execute in edit mode.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI, CallInEditor)
		void EditorAwake();
};