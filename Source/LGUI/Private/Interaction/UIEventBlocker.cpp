// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Interaction/UIEventBlocker.h"
#include "LGUI.h"

void UUIEventBlocker::OnPointerEnter_Implementation(ULexPointerEventData* EventData)
{
}
void UUIEventBlocker::OnPointerExit_Implementation(ULexPointerEventData* EventData)
{
}
bool UUIEventBlocker::OnPointerDown_Implementation(ULexPointerEventData* EventData)
{
	return AllowEventBubbleUp;
}
bool UUIEventBlocker::OnPointerUp_Implementation(ULexPointerEventData* EventData)
{
	return AllowEventBubbleUp;
}
bool UUIEventBlocker::OnPointerClick_Implementation(ULexPointerEventData* EventData)
{
	return AllowEventBubbleUp;
}
bool UUIEventBlocker::OnPointerBeginDrag_Implementation(ULexPointerEventData* EventData)
{
	return AllowEventBubbleUp;
}
bool UUIEventBlocker::OnPointerDrag_Implementation(ULexPointerEventData* EventData)
{
	return AllowEventBubbleUp;
}
bool UUIEventBlocker::OnPointerEndDrag_Implementation(ULexPointerEventData* EventData)
{
	return AllowEventBubbleUp;
}
bool UUIEventBlocker::OnPointerDrop_Implementation(ULexPointerEventData* EventData)
{
	return AllowEventBubbleUp;
}
bool UUIEventBlocker::OnPointerScroll_Implementation(ULexPointerEventData* EventData)
{
	return AllowEventBubbleUp;
}
bool UUIEventBlocker::OnSelect_Implementation(ULexBaseEventData* EventData)
{
	return AllowEventBubbleUp;
}
bool UUIEventBlocker::OnDeselect_Implementation(ULexBaseEventData* EventData)
{
	return AllowEventBubbleUp;
}