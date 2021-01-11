// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Event/Rayemitter/LGUI_SceneComponentRayEmitter.h"
#include "LGUI.h"

bool ULGUI_SceneComponentRayEmitter::EmitRay(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, TArray<AActor*>& InOutTraceOnlyActors, TArray<AActor*>& InOutTraceIgnoreActors)
{
	if (CacheTargetSceneComponent == nullptr)
	{
		if (TargetActor == nullptr)
		{
			UE_LOG(LGUI, Warning, TEXT("[LGUI_SceneComponentRayemitter/EmitRay]TargetActor is null!"));
			return false;
		}
		CacheTargetSceneComponent = TargetActor->GetRootComponent();
	}

	OutRayOrigin = CacheTargetSceneComponent->GetComponentLocation();
	switch (RayDirectionType)
	{
	case ESceneComponentRayDirection::PositiveX:
		OutRayDirection = CacheTargetSceneComponent->GetForwardVector();
		break;
	case ESceneComponentRayDirection::NagtiveX:
		OutRayDirection = -CacheTargetSceneComponent->GetForwardVector();
		break;
	case ESceneComponentRayDirection::PositiveY:
		OutRayDirection = CacheTargetSceneComponent->GetRightVector();
		break;
	case ESceneComponentRayDirection::NagtiveY:
		OutRayDirection = -CacheTargetSceneComponent->GetRightVector();
		break;
	case ESceneComponentRayDirection::PositiveZ:
		OutRayDirection = CacheTargetSceneComponent->GetUpVector();
		break;
	case ESceneComponentRayDirection::NagtiveZ:
		OutRayDirection = -CacheTargetSceneComponent->GetUpVector();
		break;
	}
	currentRayOrigin = OutRayOrigin;
	currentRayDirection = OutRayDirection;
	return true;
}
bool ULGUI_SceneComponentRayEmitter::ShouldStartDrag(ULGUIPointerEventData* InPointerEventData)
{
	if (ShouldStartDrag_HoldToDrag(InPointerEventData))return true;
	auto calculatedThreshold = clickTresholdSquare;
	if (clickThresholdRelateToRayDistance)
	{
		calculatedThreshold *= InPointerEventData->pressDistance;
	}
	return (InPointerEventData->GetWorldPointSpherical() - InPointerEventData->pressWorldPoint).SizeSquared() > calculatedThreshold;
}
