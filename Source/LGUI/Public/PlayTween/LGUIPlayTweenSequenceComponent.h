// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Core/LGUIBehaviour.h"
#include "LGUIPlayTweenSequenceComponent.generated.h"

//play tween array sequentially, one after one.
UCLASS(ClassGroup = (LGUI), meta = (BlueprintSpawnableComponent), Blueprintable)
class LGUI_API ULGUIPlayTweenSequenceComponent : public ULGUIBehaviour
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool playOnStart = true;
	UPROPERTY(EditAnywhere, Category = "LGUI", Instanced)
		TArray<class ULGUIPlayTween*> playTweenArray;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FLGUIDrawableEvent onComplete = FLGUIDrawableEvent(LGUIDrawableEventParameterType::Empty);

	bool isPlaying = false;
	int currentTweenPlayIndex = 0;
	void OnTweenComplete();

	virtual void Start() override;
public:
	void Play();
	void Stop();
};
