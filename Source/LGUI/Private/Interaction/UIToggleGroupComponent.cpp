// Copyright 2019 LexLiu. All Rights Reserved.

#include "Interaction/UIToggleGroupComponent.h"
#include "LGUI.h"
#include "Interaction/UIToggleComponent.h"

// Sets default values
UUIToggleGroupComponent::UUIToggleGroupComponent()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryComponentTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void UUIToggleGroupComponent::BeginPlay()
{
	Super::BeginPlay();
}

#if WITH_EDITOR
void UUIToggleGroupComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

void UUIToggleGroupComponent::SelectItem(UUIToggleComponent* item)
{
	if (LastSelect != item)
	{
		auto TempSelected = LastSelect;
		LastSelect = item;
		if (TempSelected != nullptr)
		{
			TempSelected->SetState(false);
		}
	}
}
void UUIToggleGroupComponent::SelectNone()
{
	if (LastSelect != nullptr)
		LastSelect->SetState(false);
}
void UUIToggleGroupComponent::SetSelection(UUIToggleComponent* Target)
{
	SelectItem(Target);
}
void UUIToggleGroupComponent::ClearSelection()
{
	SelectNone();
}
UUIToggleComponent* UUIToggleGroupComponent::GetSelectedItem()
{
	return LastSelect;
}