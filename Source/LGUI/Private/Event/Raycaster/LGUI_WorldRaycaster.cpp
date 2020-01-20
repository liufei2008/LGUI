// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Event/Raycaster/LGUI_WorldRaycaster.h"

ULGUI_WorldRaycaster::ULGUI_WorldRaycaster()
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

bool ULGUI_WorldRaycaster::Raycast(FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, FHitResult& OutHitResult)
{
	traceIgnoreActorArray.Reset();
	traceOnlyActorArray.Reset();
	if (GenerateRay(OutRayOrigin, OutRayDirection, traceOnlyActorArray, traceIgnoreActorArray))
	{
		multiWorldHitResult.Reset();
		OutRayEnd = OutRayDirection * rayLength + OutRayOrigin;
		if (traceOnlyActorArray.Num() == 0)
		{
			FCollisionQueryParams queryParams = FCollisionQueryParams::DefaultQueryParam;
			queryParams.AddIgnoredActors(traceIgnoreActorArray);
			this->GetWorld()->LineTraceMultiByChannel(multiWorldHitResult, OutRayOrigin, OutRayEnd, UEngineTypes::ConvertToCollisionChannel(traceChannel), queryParams);
		}
		else
		{
			ActorLineTraceMulti(multiWorldHitResult, true, traceOnlyActorArray, OutRayOrigin, OutRayEnd, UEngineTypes::ConvertToCollisionChannel(traceChannel));
		}
		int hitCount = multiWorldHitResult.Num();
		if (hitCount > 0)
		{
			//this->GetWorld()->LineTraceMultiByChannel() result is sorted
			//multiWorldHitResult.Sort([](const FHitResult& A, const FHitResult& B)//sort on depth
			//{
			//	return A.Distance < B.Distance;
			//});
			OutHitResult = multiWorldHitResult[0];
			return true;
		}
	}
	return false;
}