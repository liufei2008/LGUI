// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Interaction/UIEventTriggerComponent.h"
#include "LGUI.h"

void UUIEventTriggerComponent::RegisterOnPointerEnter(const FLGUIPointerEventDelegate& InDelegate)
{
	OnPointerEnterCPP.Add(InDelegate);
}
void UUIEventTriggerComponent::RegisterOnPointerExit(const FLGUIPointerEventDelegate& InDelegate)
{
	OnPointerExitCPP.Add(InDelegate);
}
void UUIEventTriggerComponent::RegisterOnPointerDown(const FLGUIPointerEventDelegate& InDelegate)
{
	OnPointerDownCPP.Add(InDelegate);
}
void UUIEventTriggerComponent::RegisterOnPointerUp(const FLGUIPointerEventDelegate& InDelegate)
{
	OnPointerUpCPP.Add(InDelegate);
}
void UUIEventTriggerComponent::RegisterOnPointerClick(const FLGUIPointerEventDelegate& InDelegate)
{
	OnPointerClickCPP.Add(InDelegate);
}
void UUIEventTriggerComponent::RegisterOnPointerBeginDrag(const FLGUIPointerEventDelegate& InDelegate)
{
	OnPointerBeginDragCPP.Add(InDelegate);
}
void UUIEventTriggerComponent::RegisterOnPointerDrag(const FLGUIPointerEventDelegate& InDelegate)
{
	OnPointerDragCPP.Add(InDelegate);
}
void UUIEventTriggerComponent::RegisterOnPointerEndDrag(const FLGUIPointerEventDelegate& InDelegate)
{
	OnPointerEndDragCPP.Add(InDelegate);
}
void UUIEventTriggerComponent::RegisterOnPointerDragEnter(const FLGUIPointerEventDelegate& InDelegate)
{
	OnPointerDragEnterCPP.Add(InDelegate);
}
void UUIEventTriggerComponent::RegisterOnPointerDragExit(const FLGUIPointerEventDelegate& InDelegate)
{
	OnPointerDragExitCPP.Add(InDelegate);
}
void UUIEventTriggerComponent::RegisterOnPointerDragDrop(const FLGUIPointerEventDelegate& InDelegate)
{
	OnPointerDragDropCPP.Add(InDelegate);
}
void UUIEventTriggerComponent::RegisterOnPointerScroll(const FLGUIPointerEventDelegate& InDelegate)
{
	OnPointerScrollCPP.Add(InDelegate);
}
void UUIEventTriggerComponent::RegisterOnPointerSelect(const FLGUIPointerEventDelegate& InDelegate)
{
	OnPointerSelectCPP.Add(InDelegate);
}
void UUIEventTriggerComponent::RegisterOnPointerDeselect(const FLGUIPointerEventDelegate& InDelegate)
{
	OnPointerDeselectCPP.Add(InDelegate);
}

