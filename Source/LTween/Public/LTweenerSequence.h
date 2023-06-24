// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once
#include "LTweener.h"
#include "LTweenerSequence.generated.h"

UCLASS(NotBlueprintType)
class LTWEEN_API ULTweenerSequence:public ULTweener
{
	GENERATED_BODY()
private:
	UPROPERTY(VisibleAnywhere, Category = LTween)TArray<TObjectPtr<ULTweener>> tweenerList;
	UPROPERTY(VisibleAnywhere, Category = LTween)TArray<TObjectPtr<ULTweener>> finishedTweenerList;
	float lastTweenStartTime = 0;
public:
	/**
	 * Adds the given tween to the end of the Sequence.
	 * Not support Tweener type: Delay/ DelayFrame/ Virtual.
	 * Has no effect if the Sequence has already started.
	 */
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = LTween)
		ULTweenerSequence* Append(UObject* WorldContextObject, ULTweener* tweener);
	/**
	 * Adds the given interval to the end of the Sequence.
	 * Has no effect if the Sequence has already started.
	 * @param interval The interval duration
	 */
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = LTween)
		ULTweenerSequence* AppendInterval(UObject* WorldContextObject, float interval);
	/**
	 * Inserts the given tween at the given time position in the Sequence, automatically adding an interval if needed.
	 * Not support Tweener type: Delay/ DelayFrame/ Virtual.
	 * Has no effect if the Sequence has already started.
	 * @param timePosition The time position where the tween will be placed
	 */
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = LTween)
		ULTweenerSequence* Insert(UObject* WorldContextObject, float timePosition, ULTweener* tweener);
	/**
	 * Adds the given tween to the beginning of the Sequence, pushing forward the other nested content.
	 * Not support Tweener type: Delay/ DelayFrame/ Virtual.
	 * Has no effect if the Sequence has already started.
	 */
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = LTween)
		ULTweenerSequence* Prepend(UObject* WorldContextObject, ULTweener* tweener);
	/**
	 * Adds the given interval to the beginning of the Sequence, pushing forward the other nested content.
	 * Has no effect if the Sequence has already started.
	 * @param interval The interval duration
	 */
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = LTween)
		ULTweenerSequence* PrependInterval(UObject* WorldContextObject, float interval);
	/**
	 * Inserts the given tween at the same time position of the last tween added to the Sequence.
	 * Note that, in case of a Join after an interval, the insertion time will be the time where the interval starts, not where it finishes.
	 * Not support Tweener type: Delay/ DelayFrame/ Virtual.
	 * Has no effect if the Sequence has already started.
	 */
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = LTween)
		ULTweenerSequence* Join(UObject* WorldContextObject, ULTweener* tweener);

protected:
	virtual void OnStartGetValue() override {};
	virtual void TweenAndApplyValue(float currentTime) override;
	virtual void SetValueForIncremental() override;
	virtual void SetValueForYoyo() override;
	virtual void SetValueForRestart() override;
	virtual void SetOriginValueForRestart() override;

	virtual void Restart()override;
	virtual void Goto(float timePoint)override;
};