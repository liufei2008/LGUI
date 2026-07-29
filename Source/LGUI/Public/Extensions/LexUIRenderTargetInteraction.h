// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Event/LexBaseRaycaster.h"
#include "Event/LexScreenSpaceRaycaster.h"
#include "Event/Interface/LexPointerEnterExitInterface.h"
#include "Event/Interface/LexPointerDownUpInterface.h"
#include "Event/Interface/LexPointerScrollInterface.h"
#include "LexUIRenderTargetInteraction.generated.h"

class ULexCanvas;
class ULexEventSystem;

/**
 * Interface for LexUIRenderTargetInteraction to provide raycast info.
 */
UINTERFACE(Blueprintable, MinimalAPI)
class ULexUIRenderTargetInteractionSourceInterface : public UInterface
{
	GENERATED_BODY()
};
/**
 * Interface for LexUIRenderTargetInteraction to provide raycast info.
 */
class LGUI_API ILexUIRenderTargetInteractionSourceInterface
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI)
	ULexCanvas* GetTargetCanvas()const;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI)
	bool PerformLineTrace(const int32& InHitFaceIndex, const FVector& InHitPoint, const FVector& InLineStart, const FVector& InLineEnd, FVector2D& OutHitUV);
};

/**
 * Perform a raycaster and interaction for LexUICanvas with RenderMode of RenderTarget.
 * This component should be placed on a actor which have a ILexUIRenderTargetInteractionSourceInterface component.
 */
UCLASS(ClassGroup = LGUI, meta = (BlueprintSpawnableComponent), Blueprintable)
class LGUI_API ULexUIRenderTargetInteraction : public ULexScreenSpaceRaycaster
	, public ILexPointerEnterExitInterface
	, public ILexPointerDownUpInterface
	, public ILexPointerScrollInterface
{
	GENERATED_BODY()
	
public:	
	ULexUIRenderTargetInteraction();
	virtual void BeginPlay()override;
	virtual void OnRegister()override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)override;
	
	virtual void ActivateRaycaster()override;
	virtual void DeactivateRaycaster()override;
protected:
	/** inherited events of this component can bubble up? */
	UPROPERTY(EditAnywhere, Category = LGUI)
		bool bAllowEventBubbleUp = false;

	UPROPERTY(VisibleAnywhere, Transient, Category = LGUI, AdvancedDisplay) TWeakObjectPtr<ULexCanvas> TargetCanvas = nullptr;
	UPROPERTY(VisibleAnywhere, Transient, Category = LGUI, AdvancedDisplay) TObjectPtr<UActorComponent> LineTraceSource = nullptr;
	UPROPERTY(VisibleAnywhere, Transient, Category = LGUI, AdvancedDisplay) TObjectPtr<ULexPointerEventData> PointerEventData = nullptr;
	TWeakObjectPtr<ULexPointerEventData> InputPointerEventData = nullptr;

	virtual bool GenerateRay(ULexPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, float& OutRayLength)override { return true; }
	virtual bool ShouldStartDrag(ULexPointerEventData* InPointerEventData)override;
	virtual void Raycast(ULexPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, TArray<FLexUIHitResult>& OutHitResultArray)override;

	virtual void OnPointerEnter_Implementation(ULexPointerEventData* EventData)override;
	virtual void OnPointerExit_Implementation(ULexPointerEventData* EventData)override;
	virtual bool OnPointerDown_Implementation(ULexPointerEventData* EventData)override;
	virtual bool OnPointerUp_Implementation(ULexPointerEventData* EventData)override;
	virtual bool OnPointerScroll_Implementation(ULexPointerEventData* EventData)override;

	bool LineTrace(FLexUIHitResultContainer& OutHitResult);
};
