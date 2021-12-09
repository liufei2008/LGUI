// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "LGUIComponentReference.h"
#include "LGUI.h"

AActor* FLGUIComponentReference::GetActor()const
{
	if (TargetComp.IsValid())
	{
		return TargetComp->GetOwner();
	}
	return nullptr;
}

bool FLGUIComponentReference::IsValid()const
{
	return TargetComp.IsValid();
}