void UUIEventTriggerComponent::UnregisterOnPointerEnter(const FLGUIPointerEventDelegate& InDelegate)
{
	OnPointerEnterCPP.Remove(InDelegate.GetHandle());
}
void UUIEventTriggerComponent::UnregisterOnPointerExit(const FLGUIPointerEventDelegate& InDelegate)
{
	OnPointerExitCPP.Remove(InDelegate.GetHandle());
}
void UUIEventTriggerComponent::UnregisterOnPointerDown(const FLGUIPointerEventDelegate& InDelegate)
{
	OnPointerDownCPP.Remove(InDelegate.GetHandle());
}
void UUIEventTriggerComponent::UnregisterOnPointerUp(const FLGUIPointerEventDelegate& InDelegate)
{
	OnPointerUpCPP.Remove(InDelegate.GetHandle());
}
void UUIEventTriggerComponent::UnregisterOnPointerClick(const FLGUIPointerEventDelegate& InDelegate)
{
	OnPointerClickCPP.Remove(InDelegate.GetHandle());
}
void UUIEventTriggerComponent::UnregisterOnPointerBeginDrag(const FLGUIPointerEventDelegate& InDelegate)
{
	OnPointerBeginDragCPP.Remove(InDelegate.GetHandle());
}
void UUIEventTriggerComponent::UnregisterOnPointerDrag(const FLGUIPointerEventDelegate& InDelegate)
{
	OnPointerDragCPP.Remove(InDelegate.GetHandle());
}
void UUIEventTriggerComponent::UnregisterOnPointerEndDrag(const FLGUIPointerEventDelegate& InDelegate)
{
	OnPointerEndDragCPP.Remove(InDelegate.GetHandle());
}
void UUIEventTriggerComponent::UnregisterOnPointerDragEnter(const FLGUIPointerEventDelegate& InDelegate)
{
	OnPointerDragEnterCPP.Remove(InDelegate.GetHandle());
}
void UUIEventTriggerComponent::UnregisterOnPointerDragExit(const FLGUIPointerEventDelegate& InDelegate)
{
	OnPointerDragExitCPP.Remove(InDelegate.GetHandle());
}
void UUIEventTriggerComponent::UnregisterOnPointerDragDrop(const FLGUIPointerEventDelegate& InDelegate)
{
	OnPointerDragDropCPP.Remove(InDelegate.GetHandle());
}
void UUIEventTriggerComponent::UnregisterOnPointerScroll(const FLGUIPointerEventDelegate& InDelegate)
{
	OnPointerScrollCPP.Remove(InDelegate.GetHandle());
}
void UUIEventTriggerComponent::UnregisterOnPointerSelect(const FLGUIPointerEventDelegate& InDelegate)
{
	OnPointerSelectCPP.Remove(InDelegate.GetHandle());
}
void UUIEventTriggerComponent::UnregisterOnPointerDeselect(const FLGUIPointerEventDelegate& InDelegate)
{
	OnPointerDeselectCPP.Remove(InDelegate.GetHandle());
}

