// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "UIBaseActor.h"
#include "Core/ActorComponent/UICustomDepthStencilMask.h"
#include "UICustomDepthStencilMaskActor.generated.h"

UCLASS(Experimental)
class LGUI_API AUICustomDepthStencilMaskActor : public AUIPostProcessBaseActor
{
	GENERATED_BODY()
	
public:	
	AUICustomDepthStencilMaskActor();

	FORCEINLINE virtual UUIItem* GetUIItem()const override { return UICustomDepthStencilMask; }
	FORCEINLINE UUICustomDepthStencilMask* GetUICustomDepthMask()const { return UICustomDepthStencilMask; }
private:
	UPROPERTY(Category = "LGUI", VisibleAnywhere, BlueprintReadOnly, Transient, meta = (AllowPrivateAccess = "true"))
		class UUICustomDepthStencilMask* UICustomDepthStencilMask;
	
};
