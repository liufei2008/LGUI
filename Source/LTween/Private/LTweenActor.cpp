// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "LTweenActor.h"
#include "Tweener/LTweenerFloat.h"
#include "Tweener/LTweenerInteger.h"
#include "Tweener/LTweenerVector.h"
#include "Tweener/LTweenerColor.h"
#include "Tweener/LTweenerLinearColor.h"
#include "Tweener/LTweenerVector2D.h"
#include "Tweener/LTweenerVector4.h"
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
#include "Engine/Engine.h"

DECLARE_CYCLE_STAT(TEXT("LTween Update"), STAT_Update, STATGROUP_LTween);

TMap<UWorld*, ALTweenActor*> ALTweenActor::WorldToInstanceMap;

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
}

ALTweenActor* ALTweenActor::GetLTweenInstance(UObject* WorldContextObject)
{
	auto world = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (auto InstancePtr = WorldToInstanceMap.Find(world))
	{
		return *InstancePtr;
	}
	else
	{
		if (IsValid(world))
		{
			FActorSpawnParameters param = FActorSpawnParameters();
			param.ObjectFlags = RF_Transient;
			auto Instance = world->SpawnActor<ALTweenActor>(param);
			Instance->existInInstanceMap = true;
			WorldToInstanceMap.Add(world, Instance);
			UE_LOG(LTween, Log, TEXT("[ALTweenActor::InitCheck]No Instance for LTweenActor, create!"));
			return Instance;
		}
		else
		{
			UE_LOG(LTween, Error, TEXT("[ALTweenActor::InitCheck]Get world fail, cannot create LTweenActor!"));
			return nullptr;
		}
	}
}

void ALTweenActor::BeginDestroy()
{
	Super::BeginDestroy();
	tweenerList.Empty();
	if (WorldToInstanceMap.Num() > 0 && existInInstanceMap)
	{
		bool removed = false;
		if (auto world = this->GetWorld())
		{
			WorldToInstanceMap.Remove(world);
			removed = true;
		}
		else
		{
			world = nullptr;
			for (auto keyValue : WorldToInstanceMap)
			{
				if (keyValue.Value == this)
				{
					world = keyValue.Key;
				}
			}
			if (world != nullptr)
			{
				WorldToInstanceMap.Remove(world);
				removed = true;
			}
		}
		if (removed)
		{
			existInInstanceMap = false;
		}
		else
		{
			UE_LOG(LTween, Warning, TEXT("[ALTweenActor::BeginDestroy]Instance not removed!"));
		}
	}
	if (WorldToInstanceMap.Num() <= 0)
	{
		UE_LOG(LTween, Log, TEXT("[ALTweenActor::BeginDestroy]All instance removed."));
	}
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
	for (int32 i = 0; i < count; i++)
	{
		auto tweener = tweenerList[i];
		if (!IsValid(tweener))
		{
			tweenerList.RemoveAt(i);
			i--;
			count--;
		}
		else
		{
			if (tweener->ToNext(DeltaTime) == false)
			{
				tweenerList.RemoveAt(i);
				tweener->ConditionalBeginDestroy();
				i--;
				count--;
			}
		}
	}
	if (updateEvent.IsBound())
		updateEvent.Broadcast(DeltaTime);
}

void ALTweenActor::CustomTick(float DeltaTime)
{
	OnTick(DeltaTime);
}

