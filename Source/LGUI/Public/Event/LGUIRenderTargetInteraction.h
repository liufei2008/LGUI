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
	UPROPERTY(Transient) TObjectPtr<ULGUIPointerEventData> PointerEventData = nullptr;
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

	struct FHitResultContainerStruct
	{
		FHitResult hitResult;
		ELGUIEventFireType eventFireType = ELGUIEventFireType::TargetActorAndAllItsComponents;

		FVector rayOrigin = FVector(0, 0, 0), rayDirection = FVector(1, 0, 0), rayEnd = FVector(1, 0, 0);

		ULGUIBaseRaycaster* raycaster = nullptr;

		TArray<USceneComponent*> hoverArray;
	};
	bool LineTrace(FHitResultContainerStruct& hitResult);
	void ProcessPointerEvent(ULGUIPointerEventData* pointerEventData, bool pointerHitAnything, const FHitResultContainerStruct& hitResult);
	void ProcessPointerEnterExit(ULGUIPointerEventData* pointerEventData, USceneComponent* oldObj, USceneComponent* newObj, ELGUIEventFireType enterFireType);
	/** find a commont root actor of two actors. return nullptr if no common root */
	AActor* FindCommonRoot(AActor* actorA, AActor* actorB);


	void CallOnPointerEnter(USceneComponent* component, ULGUIPointerEventData* eventData, ELGUIEventFireType eventFireType);
	void CallOnPointerExit(USceneComponent* component, ULGUIPointerEventData* eventData, ELGUIEventFireType eventFireType);
	void CallOnPointerDown(USceneComponent* component, ULGUIPointerEventData* eventData, ELGUIEventFireType eventFireType);
	void CallOnPointerUp(USceneComponent* component, ULGUIPointerEventData* eventData, ELGUIEventFireType eventFireType);
	void CallOnPointerClick(USceneComponent* component, ULGUIPointerEventData* eventData, ELGUIEventFireType eventFireType);
	void CallOnPointerBeginDrag(USceneComponent* component, ULGUIPointerEventData* eventData, ELGUIEventFireType eventFireType);
	void CallOnPointerDrag(USceneComponent* component, ULGUIPointerEventData* eventData, ELGUIEventFireType eventFireType);
	void CallOnPointerEndDrag(USceneComponent* component, ULGUIPointerEventData* eventData, ELGUIEventFireType eventFireType);
	void CallOnPointerScroll(USceneComponent* component, ULGUIPointerEventData* eventData, ELGUIEventFireType eventFireType);
	void CallOnPointerDragDrop(USceneComponent* component, ULGUIPointerEventData* eventData, ELGUIEventFireType eventFireType);
	void CallOnPointerSelect(USceneComponent* component, ULGUIBaseEventData* eventData, ELGUIEventFireType eventFireType);
	void CallOnPointerDeselect(USceneComponent* component, ULGUIBaseEventData* eventData, ELGUIEventFireType eventFireType);

	void BubbleOnPointerEnter(AActor* actor, ULGUIPointerEventData* eventData);
	void BubbleOnPointerExit(AActor* actor, ULGUIPointerEventData* eventData);
	void BubbleOnPointerDown(AActor* actor, ULGUIPointerEventData* eventData);
	void BubbleOnPointerUp(AActor* actor, ULGUIPointerEventData* eventData);
	void BubbleOnPointerClick(AActor* actor, ULGUIPointerEventData* eventData);
	void BubbleOnPointerBeginDrag(AActor* actor, ULGUIPointerEventData* eventData);
	void BubbleOnPointerDrag(AActor* actor, ULGUIPointerEventData* eventData);
	void BubbleOnPointerEndDrag(AActor* actor, ULGUIPointerEventData* eventData);
	void BubbleOnPointerScroll(AActor* actor, ULGUIPointerEventData* eventData);
	void BubbleOnPointerDragDrop(AActor* actor, ULGUIPointerEventData* eventData);
	void BubbleOnPointerSelect(AActor* actor, ULGUIBaseEventData* eventData);
	void BubbleOnPointerDeselect(AActor* actor, ULGUIBaseEventData* eventData);
};
