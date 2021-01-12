// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UIInteractionGroup.h"
#include "LGUI.h"
#include "Event/LGUIEventSystem.h"

UUIInteractionGroup::UUIInteractionGroup()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUIInteractionGroup::BeginPlay()
{
	Super::BeginPlay();
	if (CheckUIItem())
	{
		CacheUIItem->SetInteractionGroupStateChange(bInteractable, bIgnoreParentGroup);
	}
}

void UUIInteractionGroup::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (EndPlayReason == EEndPlayReason::RemovedFromWorld || EndPlayReason == EEndPlayReason::Destroyed)
	{
		if (CheckUIItem())
		{
			CacheUIItem->SetInteractionGroupStateChange(true, false);//reset parameters after remove this component
		}
	}
}
#if WITH_EDITOR
void UUIInteractionGroup::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.Property)
	{
		auto propName = Property->GetFName();
		if (propName == TEXT("bInteractable") || propName == TEXT("bIgnoreParentGroup"))
		{
			CacheUIItem = nullptr;
			if (CheckUIItem())
			{
				CacheUIItem->SetInteractionGroupStateChange(bInteractable, bIgnoreParentGroup);
				CacheUIItem->EditorForceUpdateImmediately();
			}
		}
	}
}
#endif

bool UUIInteractionGroup::CheckUIItem()
{
	if (IsValid(CacheUIItem))
		return true;
	else
		CacheUIItem = Cast<UUIItem>(GetOwner()->GetRootComponent());
	if (IsValid(CacheUIItem))
		return true;
	return false;
}

void UUIInteractionGroup::SetInteractable(bool value)
{
	if (value != bInteractable)
	{
		bInteractable = value;
		if (CheckUIItem())
		{
			CacheUIItem->SetInteractionGroupStateChange(bInteractable, bIgnoreParentGroup);
		}
	}
}
void UUIInteractionGroup::SetIgnoreParentGroup(bool value)
{
	if (value != bIgnoreParentGroup)
	{
		bIgnoreParentGroup = value;
		if (CheckUIItem())
		{
			CacheUIItem->SetInteractionGroupStateChange(bInteractable, bIgnoreParentGroup);
		}
	}
}