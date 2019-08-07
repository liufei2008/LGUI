// Copyright 2019 LexLiu. All Rights Reserved.

#include "Event/LGUIScreenSpaceInteraction.h"
#include "Event/Rayemitter/LGUI_SceneCapture2DMouseRayemitter.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/SceneCapture2D.h"

ULGUIScreenSpaceInteraction::ULGUIScreenSpaceInteraction()
{
	
}
void ULGUIScreenSpaceInteraction::CheckRayemitter()
{
	if (rayEmitter == nullptr)
	{
		if (auto actor = GetOwner())
		{
			if (auto sceneCaptureActor = Cast<ASceneCapture2D>(actor))
			{
				auto emitter = NewObject<ULGUI_SceneCapture2DMouseRayEmitter>(sceneCaptureActor);
				emitter->SetSceneCapture2DComponent(sceneCaptureActor->GetCaptureComponent2D());
				rayEmitter = emitter;
				sceneCaptureActor->FinishAndRegisterComponent(emitter);
			}
			else
			{
				UE_LOG(LGUI, Error, TEXT("[ULGUIScreenSpaceInteraction::CheckRayemitter]This component(LGUIScreenSpaceInteraction) should be attached to a SceneCapture2D actor!"));
			}
		}
	}
}
bool ULGUIScreenSpaceInteraction::Raycast(FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, FHitResult& OutHitResult)
{
	CheckRayemitter();
	return Super::Raycast(OutRayOrigin, OutRayDirection, OutRayEnd, OutHitResult);
}
