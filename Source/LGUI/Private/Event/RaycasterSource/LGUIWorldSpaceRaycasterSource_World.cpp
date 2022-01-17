// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Event/RaycasterSource/LGUIWorldSpaceRaycasterSource_World.h"
#include "LGUI.h"
#include "GameFramework/Actor.h"

bool ULGUIWorldSpaceRaycasterSource_World::GenerateRay(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection)
{
	if (auto IterObj = GetRaycasterObject())
	{
		OutRayOrigin = IterObj->GetComponentLocation();
		switch (RayDirectionType)
		{
		case ELGUISceneComponentDirection::PositiveX:
			OutRayDirection = IterObj->GetForwardVector();
			break;
		case ELGUISceneComponentDirection::NagtiveX:
			OutRayDirection = -IterObj->GetForwardVector();
			break;
		case ELGUISceneComponentDirection::PositiveY:
			OutRayDirection = IterObj->GetRightVector();
			break;
		case ELGUISceneComponentDirection::NagtiveY:
			OutRayDirection = -IterObj->GetRightVector();
			break;
		case ELGUISceneComponentDirection::PositiveZ:
			OutRayDirection = IterObj->GetUpVector();
			break;
		case ELGUISceneComponentDirection::NagtiveZ:
			OutRayDirection = -IterObj->GetUpVector();
			break;
		}
		return true;
	}
	return false;
}
bool ULGUIWorldSpaceRaycasterSource_World::ShouldStartDrag(ULGUIPointerEventData* InPointerEventData)
{
	if (auto IterObj = GetRaycasterObject())
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
