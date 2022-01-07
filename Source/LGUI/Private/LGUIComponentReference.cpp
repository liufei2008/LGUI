// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "LGUIComponentReference.h"
#include "LGUI.h"

FLGUIComponentReference::FLGUIComponentReference(TSubclassOf<UActorComponent> InCompClass)
{
#if WITH_EDITOR
	HelperClass = InCompClass;
#endif
}
FLGUIComponentReference::FLGUIComponentReference(UActorComponent* InComp)
{
	TargetComp = InComp;
#if WITH_EDITOR
	HelperClass = InComp->StaticClass();
#endif
}

AActor* FLGUIComponentReference::GetActor()const
{
	if (TargetComp && !TargetComp->IsPendingKill())
	{
		return ((UActorComponent*)TargetComp)->GetOwner();
	}
	return nullptr;
}

bool FLGUIComponentReference::IsValid()const
{
	return TargetComp && !TargetComp->IsPendingKill();
}

