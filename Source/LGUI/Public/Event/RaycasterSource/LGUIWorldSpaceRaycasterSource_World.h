// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Event/LGUIWorldSpaceRaycaster.h"
#include "LGUIWorldSpaceRaycasterSource_World.generated.h"



UENUM(BlueprintType, Category = LGUI)
enum class ELGUISceneComponentDirection :uint8
{
	PositiveX		UMETA(DisplayName = "X+"),
	NagtiveX		UMETA(DisplayName = "X-"),
	PositiveY		UMETA(DisplayName = "Y+"),
	NagtiveY		UMETA(DisplayName = "Y-"),
	PositiveZ		UMETA(DisplayName = "Z+"),
	NagtiveZ		UMETA(DisplayName = "Z-"),
};

/**
 * If VR mode, you can use this component to emit ray from hand controller
 */
UCLASS(ClassGroup = LGUI, Blueprintable, meta=(DisplayName="World"))
class LGUI_API ULGUIWorldSpaceRaycasterSource_World : public ULGUIWorldSpaceRaycasterSource
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LGUI)
		ELGUISceneComponentDirection RayDirectionType = ELGUISceneComponentDirection::PositiveX;
	/** if click/drag threshold relate to line trace distance ? */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = LGUI)
		bool clickThresholdRelateToRayDistance = true;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = LGUI)
		float rayDistanceMultiply = 0.003f;

public:
	virtual bool GenerateRay(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection)override;
	virtual bool ShouldStartDrag(ULGUIPointerEventData* InPointerEventData)override;
};
