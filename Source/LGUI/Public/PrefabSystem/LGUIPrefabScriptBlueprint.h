// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Blueprint.h"
#include "LGUIPrefabScriptBlueprint.generated.h"

/** Blueprint for Prefab scripting */
UCLASS(NotBlueprintType)
class LGUI_API ULGUIPrefabScriptBlueprint : public UBlueprint
{
	GENERATED_UCLASS_BODY()
public:

#if WITH_EDITORONLY_DATA
	/** The friendly name to use for UI */
	UPROPERTY(transient)
		FString FriendlyName;
#endif

#if WITH_EDITOR
	//~ Begin UBlueprint Interface
	virtual FString GetFriendlyName() const override;
	//~ End UBlueprint Interface

#endif	//#if WITH_EDITOR
};
