// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Raycaster/LGUI_UIRaycaster.h"
#include "LGUIScreenSpaceInteraction.generated.h"

//Perform a preset raycaster interaction for ScreenSpaceUI with SceneCapture2D.
//This component should be placed on a SceneCapture2D.
//When hit play, a LGUI_SceneCapture2DMouseRayemitter will be created.
UCLASS(ClassGroup = LGUI, meta = (BlueprintSpawnableComponent), Blueprintable)
class LGUI_API ULGUIScreenSpaceInteraction : public ULGUI_UIRaycaster
{
	GENERATED_BODY()
	
public:	
	ULGUIScreenSpaceInteraction();
protected:
	void CheckRayemitter();
public:
	virtual bool Raycast(FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, FHitResult& OutHitResult)override;
};
