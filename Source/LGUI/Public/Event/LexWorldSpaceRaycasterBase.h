// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LexBaseRaycaster.h"
#include "LexWorldSpaceRaycasterBase.generated.h"

class ULexWorldSpaceRaycasterBase;

/**
 * Perform a ray source for LexWorldSpaceRaycaster
 */
UCLASS(ClassGroup = (LGUI), Blueprintable, Abstract, HideCategories = (Sockets, Physics, Collision, Activation, Cooking, Rendering, Actor, Input, Lighting, Mobile, Navigation))
class LGUI_API ULexWorldSpaceRaycasterSource : public USceneComponent
{
	GENERATED_BODY()
public:
	ULexWorldSpaceRaycasterSource();
	virtual void BeginPlay() override;
protected:
	
	/** ray length for line trace hit */
	UPROPERTY(EditAnywhere, Category = LGUI)
	float RayLength = 100000;
	/** drag threshold, calculated in target's local space */
	UPROPERTY(EditAnywhere, Category = LGUI)
	float DragThreshold = 5;
	/** hold press for a little while to entering drag mode */
	UPROPERTY(EditAnywhere, Category = LGUI)
	bool bHoldToDrag = false;
	/** hold press for "holdToDragTime" to entering drag mode */
	UPROPERTY(EditAnywhere, Category = LGUI, meta = (EditCondition = "bHoldToDrag"))
	float HoldToDragTime = 0.5f;
	float DragThresholdSquare = 0;
public:
	UFUNCTION(BlueprintCallable, Category = LGUI)
	float GetRayLength()const { return RayLength; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
	float GetDragThreshold()const { return DragThreshold; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
	bool GetHoldToDrag()const { return bHoldToDrag; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
	float GetHoldToDragTime()const { return HoldToDragTime; }
	float GetDragThresholdSquare()const { return DragThresholdSquare; }
	
	UFUNCTION(BlueprintCallable, Category = LGUI)
	void SetRayLength(float Value);
	UFUNCTION(BlueprintCallable, Category = LGUI)
	void SetDragThreshold(float Value);
	UFUNCTION(BlueprintCallable, Category = LGUI)
	void SetHoldToDrag(bool Value);
	UFUNCTION(BlueprintCallable, Category = LGUI)
	void SetHoldToDragTime(float Value);
	
	/** Generate ray for raycast hit test */
	virtual bool GenerateRay(ULexPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd);
	/** Should convert press event to drag event? */
	virtual bool ShouldStartDrag(ULexPointerEventData* InPointerEventData);
protected:
	/** Generate ray for raycast hit test */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "EmitRay"), Category = "LGUI")
		bool ReceiveGenerateRay(ULexPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd);
	/** Should convert press event to drag event? */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "ShouldStartDrag"), Category = "LGUI")
		bool ReceiveShouldStartDrag(ULexPointerEventData* InPointerEventData);
};

UCLASS(ClassGroup = LGUI, Abstract, NotPlaceable, HideCategories=(Rendering, Replication, Collision, HLOD, Physics, Networking, Input, Actor, Navigation, LevelInstance, Cooking))
class LGUI_API ALexWorldSpaceRaycasterSourceActor : public AActor
{
	GENERATED_BODY()

public:
	ALexWorldSpaceRaycasterSourceActor();
	UFUNCTION(BlueprintCallable, Category=LGUI)
	ULexWorldSpaceRaycasterSource* GetRaycasterSource()const{return RaycasterSource;}
protected:
	UPROPERTY(Category = "LGUI", EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ULexWorldSpaceRaycasterSource> RaycasterSource;
};

/**
 * Perform a raycaster interaction for WorldSpaceUI and common world space objects.
 */
UCLASS(ClassGroup = LGUI, Abstract, Blueprintable)
class LGUI_API ULexWorldSpaceRaycasterBase : public ULexBaseRaycaster
{
	GENERATED_BODY()
	
public:	
	ULexWorldSpaceRaycasterBase();
	virtual void BeginPlay()override;
protected:
	virtual void OnRegister()override;
	
	UPROPERTY(EditAnywhere, Category = "LGUI")
	TWeakObjectPtr<ALexWorldSpaceRaycasterSourceActor> RaycasterSourceActor = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "LGUI", AdvancedDisplay)
	mutable TWeakObjectPtr<ULexWorldSpaceRaycasterSource> RaycasterSourceObject = nullptr;
	UPROPERTY(EditAnywhere, Category = LGUI)
	TEnumAsByte<ETraceTypeQuery> TraceChannel;
public:
	virtual bool GetAffectByGamePause()const override;
	virtual bool GenerateRay(ULexPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, float& OutRayLength) override;
	virtual bool ShouldStartDrag(ULexPointerEventData* InPointerEventData) override;

	UFUNCTION(BlueprintCallable, Category = LGUI)
	TEnumAsByte<ETraceTypeQuery> GetTraceChannel()const { return TraceChannel; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
	ULexWorldSpaceRaycasterSource* GetRaycasterSourceObject()const;

	UFUNCTION(BlueprintCallable, Category = LGUI)
	void SetTraceChannel(TEnumAsByte<ETraceTypeQuery> Value);
	UFUNCTION(BlueprintCallable, Category = LGUI)
	void SetRaycasterSourceObject(ULexWorldSpaceRaycasterSource* Value);
	UFUNCTION(BlueprintCallable, Category = LGUI)
	void SetRaycasterSourceActor(ALexWorldSpaceRaycasterSourceActor* Value);
};
