// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Event/LGUIWorldSpaceRaycaster.h"
#include "LGUIWorldSpaceRaycasterSource_CenterScreen.generated.h"

/** 
 * Sends trace from the center of the first local player's screen
 */
UCLASS(ClassGroup = LGUI, Blueprintable, meta = (DisplayName = "Center Screen"))
class LGUI_API ULGUIWorldSpaceRaycasterSource_CenterScreen : public ULGUIWorldSpaceRaycasterSource
{
	GENERATED_BODY()

public:
	virtual bool GenerateRay(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection)override;
	virtual bool ShouldStartDrag(ULGUIPointerEventData* InPointerEventData)override;
};
