// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Event/LGUIScreenSpaceInteraction.h"
#include "Event/Rayemitter/LGUI_ScreenSpaceUIMouseRayemitter.h"
#include "Core/ActorComponent/UIItem.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "LGUI.h"
#include "Utils/LGUIUtils.h"

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
			if (IsValid(renderCanvas) && renderCanvas->IsRootCanvas())
			{
				auto emitter = NewObject<ULGUI_ScreenSpaceUIMouseRayemitter>(actor);
				emitter->SetInitialValue(clickThreshold, holdToDrag, holdToDragTime);
				emitter->SetRenderCanvas(renderCanvas);
				emitter->RegisterComponent();
				actor->AddInstanceComponent(emitter);
				rayEmitter = emitter;
			}
		}
		if (!IsValid(rayEmitter))
		{
			auto msg = FString(TEXT("LGUIScreenSpaceInteraction component should be placed on ScreenSpaceUIRoot."));
#if WITH_EDITOR
			LGUIUtils::EditorNotification(FText::FromString(msg));
#endif
			UE_LOG(LGUI, Error, TEXT("%s"), *msg);
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
bool ULGUIScreenSpaceInteraction::Raycast(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, FHitResult& OutHitResult, TArray<USceneComponent*>& OutHoverArray)
{
	CheckRayemitter();
	return Super::Raycast(InPointerEventData, OutRayOrigin, OutRayDirection, OutRayEnd, OutHitResult, OutHoverArray);
}
void ULGUIScreenSpaceInteraction::SetClickThreshold(float value)
{
	if (IsValid(rayEmitter))
	{
		rayEmitter->SetInitialValue(clickThreshold, holdToDrag, holdToDragTime);
	}
}
void ULGUIScreenSpaceInteraction::SetHoldToDrag(bool value)
{
	if (IsValid(rayEmitter))
	{
		rayEmitter->SetInitialValue(clickThreshold, holdToDrag, holdToDragTime);
	}
}
void ULGUIScreenSpaceInteraction::SetHoldToDragTime(float value)
{
	if (IsValid(rayEmitter))
	{
		rayEmitter->SetInitialValue(clickThreshold, holdToDrag, holdToDragTime);
	}
}