void ALTweenActor::DisableTick()
{
	TickPaused = true;
}
void ALTweenActor::EnableTick()
{
	TickPaused = false;
}
void ALTweenActor::KillAllTweens(bool callComplete)
{
	for (auto item : tweenerList)
	{
		if (IsValid(item))
		{
			item->Kill(callComplete);
		}
	}
	tweenerList.Reset();
}
bool ALTweenActor::IsTweening(UObject* WorldContextObject, ULTweener* item)
{
	auto Instance = GetLTweenInstance(WorldContextObject);
	if (!IsValid(Instance))return false;

	if (!IsValid(item))return false;
	return Instance->tweenerList.Contains(item);
}
void ALTweenActor::KillIfIsTweening(UObject* WorldContextObject, ULTweener* item, bool callComplete)
{
	if (IsTweening(WorldContextObject, item))
	{
		item->Kill(callComplete);
	}
}
//float
ULTweener* ALTweenActor::To(UObject* WorldContextObject, const FLTweenFloatGetterFunction& getter, const FLTweenFloatSetterFunction& setter, float endValue, float duration)
{
	auto Instance = GetLTweenInstance(WorldContextObject);
	if (!IsValid(Instance))return nullptr;

	auto tweener = NewObject<ULTweenerFloat>(WorldContextObject);
	tweener->SetInitialValue(getter, setter, endValue, duration);
	Instance->tweenerList.Add(tweener);
	return tweener;
}
//interger
ULTweener* ALTweenActor::To(UObject* WorldContextObject, const FLTweenIntGetterFunction& getter, const FLTweenIntSetterFunction& setter, int endValue, float duration)
{
	auto Instance = GetLTweenInstance(WorldContextObject);
	if (!IsValid(Instance))return nullptr;

	auto tweener = NewObject<ULTweenerInteger>(WorldContextObject);
	tweener->SetInitialValue(getter, setter, endValue, duration);
	Instance->tweenerList.Add(tweener);
	return tweener;
}
//position
ULTweener* ALTweenActor::To(UObject* WorldContextObject, const FLTweenPositionGetterFunction& getter, const FLTweenPositionSetterFunction& setter, const FVector& endValue, float duration, bool sweep, FHitResult* sweepHitResult, ETeleportType teleportType)
{
	auto Instance = GetLTweenInstance(WorldContextObject);
	if (!IsValid(Instance))return nullptr;

	auto tweener = NewObject<ULTweenerPosition>(WorldContextObject);
	tweener->SetInitialValue(getter, setter, endValue, duration, sweep, sweepHitResult, teleportType);
	Instance->tweenerList.Add(tweener);
	return tweener;
}
//vector
ULTweener* ALTweenActor::To(UObject* WorldContextObject, const FLTweenVectorGetterFunction& getter, const FLTweenVectorSetterFunction& setter, const FVector& endValue, float duration)
{
	auto Instance = GetLTweenInstance(WorldContextObject);
	if (!IsValid(Instance))return nullptr;

	auto tweener = NewObject<ULTweenerVector>(WorldContextObject);
	tweener->SetInitialValue(getter, setter, endValue, duration);
	Instance->tweenerList.Add(tweener);
	return tweener;
}
//color
ULTweener* ALTweenActor::To(UObject* WorldContextObject, const FLTweenColorGetterFunction& getter, const FLTweenColorSetterFunction& setter, const FColor& endValue, float duration)
{
	auto Instance = GetLTweenInstance(WorldContextObject);
	if (!IsValid(Instance))return nullptr;

	auto tweener = NewObject<ULTweenerColor>(WorldContextObject);
	tweener->SetInitialValue(getter, setter, endValue, duration);
	Instance->tweenerList.Add(tweener);
	return tweener;
}
//linearcolor
ULTweener* ALTweenActor::To(UObject* WorldContextObject, const FLTweenLinearColorGetterFunction& getter, const FLTweenLinearColorSetterFunction& setter, const FLinearColor& endValue, float duration)
{
	auto Instance = GetLTweenInstance(WorldContextObject);
	if (!IsValid(Instance))return nullptr;

	auto tweener = NewObject<ULTweenerLinearColor>(WorldContextObject);
	tweener->SetInitialValue(getter, setter, endValue, duration);
	Instance->tweenerList.Add(tweener);
	return tweener;
}
//vector2d
ULTweener* ALTweenActor::To(UObject* WorldContextObject, const FLTweenVector2DGetterFunction& getter, const FLTweenVector2DSetterFunction& setter, const FVector2D& endValue, float duration)
{
	auto Instance = GetLTweenInstance(WorldContextObject);
	if (!IsValid(Instance))return nullptr;

	auto tweener = NewObject<ULTweenerVector2D>(WorldContextObject);
	tweener->SetInitialValue(getter, setter, endValue, duration);
	Instance->tweenerList.Add(tweener);
	return tweener;
}
//vector4
ULTweener* ALTweenActor::To(UObject* WorldContextObject, const FLTweenVector4GetterFunction& getter, const FLTweenVector4SetterFunction& setter, const FVector4& endValue, float duration)
{
	auto Instance = GetLTweenInstance(WorldContextObject);
	if (!IsValid(Instance))return nullptr;

	auto tweener = NewObject<ULTweenerVector4>(WorldContextObject);
	tweener->SetInitialValue(getter, setter, endValue, duration);
	Instance->tweenerList.Add(tweener);
	return tweener;
}
//quaternion
ULTweener* ALTweenActor::To(UObject* WorldContextObject, const FLTweenQuaternionGetterFunction& getter, const FLTweenQuaternionSetterFunction& setter, const FQuat& endValue, float duration)
{
	auto Instance = GetLTweenInstance(WorldContextObject);
	if (!IsValid(Instance))return nullptr;

	auto tweener = NewObject<ULTweenerQuaternion>(WorldContextObject);
	tweener->SetInitialValue(getter, setter, endValue, duration);
	Instance->tweenerList.Add(tweener);
	return tweener;
}
//rotator
ULTweener* ALTweenActor::To(UObject* WorldContextObject, const FLTweenRotatorGetterFunction& getter, const FLTweenRotatorSetterFunction& setter, const FRotator& endValue, float duration)
{
	auto Instance = GetLTweenInstance(WorldContextObject);
	if (!IsValid(Instance))return nullptr;

	auto tweener = NewObject<ULTweenerRotator>(WorldContextObject);
	tweener->SetInitialValue(getter, setter, endValue, duration);
	Instance->tweenerList.Add(tweener);
	return tweener;
}
//rotation euler
ULTweener* ALTweenActor::To(UObject* WorldContextObject, const FLTweenRotationQuatGetterFunction& getter, const FLTweenRotationQuatSetterFunction& setter, const FVector& eulerAngle, float duration, bool sweep, FHitResult* sweepHitResult, ETeleportType teleportType)
{
	auto Instance = GetLTweenInstance(WorldContextObject);
	if (!IsValid(Instance))return nullptr;

	auto tweener = NewObject<ULTweenerRotationEuler>(WorldContextObject);
	tweener->SetInitialValue(getter, setter, eulerAngle, duration, sweep, sweepHitResult, teleportType);
	Instance->tweenerList.Add(tweener);
	return tweener;
}
//rotation quat
ULTweener* ALTweenActor::To(UObject* WorldContextObject, const FLTweenRotationQuatGetterFunction& getter, const FLTweenRotationQuatSetterFunction& setter, const FQuat& endValue, float duration, bool sweep, FHitResult* sweepHitResult, ETeleportType teleportType)
{
	auto Instance = GetLTweenInstance(WorldContextObject);
	if (!IsValid(Instance))return nullptr;

	auto tweener = NewObject<ULTweenerRotationQuat>(WorldContextObject);
	tweener->SetInitialValue(getter, setter, endValue, duration, sweep, sweepHitResult, teleportType);
	Instance->tweenerList.Add(tweener);
	return tweener;
}
//material scalar
ULTweener* ALTweenActor::To(UObject* WorldContextObject, const FLTweenMaterialScalarGetterFunction& getter, const FLTweenMaterialScalarSetterFunction& setter, float endValue, float duration, int32 parameterIndex)
{
	auto Instance = GetLTweenInstance(WorldContextObject);
	if (!IsValid(Instance))return nullptr;

	auto tweener = NewObject<ULTweenerMaterialScalar>(WorldContextObject);
	tweener->SetInitialValue(getter, setter, endValue, duration, parameterIndex);
	Instance->tweenerList.Add(tweener);
	return tweener;
}
//material vector
ULTweener* ALTweenActor::To(UObject* WorldContextObject, const FLTweenMaterialVectorGetterFunction& getter, const FLTweenMaterialVectorSetterFunction& setter, const FLinearColor& endValue, float duration, int32 parameterIndex)
{
	auto Instance = GetLTweenInstance(WorldContextObject);
	if (!IsValid(Instance))return nullptr;

	auto tweener = NewObject<ULTweenerMaterialVector>(WorldContextObject);
	tweener->SetInitialValue(getter, setter, endValue, duration, parameterIndex);
	Instance->tweenerList.Add(tweener);
	return tweener;
}

