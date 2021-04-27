// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Event/LGUIPointerEventData.h"
#include "Event/Raycaster/LGUIBaseRaycaster.h"
#include "Event/Rayemitter/LGUIBaseRayemitter.h"
#include "LGUI.h"
#include "GameFramework/Actor.h"


FVector ULGUIPointerEventData::GetWorldPointInPlane()const
{
	switch (eventType)
	{
	default:
		return worldPoint;
		break;
	case EPointerEventType::Down:
		return pressWorldPoint;
		break;
	case EPointerEventType::Up:
	case EPointerEventType::Click:
	case EPointerEventType::BeginDrag:
	case EPointerEventType::Drag:
	case EPointerEventType::EndDrag:
	{
		auto dragRayOrigin = GetDragRayOrigin();
		auto dragRayDirection = GetDragRayDirection();
		return FMath::LinePlaneIntersection(dragRayOrigin, dragRayOrigin + dragRayDirection * pressRaycaster->rayLength, pressWorldPoint, pressWorldNormal);
	}
		break;
	}

}
FVector ULGUIPointerEventData::GetLocalPointInPlane()const
{
	return pressWorldToLocalTransform.TransformPosition(GetWorldPointInPlane());
}
FVector ULGUIPointerEventData::GetWorldPointSpherical()const
{
	switch (eventType)
	{
	default:
		return worldPoint;
		break;
	case EPointerEventType::Down:
		return pressWorldPoint;
		break;
	case EPointerEventType::Up:
	case EPointerEventType::Click:
	case EPointerEventType::BeginDrag:
	case EPointerEventType::Drag:
	case EPointerEventType::EndDrag:
		return pressRaycaster->rayEmitter->GetCurrentRayOrigin() + pressRaycaster->rayEmitter->GetCurrentRayDirection() * pressDistance;//calculated approximate hit position
		break;
	}
}
FVector ULGUIPointerEventData::GetDragRayOrigin()const
{
	switch (eventType)
	{
	default:
		return worldPoint;
		break;
	case EPointerEventType::Down:
		return pressWorldPoint;
		break;
	case EPointerEventType::Up:
	case EPointerEventType::Click:
	case EPointerEventType::BeginDrag:
	case EPointerEventType::Drag:
	case EPointerEventType::EndDrag:
		return pressRaycaster->rayEmitter->GetCurrentRayOrigin();
		break;
	}
}
FVector ULGUIPointerEventData::GetDragRayDirection()const
{
	switch (eventType)
	{
	default:
		return worldPoint;
		break;
	case EPointerEventType::Down:
		return pressWorldPoint;
		break;
	case EPointerEventType::Up:
	case EPointerEventType::Click:
	case EPointerEventType::BeginDrag:
	case EPointerEventType::Drag:
	case EPointerEventType::EndDrag:
		return pressRaycaster->rayEmitter->GetCurrentRayDirection();
		break;
	}
}
FVector ULGUIPointerEventData::GetCumulativeMoveDelta()const
{
	return GetWorldPointSpherical() - pressWorldPoint;
}

