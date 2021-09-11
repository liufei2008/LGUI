// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LGUIBaseRayemitter.h"
#include "LGUI_SceneCapture2DMouseRayemitter.generated.h"

UCLASS(ClassGroup = LGUI, meta = (BlueprintSpawnableComponent), Blueprintable)
class LGUI_API ULGUI_SceneCapture2DMouseRayemitter : public ULGUIBaseRayemitter
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LGUI)
		class ASceneCapture2D* SceneCaptureActor;
	UPROPERTY(Transient) class USceneCaptureComponent2D* sceneCaptureComp;
	/*
	Rect range for mapping main viewport coordinate to SceneCapture2D viewport coordinate
	x/y means start position from left bottom(0.0, 0.0), z/w means horizontal/vertical size(full viewport size is(1.0, 1.0))
	Default is full viewport
	*/
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LGUI)
	//	FVector4 ViewportMappingRect = FVector4(0, 0, 1, 1);

public:
	/** return value could be null, so check is before you use */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		class USceneCaptureComponent2D* GetSceneCapture2DComponent();
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetSceneCapture2DComponent(class USceneCaptureComponent2D* InComp) { sceneCaptureComp = InComp; }

	virtual bool EmitRay(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, TArray<AActor*>& InOutTraceOnlyActors, TArray<AActor*>& InOutTraceIgnoreActors)override;
	virtual bool ShouldStartDrag(ULGUIPointerEventData* InPointerEventData)override;
	virtual void MarkPress(ULGUIPointerEventData* InPointerEventData)override;

	static FMatrix ComputeViewProjectionMatrix(USceneCaptureComponent2D* InSceneCapture2D, bool InWithViewLocation = false);
	/**
	 * Deproject view point to world ray, for SceneCaptureComponent2D use
	 * @param	InSceneCapture2D	target SceneCaptureComponent2D, TextureTarget of this SceneCaptureComponent2D must be assigned, 
	 * @param	InViewPoint01		point position of range 0-1, left bottom means (0,0), right top means (1,1)
	 * @param	OutRayOrigin		result ray origin
	 * @param	OutRayDirection		result ray direction
	 */
	UFUNCTION(BlueprintCallable, Category = LGUI)
	static void DeprojectViewPointToWorldForSceneCapture2D(USceneCaptureComponent2D* InSceneCapture2D, const FVector2D& InViewPoint01, FVector& OutRayOrigin, FVector& OutRayDirection);
	static void DeprojectViewPointToWorldForSceneCapture2D(const FMatrix& InViewProjectionMatrix, const FVector2D& InViewPoint01, FVector& OutRayOrigin, FVector& OutRayDirection);
	/**
	 * Project world point to screen space point, for SceneCaptureComponent2D use
	 * @param	InSceneCapture2D	target SceneCaptureComponent2D, TextureTarget of this SceneCaptureComponent2D must be assigned,
	 * @param	InWorldPosition		world position
	 * @param	OutViewPoint		result view space point position of range 0-1, left bottom means (0,0), right top means (1,1)
	 */
	UFUNCTION(BlueprintCallable, Category = LGUI)
	static bool ProjectWorldToViewPointForSceneCapture2D(USceneCaptureComponent2D* InSceneCapture2D, const FVector& InWorldPosition, FVector2D& OutViewPoint);
	static bool ProjectWorldToViewPointForSceneCapture2D(const FMatrix& InViewProjectionMatrix, const FVector& InWorldPosition, FVector2D& OutViewPoint);
private:
	FVector2D pressMountPos;
};
