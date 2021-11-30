// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "PrefabSystem/LGUIPrefabHelperComponent.h"
#include "LGUI.h"
#include "Utils/LGUIUtils.h"
#include "PrefabSystem/LGUIPrefabHelperActor.h"
#include "Core/ActorComponent/UIItem.h"
#if WITH_EDITOR
#include "Editor.h"
#include "EditorActorFolders.h"
#include "Core/Actor/LGUIManagerActor.h"
#endif


ULGUIPrefabHelperComponent::ULGUIPrefabHelperComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	bIsEditorOnly = true;
}


void ULGUIPrefabHelperComponent::OnRegister()
{
	Super::OnRegister();
}
void ULGUIPrefabHelperComponent::OnUnregister()
{
	Super::OnUnregister();
}
