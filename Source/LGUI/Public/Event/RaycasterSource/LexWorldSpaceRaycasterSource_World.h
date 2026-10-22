// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Event/LexWorldSpaceRaycasterBase.h"
#include "LexWorldSpaceRaycasterSource_World.generated.h"



UENUM(BlueprintType, Category = "LexUI")
enum class ELexUISceneComponentDirection :uint8
{
	PositiveX		UMETA(DisplayName = "X+"),
	NegativeX		UMETA(DisplayName = "X-"),
	PositiveY		UMETA(DisplayName = "Y+"),
	NegativeY		UMETA(DisplayName = "Y-"),
	PositiveZ		UMETA(DisplayName = "Z+"),
	NegativeZ		UMETA(DisplayName = "Z-"),
};

/**
 * If VR mode, you can use this component to emit ray from hand controller
 */
UCLASS(ClassGroup = LGUI, meta=(BlueprintSpawnableComponent))
class LGUI_API ULexWorldSpaceRaycasterSource_World : public ULexWorldSpaceRaycasterSource
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LexUI")
	ELexUISceneComponentDirection RayDirectionType = ELexUISceneComponentDirection::PositiveX;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "LexUI")
	USceneComponent* TargetSceneComp = nullptr;
	/** drag threshold relate to line trace distance? If true then use ray distance as drag threshold */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "LexUI")
	bool bDragThresholdRelateToRayDistance = true;
	/** if bDragThresholdRelateToRayDistance is true, then multiply the ray distance with this value and use the result as drag threshold */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "LexUI", meta=(EditCondition=bDragThresholdRelateToRayDistance))
	float RayDistanceMultiply = 0.003f;

public:
	virtual bool GenerateRay(ULexPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd)override;
	virtual bool ShouldStartDrag(ULexPointerEventData* InPointerEventData)override;
};

/*
 * This is a preset actor that contains a LexWorldSpaceRaycasterSource_World component
 */
UCLASS(ClassGroup = LGUI)
class LGUI_API ALexWorldSpaceRaycasterSource_World_Actor : public ALexWorldSpaceRaycasterSourceActor
{
	GENERATED_BODY()

public:
	ALexWorldSpaceRaycasterSource_World_Actor();
};
