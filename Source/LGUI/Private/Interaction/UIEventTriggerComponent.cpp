// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Interaction/UIEventTriggerComponent.h"
#include "LGUI.h"

FDelegateHandle UUIEventTriggerComponent::RegisterOnPointerEnter(const FLGUIPointerEventDelegate& InDelegate)
{
	return OnPointerEnterCPP.Add(InDelegate);
}
FDelegateHandle UUIEventTriggerComponent::RegisterOnPointerExit(const FLGUIPointerEventDelegate& InDelegate)
{
	return OnPointerExitCPP.Add(InDelegate);
}
FDelegateHandle UUIEventTriggerComponent::RegisterOnPointerDown(const FLGUIPointerEventDelegate& InDelegate)
{
	return OnPointerDownCPP.Add(InDelegate);
}
FDelegateHandle UUIEventTriggerComponent::RegisterOnPointerUp(const FLGUIPointerEventDelegate& InDelegate)
{
	return OnPointerUpCPP.Add(InDelegate);
}
FDelegateHandle UUIEventTriggerComponent::RegisterOnPointerClick(const FLGUIPointerEventDelegate& InDelegate)
{
	return OnPointerClickCPP.Add(InDelegate);
}
FDelegateHandle UUIEventTriggerComponent::RegisterOnPointerBeginDrag(const FLGUIPointerEventDelegate& InDelegate)
{
	return OnPointerBeginDragCPP.Add(InDelegate);
}
FDelegateHandle UUIEventTriggerComponent::RegisterOnPointerDrag(const FLGUIPointerEventDelegate& InDelegate)
{
	return OnPointerDragCPP.Add(InDelegate);
}
FDelegateHandle UUIEventTriggerComponent::RegisterOnPointerEndDrag(const FLGUIPointerEventDelegate& InDelegate)
{
	return OnPointerEndDragCPP.Add(InDelegate);
}
FDelegateHandle UUIEventTriggerComponent::RegisterOnPointerDragDrop(const FLGUIPointerEventDelegate& InDelegate)
{
	return OnPointerDragDropCPP.Add(InDelegate);
}
FDelegateHandle UUIEventTriggerComponent::RegisterOnPointerScroll(const FLGUIPointerEventDelegate& InDelegate)
{
	return OnPointerScrollCPP.Add(InDelegate);
}
FDelegateHandle UUIEventTriggerComponent::RegisterOnPointerSelect(const FLGUIBaseEventDelegate& InDelegate)
{
	return OnPointerSelectCPP.Add(InDelegate);
}
FDelegateHandle UUIEventTriggerComponent::RegisterOnPointerDeselect(const FLGUIBaseEventDelegate& InDelegate)
{
	return OnPointerDeselectCPP.Add(InDelegate);
}

FDelegateHandle UUIEventTriggerComponent::RegisterOnPointerEnter(const TFunction<void(ULGUIPointerEventData*)>& InFunction)
{
	return OnPointerEnterCPP.AddLambda(InFunction);
}
FDelegateHandle UUIEventTriggerComponent::RegisterOnPointerExit(const TFunction<void(ULGUIPointerEventData*)>& InFunction)
{
	return OnPointerExitCPP.AddLambda(InFunction);
}
FDelegateHandle UUIEventTriggerComponent::RegisterOnPointerDown(const TFunction<void(ULGUIPointerEventData*)>& InFunction)
{
	return OnPointerDownCPP.AddLambda(InFunction);
}
FDelegateHandle UUIEventTriggerComponent::RegisterOnPointerUp(const TFunction<void(ULGUIPointerEventData*)>& InFunction)
{
	return OnPointerUpCPP.AddLambda(InFunction);
}
FDelegateHandle UUIEventTriggerComponent::RegisterOnPointerClick(const TFunction<void(ULGUIPointerEventData*)>& InFunction)
{
	return OnPointerClickCPP.AddLambda(InFunction);
}
FDelegateHandle UUIEventTriggerComponent::RegisterOnPointerBeginDrag(const TFunction<void(ULGUIPointerEventData*)>& InFunction)
{
	return OnPointerBeginDragCPP.AddLambda(InFunction);
}
FDelegateHandle UUIEventTriggerComponent::RegisterOnPointerDrag(const TFunction<void(ULGUIPointerEventData*)>& InFunction)
{
	return OnPointerDragCPP.AddLambda(InFunction);
}
FDelegateHandle UUIEventTriggerComponent::RegisterOnPointerEndDrag(const TFunction<void(ULGUIPointerEventData*)>& InFunction)
{
	return OnPointerEndDragCPP.AddLambda(InFunction);
}
FDelegateHandle UUIEventTriggerComponent::RegisterOnPointerDragDrop(const TFunction<void(ULGUIPointerEventData*)>& InFunction)
{
	return OnPointerDragDropCPP.AddLambda(InFunction);
}
FDelegateHandle UUIEventTriggerComponent::RegisterOnPointerScroll(const TFunction<void(ULGUIPointerEventData*)>& InFunction)
{
	return OnPointerScrollCPP.AddLambda(InFunction);
}
FDelegateHandle UUIEventTriggerComponent::RegisterOnPointerSelect(const TFunction<void(ULGUIBaseEventData*)>& InFunction)
{
	return OnPointerSelectCPP.AddLambda(InFunction);
}
FDelegateHandle UUIEventTriggerComponent::RegisterOnPointerDeselect(const TFunction<void(ULGUIBaseEventData*)>& InFunction)
{
	return OnPointerDeselectCPP.AddLambda(InFunction);
}

