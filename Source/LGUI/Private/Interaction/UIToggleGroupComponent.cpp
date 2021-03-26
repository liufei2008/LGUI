// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Interaction/UIToggleGroupComponent.h"
#include "LGUI.h"
#include "Interaction/UIToggleComponent.h"


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
UUIToggleComponent* UUIToggleGroupComponent::GetSelectedItem()const
{
	return LastSelect.Get();
}