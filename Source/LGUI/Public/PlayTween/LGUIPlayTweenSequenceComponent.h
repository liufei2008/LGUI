// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Event/LGUIDrawableEvent.h"
#include "LGUIPlayTweenSequenceComponent.generated.h"

//play tween array sequentially, one after one.
UCLASS(ClassGroup = (LGUI), meta = (BlueprintSpawnableComponent), Blueprintable)
class LGUI_API ULGUIPlayTweenSequenceComponent : public UActorComponent
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool playOnStart = true;
	//play tween array sequentially, one after one.
	UPROPERTY(EditAnywhere, Category = "LGUI", Instanced)
		TArray<class ULGUIPlayTween*> playTweenArray;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FLGUIDrawableEvent onComplete = FLGUIDrawableEvent(LGUIDrawableEventParameterType::Empty);

	bool isPlaying = false;
	int currentTweenPlayIndex = 0;
	void OnTweenComplete();

	virtual void BeginPlay()override;
public:
	void Play();
	void Stop();
};
