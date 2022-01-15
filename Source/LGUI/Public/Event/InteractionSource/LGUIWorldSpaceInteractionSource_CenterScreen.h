// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Event/LGUIWorldSpaceInteraction.h"
#include "LGUIWorldSpaceInteractionSource_CenterScreen.generated.h"

/** 
 * Sends trace from the center of the first local player's screen
 */
UCLASS(ClassGroup = LGUI, Blueprintable, meta = (DisplayName = "Center Screen"))
class LGUI_API ULGUIWorldSpaceInteractionSource_CenterScreen : public ULGUIWorldSpaceInteractionSource
{
	GENERATED_BODY()

public:
	virtual bool EmitRay(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection)override;
	virtual bool ShouldStartDrag(ULGUIPointerEventData* InPointerEventData)override;
};