bool UUIEventTriggerComponent::OnPointerEnter_Implementation(const FLGUIPointerEventData& eventData)
{
	OnPointerEnter.FireEvent(eventData);
	if (OnPointerEnterCPP.IsBound())OnPointerEnterCPP.Broadcast(eventData);
	return AllowEventBubbleUp;
}
bool UUIEventTriggerComponent::OnPointerExit_Implementation(const FLGUIPointerEventData& eventData)
{
	OnPointerExit.FireEvent(eventData);
	if (OnPointerExitCPP.IsBound())OnPointerExitCPP.Broadcast(eventData);
	return AllowEventBubbleUp;
}
bool UUIEventTriggerComponent::OnPointerDown_Implementation(const FLGUIPointerEventData& eventData)
{
	OnPointerDown.FireEvent(eventData);
	if (OnPointerDownCPP.IsBound())OnPointerDownCPP.Broadcast(eventData);
	return AllowEventBubbleUp;
}
bool UUIEventTriggerComponent::OnPointerUp_Implementation(const FLGUIPointerEventData& eventData)
{
	OnPointerUp.FireEvent(eventData);
	if (OnPointerUpCPP.IsBound())OnPointerUpCPP.Broadcast(eventData);
	return AllowEventBubbleUp;
}
bool UUIEventTriggerComponent::OnPointerClick_Implementation(const FLGUIPointerEventData& eventData)
{
	OnPointerClick.FireEvent(eventData);
	if (OnPointerClickCPP.IsBound())OnPointerClickCPP.Broadcast(eventData);
	return AllowEventBubbleUp;
}
bool UUIEventTriggerComponent::OnPointerBeginDrag_Implementation(const FLGUIPointerEventData& eventData)
{
	OnPointerBeginDrag.FireEvent(eventData);
	if (OnPointerBeginDragCPP.IsBound())OnPointerBeginDragCPP.Broadcast(eventData);
	return AllowEventBubbleUp;
}
bool UUIEventTriggerComponent::OnPointerDrag_Implementation(const FLGUIPointerEventData& eventData)
{
	OnPointerDrag.FireEvent(eventData);
	if (OnPointerDragCPP.IsBound())OnPointerDragCPP.Broadcast(eventData);
	return AllowEventBubbleUp;
}
bool UUIEventTriggerComponent::OnPointerEndDrag_Implementation(const FLGUIPointerEventData& eventData)
{
	OnPointerEndDrag.FireEvent(eventData);
	if (OnPointerEndDragCPP.IsBound())OnPointerEndDragCPP.Broadcast(eventData);
	return AllowEventBubbleUp;
}
bool UUIEventTriggerComponent::OnPointerDragEnter_Implementation(const FLGUIPointerEventData& eventData)
{
	OnPointerDragEnter.FireEvent(eventData);
	if (OnPointerDragEnterCPP.IsBound())OnPointerDragEnterCPP.Broadcast(eventData);
	return AllowEventBubbleUp;
}
bool UUIEventTriggerComponent::OnPointerDragExit_Implementation(const FLGUIPointerEventData& eventData)
{
	OnPointerDragExit.FireEvent(eventData);
	if (OnPointerDragExitCPP.IsBound())OnPointerDragExitCPP.Broadcast(eventData);
	return AllowEventBubbleUp;
}
bool UUIEventTriggerComponent::OnPointerDragDrop_Implementation(const FLGUIPointerEventData& eventData)
{
	OnPointerDragDrop.FireEvent(eventData);
	if (OnPointerDragDropCPP.IsBound())OnPointerDragDropCPP.Broadcast(eventData);
	return AllowEventBubbleUp;
}
bool UUIEventTriggerComponent::OnPointerScroll_Implementation(const FLGUIPointerEventData& eventData)
{
	OnPointerScroll.FireEvent(eventData);
	if (OnPointerScrollCPP.IsBound())OnPointerScrollCPP.Broadcast(eventData);
	return AllowEventBubbleUp;
}
bool UUIEventTriggerComponent::OnPointerSelect_Implementation(const FLGUIPointerEventData& eventData)
{
	OnPointerSelect.FireEvent(eventData);
	if (OnPointerSelectCPP.IsBound())OnPointerSelectCPP.Broadcast(eventData);
	return AllowEventBubbleUp;
}
bool UUIEventTriggerComponent::OnPointerDeselect_Implementation(const FLGUIPointerEventData& eventData)
{
	OnPointerDeselect.FireEvent(eventData);
	if (OnPointerDeselectCPP.IsBound())OnPointerDeselectCPP.Broadcast(eventData);
	return AllowEventBubbleUp;
}


