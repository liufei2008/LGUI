// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UILayoutBase.h"
#include "UILayoutWithAnimation.generated.h"

UCLASS(Abstract)
class LGUI_API UUILayoutWithAnimation : public UUILayoutBase
{
	GENERATED_BODY()

protected:

	friend class FUIGridLayoutCustomization;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		EUILayoutChangePositionAnimationType AnimationType = EUILayoutChangePositionAnimationType::Immediately;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float AnimationDuration = 0.3f;
	UPROPERTY(Transient)
		TArray<class ULTweener*> TweenerArray;

	void CancelAnimation(bool callComplete = false);
};
