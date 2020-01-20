// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "CollisionQueryParams.h"
#include "LGUIBaseRaycaster.generated.h"

//call event type
UENUM(BlueprintType)
enum class ELGUIEventFireType :uint8
{
	//event will call on trace target actor and all component of the actor
	TargetActorAndAllItsComponents,
	//event will call only on trace target
	OnlyTargetComponent,
};
//RayEmitter must be set for raycaster, or it will not work
UCLASS(Abstract)
class LGUI_API ULGUIBaseRaycaster : public USceneComponent
{
	GENERATED_BODY()
	
public:	
	ULGUIBaseRaycaster();
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason)override;
	virtual void OnRegister()override;
	virtual void OnUnregister()override;

	friend class FUIBaseRaycasterCustomization;

	//ignore these actors
	TArray<AActor*> traceIgnoreActorArray;
	//line trace only these actors
	TArray<AActor*> traceOnlyActorArray;
	//line trace multi for specific actors
	void ActorLineTraceMulti(TArray<FHitResult>& OutHitArray, bool InSortResult, const TArray<AActor*>& InActorArray, const FVector& InRayOrign, const FVector& InRayEnd, ECollisionChannel InTraceChannel, const struct FCollisionQueryParams& InParams = FCollisionQueryParams::DefaultQueryParam);
public:
	//for raycasters with same depth, will line trace them all and sort result on hit distance
	//for raycasters with different depth, will sort raycasters on depth, and line trace from highest depth to lowest, if hit anything then stop line trace
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LGUI)
		int32 depth = 0;
	//line trace ray emit length
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LGUI)
		float rayLength = 1000;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LGUI)
		TEnumAsByte<ETraceTypeQuery> traceChannel;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LGUI)
		ELGUIEventFireType eventFireType = ELGUIEventFireType::TargetActorAndAllItsComponents;

	//use ray emitter to emit a ray and use that ray to do linecast
	UPROPERTY(BlueprintReadWrite, Category = LGUI)
		class ULGUIBaseRayEmitter* rayEmitter;
	
	bool GenerateRay(FVector& OutRayOrigin, FVector& OutRayDirection, TArray<AActor*>& OutTraceOnlyActors, TArray<AActor*>& OutTraceIgnoreActors);
	virtual bool Raycast(FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, FHitResult& OutHitResult);

	UFUNCTION(BlueprintCallable, Category = LGUI)void ActivateRaycaster();
	UFUNCTION(BlueprintCallable, Category = LGUI)void DeactivateRaycaster();
};
