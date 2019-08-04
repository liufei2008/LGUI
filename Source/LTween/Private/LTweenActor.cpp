// Copyright 2019 LexLiu. All Rights Reserved.

#include "LTweenActor.h"
#include "Tweener/LTweenerFloat.h"
#include "Tweener/LTweenerInteger.h"
#include "Tweener/LTweenerVector.h"
#include "Tweener/LTweenerColor.h"
#include "Tweener/LTweenerLinearColor.h"
#include "Tweener/LTweenerVector2D.h"
#include "Tweener/LTweenerPosition.h"
#include "Tweener/LTweenerQuaternion.h"
#include "Tweener/LTweenerRotator.h"
#include "Tweener/LTweenerRotationEuler.h"
#include "Tweener/LTweenerRotationQuat.h"
#include "Tweener/LTweenerMaterialScalar.h"
#include "Tweener/LTweenerMaterialVector.h"

#include "Tweener/LTweenerFrame.h"
#include "Tweener/LTweenerVirtual.h"

#include "Engine/World.h"

DECLARE_CYCLE_STAT(TEXT("LTween Update"), STAT_Update, STATGROUP_LTween);

ALTweenActor* ALTweenActor::Instance = nullptr;

ALTweenActor::ALTweenActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ALTweenActor::BeginPlay()
{
	Super::BeginPlay();
}

void ALTweenActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	tweenerList.Empty();
	Instance = nullptr;
}

void ALTweenActor::Tick( float DeltaTime )
{
	Super::Tick(DeltaTime);
	if (TickPaused == false)
	{
		OnTick(DeltaTime);
	}
}
void ALTweenActor::OnTick(float DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_Update);
	
	auto count = tweenerList.Num();
	for (int32 i = count - 1; i >= 0; i--)
	{
		auto tweener = tweenerList[i];
		if (IsValid(tweener) == false)
		{
			tweenerList.RemoveAt(i);
		}
		else
		{
			if (tweener->ToNext(DeltaTime) == false)
			{
				tweenerList.RemoveAt(i);
				tweener->ConditionalBeginDestroy();
			}
		}
	}
	if (updateEvent.IsBound())
		updateEvent.Broadcast(DeltaTime);
}

void ALTweenActor::CustomTick(float DeltaTime)
{
	if (Instance != nullptr)
	{
		Instance->OnTick(DeltaTime);
	}
}

