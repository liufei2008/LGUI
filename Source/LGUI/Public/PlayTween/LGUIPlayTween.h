// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once
#include "LTweener.h"
#include "Event/LGUIDrawableEvent.h"
#include "LGUIPlayTween.generated.h"


UCLASS(BlueprintType, Abstract, DefaultToInstanced, EditInlineNew)
class LGUI_API ULGUIPlayTween : public UObject
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Property")
		LTweenLoop loopType = LTweenLoop::Once;
	UPROPERTY(EditAnywhere, Category = "Property")
		LTweenEase easeType = LTweenEase::Linear;
	/** only valid if easeType=CurveFloat */
	UPROPERTY(EditAnywhere, Category = "Property", meta=(EditCondition = "easeType == LTweenEase::CurveFloat"))
		UCurveFloat* easeCurve;
	UPROPERTY(EditAnywhere, Category = "Property")
		float duration = 0.5f;
	UPROPERTY(EditAnywhere, Category = "Property")
		float startDelay = 0.0f;
	UPROPERTY(EditAnywhere, Category = "Event")
		FLGUIDrawableEvent onStart = FLGUIDrawableEvent(LGUIDrawableEventParameterType::Empty);
	//parameter float is the progress in range 0-1
	UPROPERTY(EditAnywhere, Category = "Event")
		FLGUIDrawableEvent onUpdateProgress = FLGUIDrawableEvent(LGUIDrawableEventParameterType::Float);
	UPROPERTY(EditAnywhere, Category = "Event")
		FLGUIDrawableEvent onComplete = FLGUIDrawableEvent(LGUIDrawableEventParameterType::Empty);
	UPROPERTY(Transient)
		ULTweener* tweener;
	FSimpleMulticastDelegate onComplete_Delegate;
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void Start();
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void Stop();
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		ULTweener* GetTweener()const { return tweener; }
	FDelegateHandle RegisterOnComplete(const FSimpleDelegate& InDelegate);
	void UnregisterOnComplete(const FDelegateHandle& InDelegateHandle);
protected:
	virtual void OnUpdate(float progress)PURE_VIRTUAL(ULGUIPlayTween::OnUpdate, );
};
