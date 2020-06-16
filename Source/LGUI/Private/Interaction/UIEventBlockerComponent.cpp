﻿// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Interaction/UIEventBlockerComponent.h"
#include "LGUI.h"

bool UUIEventBlockerComponent::OnPointerEnter_Implementation(ULGUIPointerEventData* eventData)
{
	return AllowEventBubbleUp;
}
bool UUIEventBlockerComponent::OnPointerExit_Implementation(ULGUIPointerEventData* eventData)
{
	return AllowEventBubbleUp;
}
bool UUIEventBlockerComponent::OnPointerDown_Implementation(ULGUIPointerEventData* eventData)
{
	return AllowEventBubbleUp;
}
bool UUIEventBlockerComponent::OnPointerUp_Implementation(ULGUIPointerEventData* eventData)
{
	return AllowEventBubbleUp;
}
bool UUIEventBlockerComponent::OnPointerClick_Implementation(ULGUIPointerEventData* eventData)
{
	return AllowEventBubbleUp;
}
bool UUIEventBlockerComponent::OnPointerBeginDrag_Implementation(ULGUIPointerEventData* eventData)
{
	return AllowEventBubbleUp;
}
bool UUIEventBlockerComponent::OnPointerDrag_Implementation(ULGUIPointerEventData* eventData)
{
	return AllowEventBubbleUp;
}
bool UUIEventBlockerComponent::OnPointerEndDrag_Implementation(ULGUIPointerEventData* eventData)
{
	return AllowEventBubbleUp;
}
bool UUIEventBlockerComponent::OnPointerDragEnter_Implementation(ULGUIPointerEventData* eventData)
{
	return AllowEventBubbleUp;
}
bool UUIEventBlockerComponent::OnPointerDragExit_Implementation(ULGUIPointerEventData* eventData)
{
	return AllowEventBubbleUp;
}
bool UUIEventBlockerComponent::OnPointerDragDrop_Implementation(ULGUIPointerEventData* eventData)
{
	return AllowEventBubbleUp;
}
bool UUIEventBlockerComponent::OnPointerScroll_Implementation(ULGUIPointerEventData* eventData)
{
	return AllowEventBubbleUp;
}
bool UUIEventBlockerComponent::OnPointerSelect_Implementation(ULGUIPointerEventData* eventData)
{
	return AllowEventBubbleUp;
}
bool UUIEventBlockerComponent::OnPointerDeselect_Implementation(ULGUIPointerEventData* eventData)
{
	return AllowEventBubbleUp;
}