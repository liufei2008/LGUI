// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Event/Rayemitter/LGUI_SceneComponentRayemitter.h"
#include "LGUI.h"
#include "GameFramework/Actor.h"

bool ULGUI_SceneComponentRayemitter::EmitRay(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, TArray<AActor*>& InOutTraceOnlyActors, TArray<AActor*>& InOutTraceIgnoreActors)
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
bool ULGUI_SceneComponentRayemitter::ShouldStartDrag(ULGUIPointerEventData* InPointerEventData)
{
	if (ShouldStartDrag_HoldToDrag(InPointerEventData))return true;
	auto calculatedThreshold = clickTresholdSquare;
	if (clickThresholdRelateToRayDistance)
	{
		calculatedThreshold *=
			InPointerEventData->pressDistance
			* 0.01f;//1% of pressDistance
	}
	auto dragDistance = (InPointerEventData->GetWorldPointSpherical() - InPointerEventData->pressWorldPoint).Size();
	return dragDistance > calculatedThreshold;
}
void ULGUI_SceneComponentRayemitter::SetTargetActor(AActor* InActor)
{
	TargetActor = InActor;
	if (TargetActor != nullptr)
	{
		CacheTargetSceneComponent = TargetActor->GetRootComponent();
	}
}
