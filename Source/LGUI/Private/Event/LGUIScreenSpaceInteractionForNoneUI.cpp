// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Event/LGUIScreenSpaceInteractionForNoneUI.h"
#include "Event/Rayemitter/LGUI_ScreenSpaceUIMouseRayemitter.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "LGUI.h"

ULGUIScreenSpaceInteractionForNoneUI::ULGUIScreenSpaceInteractionForNoneUI()
{
	
}
void ULGUIScreenSpaceInteractionForNoneUI::CheckRayemitter()
{
	if (!IsValid(rayEmitter))
	{
		if (auto actor = GetOwner())
		{
			auto renderCanvas = GetOwner()->FindComponentByClass<ULGUICanvas>();
			if (IsValid(renderCanvas))
			{
				auto emitter = NewObject<ULGUI_ScreenSpaceUIMouseRayemitter>(actor);
				emitter->SetInitialValue(clickThreshold, holdToDrag, holdToDragTime);
				emitter->SetRenderCanvas(renderCanvas);
				actor->FinishAndRegisterComponent(emitter);
				rayEmitter = emitter;
			}
		}
		if (!IsValid(rayEmitter))
		{
			UE_LOG(LGUI, Error, TEXT("This component should be placed on a actor which have a LGUICanvas, and RenderMode of LGUICanvas should set to ScreenSpace."));
		}
	}
}
bool ULGUIScreenSpaceInteractionForNoneUI::Raycast(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, FHitResult& OutHitResult)
{
	CheckRayemitter();
	return Super::Raycast(InPointerEventData, OutRayOrigin, OutRayDirection, OutRayEnd, OutHitResult);
}
