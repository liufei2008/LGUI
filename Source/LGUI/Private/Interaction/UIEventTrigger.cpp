// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Interaction/UIEventTrigger.h"

void UUIEventTrigger::OnPointerEnter_Implementation(ULexPointerEventData* EventData)
{
	OnPointerEnterCPP.Broadcast(EventData);
	OnPointerEnter.Broadcast(EventData);
	OnPointerEnterED.FireEvent(EventData);
}
void UUIEventTrigger::OnPointerExit_Implementation(ULexPointerEventData* EventData)
{
	OnPointerExitCPP.Broadcast(EventData);
	OnPointerExit.Broadcast(EventData);
	OnPointerExitED.FireEvent(EventData);
}
bool UUIEventTrigger::OnPointerDown_Implementation(ULexPointerEventData* EventData)
{
	OnPointerDownCPP.Broadcast(EventData);
	OnPointerDown.Broadcast(EventData);
	OnPointerDownED.FireEvent(EventData);
	return AllowEventBubbleUp;
}
bool UUIEventTrigger::OnPointerUp_Implementation(ULexPointerEventData* EventData)
{
	OnPointerUpCPP.Broadcast(EventData);
	OnPointerUp.Broadcast(EventData);
	OnPointerUpED.FireEvent(EventData);
	return AllowEventBubbleUp;
}
bool UUIEventTrigger::OnPointerClick_Implementation(ULexPointerEventData* EventData)
{
	OnPointerClickCPP.Broadcast(EventData);
	OnPointerClick.Broadcast(EventData);
	OnPointerClickED.FireEvent(EventData);
	return AllowEventBubbleUp;
}
bool UUIEventTrigger::OnPointerBeginDrag_Implementation(ULexPointerEventData* EventData)
{
	OnPointerBeginDragCPP.Broadcast(EventData);
	OnPointerBeginDrag.Broadcast(EventData);
	OnPointerBeginDragED.FireEvent(EventData);
	return AllowEventBubbleUp;
}
bool UUIEventTrigger::OnPointerDrag_Implementation(ULexPointerEventData* EventData)
{
	OnPointerDragCPP.Broadcast(EventData);
	OnPointerDrag.Broadcast(EventData);
	OnPointerDragED.FireEvent(EventData);
	return AllowEventBubbleUp;
}
bool UUIEventTrigger::OnPointerEndDrag_Implementation(ULexPointerEventData* EventData)
{
	OnPointerEndDragCPP.Broadcast(EventData);
	OnPointerEndDrag.Broadcast(EventData);
	OnPointerEndDragED.FireEvent(EventData);
	return AllowEventBubbleUp;
}
bool UUIEventTrigger::OnPointerDrop_Implementation(ULexPointerEventData* EventData)
{
	OnPointerDragDropCPP.Broadcast(EventData);
	OnPointerDragDrop.Broadcast(EventData);
	OnPointerDragDropED.FireEvent(EventData);
	return AllowEventBubbleUp;
}
bool UUIEventTrigger::OnPointerScroll_Implementation(ULexPointerEventData* EventData)
{
	OnPointerScrollCPP.Broadcast(EventData);
	OnPointerScroll.Broadcast(EventData);
	OnPointerScrollED.FireEvent(EventData);
	return AllowEventBubbleUp;
}
bool UUIEventTrigger::OnSelect_Implementation(ULexBaseEventData* EventData)
{
	OnSelectCPP.Broadcast(EventData);
	OnSelect.Broadcast(EventData);
	OnSelectED.FireEvent(EventData);
	return AllowEventBubbleUp;
}
bool UUIEventTrigger::OnDeselect_Implementation(ULexBaseEventData* EventData)
{
	OnDeselectCPP.Broadcast(EventData);
	OnDeselect.Broadcast(EventData);
	OnDeselectED.FireEvent(EventData);
	return AllowEventBubbleUp;
}
