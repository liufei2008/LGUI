// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LGUIBaseRaycaster.h"
#include "LGUI_WorldRaycaster.generated.h"

//RayEmitter must be set for raycaster, or it will not work
UCLASS(ClassGroup = LGUI, meta = (BlueprintSpawnableComponent), Blueprintable)
class LGUI_API ULGUI_WorldRaycaster : public ULGUIBaseRaycaster
{
	GENERATED_BODY()
	
public:	
	ULGUI_WorldRaycaster();
protected:
	//temp array, hit result
	TArray<FHitResult> multiWorldHitResult;
public:
	virtual bool Raycast(FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, FHitResult& OutHitResult)override;
};
