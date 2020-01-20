// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Event/LGUIScreenSpaceInteraction.h"
#include "Event/Rayemitter/LGUI_ScreenSpaceUIMouseRayemitter.h"
#include "Core/ActorComponent/UIItem.h"

ULGUIScreenSpaceInteraction::ULGUIScreenSpaceInteraction()
{
	
}
void ULGUIScreenSpaceInteraction::CheckRayemitter()
{
	if (!IsValid(rayEmitter))
	{
		if (auto actor = GetOwner())
		{
			auto emitter = NewObject<ULGUI_ScreenSpaceUIMouseRayemitter>(actor);
			emitter->SetClickThreshold(clickThreshold);
			emitter->RegisterComponent();
			rayEmitter = emitter;
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
