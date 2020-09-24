// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Core/LGUIBehaviour.h"
#include "LGUIPlayTweenComponent.generated.h"


UCLASS(ClassGroup = (LGUI), meta = (BlueprintSpawnableComponent), Blueprintable)
class LGUI_API ULGUIPlayTweenComponent : public ULGUIBehaviour
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditAnywhere, Category = "LGUI", Instanced)
		class ULGUIPlayTween* playTween;

	virtual void Start() override;
};
