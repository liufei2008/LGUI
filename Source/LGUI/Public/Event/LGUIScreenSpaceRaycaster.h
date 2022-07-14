// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LGUIBaseRaycaster.h"
#include "LGUIScreenSpaceRaycaster.generated.h"

class ULGUICanvas;

/**
 * Perform a raycaster interaction for ScreenSpaceUI.
 * This component should be placed on a actor which have a LGUICanvas, and RenderMode of LGUICanvas should set to ScreenSpace.
 */
UCLASS(ClassGroup = LGUI, meta = (BlueprintSpawnableComponent), Blueprintable)
class LGUI_API ULGUIScreenSpaceRaycaster : public ULGUIBaseRaycaster
{
	GENERATED_BODY()
	
public:	
	ULGUIScreenSpaceRaycaster();
	virtual void BeginPlay()override;
protected:
	
	TWeakObjectPtr<ULGUICanvas> RootCanvas = nullptr;

	virtual bool ShouldSkipCanvas(class ULGUICanvas* UICanvas)override;
public:
	virtual bool ShouldStartDrag(ULGUIPointerEventData* InPointerEventData)override;
	virtual bool GenerateRay(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection)override;
	virtual bool Raycast(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, FHitResult& OutHitResult, TArray<USceneComponent*>& OutHoverArray)override;

	static void DeprojectViewPointToWorld(const FMatrix& InViewProjectionMatrix, const FVector2D& InViewPoint01, FVector& OutWorldLocation, FVector& OutWorldDirection);
};
