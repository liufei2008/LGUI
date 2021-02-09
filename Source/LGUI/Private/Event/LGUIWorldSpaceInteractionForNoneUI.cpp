// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Event/LGUIWorldSpaceInteractionForNoneUI.h"
#include "Event/Rayemitter/LGUI_MainViewportMouseRayemitter.h"
#include "Event/Rayemitter/LGUI_SceneComponentRayemitter.h"
#include "Event/Rayemitter/LGUI_CenterScreenRayemitter.h"
#include "GameFramework/Actor.h"

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
bool ULGUIWorldSpaceInteractionForNoneUI::Raycast(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, FHitResult& OutHitResult, TArray<USceneComponent*>& OutHoverArray)
{
	CheckRayemitter();
	return Super::Raycast(InPointerEventData, OutRayOrigin, OutRayDirection, OutRayEnd, OutHitResult, OutHoverArray);
}

void ULGUIWorldSpaceInteractionForNoneUI::SetClickThreshold(float value)
{
	if (IsValid(rayEmitter))
	{
		rayEmitter->SetInitialValue(clickThreshold, holdToDrag, holdToDragTime);
	}
}
void ULGUIWorldSpaceInteractionForNoneUI::SetHoldToDrag(bool value)
{
	if (IsValid(rayEmitter))
	{
		rayEmitter->SetInitialValue(clickThreshold, holdToDrag, holdToDragTime);
	}
}
void ULGUIWorldSpaceInteractionForNoneUI::SetHoldToDragTime(float value)
{
	if (IsValid(rayEmitter))
	{
		rayEmitter->SetInitialValue(clickThreshold, holdToDrag, holdToDragTime);
	}
}
void ULGUIWorldSpaceInteractionForNoneUI::SetInteractionSource(ELGUIWorldSpaceInteractionSource value)
{
	if (interactionSource != value)
	{
		interactionSource = value;
		rayEmitter = nullptr;
	}
}
