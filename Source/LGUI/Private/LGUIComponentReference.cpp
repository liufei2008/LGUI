// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "LGUIComponentReference.h"
#include "LGUI.h"

UActorComponent* FLGUIComponentReference::GetComponentFromTargetActor(AActor* InActor, FName InCompName, UClass* InClass)
{
	if (InActor == nullptr || InActor->IsPendingKill())
	{
		UE_LOG(LGUI, Warning, TEXT("[FLGUIComponentReference::GetComponent]Target actor not valid!"));
		return nullptr;
	}
	if (InClass == nullptr || InClass->IsPendingKill())
	{
		UE_LOG(LGUI, Warning, TEXT("[FLGUIComponentReference::GetComponent]Component class not valid!"));
		return nullptr;
	}

	TArray<UActorComponent*> components;
	InActor->GetComponents(InClass, components);
	if (components.Num() == 0)
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
		return nullptr;
	}
	else if (components.Num() == 1)
	{
		return components[0];
	}
	else
	{
		if (InCompName.IsValid() && !InCompName.IsNone())
		{
			for (auto comp : components)
			{
				if (comp->GetFName() == InCompName)
				{
					return comp;
				}
			}
			//not found on name
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
			return nullptr;
		}
		else
		{
			FString actorName =
#if WITH_EDITOR
				InActor->GetActorLabel();
#else
				InActor->GetPathName();
#endif
			UE_LOG(LGUI, Warning, TEXT("[FLGUIComponentReference::GetComponent]Target actor:%s contains multiple component of type:%s, you should choose one of them."),
				*actorName,
				*InClass->GetName()
				);
			return nullptr;
		}
	}
}
UActorComponent* FLGUIComponentReference::GetComponent()const
{
	if (!componentInstance.IsValid())
	{
		componentInstance = GetComponentFromTargetActor(targetActor.Get(), targetComonentName, targetComponentClass);
	}
	return componentInstance.Get();
}
bool FLGUIComponentReference::IsValid()const
{
	GetComponent();
	return componentInstance.IsValid();
}