bool ALTweenActor::InitCheck()
{
	if (Instance == nullptr)
	{
		if (GWorld != nullptr)
		{
			FActorSpawnParameters param = FActorSpawnParameters();
			param.ObjectFlags = RF_Transient;
			Instance = GWorld->SpawnActor<ALTweenActor>(param);
			UE_LOG(LTween, Log, TEXT("[ALTweenActor::InitCheck]No Instance for LTweenActor, create!"));
		}
		else
		{
			UE_LOG(LTween, Error, TEXT("[ALTweenActor::InitCheck]GWorld is null! This is weird..."));
			return false;
		}
	}
	return true;
}
void ALTweenActor::DisableTick()
{
	if (!InitCheck())return;
	Instance->TickPaused = true;
}
void ALTweenActor::EnableTick()
{
	if (!InitCheck())return;
	Instance->TickPaused = false;
}
bool ALTweenActor::IsTweening(ULTweener* item)
{
	if (!InitCheck())return false;
	if (item == nullptr)return false;
	if (!item->IsValidLowLevelFast())return false;
	if (!IsValid(item))return false;
	return Instance->tweenerList.Contains(item);
}
void ALTweenActor::KillIfIsTweening(ULTweener* item, bool callComplete)
{
	if (IsTweening(item))
	{
		item->Kill(callComplete);
	}
}
//float
ULTweener* ALTweenActor::To(const FloatGetterFunction& getter, const FloatSetterFunction& setter, float endValue, float duration)
{
	if (!InitCheck())return nullptr;
	auto tweener = NewObject<ULTweenerFloat>(Instance);
	tweener->SetInitialValue(getter, setter, endValue, duration);
	Instance->tweenerList.Add(tweener);
	return tweener;
}
//interger
ULTweener* ALTweenActor::To(const IntGetterFunction& getter, const IntSetterFunction& setter, int endValue, float duration)
{
	if (!InitCheck())return nullptr;
	auto tweener = NewObject<ULTweenerInteger>(Instance);
	tweener->SetInitialValue(getter, setter, endValue, duration);
	Instance->tweenerList.Add(tweener);
	return tweener;
}
//position
ULTweener* ALTweenActor::To(const PositionGetterFunction& getter, const PositionSetterFunction& setter, FVector endValue, float duration, bool sweep, FHitResult* sweepHitResult, ETeleportType teleportType)
{
	if (!InitCheck())return nullptr;
	auto tweener = NewObject<ULTweenerPosition>(Instance);
	tweener->SetInitialValue(getter, setter, endValue, duration, sweep, sweepHitResult, teleportType);
	Instance->tweenerList.Add(tweener);
	return tweener;
}
//vector
ULTweener* ALTweenActor::To(const VectorGetterFunction& getter, const VectorSetterFunction& setter, FVector endValue, float duration)
{
	if (!InitCheck())return nullptr;
	auto tweener = NewObject<ULTweenerVector>(Instance);
	tweener->SetInitialValue(getter, setter, endValue, duration);
	Instance->tweenerList.Add(tweener);
	return tweener;
}
//color
ULTweener* ALTweenActor::To(const ColorGetterFunction& getter, const ColorSetterFunction& setter, FColor endValue, float duration)
{
	if (!InitCheck())return nullptr;
	auto tweener = NewObject<ULTweenerColor>(Instance);
	tweener->SetInitialValue(getter, setter, endValue, duration);
	Instance->tweenerList.Add(tweener);
	return tweener;
}
//linearcolor
ULTweener* ALTweenActor::To(const LinearColorGetterFunction& getter, const LinearColorSetterFunction& setter, FLinearColor endValue, float duration)
{
	if (!InitCheck())return nullptr;
	auto tweener = NewObject<ULTweenerLinearColor>(Instance);
	tweener->SetInitialValue(getter, setter, endValue, duration);
	Instance->tweenerList.Add(tweener);
	return tweener;
}
//vector2d
ULTweener* ALTweenActor::To(const Vector2DGetterFunction& getter, const Vector2DSetterFunction& setter, FVector2D endValue, float duration)
{
	if (!InitCheck())return nullptr;
	auto tweener = NewObject<ULTweenerVector2D>(Instance);
	tweener->SetInitialValue(getter, setter, endValue, duration);
	Instance->tweenerList.Add(tweener);
	return tweener;
}
//quaternion
ULTweener* ALTweenActor::To(const QuaternionGetterFunction& getter, const QuaternionSetterFunction& setter, FQuat endValue, float duration)
{
	if (!InitCheck())return nullptr;
	auto tweener = NewObject<ULTweenerQuaternion>(Instance);
	tweener->SetInitialValue(getter, setter, endValue, duration);
	Instance->tweenerList.Add(tweener);
	return tweener;
}
//rotator
ULTweener* ALTweenActor::To(const RotatorGetterFunction& getter, const RotatorSetterFunction& setter, FRotator endValue, float duration)
{
	if (!InitCheck())return nullptr;
	auto tweener = NewObject<ULTweenerRotator>(Instance);
	tweener->SetInitialValue(getter, setter, endValue, duration);
	Instance->tweenerList.Add(tweener);
	return tweener;
}
//rotation euler
ULTweener* ALTweenActor::To(const RotationQuatGetterFunction& getter, const RotationQuatSetterFunction& setter, FVector eulerAngle, float duration, bool sweep, FHitResult* sweepHitResult, ETeleportType teleportType)
{
	if (!InitCheck())return nullptr;
	auto tweener = NewObject<ULTweenerRotationEuler>(Instance);
	tweener->SetInitialValue(getter, setter, eulerAngle, duration, sweep, sweepHitResult, teleportType);
	Instance->tweenerList.Add(tweener);
	return tweener;
}
//rotation quat
ULTweener* ALTweenActor::To(const RotationQuatGetterFunction& getter, const RotationQuatSetterFunction& setter, FQuat endValue, float duration, bool sweep, FHitResult* sweepHitResult, ETeleportType teleportType)
{
	if (!InitCheck())return nullptr;
	auto tweener = NewObject<ULTweenerRotationQuat>(Instance);
	tweener->SetInitialValue(getter, setter, endValue, duration, sweep, sweepHitResult, teleportType);
	Instance->tweenerList.Add(tweener);
	return tweener;
}
//material scalar
ULTweener* ALTweenActor::To(const MaterialScalarGetterFunction& getter, const MaterialScalarSetterFunction& setter, float endValue, float duration, int32 parameterIndex)
{
	if (!InitCheck())return nullptr;
	auto tweener = NewObject<ULTweenerMaterialScalar>(Instance);
	tweener->SetInitialValue(getter, setter, endValue, duration, parameterIndex);
	Instance->tweenerList.Add(tweener);
	return tweener;
}
//material vector
ULTweener* ALTweenActor::To(const MaterialVectorGetterFunction& getter, const MaterialVectorSetterFunction& setter, FLinearColor endValue, float duration, int32 parameterIndex)
{
	if (!InitCheck())return nullptr;
	auto tweener = NewObject<ULTweenerMaterialVector>(Instance);
	tweener->SetInitialValue(getter, setter, endValue, duration, parameterIndex);
	Instance->tweenerList.Add(tweener);
	return tweener;
}

ULTweener* ALTweenActor::VirtualTo(float duration)
{
	if (!InitCheck())return nullptr;
	auto tweener = NewObject<ULTweenerVirtual>(Instance);
	tweener->SetInitialValue(duration);
	Instance->tweenerList.Add(tweener);
	return tweener;
}

ULTweener* ALTweenActor::DelayFrameCall(int delayFrame)
{
	if (!InitCheck())return nullptr;
	auto tweener = NewObject<ULTweenerFrame>(Instance);
	tweener->SetInitialValue(delayFrame);
	Instance->tweenerList.Add(tweener);
	return tweener;
}


void ALTweenActor::RegisterUpdateEvent(const LTweenUpdateDelegate& update)
{
	if (!InitCheck())return;
	Instance->updateEvent.Add(update);
}
void ALTweenActor::UnregisterUpdateEvent(const LTweenUpdateDelegate& update)
{
	if (!InitCheck())return;
	Instance->updateEvent.Remove(update.GetHandle());
}
void ALTweenActor::UnregisterUpdateEvent(const FDelegateHandle& delegateHandle)
{
	if (!InitCheck())return;
	Instance->updateEvent.Remove(delegateHandle);
}