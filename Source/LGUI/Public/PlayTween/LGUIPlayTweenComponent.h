// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "LGUIPlayTweenComponent.generated.h"


UCLASS(ClassGroup = (LGUI), meta = (BlueprintSpawnableComponent), Blueprintable)
class LGUI_API ULGUIPlayTweenComponent : public UActorComponent
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool playOnStart = true;
	UPROPERTY(EditAnywhere, Category = "LGUI", Instanced)
		class ULGUIPlayTween* playTween;

	virtual void BeginPlay() override;
};
