// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Layout/UILayoutWithAnimation.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIItem.h"
#include "LTweenActor.h"
#include "LTweenBPLibrary.h"

void UUILayoutWithAnimation::CancelAnimation(bool callComplete)
{
	if (TweenerArray.Num() > 0)
	{
		for (auto item : TweenerArray)
		{
			if (IsValid(item))
			{
				ULTweenBPLibrary::KillIfIsTweening(this, item, callComplete);
			}
		}
		TweenerArray.Reset();
	}
}
