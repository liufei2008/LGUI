// Copyright 2019-2021 LexLiu. All Rights Reserved.

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
void UUIEventTriggerComponent::RegisterOnPointerDragDrop(const FLGUIPointerEventDelegate& InDelegate)
{
	OnPointerDragDropCPP.Add(InDelegate);
}
void UUIEventTriggerComponent::RegisterOnPointerScroll(const FLGUIPointerEventDelegate& InDelegate)
{
	OnPointerScrollCPP.Add(InDelegate);
}
void UUIEventTriggerComponent::RegisterOnPointerSelect(const FLGUIBaseEventDelegate& InDelegate)
{
	OnPointerSelectCPP.Add(InDelegate);
}
void UUIEventTriggerComponent::RegisterOnPointerDeselect(const FLGUIBaseEventDelegate& InDelegate)
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
void UUIEventTriggerComponent::UnregisterOnPointerDragDrop(const FLGUIPointerEventDelegate& InDelegate)
{
	OnPointerDragDropCPP.Remove(InDelegate.GetHandle());
}
void UUIEventTriggerComponent::UnregisterOnPointerScroll(const FLGUIPointerEventDelegate& InDelegate)
{
	OnPointerScrollCPP.Remove(InDelegate.GetHandle());
}
void UUIEventTriggerComponent::UnregisterOnPointerSelect(const FLGUIBaseEventDelegate& InDelegate)
{
	OnPointerSelectCPP.Remove(InDelegate.GetHandle());
}
void UUIEventTriggerComponent::UnregisterOnPointerDeselect(const FLGUIBaseEventDelegate& InDelegate)
{
	OnPointerDeselectCPP.Remove(InDelegate.GetHandle());
}

bool UUIEventTriggerComponent::OnPointerEnter_Implementation(ULGUIPointerEventData* eventData)
{
	OnPointerEnter.FireEvent(eventData);
	if (OnPointerEnterCPP.IsBound())OnPointerEnterCPP.Broadcast(eventData);
	return AllowEventBubbleUp;
}
bool UUIEventTriggerComponent::OnPointerExit_Implementation(ULGUIPointerEventData* eventData)
{
	OnPointerExit.FireEvent(eventData);
	if (OnPointerExitCPP.IsBound())OnPointerExitCPP.Broadcast(eventData);
	return AllowEventBubbleUp;
}
bool UUIEventTriggerComponent::OnPointerDown_Implementation(ULGUIPointerEventData* eventData)
{
	OnPointerDown.FireEvent(eventData);
	if (OnPointerDownCPP.IsBound())OnPointerDownCPP.Broadcast(eventData);
	return AllowEventBubbleUp;
}
bool UUIEventTriggerComponent::OnPointerUp_Implementation(ULGUIPointerEventData* eventData)
{
	OnPointerUp.FireEvent(eventData);
	if (OnPointerUpCPP.IsBound())OnPointerUpCPP.Broadcast(eventData);
	return AllowEventBubbleUp;
}
bool UUIEventTriggerComponent::OnPointerClick_Implementation(ULGUIPointerEventData* eventData)
{
	OnPointerClick.FireEvent(eventData);
	if (OnPointerClickCPP.IsBound())OnPointerClickCPP.Broadcast(eventData);
	return AllowEventBubbleUp;
}
bool UUIEventTriggerComponent::OnPointerBeginDrag_Implementation(ULGUIPointerEventData* eventData)
{
	OnPointerBeginDrag.FireEvent(eventData);
	if (OnPointerBeginDragCPP.IsBound())OnPointerBeginDragCPP.Broadcast(eventData);
	return AllowEventBubbleUp;
}
bool UUIEventTriggerComponent::OnPointerDrag_Implementation(ULGUIPointerEventData* eventData)
{
	OnPointerDrag.FireEvent(eventData);
	if (OnPointerDragCPP.IsBound())OnPointerDragCPP.Broadcast(eventData);
	return AllowEventBubbleUp;
}
bool UUIEventTriggerComponent::OnPointerEndDrag_Implementation(ULGUIPointerEventData* eventData)
{
	OnPointerEndDrag.FireEvent(eventData);
	if (OnPointerEndDragCPP.IsBound())OnPointerEndDragCPP.Broadcast(eventData);
	return AllowEventBubbleUp;
}
bool UUIEventTriggerComponent::OnPointerDragDrop_Implementation(ULGUIPointerEventData* eventData)
{
	OnPointerDragDrop.FireEvent(eventData);
	if (OnPointerDragDropCPP.IsBound())OnPointerDragDropCPP.Broadcast(eventData);
	return AllowEventBubbleUp;
}
bool UUIEventTriggerComponent::OnPointerScroll_Implementation(ULGUIPointerEventData* eventData)
{
	OnPointerScroll.FireEvent(eventData);
	if (OnPointerScrollCPP.IsBound())OnPointerScrollCPP.Broadcast(eventData);
	return AllowEventBubbleUp;
}
bool UUIEventTriggerComponent::OnPointerSelect_Implementation(ULGUIBaseEventData* eventData)
{
	OnPointerSelect.FireEvent(eventData);
	if (OnPointerSelectCPP.IsBound())OnPointerSelectCPP.Broadcast(eventData);
	return AllowEventBubbleUp;
}
bool UUIEventTriggerComponent::OnPointerDeselect_Implementation(ULGUIBaseEventData* eventData)
{
	OnPointerDeselect.FireEvent(eventData);
	if (OnPointerDeselectCPP.IsBound())OnPointerDeselectCPP.Broadcast(eventData);
	return AllowEventBubbleUp;
}


