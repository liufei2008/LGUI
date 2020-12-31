// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Event/LGUIWorldSpaceInteractionForNoneUI.h"
#include "Event/Rayemitter/LGUI_MainViewportMouseRayemitter.h"
#include "Event/Rayemitter/LGUI_SceneComponentRayemitter.h"
#include "Event/Rayemitter/LGUI_CenterScreenRayemitter.h"

ULGUIWorldSpaceInteractionForNoneUI::ULGUIWorldSpaceInteractionForNoneUI()
{
	
}
void ULGUIWorldSpaceInteractionForNoneUI::CheckRayemitter()
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
bool ULGUIWorldSpaceInteractionForNoneUI::Raycast(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, FHitResult& OutHitResult)
{
	CheckRayemitter();
	return Super::Raycast(InPointerEventData, OutRayOrigin, OutRayDirection, OutRayEnd, OutHitResult);
}
