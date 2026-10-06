// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LexWorldSpaceRaycasterBase.h"
#include "LexWorldSpaceRaycasterForWorldTrigger.generated.h"

/**
 * Raycast on common world space objects like StaticMesh and Trigger
 */
UCLASS(ClassGroup = LGUI, meta=(BlueprintSpawnableComponent))
class LGUI_API ULexWorldSpaceRaycasterForWorldTrigger : public ULexWorldSpaceRaycasterBase
{
	GENERATED_BODY()

public:
	ULexWorldSpaceRaycasterForWorldTrigger();

protected:
	/** Will get FaceIndex when line trace world object's mesh. */
	UPROPERTY(EditAnywhere, Category = "LexUI")
	bool bRequireFaceIndex = false;
	
	virtual void BeginPlay() override;
	virtual void Raycast(ULexPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, TArray<FLexUIHitResult>& OutHitResultArray)override;
};
