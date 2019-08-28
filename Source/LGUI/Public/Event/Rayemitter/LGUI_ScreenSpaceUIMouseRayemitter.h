// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LGUIBaseRayemitter.h"
#include "LGUI_ScreenSpaceUIMouseRayemitter.generated.h"

//For screen space UI mouse input. This component should only attached on a actor which have a UIRoot, and RenderMode of UIRoot set to ScreenSpace
UCLASS(ClassGroup = LGUI, meta = (BlueprintSpawnableComponent), Blueprintable)
class LGUI_API ULGUI_ScreenSpaceUIMouseRayemitter : public ULGUIBaseRayEmitter
{
	GENERATED_BODY()

public:	
	ULGUI_ScreenSpaceUIMouseRayemitter();
	virtual bool EmitRay(FVector& OutRayOrigin, FVector& OutRayDirection, TArray<AActor*>& InOutTraceOnlyActors, TArray<AActor*>& InOutTraceIgnoreActors)override;
	virtual bool ShouldStartDrag(const FLGUIPointerEventData& InPointerEventData)override;
	virtual void MarkPress(const FLGUIPointerEventData& InPointerEventData)override;
	static void DeprojectViewPointToWorld(const FMatrix& InViewProjectionMatrix, const FVector2D& InViewPoint01, FVector& OutWorldLocation, FVector& OutWorldDirection);
private:
	FVector2D pressMountPos;
	UPROPERTY(Transient)class UUIRoot* UIRootComp = nullptr;
};
