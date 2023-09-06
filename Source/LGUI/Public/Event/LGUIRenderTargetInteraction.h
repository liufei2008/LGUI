// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LGUIBaseRaycaster.h"
#include "Event/Interface/LGUIPointerEnterExitInterface.h"
#include "Event/Interface/LGUIPointerDownUpInterface.h"
#include "Event/Interface/LGUIPointerScrollInterface.h"
#include "LGUIRenderTargetInteraction.generated.h"

class ULGUICanvas;
class ULGUIRenderTargetGeometrySource;
class ULGUIEventSystem;

/**
 * Perform a raycaster and interaction for LGUIRenderTargetGeometrySource object, which shows the LGUI RenderTarget UI.
 * This component should be placed on a actor which have a LGUIRenderTargetGeometrySource component.
 */
UCLASS(ClassGroup = LGUI, meta = (BlueprintSpawnableComponent), Blueprintable)
class LGUI_API ULGUIRenderTargetInteraction : public ULGUIBaseRaycaster
	, public ILGUIPointerEnterExitInterface
	, public ILGUIPointerDownUpInterface
	, public ILGUIPointerScrollInterface
{
	GENERATED_BODY()
	
public:	
	ULGUIRenderTargetInteraction();
	virtual void BeginPlay()override;
	virtual void OnRegister()override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)override;
	
	virtual void ActivateRaycaster()override;
	virtual void DeactivateRaycaster()override;
protected:
	/** inherited events of this component can bubble up? */
	UPROPERTY(EditAnywhere, Category = LGUI)
		bool bAllowEventBubbleUp = false;

	TWeakObjectPtr<ULGUICanvas> TargetCanvas = nullptr;
	TWeakObjectPtr<ULGUIRenderTargetGeometrySource> GeometrySource = nullptr;
	UPROPERTY(Transient) ULGUIPointerEventData* PointerEventData = nullptr;
	TWeakObjectPtr<ULGUIPointerEventData> InputPointerEventData = nullptr;

	virtual bool ShouldSkipCanvas(class ULGUICanvas* UICanvas)override;
	virtual bool GenerateRay(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection)override { return true; }
	virtual bool ShouldStartDrag(ULGUIPointerEventData* InPointerEventData)override;
	virtual bool Raycast(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, FHitResult& OutHitResult, TArray<USceneComponent*>& OutHoverArray)override;

	virtual bool OnPointerEnter_Implementation(ULGUIPointerEventData* eventData)override;
	virtual bool OnPointerExit_Implementation(ULGUIPointerEventData* eventData)override;
	virtual bool OnPointerDown_Implementation(ULGUIPointerEventData* eventData)override;
	virtual bool OnPointerUp_Implementation(ULGUIPointerEventData* eventData)override;
	virtual bool OnPointerScroll_Implementation(ULGUIPointerEventData* eventData)override;

	bool LineTrace(FLGUIHitResult& hitResult);
};
