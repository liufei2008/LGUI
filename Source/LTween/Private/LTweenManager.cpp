// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LTweenManager.h"
#include "Tweener/LTweenerFloat.h"
#include "Tweener/LTweenerDouble.h"
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

#include "LTweenerSequence.h"

#include "Engine/World.h"
#include "Engine/Engine.h"

DECLARE_CYCLE_STAT(TEXT("LTween Update"), STAT_Update, STATGROUP_LTween);

//~USubsystem interface
void ULTweenManager::Initialize(FSubsystemCollectionBase& Collection)
{
	const UGameInstance* LocalGameInstance = GetGameInstance();
	check(LocalGameInstance);
}

void ULTweenManager::Deinitialize()
{
	tweenerList.Empty();
}

bool ULTweenManager::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}
//~End of USubsystem interface

//~FTickableObjectBase interface
void ULTweenManager::Tick(float DeltaTime)
{
	if (TickPaused == false)
	{
		OnTick(DeltaTime);
	}
}

ETickableTickType ULTweenManager::GetTickableTickType() const
{
	return ETickableTickType::Conditional;
}

bool ULTweenManager::IsTickable() const
{
	return !HasAnyFlags(RF_ClassDefaultObject);
}

TStatId ULTweenManager::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(ULTweenManager, STATGROUP_Tickables);
}

UWorld* ULTweenManager::GetTickableGameObjectWorld() const
{
	return GetGameInstance()->GetWorld();
}
//~End of FTickableObjectBase interface

#include "Kismet/GameplayStatics.h"
ULTweenManager* ULTweenManager::GetLTweenInstance(UObject* WorldContextObject)
{
	auto GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	return GameInstance != nullptr ? GameInstance->GetSubsystem<ULTweenManager>() : nullptr;
}

void ULTweenManager::OnTick(float DeltaTime)
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

void ULTweenManager::CustomTick(float DeltaTime)
{
	OnTick(DeltaTime);
}

