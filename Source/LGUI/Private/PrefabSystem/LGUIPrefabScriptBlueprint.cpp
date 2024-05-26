// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "PrefabSystem/LGUIPrefabScriptBlueprint.h"

ULGUIPrefabScriptBlueprint::ULGUIPrefabScriptBlueprint(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

#if WITH_EDITOR

FString ULGUIPrefabScriptBlueprint::GetFriendlyName() const
{
#if WITH_EDITORONLY_DATA
	return FriendlyName;
#endif
	return UBlueprint::GetFriendlyName();
}

#endif	//#if WITH_EDITOR