ULTweener* ALTweenActor::VirtualTo(UObject* WorldContextObject, float duration)
{
	auto Instance = GetLTweenInstance(WorldContextObject);
	if (!IsValid(Instance))return nullptr;

	auto tweener = NewObject<ULTweenerVirtual>(WorldContextObject);
	tweener->SetInitialValue(duration);
	Instance->tweenerList.Add(tweener);
	return tweener;
}

ULTweener* ALTweenActor::DelayFrameCall(UObject* WorldContextObject, int delayFrame)
{
	auto Instance = GetLTweenInstance(WorldContextObject);
	if (!IsValid(Instance))return nullptr;

	auto tweener = NewObject<ULTweenerFrame>(WorldContextObject);
	tweener->SetInitialValue(delayFrame);
	Instance->tweenerList.Add(tweener);
	return tweener;
}


FDelegateHandle ALTweenActor::RegisterUpdateEvent(UObject* WorldContextObject, const LTweenUpdateDelegate& update)
{
	auto Instance = GetLTweenInstance(WorldContextObject);
	if (!IsValid(Instance))return FDelegateHandle();

	return Instance->updateEvent.Add(update);
}
void ALTweenActor::UnregisterUpdateEvent(UObject* WorldContextObject, const FDelegateHandle& delegateHandle)
{
	auto Instance = GetLTweenInstance(WorldContextObject);
	if (!IsValid(Instance))return;

	Instance->updateEvent.Remove(delegateHandle);
}