void ULTweenManager::DisableTick()
{
	TickPaused = true;
}
void ULTweenManager::EnableTick()
{
	TickPaused = false;
}
void ULTweenManager::KillAllTweens(bool callComplete)
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
bool ULTweenManager::IsTweening(UObject* WorldContextObject, ULTweener* item)
{
	if (!IsValid(item))return false;

	auto Instance = GetLTweenInstance(WorldContextObject);
	if (!IsValid(Instance))return false;

	return Instance->tweenerList.Contains(item);
}
void ULTweenManager::KillIfIsTweening(UObject* WorldContextObject, ULTweener* item, bool callComplete)
{
	if (IsTweening(WorldContextObject, item))
	{
		item->Kill(callComplete);
	}
}
void ULTweenManager::RemoveTweener(UObject* WorldContextObject, ULTweener* item)
{
	if (!IsValid(item))return;

	auto Instance = GetLTweenInstance(WorldContextObject);
	if (!IsValid(Instance))return;
	Instance->tweenerList.Remove(item);
}
//float
ULTweener* ULTweenManager::To(UObject* WorldContextObject, const FLTweenFloatGetterFunction& getter, const FLTweenFloatSetterFunction& setter, float endValue, float duration)
{
	auto Instance = GetLTweenInstance(WorldContextObject);
	if (!IsValid(Instance))return nullptr;

	auto tweener = NewObject<ULTweenerFloat>(WorldContextObject);
	tweener->SetInitialValue(getter, setter, endValue, duration);
	Instance->tweenerList.Add(tweener);
	return tweener;
}
//float
ULTweener* ULTweenManager::To(UObject* WorldContextObject, const FLTweenDoubleGetterFunction& getter, const FLTweenDoubleSetterFunction& setter, double endValue, float duration)
{
	auto Instance = GetLTweenInstance(WorldContextObject);
	if (!IsValid(Instance))return nullptr;

	auto tweener = NewObject<ULTweenerDouble>(WorldContextObject);
	tweener->SetInitialValue(getter, setter, endValue, duration);
	Instance->tweenerList.Add(tweener);
	return tweener;
}
//interger
ULTweener* ULTweenManager::To(UObject* WorldContextObject, const FLTweenIntGetterFunction& getter, const FLTweenIntSetterFunction& setter, int endValue, float duration)
{
	auto Instance = GetLTweenInstance(WorldContextObject);
	if (!IsValid(Instance))return nullptr;

	auto tweener = NewObject<ULTweenerInteger>(WorldContextObject);
	tweener->SetInitialValue(getter, setter, endValue, duration);
	Instance->tweenerList.Add(tweener);
	return tweener;
}
//position
ULTweener* ULTweenManager::To(UObject* WorldContextObject, const FLTweenPositionGetterFunction& getter, const FLTweenPositionSetterFunction& setter, const FVector& endValue, float duration, bool sweep, FHitResult* sweepHitResult, ETeleportType teleportType)
{
	auto Instance = GetLTweenInstance(WorldContextObject);
	if (!IsValid(Instance))return nullptr;

	auto tweener = NewObject<ULTweenerPosition>(WorldContextObject);
	tweener->SetInitialValue(getter, setter, endValue, duration, sweep, sweepHitResult, teleportType);
	Instance->tweenerList.Add(tweener);
	return tweener;
}
//vector
ULTweener* ULTweenManager::To(UObject* WorldContextObject, const FLTweenVectorGetterFunction& getter, const FLTweenVectorSetterFunction& setter, const FVector& endValue, float duration)
{
	auto Instance = GetLTweenInstance(WorldContextObject);
	if (!IsValid(Instance))return nullptr;

	auto tweener = NewObject<ULTweenerVector>(WorldContextObject);
	tweener->SetInitialValue(getter, setter, endValue, duration);
	Instance->tweenerList.Add(tweener);
	return tweener;
}
//color
ULTweener* ULTweenManager::To(UObject* WorldContextObject, const FLTweenColorGetterFunction& getter, const FLTweenColorSetterFunction& setter, const FColor& endValue, float duration)
{
	auto Instance = GetLTweenInstance(WorldContextObject);
	if (!IsValid(Instance))return nullptr;

	auto tweener = NewObject<ULTweenerColor>(WorldContextObject);
	tweener->SetInitialValue(getter, setter, endValue, duration);
	Instance->tweenerList.Add(tweener);
	return tweener;
}
//linearcolor
ULTweener* ULTweenManager::To(UObject* WorldContextObject, const FLTweenLinearColorGetterFunction& getter, const FLTweenLinearColorSetterFunction& setter, const FLinearColor& endValue, float duration)
{
	auto Instance = GetLTweenInstance(WorldContextObject);
	if (!IsValid(Instance))return nullptr;

	auto tweener = NewObject<ULTweenerLinearColor>(WorldContextObject);
	tweener->SetInitialValue(getter, setter, endValue, duration);
	Instance->tweenerList.Add(tweener);
	return tweener;
}
//vector2d
ULTweener* ULTweenManager::To(UObject* WorldContextObject, const FLTweenVector2DGetterFunction& getter, const FLTweenVector2DSetterFunction& setter, const FVector2D& endValue, float duration)
{
	auto Instance = GetLTweenInstance(WorldContextObject);
	if (!IsValid(Instance))return nullptr;

	auto tweener = NewObject<ULTweenerVector2D>(WorldContextObject);
	tweener->SetInitialValue(getter, setter, endValue, duration);
	Instance->tweenerList.Add(tweener);
	return tweener;
}
//vector4
ULTweener* ULTweenManager::To(UObject* WorldContextObject, const FLTweenVector4GetterFunction& getter, const FLTweenVector4SetterFunction& setter, const FVector4& endValue, float duration)
{
	auto Instance = GetLTweenInstance(WorldContextObject);
	if (!IsValid(Instance))return nullptr;

	auto tweener = NewObject<ULTweenerVector4>(WorldContextObject);
	tweener->SetInitialValue(getter, setter, endValue, duration);
	Instance->tweenerList.Add(tweener);
	return tweener;
}
//quaternion
ULTweener* ULTweenManager::To(UObject* WorldContextObject, const FLTweenQuaternionGetterFunction& getter, const FLTweenQuaternionSetterFunction& setter, const FQuat& endValue, float duration)
{
	auto Instance = GetLTweenInstance(WorldContextObject);
	if (!IsValid(Instance))return nullptr;

	auto tweener = NewObject<ULTweenerQuaternion>(WorldContextObject);
	tweener->SetInitialValue(getter, setter, endValue, duration);
	Instance->tweenerList.Add(tweener);
	return tweener;
}
//rotator
ULTweener* ULTweenManager::To(UObject* WorldContextObject, const FLTweenRotatorGetterFunction& getter, const FLTweenRotatorSetterFunction& setter, const FRotator& endValue, float duration)
{
	auto Instance = GetLTweenInstance(WorldContextObject);
	if (!IsValid(Instance))return nullptr;

	auto tweener = NewObject<ULTweenerRotator>(WorldContextObject);
	tweener->SetInitialValue(getter, setter, endValue, duration);
	Instance->tweenerList.Add(tweener);
	return tweener;
}
//rotation euler
ULTweener* ULTweenManager::To(UObject* WorldContextObject, const FLTweenRotationQuatGetterFunction& getter, const FLTweenRotationQuatSetterFunction& setter, const FVector& eulerAngle, float duration, bool sweep, FHitResult* sweepHitResult, ETeleportType teleportType)
{
	auto Instance = GetLTweenInstance(WorldContextObject);
	if (!IsValid(Instance))return nullptr;

	auto tweener = NewObject<ULTweenerRotationEuler>(WorldContextObject);
	tweener->SetInitialValue(getter, setter, eulerAngle, duration, sweep, sweepHitResult, teleportType);
	Instance->tweenerList.Add(tweener);
	return tweener;
}
//rotation quat
ULTweener* ULTweenManager::To(UObject* WorldContextObject, const FLTweenRotationQuatGetterFunction& getter, const FLTweenRotationQuatSetterFunction& setter, const FQuat& endValue, float duration, bool sweep, FHitResult* sweepHitResult, ETeleportType teleportType)
{
	auto Instance = GetLTweenInstance(WorldContextObject);
	if (!IsValid(Instance))return nullptr;

	auto tweener = NewObject<ULTweenerRotationQuat>(WorldContextObject);
	tweener->SetInitialValue(getter, setter, endValue, duration, sweep, sweepHitResult, teleportType);
	Instance->tweenerList.Add(tweener);
	return tweener;
}
//material scalar
ULTweener* ULTweenManager::To(UObject* WorldContextObject, const FLTweenMaterialScalarGetterFunction& getter, const FLTweenMaterialScalarSetterFunction& setter, float endValue, float duration, int32 parameterIndex)
{
	auto Instance = GetLTweenInstance(WorldContextObject);
	if (!IsValid(Instance))return nullptr;

	auto tweener = NewObject<ULTweenerMaterialScalar>(WorldContextObject);
	tweener->SetInitialValue(getter, setter, endValue, duration, parameterIndex);
	Instance->tweenerList.Add(tweener);
	return tweener;
}
//material vector
ULTweener* ULTweenManager::To(UObject* WorldContextObject, const FLTweenMaterialVectorGetterFunction& getter, const FLTweenMaterialVectorSetterFunction& setter, const FLinearColor& endValue, float duration, int32 parameterIndex)
{
	auto Instance = GetLTweenInstance(WorldContextObject);
	if (!IsValid(Instance))return nullptr;

	auto tweener = NewObject<ULTweenerMaterialVector>(WorldContextObject);
	tweener->SetInitialValue(getter, setter, endValue, duration, parameterIndex);
	Instance->tweenerList.Add(tweener);
	return tweener;
}

