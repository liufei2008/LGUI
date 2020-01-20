// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "UIBaseActor.h"
#include "Core/ActorComponent/UIItem.h"
#include "UIContainerActor.generated.h"

UCLASS()
class LGUI_API AUIContainerActor : public AUIBaseActor
{
	GENERATED_BODY()
	
public:	
	AUIContainerActor();

	FORCEINLINE virtual UUIItem* GetUIItem()const override{ return UIItem; }
private:
	UPROPERTY(Category = "LGUI", VisibleAnywhere, BlueprintReadOnly, Transient, meta = (AllowPrivateAccess = "true"))
		class UUIItem* UIItem;
};
