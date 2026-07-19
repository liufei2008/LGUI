// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/LexUIBehaviour.h"
#include "LexUIPlayTweenComponent.generated.h"


UCLASS(ClassGroup = (LGUI), meta = (BlueprintSpawnableComponent), Blueprintable)
class LGUI_API ULexUIPlayTweenComponent : public ULexUIBehaviour
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool bPlayOnStart = true;
	UPROPERTY(EditAnywhere, Category = "LGUI", Instanced)
		TObjectPtr<class ULexUIPlayTween> PlayTween;

	virtual void Awake() override;
public:
	UFUNCTION(BlueprintCallable, Category = LGUI)
		class ULexUIPlayTween* GetPlayTween()const { return PlayTween; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void Play();
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void Stop();
};
