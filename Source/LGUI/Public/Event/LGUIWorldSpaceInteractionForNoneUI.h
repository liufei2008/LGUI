// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Raycaster/LGUI_WorldRaycaster.h"
#include "Event/LGUIWorldSpaceInteraction.h"
#include "LGUIWorldSpaceInteractionForNoneUI.generated.h"

//Perform a preset raycaster interaction for WorldSpaceUI.
//When hit play, a Rayemitter will be created depend on interactionSource.
UCLASS(ClassGroup = LGUI, meta = (BlueprintSpawnableComponent), Blueprintable)
class LGUI_API ULGUIWorldSpaceInteractionForNoneUI : public ULGUI_WorldRaycaster
{
	GENERATED_BODY()
	
public:	
	ULGUIWorldSpaceInteractionForNoneUI();
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		ELGUIWorldSpaceInteractionSource interactionSource = ELGUIWorldSpaceInteractionSource::Mouse;
	void CheckRayemitter();
public:
	virtual bool Raycast(FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, FHitResult& OutHitResult)override;
};