FString ULGUIPointerEventData::ToString()const
{
	FString result;
	if (IsValid(enterComponent))
	{
		result += FString::Printf(TEXT("\n		enterComponent actor:%s, comp:%s"),
#if WITH_EDITOR
			* (enterComponent->GetOwner()->GetActorLabel()),
#else
			* (enterComponent->GetOwner()->GetName()),
#endif
			* (enterComponent->GetPathName()));
	}
	else
	{
		result += TEXT("\n		enterComponent is null");
	}
	if (enterComponentStack.Num() > 0)
	{
		result += FString::Printf(TEXT("\n		enterActorStack count:%d"), enterComponentStack.Num());
	}
	else
	{
		result += TEXT("\n		enterActorStack empty");
	}
	if (IsValid(dragComponent))
	{
		result += FString::Printf(TEXT("\n		dragComponent actor:%s, comp:%s"),
#if WITH_EDITOR
			* (dragComponent->GetOwner()->GetActorLabel()),
#else
			* (dragComponent->GetOwner()->GetName()),
#endif
			* (dragComponent->GetPathName()));
	}
	else
	{
		result += TEXT("\n		dragComponent is null");
	}
	result += FString::Printf(TEXT("\n		worldPoint:%s"), *(worldPoint.ToString()));
	result += FString::Printf(TEXT("\n		moveDelta:%s"), *(worldNormal.ToString()));

	result += FString::Printf(TEXT("\n		scrollAxisValue:%f"), scrollAxisValue);

	result += FString::Printf(TEXT("\n		rayOrigin:%s"), *(rayOrigin.ToString()));
	result += FString::Printf(TEXT("\n		rayDirection:%s"), *(rayDirection.ToString()));
	result += FString::Printf(TEXT("\n		raycaster:%s"), *(IsValid(raycaster) ? raycaster->GetName() : TEXT("null")));
	switch (mouseButtonType)
	{
	case EMouseButtonType::Left:
		result += TEXT("\n		mouseButtonType:Left");
		break;
	case EMouseButtonType::Middle:
		result += TEXT("\n		mouseButtonType:Middle");
		break;
	case EMouseButtonType::Right:
		result += TEXT("\n		mouseButtonType:Right");
		break;
	}

	result += FString::Printf(TEXT("\n		pressComponent:%s"), *(IsValid(pressComponent) ? pressComponent->GetName() : TEXT("null")));
	result += FString::Printf(TEXT("\n		pressWorldPoint:%s"), *(pressWorldPoint.ToString()));
	result += FString::Printf(TEXT("\n		pressWorldNormal:%s"), *(pressWorldNormal.ToString()));
	result += FString::Printf(TEXT("\n		pressDistance:%f"), pressDistance);
	result += FString::Printf(TEXT("\n		pressRayOrigin:%s"), *(pressRayOrigin.ToString()));
	result += FString::Printf(TEXT("\n		pressRayDirection:%s"), *(pressRayDirection.ToString()));
	result += FString::Printf(TEXT("\n		pressRaycaster:%s"), *(IsValid(pressRaycaster) ? pressRaycaster->GetName() : TEXT("null")));
	result += FString::Printf(TEXT("\n		clickTime:%f"), clickTime);
	result += FString::Printf(TEXT("\n		pressTime:%f"), pressTime);

	result += FString::Printf(TEXT("\n		isDragging:%s"), isDragging ? TEXT("true") : TEXT("false"));
	result += FString::Printf(TEXT("\n		dragComponent:%s"), *(IsValid(dragComponent) ? dragComponent->GetName() : TEXT("null")));

	switch (eventType)
	{
	case EPointerEventType::Click:
		result += TEXT("\n		eventType:Click");
		break;
	case EPointerEventType::Enter:
		result += TEXT("\n		eventType:Enter");
		break;
	case EPointerEventType::Exit:
		result += TEXT("\n		eventType:Exit");
		break;
	case EPointerEventType::Down:
		result += TEXT("\n		eventType:Down");
		break;
	case EPointerEventType::Up:
		result += TEXT("\n		eventType:Up");
		break;
	case EPointerEventType::BeginDrag:
		result += TEXT("\n		eventType:BeginDrag");
		break;
	case EPointerEventType::Drag:
		result += TEXT("\n		eventType:Drag");
		break;
	case EPointerEventType::EndDrag:
		result += TEXT("\n		eventType:EndDrag");
		break;
	case EPointerEventType::Scroll:
		result += TEXT("\n		eventType:Scroll");
		break;
	case EPointerEventType::DragDrop:
		result += TEXT("\n		eventType:DragDrop");
		break;
	case EPointerEventType::Select:
		result += TEXT("\n		eventType:Select");
		break;
	case EPointerEventType::Deselect:
		result += TEXT("\n		eventType:Deselect");
		break;
	}



	if (IsValid(pressComponent))
	{
		result += FString::Printf(TEXT("\n		pressHitComponent actor:%s, comp:%s"),
#if WITH_EDITOR
			* (pressComponent->GetOwner()->GetActorLabel()),
#else
			* (pressComponent->GetOwner()->GetName()),
#endif
			* (pressComponent->GetPathName()));
	}
	else
	{
		result += TEXT("\n		pressHitComponent is null");
	}
	result += FString::Printf(TEXT("\n		pressWorldPoint:%s"), *(pressWorldPoint.ToString()));
	result += FString::Printf(TEXT("\n		pressWorldNormal:%s"), *(pressWorldNormal.ToString()));
	result += FString::Printf(TEXT("\n		pressDistance:%f"), pressDistance);
	result += FString::Printf(TEXT("\n		pressRayOrigin:%s"), *(pressRayOrigin.ToString()));
	result += FString::Printf(TEXT("\n		pressRayDirection:%s"), *(pressRayDirection.ToString()));
	result += FString::Printf(TEXT("\n		pressRaycaster:%s"), *(IsValid(pressRaycaster) ? pressRaycaster->GetName() : TEXT("null")));
	result += FString::Printf(TEXT("\n		pressTime:%f"), clickTime);

	return result;
}
