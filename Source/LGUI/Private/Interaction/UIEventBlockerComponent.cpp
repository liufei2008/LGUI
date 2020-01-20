// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Interaction/UIEventBlockerComponent.h"
#include "LGUI.h"


UUIEventBlockerComponent::UUIEventBlockerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUIEventBlockerComponent::BeginPlay()
{
	Super::BeginPlay();
}

#if WITH_EDITOR
void UUIEventBlockerComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

bool UUIEventBlockerComponent::OnPointerEnter_Implementation(const FLGUIPointerEventData& eventData)
{
	return AllowEventBubbleUp;
}
bool UUIEventBlockerComponent::OnPointerExit_Implementation(const FLGUIPointerEventData& eventData)
{
	return AllowEventBubbleUp;
}
bool UUIEventBlockerComponent::OnPointerDown_Implementation(const FLGUIPointerEventData& eventData)
{
	return AllowEventBubbleUp;
}
bool UUIEventBlockerComponent::OnPointerUp_Implementation(const FLGUIPointerEventData& eventData)
{
	return AllowEventBubbleUp;
}
bool UUIEventBlockerComponent::OnPointerClick_Implementation(const FLGUIPointerEventData& eventData)
{
	return AllowEventBubbleUp;
}
bool UUIEventBlockerComponent::OnPointerBeginDrag_Implementation(const FLGUIPointerEventData& eventData)
{
	return AllowEventBubbleUp;
}
bool UUIEventBlockerComponent::OnPointerDrag_Implementation(const FLGUIPointerEventData& eventData)
{
	return AllowEventBubbleUp;
}
bool UUIEventBlockerComponent::OnPointerEndDrag_Implementation(const FLGUIPointerEventData& eventData)
{
	return AllowEventBubbleUp;
}
bool UUIEventBlockerComponent::OnPointerDragEnter_Implementation(const FLGUIPointerEventData& eventData)
{
	return AllowEventBubbleUp;
}
bool UUIEventBlockerComponent::OnPointerDragExit_Implementation(const FLGUIPointerEventData& eventData)
{
	return AllowEventBubbleUp;
}
bool UUIEventBlockerComponent::OnPointerDragDrop_Implementation(const FLGUIPointerEventData& eventData)
{
	return AllowEventBubbleUp;
}
bool UUIEventBlockerComponent::OnPointerScroll_Implementation(const FLGUIPointerEventData& eventData)
{
	return AllowEventBubbleUp;
}
bool UUIEventBlockerComponent::OnPointerSelect_Implementation(const FLGUIPointerEventData& eventData)
{
	return AllowEventBubbleUp;
}
bool UUIEventBlockerComponent::OnPointerDeselect_Implementation(const FLGUIPointerEventData& eventData)
{
	return AllowEventBubbleUp;
}