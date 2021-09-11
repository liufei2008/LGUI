// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LGUIBaseRaycaster.h"
#include "LGUI_UIRaycaster.generated.h"

UENUM(BlueprintType, Category = LGUI)
enum class EUIRaycastSortType :uint8
{
	/** sort on UI element's depth */
	DependOnUIDepth,
	/** sort on UI element's hit distance */
	DependOnDistance,
};

/**
 * Rayemitter must be set for raycaster, or it will not work
 */
UCLASS(ClassGroup = LGUI, meta = (BlueprintSpawnableComponent), Blueprintable)
class LGUI_API ULGUI_UIRaycaster : public ULGUIBaseRaycaster
{
	GENERATED_BODY()
	
public:	
	ULGUI_UIRaycaster();
protected:
	/** temp array, hit result */
	TArray<FHitResult> multiUIHitResult;
	bool IsHitVisibleUI(class UUIItem* HitUI, const FVector& HitPoint);
	bool IsUIInteractionGroupAllowHit(class UUIItem* HitUI);

	void LineTraceUIHierarchy(TArray<FHitResult>& OutHitArray, bool InSortResult, const TArray<AActor*>& InActorArray, const FVector& InRayOrign, const FVector& InRayEnd, ETraceTypeQuery InTraceChannel);
	void LineTraceUIHierarchyRecursive(TArray<FHitResult>& OutHitArray, USceneComponent* InSceneComp, const FVector& InRayOrign, const FVector& InRayEnd, ETraceTypeQuery InTraceChannel);

	virtual bool ShouldSkipUIItem(class UUIItem* UIItem) { return false; }
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LGUI)
		EUIRaycastSortType uiSortType = EUIRaycastSortType::DependOnUIDepth;
	/** if two UI element hit distance's difference less than threshold, then sort the two UI element on depth */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = LGUI)
		float uiSortDependOnDistanceThreshold = 0.001f;
	virtual bool Raycast(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, FHitResult& OutHitResult, TArray<USceneComponent*>& OutHoverArray)override;
};
