// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Event/LGUIPointerEventData.h"
#include "LGUIBaseRayEmitter.generated.h"

//RayEmitter for raycaster
UCLASS(Abstract)
class LGUI_API ULGUIBaseRayEmitter : public UActorComponent
{
	GENERATED_BODY()
public:
	ULGUIBaseRayEmitter();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason)override;

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
	virtual bool ShouldStartDrag(const FLGUIPointerEventData& InPointerEventData);
	virtual void MarkPress(const FLGUIPointerEventData& InPointerEventData) {};
	virtual FVector GetCurrentRayOrigin() { return currentRayOrigin; }
	virtual FVector GetCurrentRayDirection() { return currentRayDirection; }
};
