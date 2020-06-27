// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Event/LGUIWorldSpaceInteraction.h"
#include "Event/Rayemitter/LGUI_MainViewportMouseRayemitter.h"
#include "Event/Rayemitter/LGUI_SceneComponentRayemitter.h"
#include "Event/Rayemitter/LGUI_CenterScreenRayemitter.h"
#include "Core/ActorComponent/UIItem.h"

ULGUIWorldSpaceInteraction::ULGUIWorldSpaceInteraction()
{
	
}
bool ULGUIWorldSpaceInteraction::ShouldSkipUIItem(class UUIItem* UIItem)
{
	if (UIItem != nullptr)
	{
		return UIItem->IsScreenSpaceOverlayUI();
	}
	return true;
}
void ULGUIWorldSpaceInteraction::CheckRayemitter()
{
	if (!IsValid(rayEmitter))
	{
		switch (interactionSource)
		{
		default:
		case ELGUIWorldSpaceInteractionSource::World:
		{
			auto emitter = NewObject<ULGUI_SceneComponentRayEmitter>(this);
			emitter->SetTargetSceneComponent(this);
			rayEmitter = emitter;
		}
		break;
		case ELGUIWorldSpaceInteractionSource::Mouse:
		{
			rayEmitter = NewObject<ULGUI_MainViewportMouseRayEmitter>(this);
		}
		break;
		case ELGUIWorldSpaceInteractionSource::CenterScreen:
		{
			rayEmitter = NewObject<ULGUI_CenterScreenRayemitter>(this);
		}
		break;
		}
		rayEmitter->SetInitialValue(clickThreshold, holdToDrag, holdToDragTime);
	}
}
bool ULGUIWorldSpaceInteraction::Raycast(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, FHitResult& OutHitResult)
{
	CheckRayemitter();
	return Super::Raycast(InPointerEventData, OutRayOrigin, OutRayDirection, OutRayEnd, OutHitResult);
}
