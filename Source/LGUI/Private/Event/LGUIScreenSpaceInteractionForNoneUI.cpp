// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Event/LGUIScreenSpaceInteractionForNoneUI.h"
#include "Event/Rayemitter/LGUI_ScreenSpaceUIMouseRayemitter.h"

ULGUIScreenSpaceInteractionForNoneUI::ULGUIScreenSpaceInteractionForNoneUI()
{
	
}
void ULGUIScreenSpaceInteractionForNoneUI::CheckRayemitter()
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
bool ULGUIScreenSpaceInteractionForNoneUI::Raycast(FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, FHitResult& OutHitResult)
{
	CheckRayemitter();
	return Super::Raycast(OutRayOrigin, OutRayDirection, OutRayEnd, OutHitResult);
}
