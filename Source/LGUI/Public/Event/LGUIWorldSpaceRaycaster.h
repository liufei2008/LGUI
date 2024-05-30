// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LGUIBaseRaycaster.h"
#include "LGUIWorldSpaceRaycaster.generated.h"

class ULGUIWorldSpaceRaycaster;

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
	TWeakObjectPtr<ULGUIBaseRaycaster> RaycasterObject = nullptr;
public:
	UFUNCTION(BlueprintCallable, Category = LGUI)
		ULGUIBaseRaycaster* GetRaycasterObject()const;
	/** Called by LGUIWorldSpaceRaycaster when register, use as initialize. */
	virtual void Init(ULGUIBaseRaycaster* InRaycaster);
	/** Generate ray for raycast hit test */
	virtual bool GenerateRay(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection);
	/** Should convert press event to drag event? */
	virtual bool ShouldStartDrag(ULGUIPointerEventData* InPointerEventData);
protected:
	/** Called by LGUIWorldSpaceRaycaster when register, use as initialize. */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Init"), Category = "LGUI")
		void ReceiveInit(ULGUIBaseRaycaster* InRaycaster);
	/** Generate ray for raycast hit test */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "EmitRay"), Category = "LGUI")
		bool ReceiveGenerateRay(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection);
	/** Should convert press event to drag event? */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "ShouldStartDrag"), Category = "LGUI")
		bool ReceiveShouldStartDrag(ULGUIPointerEventData* InPointerEventData);
};

enum class ELGUIRenderMode :uint8;

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
	UPROPERTY(EditAnywhere, Category = "LGUI")
		ELGUIInteractionTarget interactionTarget = ELGUIInteractionTarget::UIAndWorld;
	/** Will get FaceIndex when line trace world object's mesh. */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool bRequireFaceIndex = false;

	UPROPERTY(EditAnywhere, Instanced, Category = "LGUI")
		TObjectPtr<ULGUIWorldSpaceRaycasterSource> RaycasterSourceObject = nullptr;
	virtual bool ShouldSkipCanvas(class ULGUICanvas* UICanvas)override;
	TArray<ELGUIRenderMode> RenderModeArray;
public:
	virtual bool GetAffectByGamePause()const override;
	virtual bool GenerateRay(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection) override;
	virtual bool Raycast(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, FHitResult& OutHitResult, TArray<USceneComponent*>& OutHoverArray)override;
	virtual bool ShouldStartDrag(ULGUIPointerEventData* InPointerEventData) override;

	UFUNCTION(BlueprintCallable, Category = LGUI)
		ULGUIWorldSpaceRaycasterSource* GetRaycasterSourceObject()const { return RaycasterSourceObject; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetRaycasterSourceObject(ULGUIWorldSpaceRaycasterSource* NewSource);
};
