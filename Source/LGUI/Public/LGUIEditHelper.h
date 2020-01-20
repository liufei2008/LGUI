// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "LGUIEditHelper.generated.h"

//for create a button in editor
USTRUCT(BlueprintType)
struct LGUI_API FLGUIEditHelperButton
{
	GENERATED_BODY()
private:
	friend class FLGUIEditHelperButtonCustomization;
	UPROPERTY(Transient, EditAnywhere, Category = "LGUI")
		int32 clickCount = 0;
	UPROPERTY(Transient, EditAnywhere, Category = "LGUI")
		int32 prevClickCount = 0;
public:
	bool IsClicked()
	{
		bool result = prevClickCount != clickCount;
		prevClickCount = clickCount;
		return result;
	}
};