FLGUIDelegateHandleWrapper UUIEventTriggerComponent::RegisterOnPointerEnter(const FLGUIPointerEventDynamicDelegate& InDelegate)
{
	auto delegateHandle = OnPointerEnterCPP.AddLambda([InDelegate](const FLGUIPointerEventData& eventData) {InDelegate.ExecuteIfBound(eventData); });
	return FLGUIDelegateHandleWrapper(delegateHandle);
}
FLGUIDelegateHandleWrapper UUIEventTriggerComponent::RegisterOnPointerExit(const FLGUIPointerEventDynamicDelegate& InDelegate)
{
	auto delegateHandle = OnPointerExitCPP.AddLambda([InDelegate](const FLGUIPointerEventData& eventData) {InDelegate.ExecuteIfBound(eventData); });
	return FLGUIDelegateHandleWrapper(delegateHandle);
}
FLGUIDelegateHandleWrapper UUIEventTriggerComponent::RegisterOnPointerDown(const FLGUIPointerEventDynamicDelegate& InDelegate)
{
	auto delegateHandle = OnPointerDownCPP.AddLambda([InDelegate](const FLGUIPointerEventData& eventData) {InDelegate.ExecuteIfBound(eventData); });
	return FLGUIDelegateHandleWrapper(delegateHandle);
}
FLGUIDelegateHandleWrapper UUIEventTriggerComponent::RegisterOnPointerUp(const FLGUIPointerEventDynamicDelegate& InDelegate)
{
	auto delegateHandle = OnPointerUpCPP.AddLambda([InDelegate](const FLGUIPointerEventData& eventData) {InDelegate.ExecuteIfBound(eventData); });
	return FLGUIDelegateHandleWrapper(delegateHandle);
}
FLGUIDelegateHandleWrapper UUIEventTriggerComponent::RegisterOnPointerClick(const FLGUIPointerEventDynamicDelegate& InDelegate)
{
	auto delegateHandle = OnPointerClickCPP.AddLambda([InDelegate](const FLGUIPointerEventData& eventData) {InDelegate.ExecuteIfBound(eventData); });
	return FLGUIDelegateHandleWrapper(delegateHandle);
}
FLGUIDelegateHandleWrapper UUIEventTriggerComponent::RegisterOnPointerBeginDrag(const FLGUIPointerEventDynamicDelegate& InDelegate)
{
	auto delegateHandle = OnPointerBeginDragCPP.AddLambda([InDelegate](const FLGUIPointerEventData& eventData) {InDelegate.ExecuteIfBound(eventData); });
	return FLGUIDelegateHandleWrapper(delegateHandle);
}
FLGUIDelegateHandleWrapper UUIEventTriggerComponent::RegisterOnPointerDrag(const FLGUIPointerEventDynamicDelegate& InDelegate)
{
	auto delegateHandle = OnPointerDragCPP.AddLambda([InDelegate](const FLGUIPointerEventData& eventData) {InDelegate.ExecuteIfBound(eventData); });
	return FLGUIDelegateHandleWrapper(delegateHandle);
}
FLGUIDelegateHandleWrapper UUIEventTriggerComponent::RegisterOnPointerEndDrag(const FLGUIPointerEventDynamicDelegate& InDelegate)
{
	auto delegateHandle = OnPointerEndDragCPP.AddLambda([InDelegate](const FLGUIPointerEventData& eventData) {InDelegate.ExecuteIfBound(eventData); });
	return FLGUIDelegateHandleWrapper(delegateHandle);
}
FLGUIDelegateHandleWrapper UUIEventTriggerComponent::RegisterOnPointerDragEnter(const FLGUIPointerEventDynamicDelegate& InDelegate)
{
	auto delegateHandle = OnPointerDragEnterCPP.AddLambda([InDelegate](const FLGUIPointerEventData& eventData) {InDelegate.ExecuteIfBound(eventData); });
	return FLGUIDelegateHandleWrapper(delegateHandle);
}
FLGUIDelegateHandleWrapper UUIEventTriggerComponent::RegisterOnPointerDragExit(const FLGUIPointerEventDynamicDelegate& InDelegate)
{
	auto delegateHandle = OnPointerDragExitCPP.AddLambda([InDelegate](const FLGUIPointerEventData& eventData) {InDelegate.ExecuteIfBound(eventData); });
	return FLGUIDelegateHandleWrapper(delegateHandle);
}
FLGUIDelegateHandleWrapper UUIEventTriggerComponent::RegisterOnPointerDragDrop(const FLGUIPointerEventDynamicDelegate& InDelegate)
{
	auto delegateHandle = OnPointerDragDropCPP.AddLambda([InDelegate](const FLGUIPointerEventData& eventData) {InDelegate.ExecuteIfBound(eventData); });
	return FLGUIDelegateHandleWrapper(delegateHandle);
}
FLGUIDelegateHandleWrapper UUIEventTriggerComponent::RegisterOnPointerScroll(const FLGUIPointerEventDynamicDelegate& InDelegate)
{
	auto delegateHandle = OnPointerScrollCPP.AddLambda([InDelegate](const FLGUIPointerEventData& eventData) {InDelegate.ExecuteIfBound(eventData); });
	return FLGUIDelegateHandleWrapper(delegateHandle);
}
FLGUIDelegateHandleWrapper UUIEventTriggerComponent::RegisterOnPointerSelect(const FLGUIPointerEventDynamicDelegate& InDelegate)
{
	auto delegateHandle = OnPointerSelectCPP.AddLambda([InDelegate](const FLGUIPointerEventData& eventData) {InDelegate.ExecuteIfBound(eventData); });
	return FLGUIDelegateHandleWrapper(delegateHandle);
}
FLGUIDelegateHandleWrapper UUIEventTriggerComponent::RegisterOnPointerDeselect(const FLGUIPointerEventDynamicDelegate& InDelegate)
{
	auto delegateHandle = OnPointerDeselectCPP.AddLambda([InDelegate](const FLGUIPointerEventData& eventData) {InDelegate.ExecuteIfBound(eventData); });
	return FLGUIDelegateHandleWrapper(delegateHandle);
}

