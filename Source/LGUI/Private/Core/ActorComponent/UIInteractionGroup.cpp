// Copyright 2019 LexLiu. All Rights Reserved.

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

void UUIInteractionGroup::SetInteractable(const bool& InBool)
{
	if (InBool != bInteractable)
	{
		bInteractable = InBool;
		if (CheckUIItem())
		{
			CacheUIItem->SetInteractionGroupStateChange(bInteractable, bIgnoreParentGroup);
		}

		if (!bInteractable)//if not interactable, need to check current ray hit object, if the object is interacting
		{
			if (ULGUIEventSystem::GetLGUIEventSystemInstance() != nullptr)
			{
				if (auto hitComp = ULGUIEventSystem::GetLGUIEventSystemInstance()->GetCurrentHitComponent())
				{
					if (hitComp->IsAttachedTo(GetOwner()->GetRootComponent()))//target is child of this
					{
						//walk up hierarchy and clear any interaction event
						auto LoopActor = hitComp->GetOwner();
						while (IsValid(LoopActor))
						{
							if (auto GroupComp = LoopActor->FindComponentByClass<UUIInteractionGroup>())
							{
								if (GroupComp == this)
								{
									ULGUIEventSystem::GetLGUIEventSystemInstance()->ClearEvent();
									break;
								}
								else
								{
									if (GroupComp->GetIgnoreParentGroup())
									{
										break;
									}
									else
									{
										LoopActor = LoopActor->GetAttachParentActor();
										continue;
									}
								}
							}
							else
							{
								LoopActor = LoopActor->GetAttachParentActor();
								continue;
							}
						}
					}
				}
			}
		}
	}
}
void UUIInteractionGroup::SetIgnoreParentGroup(const bool& InBool)
{
	if (InBool != bIgnoreParentGroup)
	{
		bIgnoreParentGroup = InBool;
		if (CheckUIItem())
		{
			CacheUIItem->SetInteractionGroupStateChange(bInteractable, bIgnoreParentGroup);
		}
	}
}