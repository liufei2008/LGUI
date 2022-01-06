// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "UIBaseActor.h"
#include "Core/ActorComponent/UIBackgroundBlur.h"
#include "UIBackgroundBlurActor.generated.h"

UCLASS()
class LGUI_API AUIBackgroundBlurActor : public AUIBasePostProcessActor
{
	GENERATED_BODY()
	
public:	
	AUIBackgroundBlurActor();

	FORCEINLINE virtual UUIItem* GetUIItem()const override { return UIBackgroundBlur; }
	FORCEINLINE UUIBackgroundBlur* GetUIBackgroundBlur()const { return UIBackgroundBlur; }
private:
	UPROPERTY(Category = "LGUI", VisibleAnywhere, BlueprintReadOnly, Transient, meta = (AllowPrivateAccess = "true"))
		class UUIBackgroundBlur* UIBackgroundBlur;
	
};