FLGUIDelegateHandleWrapper UUIEventTriggerComponent::RegisterOnPointerEnter(const FLGUIPointerEventDynamicDelegate& InDelegate)
{
	auto delegateHandle = OnPointerEnterCPP.AddLambda([InDelegate](ULGUIPointerEventData* eventData) {InDelegate.ExecuteIfBound(eventData); });
	return FLGUIDelegateHandleWrapper(delegateHandle);
}
FLGUIDelegateHandleWrapper UUIEventTriggerComponent::RegisterOnPointerExit(const FLGUIPointerEventDynamicDelegate& InDelegate)
{
	auto delegateHandle = OnPointerExitCPP.AddLambda([InDelegate](ULGUIPointerEventData* eventData) {InDelegate.ExecuteIfBound(eventData); });
	return FLGUIDelegateHandleWrapper(delegateHandle);
}
FLGUIDelegateHandleWrapper UUIEventTriggerComponent::RegisterOnPointerDown(const FLGUIPointerEventDynamicDelegate& InDelegate)
{
	auto delegateHandle = OnPointerDownCPP.AddLambda([InDelegate](ULGUIPointerEventData* eventData) {InDelegate.ExecuteIfBound(eventData); });
	return FLGUIDelegateHandleWrapper(delegateHandle);
}
FLGUIDelegateHandleWrapper UUIEventTriggerComponent::RegisterOnPointerUp(const FLGUIPointerEventDynamicDelegate& InDelegate)
{
	auto delegateHandle = OnPointerUpCPP.AddLambda([InDelegate](ULGUIPointerEventData* eventData) {InDelegate.ExecuteIfBound(eventData); });
	return FLGUIDelegateHandleWrapper(delegateHandle);
}
FLGUIDelegateHandleWrapper UUIEventTriggerComponent::RegisterOnPointerClick(const FLGUIPointerEventDynamicDelegate& InDelegate)
{
	auto delegateHandle = OnPointerClickCPP.AddLambda([InDelegate](ULGUIPointerEventData* eventData) {InDelegate.ExecuteIfBound(eventData); });
	return FLGUIDelegateHandleWrapper(delegateHandle);
}
FLGUIDelegateHandleWrapper UUIEventTriggerComponent::RegisterOnPointerBeginDrag(const FLGUIPointerEventDynamicDelegate& InDelegate)
{
	auto delegateHandle = OnPointerBeginDragCPP.AddLambda([InDelegate](ULGUIPointerEventData* eventData) {InDelegate.ExecuteIfBound(eventData); });
	return FLGUIDelegateHandleWrapper(delegateHandle);
}
FLGUIDelegateHandleWrapper UUIEventTriggerComponent::RegisterOnPointerDrag(const FLGUIPointerEventDynamicDelegate& InDelegate)
{
	auto delegateHandle = OnPointerDragCPP.AddLambda([InDelegate](ULGUIPointerEventData* eventData) {InDelegate.ExecuteIfBound(eventData); });
	return FLGUIDelegateHandleWrapper(delegateHandle);
}
FLGUIDelegateHandleWrapper UUIEventTriggerComponent::RegisterOnPointerEndDrag(const FLGUIPointerEventDynamicDelegate& InDelegate)
{
	auto delegateHandle = OnPointerEndDragCPP.AddLambda([InDelegate](ULGUIPointerEventData* eventData) {InDelegate.ExecuteIfBound(eventData); });
	return FLGUIDelegateHandleWrapper(delegateHandle);
}
FLGUIDelegateHandleWrapper UUIEventTriggerComponent::RegisterOnPointerDragDrop(const FLGUIPointerEventDynamicDelegate& InDelegate)
{
	auto delegateHandle = OnPointerDragDropCPP.AddLambda([InDelegate](ULGUIPointerEventData* eventData) {InDelegate.ExecuteIfBound(eventData); });
	return FLGUIDelegateHandleWrapper(delegateHandle);
}
FLGUIDelegateHandleWrapper UUIEventTriggerComponent::RegisterOnPointerScroll(const FLGUIPointerEventDynamicDelegate& InDelegate)
{
	auto delegateHandle = OnPointerScrollCPP.AddLambda([InDelegate](ULGUIPointerEventData* eventData) {InDelegate.ExecuteIfBound(eventData); });
	return FLGUIDelegateHandleWrapper(delegateHandle);
}
FLGUIDelegateHandleWrapper UUIEventTriggerComponent::RegisterOnPointerSelect(const FLGUIBaseEventDynamicDelegate& InDelegate)
{
	auto delegateHandle = OnPointerSelectCPP.AddLambda([InDelegate](ULGUIBaseEventData* eventData) {InDelegate.ExecuteIfBound(eventData); });
	return FLGUIDelegateHandleWrapper(delegateHandle);
}
FLGUIDelegateHandleWrapper UUIEventTriggerComponent::RegisterOnPointerDeselect(const FLGUIBaseEventDynamicDelegate& InDelegate)
{
	auto delegateHandle = OnPointerDeselectCPP.AddLambda([InDelegate](ULGUIBaseEventData* eventData) {InDelegate.ExecuteIfBound(eventData); });
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
