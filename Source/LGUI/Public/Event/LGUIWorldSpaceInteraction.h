// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Raycaster/LGUI_UIRaycaster.h"
#include "LGUIWorldSpaceInteraction.generated.h"

//The interaction source for world space UI, actually the ray emitter.
UENUM(BlueprintType)
enum class ELGUIWorldSpaceInteractionSource :uint8
{
	/** Sends traces from the world location and orientation of the interaction component. */
	World,
	/** Sends traces from the mouse location of the first local player controller. */
	Mouse,
	/** Sends trace from the center of the first local player's screen. */
	CenterScreen,
	/**
	 * Sends traces from a custom location determined by the user.  Will use whatever
	 * FHitResult is set by the call to SetCustomHitResult.
	 */
	//Custom
};
//Perform a preset raycaster interaction for WorldSpaceUI.
//When hit play, a Rayemitter will be created depend on interactionSource.
UCLASS(ClassGroup = LGUI, meta = (BlueprintSpawnableComponent), Blueprintable)
class LGUI_API ULGUIWorldSpaceInteraction : public ULGUI_UIRaycaster
{
	GENERATED_BODY()
	
public:	
	ULGUIWorldSpaceInteraction();
protected:
	//click/drag threshold, calculated in target's local space
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = LGUI)
		float clickThreshold = 5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		ELGUIWorldSpaceInteractionSource interactionSource = ELGUIWorldSpaceInteractionSource::Mouse;
	void CheckRayemitter();
	virtual bool ShouldSkipUIItem(class UUIItem* UIItem)override;
public:
	virtual bool Raycast(FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, FHitResult& OutHitResult)override;
};
