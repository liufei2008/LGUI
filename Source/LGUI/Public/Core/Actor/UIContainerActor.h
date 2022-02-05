// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "UIBaseActor.h"
#include "Core/ActorComponent/UIItem.h"
#include "UIContainerActor.generated.h"

UCLASS(ClassGroup = LGUI)
class LGUI_API AUIContainerActor : public AUIBaseActor
{
	GENERATED_BODY()
	
public:	
	AUIContainerActor();

	FORCEINLINE virtual UUIItem* GetUIItem()const override{ return UIItem; }
private:
	UPROPERTY(Category = "LGUI", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UUIItem* UIItem;
};
