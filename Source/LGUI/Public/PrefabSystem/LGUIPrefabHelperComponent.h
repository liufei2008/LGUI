// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "LGUI.h"
#include "LGUIPrefab.h"
#include "Components/SceneComponent.h"
#include "LGUIPrefabHelperComponent.generated.h"


class ALGUIPrefabHelperActor;

//helper component for PrefabSystem. for manage a prefab actor in level. only use this in editor
UCLASS(HideCategories = (Collision, LOD, Physics, Cooking, Rendering, Activation, Actor, Input), ClassGroup = (LGUI), NotBlueprintType, NotBlueprintable)
class LGUI_API ULGUIPrefabHelperComponent : public USceneComponent
{
	GENERATED_BODY()

public:	

	ULGUIPrefabHelperComponent();
	virtual void OnRegister()override;
	virtual void OnUnregister()override;
};