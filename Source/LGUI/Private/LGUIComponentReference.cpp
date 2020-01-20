// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "LGUIComponentReference.h"
#include "LGUI.h"

UActorComponent* FLGUIComponentReference::GetComponentFromTargetActor(AActor* InActor, FName InCompName, UClass* InClass)
{
	UActorComponent* resultComp = nullptr;
	if (resultComp == nullptr)
	{
		if (InActor == nullptr)
			return nullptr;
		auto& components = InActor->GetComponents();
		if (InCompName.IsNone())
		{
			resultComp = InActor->FindComponentByClass(InClass);
		}
		else
		{
			TInlineComponentArray<UActorComponent*> tempCompArray;
			if (InClass != nullptr)
			{
				for (UActorComponent* comp : components)
				{
					if (comp->IsA(InClass))
					{
						tempCompArray.Add(comp);
					}
				}
			}
			if (tempCompArray.Num() == 0)
			{
				FString actorName =
#if WITH_EDITOR
					InActor->GetActorLabel();
#else
					InActor->GetPathName();
#endif
				UE_LOG(LGUI, Error, TEXT("[FLGUIComponentReference::GetComponent]Target actor:%s does not contain a Component of type:%s!"),
					*actorName,
					*InClass->GetName()
				);
				resultComp = nullptr;
			}
			else
			{
				for (UActorComponent* comp : tempCompArray)
				{
					if (comp->GetFName() == InCompName)
					{
						resultComp = comp;
						break;
					}
				}
				if (!IsValid(resultComp))
				{
					FString actorName =
#if WITH_EDITOR
						InActor->GetActorLabel();
#else
						InActor->GetPathName();
#endif
					UE_LOG(LGUI, Warning, TEXT("[FLGUIComponentReference::GetComponent]Target actor:%s does not contain a Component of name:%s, use default one"),
						*actorName,
						*InCompName.ToString()
					);
					resultComp = tempCompArray[0];
				}
			}
		}
		if (!IsValid(resultComp))
		{
			FString actorName =
#if WITH_EDITOR
				InActor->GetActorLabel();
#else
				InActor->GetPathName();
#endif
			UE_LOG(LGUI, Warning, TEXT("[FLGUIComponentReference::GetComponent]Missing component of type:%s name:%s on Target actor:%s!"),
				*InClass->GetName(),
				*InCompName.ToString(),
				*actorName
			);
		}
	}
	return resultComp;
}
UActorComponent* FLGUIComponentReference::GetComponent()const
{
	componentInstance = GetComponentFromTargetActor(targetActor, targetComonentName, targetComponentClass);
	return componentInstance;
}