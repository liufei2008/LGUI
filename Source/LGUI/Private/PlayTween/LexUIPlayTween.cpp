// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "PlayTween/LexUIPlayTween.h"
#include "LGUI.h"
#include "LTweenManager.h"

void ULexUIPlayTween::Stop()
{
	ULTweenManager::KillIfIsTweening(this, Tweener, false);
}
void ULexUIPlayTween::Start()
{
	Tweener = ULTweenManager::To(this
		, FLTweenFloatGetterFunction::CreateLambda([] { return 0.0f; })
		, FLTweenFloatSetterFunction::CreateUObject(this, &ULexUIPlayTween::OnUpdate)
		, 1.0f, Duration);
	if(Tweener)
		Tweener
		->SetDelay(StartDelay)
		->SetLoop(LoopType, LoopCount)
		->SetEase(EaseType)
		->SetCurveFloat(EaseCurve)
		->OnStart([&] {
			OnStart.FireEvent();
			OnStartCPP.Broadcast();
			OnStartBP.Broadcast();
		})
		->OnUpdate([&](float progress) {
			OnUpdateProgress.FireEvent(progress);
			OnUpdateProgressCPP.Broadcast(progress);
			OnUpdateProgressBP.Broadcast(progress);
		})
		->OnCycleComplete([&] {
			OnCycleComplete.FireEvent();
			OnCycleCompleteCPP.Broadcast(Tweener->GetLoopCycleCount());
			OnCycleCompleteBP.Broadcast(Tweener->GetLoopCycleCount());
		})
		->OnComplete([&] {
			OnComplete.FireEvent();
			OnCompleteCPP.Broadcast();
			OnCompleteBP.Broadcast();
		})
		->SetAffectByGamePause(bAffectByGamePause)
		->SetAffectByTimeDilation(bAffectByTimeDilation);
}
