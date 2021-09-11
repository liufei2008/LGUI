// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "CollisionQueryParams.h"
#include "Event/LGUIPointerEventData.h"
#include "LGUIBaseRaycaster.generated.h"

/** 
 * Rayemitter must be set for raycaster, or it will not work
 */
UCLASS(Abstract)
class LGUI_API ULGUIBaseRaycaster : public USceneComponent
{
	GENERATED_BODY()
	
public:	
	ULGUIBaseRaycaster();
protected:
	virtual void Activate(bool bReset = false)override;
	virtual void Deactivate()override;
	virtual void OnUnregister()override;

	friend class FUIBaseRaycasterCustomization;

	/** ignore these actors */
	TArray<AActor*> traceIgnoreActorArray;
	/** line trace only these actors */
	TArray<AActor*> traceOnlyActorArray;
	/** line trace multi for specific actors */
	void ActorLineTraceMulti(TArray<FHitResult>& OutHitArray, bool InSortResult, const TArray<AActor*>& InActorArray, const FVector& InRayOrign, const FVector& InRayEnd, ECollisionChannel InTraceChannel, const struct FCollisionQueryParams& InParams = FCollisionQueryParams::DefaultQueryParam);
public:
	/** 
	 * For LGUIBaseRaycasters with same depth, LGUI will line trace them all and sort result on hit distance.
	 * For LGUIBaseRaycasters with different depth, LGUI will sort raycasters on depth, and line trace from highest depth to lowest, if hit anything then stop line trace.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LGUI)
		int32 depth = 0;
	/** line trace ray emit length */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LGUI)
		float rayLength = 10000;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LGUI)
		TEnumAsByte<ETraceTypeQuery> traceChannel;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LGUI)
		ELGUIEventFireType eventFireType = ELGUIEventFireType::TargetActorAndAllItsComponents;

	/** use ray emitter to emit a ray and use that ray to do linecast */
	UPROPERTY(BlueprintReadWrite, Category = LGUI)
		class ULGUIBaseRayemitter* rayEmitter;
	
	bool GenerateRay(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, TArray<AActor*>& OutTraceOnlyActors, TArray<AActor*>& OutTraceIgnoreActors);
	virtual bool Raycast(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, FHitResult& OutHitResult, TArray<USceneComponent*>& OutHoverArray) PURE_VIRTUAL(ULGUIBaseRaycaster::Raycast, return false;);

	UFUNCTION(BlueprintCallable, Category = LGUI)void ActivateRaycaster();
	UFUNCTION(BlueprintCallable, Category = LGUI)void DeactivateRaycaster();
};