void UUIEventTriggerComponent::UnregisterOnPointerEnter(const FDelegateHandle& InHandle)
{
	OnPointerEnterCPP.Remove(InHandle);
}
void UUIEventTriggerComponent::UnregisterOnPointerExit(const FDelegateHandle& InHandle)
{
	OnPointerExitCPP.Remove(InHandle);
}
void UUIEventTriggerComponent::UnregisterOnPointerDown(const FDelegateHandle& InHandle)
{
	OnPointerDownCPP.Remove(InHandle);
}
void UUIEventTriggerComponent::UnregisterOnPointerUp(const FDelegateHandle& InHandle)
{
	OnPointerUpCPP.Remove(InHandle);
}
void UUIEventTriggerComponent::UnregisterOnPointerClick(const FDelegateHandle& InHandle)
{
	OnPointerClickCPP.Remove(InHandle);
}
void UUIEventTriggerComponent::UnregisterOnPointerBeginDrag(const FDelegateHandle& InHandle)
{
	OnPointerBeginDragCPP.Remove(InHandle);
}
void UUIEventTriggerComponent::UnregisterOnPointerDrag(const FDelegateHandle& InHandle)
{
	OnPointerDragCPP.Remove(InHandle);
}
void UUIEventTriggerComponent::UnregisterOnPointerEndDrag(const FDelegateHandle& InHandle)
{
	OnPointerEndDragCPP.Remove(InHandle);
}
void UUIEventTriggerComponent::UnregisterOnPointerDragDrop(const FDelegateHandle& InHandle)
{
	OnPointerDragDropCPP.Remove(InHandle);
}
void UUIEventTriggerComponent::UnregisterOnPointerScroll(const FDelegateHandle& InHandle)
{
	OnPointerScrollCPP.Remove(InHandle);
}
void UUIEventTriggerComponent::UnregisterOnPointerSelect(const FDelegateHandle& InHandle)
{
	OnPointerSelectCPP.Remove(InHandle);
}
void UUIEventTriggerComponent::UnregisterOnPointerDeselect(const FDelegateHandle& InHandle)
{
	OnPointerDeselectCPP.Remove(InHandle);
}

bool UUIEventTriggerComponent::OnPointerEnter_Implementation(ULGUIPointerEventData* eventData)
{
	OnPointerEnter.FireEvent(eventData);
	OnPointerEnterCPP.Broadcast(eventData);
	return AllowEventBubbleUp;
}
bool UUIEventTriggerComponent::OnPointerExit_Implementation(ULGUIPointerEventData* eventData)
{
	OnPointerExit.FireEvent(eventData);
	OnPointerExitCPP.Broadcast(eventData);
	return AllowEventBubbleUp;
}
bool UUIEventTriggerComponent::OnPointerDown_Implementation(ULGUIPointerEventData* eventData)
{
	OnPointerDown.FireEvent(eventData);
	OnPointerDownCPP.Broadcast(eventData);
	return AllowEventBubbleUp;
}
bool UUIEventTriggerComponent::OnPointerUp_Implementation(ULGUIPointerEventData* eventData)
{
	OnPointerUp.FireEvent(eventData);
	OnPointerUpCPP.Broadcast(eventData);
	return AllowEventBubbleUp;
}
bool UUIEventTriggerComponent::OnPointerClick_Implementation(ULGUIPointerEventData* eventData)
{
	OnPointerClick.FireEvent(eventData);
	OnPointerClickCPP.Broadcast(eventData);
	return AllowEventBubbleUp;
}
bool UUIEventTriggerComponent::OnPointerBeginDrag_Implementation(ULGUIPointerEventData* eventData)
{
	OnPointerBeginDrag.FireEvent(eventData);
	OnPointerBeginDragCPP.Broadcast(eventData);
	return AllowEventBubbleUp;
}
bool UUIEventTriggerComponent::OnPointerDrag_Implementation(ULGUIPointerEventData* eventData)
{
	OnPointerDrag.FireEvent(eventData);
	OnPointerDragCPP.Broadcast(eventData);
	return AllowEventBubbleUp;
}
bool UUIEventTriggerComponent::OnPointerEndDrag_Implementation(ULGUIPointerEventData* eventData)
{
	OnPointerEndDrag.FireEvent(eventData);
	OnPointerEndDragCPP.Broadcast(eventData);
	return AllowEventBubbleUp;
}
bool UUIEventTriggerComponent::OnPointerDragDrop_Implementation(ULGUIPointerEventData* eventData)
{
	OnPointerDragDrop.FireEvent(eventData);
	OnPointerDragDropCPP.Broadcast(eventData);
	return AllowEventBubbleUp;
}
bool UUIEventTriggerComponent::OnPointerScroll_Implementation(ULGUIPointerEventData* eventData)
{
	OnPointerScroll.FireEvent(eventData);
	OnPointerScrollCPP.Broadcast(eventData);
	return AllowEventBubbleUp;
}
bool UUIEventTriggerComponent::OnPointerSelect_Implementation(ULGUIBaseEventData* eventData)
{
	OnPointerSelect.FireEvent(eventData);
	OnPointerSelectCPP.Broadcast(eventData);
	return AllowEventBubbleUp;
}
bool UUIEventTriggerComponent::OnPointerDeselect_Implementation(ULGUIBaseEventData* eventData)
{
	OnPointerDeselect.FireEvent(eventData);
	OnPointerDeselectCPP.Broadcast(eventData);
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
