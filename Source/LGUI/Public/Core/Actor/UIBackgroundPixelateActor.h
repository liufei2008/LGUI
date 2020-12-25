// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "UIBaseActor.h"
#include "Core/ActorComponent/UIBackgroundPixelate.h"
#include "UIBackgroundPixelateActor.generated.h"

//Not supported on mobile device!
UCLASS()
class LGUI_API AUIBackgroundPixelateActor : public AUIBaseActor
{
	GENERATED_BODY()
	
public:	
	AUIBackgroundPixelateActor();

	FORCEINLINE virtual UUIItem* GetUIItem()const override { return UIBackgroundPixelate; }
	FORCEINLINE UUIBackgroundPixelate* GetUIBackgroundPixelate()const { return UIBackgroundPixelate; }
private:
	UPROPERTY(Category = "LGUI", VisibleAnywhere, BlueprintReadOnly, Transient, meta = (AllowPrivateAccess = "true"))
		class UUIBackgroundPixelate* UIBackgroundPixelate;
	
};
