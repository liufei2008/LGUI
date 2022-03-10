﻿// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "LGUIComponentReference.h"
#include "LGUI.h"

FLGUIComponentReference::FLGUIComponentReference(TSubclassOf<UActorComponent> InCompClass)
{
#if WITH_EDITOR
	HelperClass = InCompClass;
	AllLGUIComponentReferenceArray.Add(this);
#endif
}
FLGUIComponentReference::FLGUIComponentReference(UActorComponent* InComp)
{
	TargetComp = InComp;
#if WITH_EDITOR
	HelperClass = InComp->StaticClass();
	AllLGUIComponentReferenceArray.Add(this);
#endif
}
FLGUIComponentReference::FLGUIComponentReference()
{
#if WITH_EDITOR
	AllLGUIComponentReferenceArray.Add(this);
#endif
}
FLGUIComponentReference::~FLGUIComponentReference()
{
#if WITH_EDITOR
	AllLGUIComponentReferenceArray.Remove(this);
#endif
}

#if WITH_EDITOR
TArray<FLGUIComponentReference*> FLGUIComponentReference::AllLGUIComponentReferenceArray;
void FLGUIComponentReference::RefreshAllOnBlueprintRecompile()
{
	for (auto Item : AllLGUIComponentReferenceArray)
	{
		Item->RefreshOnBlueprintRecompile();
	}
}
void FLGUIComponentReference::RefreshOnBlueprintRecompile()
{
	if (!IsValid(TargetComp))
	{
		if (IsValid(HelperActor) && IsValid(HelperClass))
		{
			TArray<UActorComponent*> Components;
			HelperActor->GetComponents(HelperClass, Components);
			if (Components.Num() == 1)
			{
				TargetComp = Components[0];
			}
			else if (Components.Num() > 1)
			{
				for (auto Comp : Components)
				{
					if (Comp->GetFName() == HelperComponentName)
					{
						TargetComp = Comp;
					}
				}
			}
		}
	}
}
#endif

AActor* FLGUIComponentReference::GetActor()const
{
	if (TargetComp && !TargetComp->IsPendingKill())
	{
		return ((UActorComponent*)TargetComp)->GetOwner();
	}
	return nullptr;
}

bool FLGUIComponentReference::IsValidComponentReference()const
{
	return IsValid(TargetComp);
}

