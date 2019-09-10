// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Raycaster/LGUI_UIRaycaster.h"
#include "LGUIScreenSpaceInteraction.generated.h"

//Perform a preset raycaster interaction for ScreenSpaceUI.
//This component should be placed on a actor which have a UIRoot, and RenderMode of UIRoot set to ScreenSpace.
//When hit play, a LGUI_ScreenSpaceUIMouseRayemitter will be created.
UCLASS(ClassGroup = LGUI, meta = (BlueprintSpawnableComponent), Blueprintable)
class LGUI_API ULGUIScreenSpaceInteraction : public ULGUI_UIRaycaster
{
	GENERATED_BODY()
	
public:	
	ULGUIScreenSpaceInteraction();
protected:
	//click/drag threshold, calculated in target's local space
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = LGUI)
		float clickThreshold = 5;
	void CheckRayemitter();
	virtual bool ShouldSkipUIItem(class UUIItem* UIItem)override;
public:
	virtual bool Raycast(FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, FHitResult& OutHitResult)override;
};
