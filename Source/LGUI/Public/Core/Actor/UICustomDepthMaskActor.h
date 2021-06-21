// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "UIBaseActor.h"
#include "Core/ActorComponent/UICustomDepthMask.h"
#include "UICustomDepthMaskActor.generated.h"

UCLASS(Experimental)
class LGUI_API AUICustomDepthMaskActor : public AUIPostProcessBaseActor
{
	GENERATED_BODY()
	
public:	
	AUICustomDepthMaskActor();

	FORCEINLINE virtual UUIItem* GetUIItem()const override { return UICustomDepthMask; }
	FORCEINLINE UUICustomDepthMask* GetUICustomDepthMask()const { return UICustomDepthMask; }
private:
	UPROPERTY(Category = "LGUI", VisibleAnywhere, BlueprintReadOnly, Transient, meta = (AllowPrivateAccess = "true"))
		class UUICustomDepthMask* UICustomDepthMask;
	
};
