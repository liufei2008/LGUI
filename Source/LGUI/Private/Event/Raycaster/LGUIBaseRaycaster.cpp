// Copyright 2019 LexLiu. All Rights Reserved.

#include "Event/Raycaster/LGUIBaseRaycaster.h"
#include "Event/RayEmitter/LGUIBaseRayEmitter.h"
#include "Event/RayEmitter/LGUI_MainViewportMouseRayEmitter.h"
#include "Event/RayEmitter/LGUI_SceneCapture2DMouseRayEmitter.h"
#include "Event/RayEmitter/LGUI_SceneComponentRayEmitter.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/SceneCapture2D.h"
#include "Core/ActorComponent/UIItem.h"

ULGUIBaseRaycaster::ULGUIBaseRaycaster()
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
}
void ULGUIBaseRaycaster::BeginPlay()
{
	Super::BeginPlay();
	ActivateRaycaster();
}
void ULGUIBaseRaycaster::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	DeactivateRaycaster();
}
void ULGUIBaseRaycaster::OnRegister()
{
	Super::OnRegister();
}
void ULGUIBaseRaycaster::OnUnregister()
{
	Super::OnUnregister();
}
void ULGUIBaseRaycaster::ActivateRaycaster()
{
	ALGUIManagerActor::AddRaycaster(this);
}
void ULGUIBaseRaycaster::DeactivateRaycaster()
{
	ALGUIManagerActor::RemoveRaycaster(this);
}
bool ULGUIBaseRaycaster::GenerateRay(FVector& OutRayOrigin, FVector& OutRayDirection, TArray<AActor*>& OutTraceOnlyActors, TArray<AActor*>& OutTraceIgnoreActors)
{
	if (rayEmitter == nullptr)
	{
		UE_LOG(LGUI, Warning, TEXT("[ULGUIBaseRaycaster::GenerateRay]Raycaster:%s ray emitter is not set! This raycaster will not work without a rayemitter!"), *(this->GetPathName()));
		return false;
	}
	return rayEmitter->EmitRay(OutRayOrigin, OutRayDirection, OutTraceOnlyActors, OutTraceIgnoreActors);
}

void ULGUIBaseRaycaster::ActorLineTraceMulti(TArray<FHitResult>& OutHitArray, bool InSortResult, const TArray<AActor*>& InActorArray, const FVector& InRayOrign, const FVector& InRayEnd, ECollisionChannel InTraceChannel, const struct FCollisionQueryParams& InParams)
{
	for (int actorIndex = 0, actorCount = InActorArray.Num(); actorIndex < actorCount; actorIndex++)
	{
		auto actor = InActorArray[actorIndex];
		if (IsValid(actor))
		{
			FHitResult thisHit;
			if (actor->ActorLineTraceSingle(thisHit, InRayOrign, InRayEnd, InTraceChannel, InParams))
			{
				OutHitArray.Add(thisHit);
			}
		}
	}
	if (InSortResult)
	{
		OutHitArray.Sort([](const FHitResult& A, const FHitResult& B)//sort on depth
		{
			return A.Distance < B.Distance;
		});
	}
}

bool ULGUIBaseRaycaster::Raycast(FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, FHitResult& OutHitResult)
{
	UE_LOG(LGUI, Error, TEXT("[LGUI_UIRaycaster]Function Raycast must be implemented!"));
	return false;
}