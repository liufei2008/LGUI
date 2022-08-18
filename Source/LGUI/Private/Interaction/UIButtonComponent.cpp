// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Interaction/UIButtonComponent.h"
#include "LGUI.h"

bool UUIButtonComponent::OnPointerClick_Implementation(ULGUIPointerEventData* eventData)
{
	OnClickCPP.Broadcast();
	OnClick.FireEvent();
	return AllowEventBubbleUp;
}
FDelegateHandle UUIButtonComponent::RegisterClickEvent(const FSimpleDelegate& InDelegate)
{
	return OnClickCPP.Add(InDelegate);
}
FDelegateHandle UUIButtonComponent::RegisterClickEvent(const TFunction<void()>& InFunction)
{
	return OnClickCPP.AddLambda(InFunction);
}
void UUIButtonComponent::UnregisterClickEvent(const FDelegateHandle& InHandle)
{
	OnClickCPP.Remove(InHandle);
}

FLGUIDelegateHandleWrapper UUIButtonComponent::RegisterClickEvent(const FLGUIButtonDynamicDelegate& InDelegate)
{
	auto delegateHandle = OnClickCPP.AddLambda([InDelegate] {
		InDelegate.ExecuteIfBound();
	});
	return FLGUIDelegateHandleWrapper(delegateHandle);
}
void UUIButtonComponent::UnregisterClickEvent(const FLGUIDelegateHandleWrapper& InDelegateHandle)
{
	OnClickCPP.Remove(InDelegateHandle.DelegateHandle);
}