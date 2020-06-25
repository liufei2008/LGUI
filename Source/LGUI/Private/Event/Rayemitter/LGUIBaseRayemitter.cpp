// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Event/Rayemitter/LGUIBaseRayEmitter.h"
#include "LGUI.h"

bool ULGUIBaseRayEmitter::EmitRay(FVector& OutRayOrigin, FVector& OutRayDirection, TArray<AActor*>& InOutTraceOnlyActors, TArray<AActor*>& InOutTraceIgnoreActors)
{
	UE_LOG(LGUI, Error, TEXT("[ULGUIBaseRayEmitter]Function EmitRay must be implemented!"));
	return false;
}

bool ULGUIBaseRayEmitter::ShouldStartDrag(ULGUIPointerEventData* InPointerEventData)
{
	UE_LOG(LGUI, Error, TEXT("[ULGUIBaseRayEmitter]Function ShouldStartDrag must be implemented!"));
	return false;
}
