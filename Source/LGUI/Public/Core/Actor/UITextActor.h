// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "UIBaseActor.h"
#include "Core/ActorComponent/UIText.h"
#include "UITextActor.generated.h"

UCLASS()
class LGUI_API AUITextActor : public AUIBaseActor
{
	GENERATED_BODY()
	
public:	
	AUITextActor();

	FORCEINLINE virtual UUIItem* GetUIItem()const override { return UIText; }
	FORCEINLINE UUIText* GetUIText()const { return UIText; }
private:
	UPROPERTY(Category = "LGUI", VisibleAnywhere, BlueprintReadOnly, Transient, meta = (AllowPrivateAccess = "true"))
		UUIText* UIText;
	
};
