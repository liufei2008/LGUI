// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Interaction/UIToggleGroupComponent.h"
#include "LGUI.h"
#include "Interaction/UIToggleComponent.h"


void UUIToggleGroupComponent::SetSelection(UUIToggleComponent* Target)
{
	if (!IsValid(Target))
	{
		UE_LOG(LGUI, Error, TEXT("[UUIToggleGroupComponent::SelectItem]Toggle item is not valid!"));
		return;
	}
	if (LastSelect.Get() != Target)
	{
		auto TempSelected = LastSelect;
		LastSelect = Target;
		if (TempSelected.IsValid())
		{
			TempSelected->SetValue(false);
		}
		if (OnToggleCPP.IsBound())OnToggleCPP.Broadcast(Target);
		OnToggle.FireEvent(Target->GetOwner());
	}
}
void UUIToggleGroupComponent::ClearSelection()
{
	if (LastSelect.IsValid())
	{
		LastSelect->SetValue(false);
		LastSelect.Reset();

		if (OnToggleCPP.IsBound())OnToggleCPP.Broadcast(nullptr);
		OnToggle.FireEvent((AActor*)nullptr);
	}
}
UUIToggleComponent* UUIToggleGroupComponent::GetSelectedItem()const
{
	return LastSelect.Get();
}

FDelegateHandle UUIToggleGroupComponent::RegisterToggleEvent(const FLGUIToggleGroupDelegate& InDelegate)
{
	return OnToggleCPP.Add(InDelegate);
}
FDelegateHandle UUIToggleGroupComponent::RegisterToggleEvent(const TFunction<void(UUIToggleComponent*)>& InFunction)
{
	return OnToggleCPP.AddLambda(InFunction);
}
void UUIToggleGroupComponent::UnregisterToggleEvent(const FDelegateHandle& InHandle)
{
	OnToggleCPP.Remove(InHandle);
}

FLGUIDelegateHandleWrapper UUIToggleGroupComponent::RegisterToggleEvent(const FLGUIToggleGroupDynamicDelegate& InDelegate)
{
	auto delegateHandle = OnToggleCPP.AddLambda([InDelegate](UUIToggleComponent* Value) {
		if (InDelegate.IsBound())InDelegate.Execute(Value);
		});
	return FLGUIDelegateHandleWrapper(delegateHandle);
}
void UUIToggleGroupComponent::UnregisterToggleEvent(const FLGUIDelegateHandleWrapper& InDelegateHandle)
{
	OnToggleCPP.Remove(InDelegateHandle.DelegateHandle);
}