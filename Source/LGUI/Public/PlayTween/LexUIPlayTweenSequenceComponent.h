// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "LexUIPlayTween.h"
#include "Core/LexUIBehaviour.h"
#include "LexUIPlayTweenSequenceComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLexUIPlayTweenSequenceCompleteDynamicDelegate);

//play tween array sequentially, one after one.
UCLASS(ClassGroup = (LGUI), meta = (BlueprintSpawnableComponent), Blueprintable)
class LGUI_API ULexUIPlayTweenSequenceComponent : public ULexUIBehaviour
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool bPlayOnStart = true;
	/**
	 * Play next tween when tween cycle complete, or wait until all loop complete (which could stuck at single tween if the tween's loop is infinite).
	 */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool bPlayNextWhenCycleComplete = false;
	/** Play tween array sequentially, one after one. */
	UPROPERTY(EditAnywhere, Category = "LGUI", Instanced)
		TArray<TObjectPtr<class ULexUIPlayTween>> PlayTweenArray;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FLexUIEventDelegate OnComplete = FLexUIEventDelegate(ELexUIEventDelegateParameterType::Empty);

	bool bIsPlaying = false;
	int CurrentTweenPlayIndex = 0;
	void OnTweenComplete();
	FDelegateHandle OnCompleteDelegateHandle;
	DECLARE_EVENT(ULexUIPlayTweenSequenceComponent, FOnCompleteEvent);

	virtual void Awake()override;
public:
	FOnCompleteEvent OnCompleteCPP;
	UPROPERTY(BlueprintAssignable, Category = "LGUI", meta=(DisplayName="OnComplete"))
	FOnLexUIPlayTweenSequenceCompleteDynamicDelegate OnCompleteBP;

	UFUNCTION(BlueprintCallable, Category = LGUI)
	void Play();
	UFUNCTION(BlueprintCallable, Category = LGUI)
	void Stop();
};
