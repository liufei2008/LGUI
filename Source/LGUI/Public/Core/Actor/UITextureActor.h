// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "UIBaseActor.h"
#include "Core/ActorComponent/UITexture.h"
#include "UITextureActor.generated.h"

UCLASS()
class LGUI_API AUITextureActor : public AUIBaseActor
{
	GENERATED_BODY()
	
public:	
	AUITextureActor();

	FORCEINLINE virtual UUIItem* GetUIItem()const override { return UITexture; }
	FORCEINLINE UUITexture* GetUITexture()const { return UITexture; }
private:
	UPROPERTY(Category = "LGUI", VisibleAnywhere, BlueprintReadOnly, Transient, meta = (AllowPrivateAccess = "true"))
		class UUITexture* UITexture;
	
};
