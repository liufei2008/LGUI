// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LGUIBaseRaycaster.h"
#include "LGUIWorldSpaceRaycaster.generated.h"

class ULGUIWorldSpaceRaycaster;

/**
 * NOTE! This type is not valid anymore
 * The interaction source for world space UI, actually the ray emitter object.
 */
UENUM(BlueprintType, Category = LGUI)
enum class ELGUIWorldSpaceRaycasterSource :uint8
{
	/** Sends traces from the world location and orientation of the interaction component. */
	World,
	/** Sends traces from mouse or touch location of the first local player controller. */
	Mouse,
	/** Sends trace from the center of the first local player's screen. */
	CenterScreen,
	/**
	 * Sends traces from a custom location determined by the user.  Will use whatever
	 * FHitResult is set by the call to SetCustomHitResult.
	 */
	//Custom
};

/** Interaction target for world space */
UENUM(BlueprintType, Category = LGUI)
enum class ELGUIInteractionTarget :uint8
{
	/** Only hit UI object */
	UI,
	/** Only hit world object */
	World,
	/** Hit UI and world object */
	UIAndWorld		UMETA(DisplayName="UI and World"),
};

/**
 * Interaction source for LGUIWorldSpaceRaycaster
 */
UCLASS(BlueprintType, Blueprintable, Abstract, DefaultToInstanced, EditInlineNew)
class LGUI_API ULGUIWorldSpaceRaycasterSource : public UObject
{
	GENERATED_BODY()
private:
	TWeakObjectPtr<ULGUIWorldSpaceRaycaster> InteractionObject = nullptr;
public:
	UFUNCTION(BlueprintCallable, Category = LGUI)
		ULGUIWorldSpaceRaycaster* GetInteractionObject();
	void SetInteractionObject(ULGUIWorldSpaceRaycaster* InObj) { InteractionObject = InObj; }
	virtual bool EmitRay(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection);
	virtual bool ShouldStartDrag(ULGUIPointerEventData* InPointerEventData);
protected:
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "EmitRay"), Category = "LGUI")bool ReceiveEmitRay(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection);
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "ShouldStartDrag"), Category = "LGUI")bool ReceiveShouldStartDrag(ULGUIPointerEventData* InPointerEventData);
};

/**
 * Perform a raycaster interaction for WorldSpaceUI and common world space objects.
 */
UCLASS(ClassGroup = LGUI, meta = (BlueprintSpawnableComponent), Blueprintable)
class LGUI_API ULGUIWorldSpaceRaycaster : public ULGUIBaseRaycaster
{
	GENERATED_BODY()
	
public:	
	ULGUIWorldSpaceRaycaster();
	virtual void BeginPlay()override;
	virtual void OnRegister()override;
protected:
	/** Old data */
	UPROPERTY(EditAnywhere, Category = "LGUI-old", AdvancedDisplay)
		ELGUIWorldSpaceRaycasterSource interactionSource = ELGUIWorldSpaceRaycasterSource::Mouse;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		ELGUIInteractionTarget interactionTarget = ELGUIInteractionTarget::UIAndWorld;

	UPROPERTY(EditAnywhere, Instanced, Category = "LGUI")
		ULGUIWorldSpaceRaycasterSource* InteractionSourceObject = nullptr;
	virtual bool ShouldSkipUIItem(class UUIItem* UIItem)override;
public:
	virtual bool GenerateRay(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection) override;
	virtual bool Raycast(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, FHitResult& OutHitResult, TArray<USceneComponent*>& OutHoverArray)override;
	virtual bool ShouldStartDrag(ULGUIPointerEventData* InPointerEventData) override;

	UFUNCTION(BlueprintCallable, Category = LGUI)
		ULGUIWorldSpaceRaycasterSource* GetInteractionSourceObject()const { return InteractionSourceObject; }
};
