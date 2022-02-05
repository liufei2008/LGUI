// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "UIBaseActor.h"
#include "Core/ActorComponent/UICustomDepthStencilMask.h"
#include "UICustomDepthStencilMaskActor.generated.h"

UCLASS(Experimental, ClassGroup = LGUI)
class LGUI_API AUICustomDepthStencilMaskActor : public AUIBasePostProcessActor
{
	GENERATED_BODY()
	
public:	
	AUICustomDepthStencilMaskActor();

	virtual UUIItem* GetUIItem()const override { return UICustomDepthStencilMask; }
	virtual class UUIPostProcessRenderable* GetUIPostProcessRenderable()const override { return UICustomDepthStencilMask; }
	FORCEINLINE UUICustomDepthStencilMask* GetUICustomDepthMask()const { return UICustomDepthStencilMask; }
private:
	UPROPERTY(Category = "LGUI", VisibleAnywhere, BlueprintReadOnly, Transient, meta = (AllowPrivateAccess = "true"))
		class UUICustomDepthStencilMask* UICustomDepthStencilMask;
	
};
