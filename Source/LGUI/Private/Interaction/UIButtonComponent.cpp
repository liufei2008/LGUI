// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Interaction/UIButtonComponent.h"
#include "LGUI.h"


UUIButtonComponent::UUIButtonComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUIButtonComponent::BeginPlay()
{
	Super::BeginPlay();
}

#if WITH_EDITOR
void UUIButtonComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

bool UUIButtonComponent::OnPointerClick_Implementation(const FLGUIPointerEventData& eventData)
{
	if (OnClickCPP.IsBound())OnClickCPP.Broadcast();
	OnClick.FireEvent();
	return AllowEventBubbleUp;
}
void UUIButtonComponent::RegisterClickEvent(const FSimpleDelegate& InDelegate)
{
	OnClickCPP.Add(InDelegate);
}
void UUIButtonComponent::UnregisterClickEvent(const FSimpleDelegate& InDelegate)
{
	OnClickCPP.Remove(InDelegate.GetHandle());
}

FLGUIDelegateHandleWrapper UUIButtonComponent::RegisterClickEvent(const FLGUIButtonDynamicDelegate& InDelegate)
{
	auto delegateHandle = OnClickCPP.AddLambda([InDelegate] {
		if (InDelegate.IsBound())InDelegate.Execute();
	});
	return FLGUIDelegateHandleWrapper(delegateHandle);
}
void UUIButtonComponent::UnregisterClickEvent(const FLGUIDelegateHandleWrapper& InDelegateHandle)
{
	OnClickCPP.Remove(InDelegateHandle.DelegateHandle);
}