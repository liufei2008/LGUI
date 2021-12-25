// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "LGUIComponentReference.h"
#include "LGUI.h"

#if WITH_EDITORONLY_DATA
TArray<FLGUIComponentReference*> FLGUIComponentReference::AllLGUIComponentReferenceArray;
#endif

FLGUIComponentReference::FLGUIComponentReference()
{
#if WITH_EDITOR
	AllLGUIComponentReferenceArray.Add(this);
#endif
}
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
FLGUIComponentReference::~FLGUIComponentReference()
{
#if WITH_EDITOR
	AllLGUIComponentReferenceArray.Remove(this);
#endif
}

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

#if WITH_EDITOR
void FLGUIComponentReference::RefreshOnBlueprintCompiled()
{
	if (TargetComp.IsStale())
	{
		TargetComp = nullptr;
		if (HelperActor.IsValid())
		{
			if (HelperClass != nullptr)
			{
				TArray<UActorComponent*> Components;
				HelperActor->GetComponents(HelperClass, Components);
				if (Components.Num() == 1)
				{
					TargetComp = Components[0];
				}
				else
				{
					if (HelperComponentName.IsValid())
					{
						for (auto& Comp : Components)
						{
							if (Comp->HasAnyFlags(EObjectFlags::RF_Transient))continue;
							if (HelperComponentName == Comp->GetFName())
							{
								TargetComp = Comp;
							}
						}
					}
					else
					{
						TargetComp = Components[0];
					}
				}
			}
		}
	}
}
void FLGUIComponentReference::RefreshAll_OnBlueprintCompiled()
{
	for (auto& Item : AllLGUIComponentReferenceArray)
	{
		Item->RefreshOnBlueprintCompiled();
	}
}
#endif
