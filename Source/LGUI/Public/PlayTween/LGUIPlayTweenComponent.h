// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LGUIPlayTweenComponent.generated.h"


UCLASS(ClassGroup = (LGUI), meta = (BlueprintSpawnableComponent), Blueprintable)
class LGUI_API ULGUIPlayTweenComponent : public UActorComponent
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool playOnStart = true;
	UPROPERTY(EditAnywhere, Category = "LGUI", Instanced)
		TObjectPtr<class ULGUIPlayTween> playTween;

	virtual void BeginPlay() override;
public:
	UFUNCTION(BlueprintCallable, Category = LGUI)
		class ULGUIPlayTween* GetPlayTween()const { return playTween; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void Play();
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void Stop();
};
