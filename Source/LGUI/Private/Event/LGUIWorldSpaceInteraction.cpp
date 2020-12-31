// Copyright 2019-2021 LexLiu. All Rights Reserved.

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
		return !UIItem->IsWorldSpaceUI();
	}
	return true;
}
void ULGUIWorldSpaceInteraction::CheckRayemitter()
{
	if (!IsValid(rayEmitter))
	{
		if (auto actor = GetOwner())
		{
			switch (interactionSource)
			{
			default:
			case ELGUIWorldSpaceInteractionSource::World:
			{
				auto emitter = NewObject<ULGUI_SceneComponentRayEmitter>(actor);
				emitter->SetTargetSceneComponent(this);
				rayEmitter = emitter;
			}
			break;
			case ELGUIWorldSpaceInteractionSource::Mouse:
			{
				rayEmitter = NewObject<ULGUI_MainViewportMouseRayEmitter>(actor);
			}
			break;
			case ELGUIWorldSpaceInteractionSource::CenterScreen:
			{
				rayEmitter = NewObject<ULGUI_CenterScreenRayemitter>(actor);
			}
			break;
			}
			rayEmitter->SetInitialValue(clickThreshold, holdToDrag, holdToDragTime);
			actor->FinishAndRegisterComponent(rayEmitter);
		}
	}
}
bool ULGUIWorldSpaceInteraction::Raycast(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, FHitResult& OutHitResult)
{
	CheckRayemitter();
	return Super::Raycast(InPointerEventData, OutRayOrigin, OutRayDirection, OutRayEnd, OutHitResult);
}
