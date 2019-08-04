// Copyright 2019 LexLiu. All Rights Reserved.

#include "Event/LGUIWorldSpaceInteraction.h"
#include "Event/Rayemitter/LGUI_MainViewportMouseRayemitter.h"
#include "Event/Rayemitter/LGUI_SceneComponentRayemitter.h"
#include "Event/Rayemitter/LGUI_CenterScreenRayemitter.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/SceneCapture2D.h"

ULGUIWorldSpaceInteraction::ULGUIWorldSpaceInteraction()
{
	
}
void ULGUIWorldSpaceInteraction::CheckRayemitter()
{
	if (rayEmitter == nullptr)
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
			actor->FinishAndRegisterComponent(rayEmitter);
		}
	}
}
bool ULGUIWorldSpaceInteraction::Raycast(FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, FHitResult& OutHitResult)
{
	CheckRayemitter();
	return Super::Raycast(OutRayOrigin, OutRayDirection, OutRayEnd, OutHitResult);
}
