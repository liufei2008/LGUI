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
#include "Tweener/LTweenerUpdate.h"

#include "LTweenerSequence.h"

#include "Engine/World.h"
#include "Engine/Engine.h"

ULTweenTickHelperComponent::ULTweenTickHelperComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickGroup = ETickingGroup::TG_DuringPhysics;
	PrimaryComponentTick.bTickEvenWhenPaused = true;
}
void ULTweenTickHelperComponent::BeginPlay()
{
	Super::BeginPlay();
}
void ULTweenTickHelperComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (Target.IsValid())
	{
		Target->Tick((ELTweenTickType)((uint8)PrimaryComponentTick.TickGroup), DeltaTime);
	}
}
void ULTweenTickHelperComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

ALTweenTickHelperActor::ALTweenTickHelperActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.TickGroup = ETickingGroup::TG_DuringPhysics;
	PrimaryActorTick.bTickEvenWhenPaused = true;

	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
}
void ALTweenTickHelperActor::BeginPlay()
{
	Super::BeginPlay();
	if (auto LTweenManager = ULTweenManager::GetLTweenInstance(this))
	{
		SetupTick(LTweenManager);
	}
	else
	{
		//If GameInstance subsystem not created yet, then register a event to wait it create
		OnLTweenManagerCreatedDelegateHandle = ULTweenManager::OnLTweenManagerCreated.AddUObject(this, &ALTweenTickHelperActor::OnLTweenManagerCreated);
	}
}
void ALTweenTickHelperActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (Target.IsValid())
	{
		Target->Tick(ELTweenTickType::DuringPhysics, DeltaSeconds);
	}
}
void ALTweenTickHelperActor::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (OnLTweenManagerCreatedDelegateHandle.IsValid())
	{
		ULTweenManager::OnLTweenManagerCreated.Remove(OnLTweenManagerCreatedDelegateHandle);
	}
}
void ALTweenTickHelperActor::OnLTweenManagerCreated(ULTweenManager* LTweenManager)
{
	SetupTick(LTweenManager);
}
void ALTweenTickHelperActor::SetupTick(ULTweenManager* LTweenManager)
{
	auto CreateComp = [LTweenManager, this](ETickingGroup TickingGroup, FName Name) {
		auto TickComp_DuringPhysics = NewObject<ULTweenTickHelperComponent>(this, Name);
		TickComp_DuringPhysics->SetTickGroup(TickingGroup);
		TickComp_DuringPhysics->RegisterComponent();
		TickComp_DuringPhysics->Target = LTweenManager;
		this->AddInstanceComponent(TickComp_DuringPhysics);
		};
	CreateComp(ETickingGroup::TG_PrePhysics, TEXT("PrePhysics"));
	CreateComp(ETickingGroup::TG_PostPhysics, TEXT("PostPhysics"));
	CreateComp(ETickingGroup::TG_PostUpdateWork, TEXT("PostUpdateWork"));
	this->Target = LTweenManager;
}



bool ULTweenTickHelperWorldSubsystem::ShouldCreateSubsystem(UObject* Outer) const 
{
	if (auto World = Outer->GetWorld())
	{
		if (World->IsGameWorld())
		{
			return true;
		}
	}
	return false;
}
void ULTweenTickHelperWorldSubsystem::PostInitialize()
{
	if (auto World = GetWorld())
	{
		if (World->IsGameWorld())
		{
			World->SpawnActor<ALTweenTickHelperActor>();
		}
	}
}


DECLARE_CYCLE_STAT(TEXT("LTween Update"), STAT_Update, STATGROUP_LTween);

FLTweenManagerCreated ULTweenManager::OnLTweenManagerCreated;
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

void ULTweenManager::Tick(ELTweenTickType TickType, float DeltaTime)
{
	if (bTickPaused)return;
	if (TickType == ELTweenTickType::Manual)
	{
		OnTick(TickType, DeltaTime, DeltaTime);
	}
	else
	{
		if (auto World = GetWorld())
		{
			OnTick(TickType, World->DeltaTimeSeconds, World->DeltaRealTimeSeconds);
		}
		else
		{
			OnTick(TickType, DeltaTime, DeltaTime);
		}
	}
}

#include "Kismet/GameplayStatics.h"
ULTweenManager* ULTweenManager::GetLTweenInstance(UObject* WorldContextObject)
{
	if (auto GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject))
		return GameInstance->GetSubsystem<ULTweenManager>();
	else
		return nullptr;
}

void ULTweenManager::OnTick(ELTweenTickType TickType, float DeltaTime, float UnscaledDeltaTime)
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
			if (tweener->GetTickType() != TickType)continue;
			if (tweener->ToNext(DeltaTime, UnscaledDeltaTime) == false)
			{
				tweenerList.RemoveAt(i);
				tweener->ConditionalBeginDestroy();
				i--;
				count--;
			}
		}
	}
	if (TickType == ELTweenTickType::DuringPhysics)
	{
		if (updateEvent.IsBound())
			updateEvent.Broadcast(DeltaTime);
	}
}

void ULTweenManager::CustomTick(float DeltaTime)
{
	OnTick(ELTweenTickType::DuringPhysics, DeltaTime, DeltaTime);
}

void ULTweenManager::DisableTick()
{
	bTickPaused = true;
}
void ULTweenManager::EnableTick()
{
	bTickPaused = false;
}
void ULTweenManager::ManualTick(float DeltaTime)
{
	Tick(ELTweenTickType::Manual, DeltaTime);
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

ULTweener* ULTweenManager::UpdateCall(UObject* WorldContextObject)
{
	auto Instance = GetLTweenInstance(WorldContextObject);
	if (!IsValid(Instance))return nullptr;

	auto tweener = NewObject<ULTweenerUpdate>(WorldContextObject);
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