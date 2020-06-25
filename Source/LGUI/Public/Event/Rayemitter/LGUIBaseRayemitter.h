// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Event/LGUIPointerEventData.h"
#include "LGUIBaseRayEmitter.generated.h"

//RayEmitter for raycaster
UCLASS(Abstract)
class LGUI_API ULGUIBaseRayEmitter : public UObject
{
	GENERATED_BODY()
protected:
	//click/drag threshold, calculated in target's local space
	UPROPERTY(EditAnywhere, Category = LGUI)
		float clickThreshold = 5;

	FVector currentRayOrigin, currentRayDirection;
public:
	UFUNCTION(BlueprintCallable, Category = LGUI)
	void SetClickThreshold(float value) { clickThreshold = value; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
	float GetClickThreshold() { return clickThreshold; }
	virtual bool EmitRay(FVector& OutRayOrigin, FVector& OutRayDirection, TArray<AActor*>& InOutTraceOnlyActors, TArray<AActor*>& InOutTraceIgnoreActors);
	virtual bool ShouldStartDrag(ULGUIPointerEventData* InPointerEventData);
	virtual void MarkPress(ULGUIPointerEventData* InPointerEventData) {};
	virtual FVector GetCurrentRayOrigin() { return currentRayOrigin; }
	virtual FVector GetCurrentRayDirection() { return currentRayDirection; }
};
