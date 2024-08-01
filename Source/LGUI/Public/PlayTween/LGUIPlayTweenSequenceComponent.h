// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Event/LGUIEventDelegate.h"
#include "PrefabSystem/ILGUIPrefabInterface.h"
#include "LGUIPlayTweenSequenceComponent.generated.h"

//play tween array sequentially, one after one.
UCLASS(ClassGroup = (LGUI), meta = (BlueprintSpawnableComponent), Blueprintable)
class LGUI_API ULGUIPlayTweenSequenceComponent : public UActorComponent, public ILGUIPrefabInterface
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool playOnStart = true;
	/**
	 * Play next tween when tween cycle complete, or wait until all loop complete (which could stuck at single tween if the tween's loop is infinite).
	 */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool bPlayNextWhenCycleComplete = false;
	/** Play tween array sequentially, one after one. */
	UPROPERTY(EditAnywhere, Category = "LGUI", Instanced)
		TArray<TObjectPtr<class ULGUIPlayTween>> playTweenArray;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FLGUIEventDelegate onComplete = FLGUIEventDelegate(ELGUIEventDelegateParameterType::Empty);

	bool isPlaying = false;
	int currentTweenPlayIndex = 0;
	void OnTweenComplete();
	FDelegateHandle onCompleteDelegateHandle;
	FSimpleMulticastDelegate onComplete_Delegate;

	virtual void BeginPlay()override;
	// Begin ILGUIPrefabInterface
	virtual void Awake_Implementation()override;
	// End ILGUIPrefabInterface
public:
	FDelegateHandle RegisterOnComplete(const FSimpleDelegate& InDelegate);
	FDelegateHandle RegisterOnComplete(const TFunction<void()>& InFunction);
	void UnregisterOnComplete(const FDelegateHandle& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FLGUIDelegateHandleWrapper RegisterOnComplete(const FLGUIPlayTweenCompleteDynamicDelegate& InDelegate);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void UnregisterOnComplete(const FLGUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI)
	void Play();
	UFUNCTION(BlueprintCallable, Category = LGUI)
	void Stop();
};
