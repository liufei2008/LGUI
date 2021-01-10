// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Raycaster/LGUI_WorldRaycaster.h"
#include "LGUIScreenSpaceInteractionForNoneUI.generated.h"

/**
 * Perform a preset raycaster interaction for ScreenSpaceUI for none UI.
 * This component should be placed on a actor which have a LGUICanvas, and RenderMode of LGUICanvas should set to ScreenSpace.
 * When hit play, a LGUI_ScreenSpaceUIMouseRayemitter will be created.
 */
UCLASS(ClassGroup = LGUI, meta = (BlueprintSpawnableComponent), Blueprintable)
class LGUI_API ULGUIScreenSpaceInteractionForNoneUI : public ULGUI_WorldRaycaster
{
	GENERATED_BODY()
	
public:	
	ULGUIScreenSpaceInteractionForNoneUI();
protected:
	//click/drag threshold, calculated in target's local space
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = LGUI)
		float clickThreshold = 5;
	//hold press for a little while to entering drag mode
	UPROPERTY(EditAnywhere, Category = LGUI)
		bool holdToDrag = false;
	//hold press for "holdToDragTime" to entering drag mode
	UPROPERTY(EditAnywhere, Category = LGUI)
		float holdToDragTime = 0.5f;
	void CheckRayemitter();
public:
	virtual bool Raycast(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, FHitResult& OutHitResult, TArray<USceneComponent*>& OutHoverArray)override;
};
