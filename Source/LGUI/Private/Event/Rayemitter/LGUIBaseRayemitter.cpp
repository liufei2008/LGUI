// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Event/Rayemitter/LGUIBaseRayemitter.h"
#include "LGUI.h"

void ULGUIBaseRayemitter::BeginPlay()
{
	Super::BeginPlay();
	clickTresholdSquare = clickThreshold * clickThreshold;
}
bool ULGUIBaseRayemitter::EmitRay(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, TArray<AActor*>& InOutTraceOnlyActors, TArray<AActor*>& InOutTraceIgnoreActors)
{
	UE_LOG(LGUI, Error, TEXT("[ULGUIBaseRayemitter]Function EmitRay must be implemented!"));
	return false;
}

bool ULGUIBaseRayemitter::ShouldStartDrag(ULGUIPointerEventData* InPointerEventData)
{
	UE_LOG(LGUI, Error, TEXT("[ULGUIBaseRayemitter]Function ShouldStartDrag must be implemented!"));
	return false;
}

void ULGUIBaseRayemitter::SetInitialValue(float InClickThreshold, bool InHoldToDrag, float InHoldToDragTime)
{
	clickThreshold = InClickThreshold;
	clickTresholdSquare = clickThreshold * clickThreshold;
	holdToDrag = InHoldToDrag;
	holdToDragTime = InHoldToDragTime;
}
void ULGUIBaseRayemitter::GetInitialValue(float& OutClickThreshold, bool& OutHoldToDrag, float& OutHoldToDragTime)
{
	OutClickThreshold = clickThreshold;
	OutHoldToDrag = holdToDrag;
	OutHoldToDragTime = holdToDragTime;
}
bool ULGUIBaseRayemitter::ShouldStartDrag_HoldToDrag(ULGUIPointerEventData* InPointerEventData)
{
	if (holdToDrag)
	{
		return GetWorld()->TimeSeconds - InPointerEventData->pressTime > holdToDragTime;
	}
	return false;
}
void ULGUIBaseRayemitter::MarkPress(ULGUIPointerEventData* InPointerEventData)
{
	
}
