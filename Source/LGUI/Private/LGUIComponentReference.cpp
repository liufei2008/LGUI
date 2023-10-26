// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LGUIComponentReference.h"
#include "LGUI.h"

FLGUIComponentReference::FLGUIComponentReference(TSubclassOf<UActorComponent> InCompClass)
{
	HelperClass = InCompClass;
}
FLGUIComponentReference::FLGUIComponentReference(UActorComponent* InComp)
{
	TargetComp = InComp;
	HelperClass = InComp->GetClass();
	HelperActor = TargetComp->GetOwner();
	HelperComponentName = TargetComp->GetFName();
}
FLGUIComponentReference::FLGUIComponentReference()
{
	
}

bool FLGUIComponentReference::CheckTargetObject()const
{
	if (IsValid(TargetComp))
	{
		return true;
	}
	else
	{
		if (IsValid(HelperActor))
		{
			if (IsValid(HelperClass))
			{
				TArray<UActorComponent*> Components;
				HelperActor->GetComponents(HelperClass, Components);
				if (Components.Num() == 1)
				{
					TargetComp = Components[0];
				}
				else if (Components.Num() > 1)
				{
					if (!HelperComponentName.IsNone())
					{
						for (auto& Comp : Components)
						{
							if (Comp->GetFName() == HelperComponentName)
							{
								TargetComp = Comp;
								return true;
							}
						}
						FString ActorName =
#if WITH_EDITOR
							HelperActor->GetActorLabel();
#else
							HelperActor->GetPathName();
#endif
						UE_LOG(LGUI, Error, TEXT("[%s].%d Can't find component of name '%s' on actor '%s'"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *HelperComponentName.ToString(), *ActorName);
					}
				}
			}
		}

		return IsValid(TargetComp);
	}
}
AActor* FLGUIComponentReference::GetActor()const
{
	return HelperActor;
}

bool FLGUIComponentReference::IsValidComponentReference()const
{
	return CheckTargetObject();
}