void UUIEventTriggerComponent::UnregisterOnPointerEnter(const FLGUIDelegateHandleWrapper& InDelegateHandle)
{
	OnPointerEnterCPP.Remove(InDelegateHandle.DelegateHandle);
}
void UUIEventTriggerComponent::UnregisterOnPointerExit(const FLGUIDelegateHandleWrapper& InDelegateHandle)
{
	OnPointerExitCPP.Remove(InDelegateHandle.DelegateHandle);
}
void UUIEventTriggerComponent::UnregisterOnPointerDown(const FLGUIDelegateHandleWrapper& InDelegateHandle)
{
	OnPointerDownCPP.Remove(InDelegateHandle.DelegateHandle);
}
void UUIEventTriggerComponent::UnregisterOnPointerUp(const FLGUIDelegateHandleWrapper& InDelegateHandle)
{
	OnPointerUpCPP.Remove(InDelegateHandle.DelegateHandle);
}
void UUIEventTriggerComponent::UnregisterOnPointerClick(const FLGUIDelegateHandleWrapper& InDelegateHandle)
{
	OnPointerClickCPP.Remove(InDelegateHandle.DelegateHandle);
}
void UUIEventTriggerComponent::UnregisterOnPointerBeginDrag(const FLGUIDelegateHandleWrapper& InDelegateHandle)
{
	OnPointerBeginDragCPP.Remove(InDelegateHandle.DelegateHandle);
}
void UUIEventTriggerComponent::UnregisterOnPointerDrag(const FLGUIDelegateHandleWrapper& InDelegateHandle)
{
	OnPointerDragCPP.Remove(InDelegateHandle.DelegateHandle);
}
void UUIEventTriggerComponent::UnregisterOnPointerEndDrag(const FLGUIDelegateHandleWrapper& InDelegateHandle)
{
	OnPointerEndDragCPP.Remove(InDelegateHandle.DelegateHandle);
}
void UUIEventTriggerComponent::UnregisterOnPointerDragEnter(const FLGUIDelegateHandleWrapper& InDelegateHandle)
{
	OnPointerDragEnterCPP.Remove(InDelegateHandle.DelegateHandle);
}
void UUIEventTriggerComponent::UnregisterOnPointerDragExit(const FLGUIDelegateHandleWrapper& InDelegateHandle)
{
	OnPointerDragExitCPP.Remove(InDelegateHandle.DelegateHandle);
}
void UUIEventTriggerComponent::UnregisterOnPointerDragDrop(const FLGUIDelegateHandleWrapper& InDelegateHandle)
{
	OnPointerDragDropCPP.Remove(InDelegateHandle.DelegateHandle);
}
void UUIEventTriggerComponent::UnregisterOnPointerScroll(const FLGUIDelegateHandleWrapper& InDelegateHandle)
{
	OnPointerScrollCPP.Remove(InDelegateHandle.DelegateHandle);
}
void UUIEventTriggerComponent::UnregisterOnPointerSelect(const FLGUIDelegateHandleWrapper& InDelegateHandle)
{
	OnPointerSelectCPP.Remove(InDelegateHandle.DelegateHandle);
}
void UUIEventTriggerComponent::UnregisterOnPointerDeselect(const FLGUIDelegateHandleWrapper& InDelegateHandle)
{
	OnPointerDeselectCPP.Remove(InDelegateHandle.DelegateHandle);
}
