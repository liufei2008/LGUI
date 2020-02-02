// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Event/Rayemitter/LGUI_SceneComponentRayEmitter.h"
#include "LGUI.h"

ULGUI_SceneComponentRayEmitter::ULGUI_SceneComponentRayEmitter()
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

bool ULGUI_SceneComponentRayEmitter::EmitRay(FVector& OutRayOrigin, FVector& OutRayDirection, TArray<AActor*>& InOutTraceOnlyActors, TArray<AActor*>& InOutTraceIgnoreActors)
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
bool ULGUI_SceneComponentRayEmitter::ShouldStartDrag(const FLGUIPointerEventData& InPointerEventData)
{
	auto calculatedThreshold = clickThreshold;
	if (clickThresholdRelateToRayDistance)
	{
		calculatedThreshold *= InPointerEventData.pressDistance;
	}
	return InPointerEventData.cumulativeMoveDelta.Size() > calculatedThreshold;
}
