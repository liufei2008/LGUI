// Copyright 2019-2020 LexLiu. All Rights Reserved.

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
bool ULGUIWorldSpaceInteractionForNoneUI::Raycast(FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, FHitResult& OutHitResult)
{
	CheckRayemitter();
	return Super::Raycast(OutRayOrigin, OutRayDirection, OutRayEnd, OutHitResult);
}
