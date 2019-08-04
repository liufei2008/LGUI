// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LGUIBaseRayemitter.h"
#include "LGUI_SceneComponentRayEmitter.generated.h"


UENUM(BlueprintType)
enum class ESceneComponentRayDirection :uint8
{
	PositiveX		UMETA(DisplayName = "X+"),
	NagtiveX		UMETA(DisplayName = "X-"),
	PositiveY		UMETA(DisplayName = "Y+"),
	NagtiveY		UMETA(DisplayName = "Y-"),
	PositiveZ		UMETA(DisplayName = "Z+"),
	NagtiveZ		UMETA(DisplayName = "Z-"),
};
//If VR mode, you can use this component to emit ray from hand controller
UCLASS(ClassGroup = LGUI, meta = (BlueprintSpawnableComponent), Blueprintable)
class LGUI_API ULGUI_SceneComponentRayEmitter : public ULGUIBaseRayEmitter
{
	GENERATED_BODY()
public:
	ULGUI_SceneComponentRayEmitter();
protected:
	//use TargetActor's RootComponent as ray object, world location as RayOrigin, axis direction as RayDirection
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LGUI)
		AActor* TargetActor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LGUI)
		ESceneComponentRayDirection RayDirectionType = ESceneComponentRayDirection::PositiveX;
	//if click/drag threshold relate to line trace distance ?
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = LGUI)
		bool clickThresholdRelateToRayDistance = true;

	UPROPERTY(Transient) USceneComponent* CacheTargetSceneComponent = nullptr;
public:
	UFUNCTION(BlueprintCallable, Category = LGUI)AActor* GetTargetActor() { return TargetActor; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetTargetActor(AActor* InActor)
	{
		TargetActor = InActor;
		if (TargetActor != nullptr)
		{
			CacheTargetSceneComponent = TargetActor->GetRootComponent();
		}
	}
	UFUNCTION(BlueprintCallable, Category = LGUI)void SetTargetSceneComponent(USceneComponent* InSceneComp) { CacheTargetSceneComponent = InSceneComp; }
	UFUNCTION(BlueprintCallable, Category = LGUI)USceneComponent* GetTargetSceneComponent() { return CacheTargetSceneComponent; }
	virtual bool EmitRay(FVector& OutRayOrigin, FVector& OutRayDirection, TArray<AActor*>& InOutTraceOnlyActors, TArray<AActor*>& InOutTraceIgnoreActors)override;
	virtual bool ShouldStartDrag(const FLGUIPointerEventData& InPointerEventData)override;
};