ULTweener* ULTweenManager::VirtualTo(UObject* WorldContextObject, float duration)
{
	auto Instance = GetLTweenInstance(WorldContextObject);
	if (!IsValid(Instance))return nullptr;

	auto tweener = NewObject<ULTweenerVirtual>(WorldContextObject);
	tweener->SetInitialValue(duration);
	Instance->tweenerList.Add(tweener);
	return tweener;
}

ULTweener* ULTweenManager::DelayFrameCall(UObject* WorldContextObject, int delayFrame)
{
	auto Instance = GetLTweenInstance(WorldContextObject);
	if (!IsValid(Instance))return nullptr;

	auto tweener = NewObject<ULTweenerFrame>(WorldContextObject);
	tweener->SetInitialValue(delayFrame);
	Instance->tweenerList.Add(tweener);
	return tweener;
}

ULTweenerSequence* ULTweenManager::CreateSequence(UObject* WorldContextObject)
{
	auto Instance = GetLTweenInstance(WorldContextObject);
	if (!IsValid(Instance))return nullptr;

	auto tweener = NewObject<ULTweenerSequence>(WorldContextObject);
	Instance->tweenerList.Add(tweener);
	return tweener;
}

FDelegateHandle ULTweenManager::RegisterUpdateEvent(UObject* WorldContextObject, const FLTweenUpdateDelegate& update)
{
	auto Instance = GetLTweenInstance(WorldContextObject);
	if (!IsValid(Instance))return FDelegateHandle();

	return Instance->updateEvent.Add(update);
}
void ULTweenManager::UnregisterUpdateEvent(UObject* WorldContextObject, const FDelegateHandle& delegateHandle)
{
	auto Instance = GetLTweenInstance(WorldContextObject);
	if (!IsValid(Instance))return;

	Instance->updateEvent.Remove(delegateHandle);
}