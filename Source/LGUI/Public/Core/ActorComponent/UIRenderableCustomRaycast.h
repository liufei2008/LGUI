// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once
#include "Core/LGUILifeCycleUIBehaviour.h"
#include "UIRenderableCustomRaycast.generated.h"

/**
 * This component is only used when UIBaseRenderable's RaycastType = Custom
 */ 
UCLASS(HideCategories = (Collision, LOD, Physics, Cooking, Rendering, Activation, Actor, Input, Lighting, Mobile), ClassGroup = (LGUI), Abstract, Blueprintable)
class LGUI_API UUIRenderableCustomRaycast :public ULGUILifeCycleUIBehaviour
{
	GENERATED_BODY()

protected:
	virtual void Awake()override;
	/**
	 * Called by UIBaseRenderable when do raycast hit test.
	 * @param	InLocalSpaceRayStart	Ray start point in this UI's local space
	 * @param	InLocalSpaceRayEnd		Ray end point in this UI's local space
	 * @param	InHitPointOnPlane		Ray-Plane hit point in this UI's local space
	 * @return	true if hit this
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "LGUI", meta = (DisplayName = "OnRaycast"))
		bool ReceiveOnRaycast(const FVector& InLocalSpaceRayStart, const FVector& InLocalSpaceRayEnd, const FVector2D& InHitPointOnPlane, FVector& OutHitPoint, FVector& OutHitNormal);
public:
	/**
	 * Called by UIBaseRenderable when do raycast hit test.
	 * @param	InLocalSpaceRayStart	Ray start point in this UI's local space
	 * @param	InLocalSpaceRayEnd		Ray end point in this UI's local space
	 * @param	InHitPointOnPlane		Ray-Plane hit point in this UI's local space
	 * @return	true if hit this
	 */
	virtual bool OnRaycast(const FVector& InLocalSpaceRayStart, const FVector& InLocalSpaceRayEnd, const FVector2D& InHitPointOnPlane, FVector& OutHitPoint, FVector& OutHitNormal);
};