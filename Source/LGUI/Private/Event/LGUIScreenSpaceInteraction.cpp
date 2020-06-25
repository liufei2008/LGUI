// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Event/LGUIScreenSpaceInteraction.h"
#include "Event/Rayemitter/LGUI_ScreenSpaceUIMouseRayemitter.h"
#include "Core/ActorComponent/UIItem.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "LGUI.h"

ULGUIScreenSpaceInteraction::ULGUIScreenSpaceInteraction()
{
	
}
void ULGUIScreenSpaceInteraction::CheckRayemitter()
{
	if (!IsValid(rayEmitter))
	{
		if (auto actor = GetOwner())
		{
			auto renderCanvas = GetOwner()->FindComponentByClass<ULGUICanvas>();
			if (IsValid(renderCanvas))
			{
				auto emitter = NewObject<ULGUI_ScreenSpaceUIMouseRayemitter>(this);
				emitter->SetClickThreshold(clickThreshold);
				emitter->SetRenderCanvas(renderCanvas);
				rayEmitter = emitter;
			}
		}
		if (!IsValid(rayEmitter))
		{
			UE_LOG(LGUI, Error, TEXT("This component should be placed on a actor which have a LGUICanvas, and RenderMode of LGUICanvas should set to ScreenSpace."));
		}
	}
}
bool ULGUIScreenSpaceInteraction::ShouldSkipUIItem(UUIItem* UIItem)
{
	if (UIItem != nullptr)
	{
		return !UIItem->IsScreenSpaceOverlayUI();
	}
	return true;
}
bool ULGUIScreenSpaceInteraction::Raycast(FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, FHitResult& OutHitResult)
{
	CheckRayemitter();
	return Super::Raycast(OutRayOrigin, OutRayDirection, OutRayEnd, OutHitResult);
}
