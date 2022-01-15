// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Event/InteractionSource/LGUIWorldSpaceInteractionSource_World.h"
#include "LGUI.h"
#include "GameFramework/Actor.h"

bool ULGUIWorldSpaceInteractionSource_World::EmitRay(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection)
{
	if (!CacheTargetSceneComponent.IsValid())
	{
		return false;
	}

	OutRayOrigin = CacheTargetSceneComponent->GetComponentLocation();
	switch (RayDirectionType)
	{
	case ELGUISceneComponentDirection::PositiveX:
		OutRayDirection = CacheTargetSceneComponent->GetForwardVector();
		break;
	case ELGUISceneComponentDirection::NagtiveX:
		OutRayDirection = -CacheTargetSceneComponent->GetForwardVector();
		break;
	case ELGUISceneComponentDirection::PositiveY:
		OutRayDirection = CacheTargetSceneComponent->GetRightVector();
		break;
	case ELGUISceneComponentDirection::NagtiveY:
		OutRayDirection = -CacheTargetSceneComponent->GetRightVector();
		break;
	case ELGUISceneComponentDirection::PositiveZ:
		OutRayDirection = CacheTargetSceneComponent->GetUpVector();
		break;
	case ELGUISceneComponentDirection::NagtiveZ:
		OutRayDirection = -CacheTargetSceneComponent->GetUpVector();
		break;
	}
	return true;
}
bool ULGUIWorldSpaceInteractionSource_World::ShouldStartDrag(ULGUIPointerEventData* InPointerEventData)
{
	if (auto IterObj = GetInteractionObject())
	{
		auto calculatedThreshold = IterObj->GetClickThresholdSquare();
		if (clickThresholdRelateToRayDistance)
		{
			calculatedThreshold *= InPointerEventData->pressDistance * rayDistanceMultiply;
		}
		auto dragDistance = (InPointerEventData->GetWorldPointSpherical() - InPointerEventData->pressWorldPoint).Size();
		return dragDistance > calculatedThreshold;
	}
	return false;
}
