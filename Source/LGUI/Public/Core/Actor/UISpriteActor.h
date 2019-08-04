// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "UIBaseActor.h"
#include "Core/ActorComponent/UISprite.h"
#include "UISpriteActor.generated.h"

UCLASS()
class LGUI_API AUISpriteActor : public AUIBaseActor
{
	GENERATED_BODY()
	
public:	
	AUISpriteActor();

	FORCEINLINE virtual UUIItem* GetUIItem()const override { return UISprite; }
	FORCEINLINE UUISprite* GetUISprite()const { return UISprite; }
private:
	UPROPERTY(Category = "LGUI", VisibleAnywhere, BlueprintReadOnly, Transient, meta = (AllowPrivateAccess = "true"))
		class UUISprite* UISprite;
	
};
