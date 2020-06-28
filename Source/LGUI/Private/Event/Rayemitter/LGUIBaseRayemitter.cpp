// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Event/Rayemitter/LGUIBaseRayEmitter.h"
#include "LGUI.h"

void ULGUIBaseRayEmitter::BeginPlay()
{
	Super::BeginPlay();
	clickTresholdSquare = clickThreshold * clickThreshold;
}
bool ULGUIBaseRayEmitter::EmitRay(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, TArray<AActor*>& InOutTraceOnlyActors, TArray<AActor*>& InOutTraceIgnoreActors)
{
	UE_LOG(LGUI, Error, TEXT("[ULGUIBaseRayEmitter]Function EmitRay must be implemented!"));
	return false;
}

bool ULGUIBaseRayEmitter::ShouldStartDrag(ULGUIPointerEventData* InPointerEventData)
{
	UE_LOG(LGUI, Error, TEXT("[ULGUIBaseRayEmitter]Function ShouldStartDrag must be implemented!"));
	return false;
}

void ULGUIBaseRayEmitter::SetInitialValue(float InClickThreshold, bool InHoldToDrag, float InHoldToDragTime)
{
	clickThreshold = InClickThreshold;
	clickTresholdSquare = clickThreshold * clickThreshold;
	holdToDrag = InHoldToDrag;
	holdToDragTime = InHoldToDragTime;
}
void ULGUIBaseRayEmitter::GetInitialValue(float& OutClickThreshold, bool& OutHoldToDrag, float& OutHoldToDragTime)
{
	OutClickThreshold = clickThreshold;
	OutHoldToDrag = holdToDrag;
	OutHoldToDragTime = holdToDragTime;
}
bool ULGUIBaseRayEmitter::ShouldStartDrag_HoldToDrag(ULGUIPointerEventData* InPointerEventData)
{
	if (holdToDrag)
	{
		return GetWorld()->TimeSeconds - InPointerEventData->pressTime > holdToDragTime;
	}
	return false;
}
void ULGUIBaseRayEmitter::MarkPress(ULGUIPointerEventData* InPointerEventData)
{
	
}
