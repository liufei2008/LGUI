// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LTweenBPLibrary.h"
#include "LTween.h"
#include "Components/SceneComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/MeshComponent.h"


ULTweener* ULTweenBPLibrary::FloatTo(UObject* WorldContextObject, const FLTweenFloatSetterDynamic& setter, float startValue, float endValue, float duration, float delay, ELTweenEase ease)
{
	auto Tweener = ULTweenManager::To(WorldContextObject, FLTweenFloatGetterFunction::CreateLambda([startValue]
	{
		return startValue;
	}), FLTweenFloatSetterFunction::CreateLambda([setter](float value)
	{
		if (setter.IsBound())
			setter.Execute(value);
	}), endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
	}
	return Tweener;
}
ULTweener* ULTweenBPLibrary::DoubleTo(UObject* WorldContextObject, const FLTweenDoubleSetterDynamic& setter, double startValue, double endValue, float duration, float delay, ELTweenEase ease)
{
	auto Tweener = ULTweenManager::To(WorldContextObject, FLTweenDoubleGetterFunction::CreateLambda([startValue]
	{
		return startValue;
	}), FLTweenDoubleSetterFunction::CreateLambda([setter](auto value)
	{
		if (setter.IsBound())
			setter.Execute(value);
	}), endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
	}
	return Tweener;
}
ULTweener* ULTweenBPLibrary::IntTo(UObject* WorldContextObject, const FLTweenIntSetterDynamic& setter, int startValue, int endValue, float duration, float delay, ELTweenEase ease)
{
	auto Tweener = ULTweenManager::To(WorldContextObject, FLTweenIntGetterFunction::CreateLambda([startValue]
	{
		return startValue;
	}), FLTweenIntSetterFunction::CreateLambda([setter](int value)
	{
		if (setter.IsBound())
			setter.Execute(value);
	}), endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
	}
	return Tweener;
}
ULTweener* ULTweenBPLibrary::Vector2To(UObject* WorldContextObject, const FLTweenVector2SetterDynamic& setter, FVector2D startValue, FVector2D endValue, float duration, float delay, ELTweenEase ease)
{
	auto Tweener = ULTweenManager::To(WorldContextObject, FLTweenVector2DGetterFunction::CreateLambda([startValue]
	{
		return startValue;
	}), FLTweenVector2DSetterFunction::CreateLambda([setter](const FVector2D& value)
	{
		if (setter.IsBound())
			setter.Execute(value);
	}), endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
	}
	return Tweener;
}
ULTweener* ULTweenBPLibrary::Vector3To(UObject* WorldContextObject, const FLTweenVector3SetterDynamic& setter, FVector startValue, FVector endValue, float duration, float delay, ELTweenEase ease)
{
	auto Tweener = ULTweenManager::To(WorldContextObject, FLTweenVectorGetterFunction::CreateLambda([startValue]
	{
		return startValue;
	}), FLTweenVectorSetterFunction::CreateLambda([setter](const FVector& value)
	{
		if (setter.IsBound())
			setter.Execute(value);
	}), endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
	}
	return Tweener;
}
ULTweener* ULTweenBPLibrary::Vector4To(UObject* WorldContextObject, const FLTweenVector4SetterDynamic& setter, FVector4 startValue, FVector4 endValue, float duration, float delay, ELTweenEase ease)
{
	auto Tweener = ULTweenManager::To(WorldContextObject, FLTweenVector4GetterFunction::CreateLambda([startValue]
	{
		return startValue;
	}), FLTweenVector4SetterFunction::CreateLambda([setter](const FVector4& value)
	{
		if (setter.IsBound())
			setter.Execute(value);
	}), endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
	}
	return Tweener;
}
ULTweener* ULTweenBPLibrary::ColorTo(UObject* WorldContextObject, const FLTweenColorSetterDynamic& setter, FColor startValue, FColor endValue, float duration, float delay, ELTweenEase ease)
{
	auto Tweener = ULTweenManager::To(WorldContextObject, FLTweenColorGetterFunction::CreateLambda([startValue]
	{
		return startValue;
	}), FLTweenColorSetterFunction::CreateLambda([setter](const FColor& value)
	{
		if (setter.IsBound())
			setter.Execute(value);
	}), endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
	}
	return Tweener;
}
ULTweener* ULTweenBPLibrary::LinearColorTo(UObject* WorldContextObject, const FLTweenLinearColorSetterDynamic& setter, FLinearColor startValue, FLinearColor endValue, float duration, float delay, ELTweenEase ease)
{
	auto Tweener = ULTweenManager::To(WorldContextObject, FLTweenLinearColorGetterFunction::CreateLambda([startValue]
	{
		return startValue;
	}), FLTweenLinearColorSetterFunction::CreateLambda([setter](const FLinearColor& value)
	{
		if (setter.IsBound())
			setter.Execute(value);
	}), endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
	}
	return Tweener;
}
ULTweener* ULTweenBPLibrary::QuaternionTo(UObject* WorldContextObject, const FLTweenQuaternionSetterDynamic& setter, FQuat startValue, FQuat endValue, float duration, float delay, ELTweenEase ease)
{
	auto Tweener = ULTweenManager::To(WorldContextObject, FLTweenQuaternionGetterFunction::CreateLambda([startValue]
	{
		return startValue;
	}), FLTweenQuaternionSetterFunction::CreateLambda([setter](const FQuat& value)
	{
		if (setter.IsBound())
			setter.Execute(value);
	}), endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
	}
	return Tweener;
}
ULTweener* ULTweenBPLibrary::RotatorTo(UObject* WorldContextObject, const FLTweenRotatorSetterDynamic& setter, FRotator startValue, FRotator endValue, float duration, float delay, ELTweenEase ease)
{
	auto Tweener = ULTweenManager::To(WorldContextObject, FLTweenRotatorGetterFunction::CreateLambda([startValue]
	{
		return startValue;
	}), FLTweenRotatorSetterFunction::CreateLambda([setter](const FRotator& value)
	{
		if (setter.IsBound())
			setter.Execute(value);
	}), endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
	}
	return Tweener;
}

#pragma region LocationXYZ
ULTweener* ULTweenBPLibrary::RelativeLocationXTo(USceneComponent* target, double endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::RelativeLocationXTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto Tweener = ULTweenManager::To(target, FLTweenDoubleGetterFunction::CreateWeakLambda(target, [target]
	{
		return target->GetRelativeLocation().X;
	}), FLTweenDoubleSetterFunction::CreateWeakLambda(target, [target](auto value) {
		auto location = target->GetRelativeLocation();
		location.X = value;
		target->SetRelativeLocation(location);
	}), endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
	}
	return Tweener;
}
ULTweener* ULTweenBPLibrary::RelativeLocationYTo(USceneComponent* target, double endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::RelativeLocationYTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto Tweener = ULTweenManager::To(target, FLTweenDoubleGetterFunction::CreateWeakLambda(target, [target]
	{
		return target->GetRelativeLocation().Y;
	}), FLTweenDoubleSetterFunction::CreateWeakLambda(target, [target](auto value) {
		auto location = target->GetRelativeLocation();
		location.Y = value;
		target->SetRelativeLocation(location);
	}), endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
	}
	return Tweener;
}
ULTweener* ULTweenBPLibrary::RelativeLocationZTo(USceneComponent* target, double endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::RelativeLocationZTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto Tweener = ULTweenManager::To(target, FLTweenDoubleGetterFunction::CreateWeakLambda(target, [target] 
	{
		return target->GetRelativeLocation().Z;
	}), FLTweenDoubleSetterFunction::CreateWeakLambda(target, [=](auto value) {
		auto location = target->GetRelativeLocation();
		location.Z = value;
		target->SetRelativeLocation(location);
	}), endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
	}
	return Tweener;
}
ULTweener* ULTweenBPLibrary::RelativeLocationXTo_Sweep(USceneComponent* target, double endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::RelativeLocationXTo_Sweep] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto Tweener = ULTweenManager::To(target, FLTweenDoubleGetterFunction::CreateWeakLambda(target, [target]
	{
		return target->GetRelativeLocation().X;
	}), FLTweenDoubleSetterFunction::CreateWeakLambda(target, [target, &sweepHitResult, sweep, teleport](auto value) {
		auto location = target->GetRelativeLocation();
		location.X = value;
		target->SetRelativeLocation(location, teleport, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport));
	}), endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
	}
	return Tweener;
}
ULTweener* ULTweenBPLibrary::RelativeLocationYTo_Sweep(USceneComponent* target, double endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::RelativeLocationYTo_Sweep] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto Tweener = ULTweenManager::To(target, FLTweenDoubleGetterFunction::CreateWeakLambda(target, [target]
	{
		return target->GetRelativeLocation().Y;
	}), FLTweenDoubleSetterFunction::CreateWeakLambda(target, [target, &sweepHitResult, sweep, teleport](auto value) {
		auto location = target->GetRelativeLocation();
		location.Y = value;
		target->SetRelativeLocation(location, teleport, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport));
	}), endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
	}
	return Tweener;
}
ULTweener* ULTweenBPLibrary::RelativeLocationZTo_Sweep(USceneComponent* target, double endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::RelativeLocationZTo_Sweep] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto Tweener = ULTweenManager::To(target, FLTweenDoubleGetterFunction::CreateWeakLambda(target, [target]
	{
		return target->GetRelativeLocation().Z;
	}), FLTweenDoubleSetterFunction::CreateWeakLambda(target, [target, &sweepHitResult, sweep, teleport](auto value) {
		auto location = target->GetRelativeLocation();
		location.Z = value;
		target->SetRelativeLocation(location, teleport, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport));
	}), endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
	}
	return Tweener;
}



ULTweener* ULTweenBPLibrary::WorldLocationXTo(USceneComponent* target, double endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::WorldLocationXTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto Tweener = ULTweenManager::To(target, FLTweenDoubleGetterFunction::CreateWeakLambda(target, [target] 
	{
		return target->GetComponentLocation().X;
	}), FLTweenDoubleSetterFunction::CreateWeakLambda(target, [target](auto value) {
		auto location = target->GetComponentLocation();
		location.X = value;
		target->SetWorldLocation(location);
	}), endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
	}
	return Tweener;
}
ULTweener* ULTweenBPLibrary::WorldLocationYTo(USceneComponent* target, double endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::WorldLocationYTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto Tweener = ULTweenManager::To(target, FLTweenDoubleGetterFunction::CreateWeakLambda(target, [target]
	{
		return target->GetComponentLocation().Y;
	}), FLTweenDoubleSetterFunction::CreateWeakLambda(target, [target](auto value) {
		auto location = target->GetComponentLocation();
		location.Y = value;
		target->SetWorldLocation(location);
	}), endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
	}
	return Tweener;
}
ULTweener* ULTweenBPLibrary::WorldLocationZTo(USceneComponent* target, double endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::WorldLocationZTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto Tweener = ULTweenManager::To(target, FLTweenDoubleGetterFunction::CreateWeakLambda(target, [target]
	{
		return target->GetComponentLocation().Z;
	}), FLTweenDoubleSetterFunction::CreateWeakLambda(target, [target](auto value) {
		auto location = target->GetComponentLocation();
		location.Z = value;
		target->SetWorldLocation(location);
	}), endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
	}
	return Tweener;
}
ULTweener* ULTweenBPLibrary::WorldLocationXTo_Sweep(USceneComponent* target, double endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::WorldLocationXTo_Sweep] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto Tweener = ULTweenManager::To(target, FLTweenDoubleGetterFunction::CreateWeakLambda(target, [target]
	{
		return target->GetComponentLocation().X;
	}), FLTweenDoubleSetterFunction::CreateWeakLambda(target, [target, &sweepHitResult, sweep, teleport](auto value) {
		auto location = target->GetComponentLocation();
		location.X = value;
		target->SetWorldLocation(location, teleport, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport));
	}), endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
	}
	return Tweener;
}
ULTweener* ULTweenBPLibrary::WorldLocationYTo_Sweep(USceneComponent* target, double endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::WorldLocationYTo_Sweep] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto Tweener = ULTweenManager::To(target, FLTweenDoubleGetterFunction::CreateWeakLambda(target, [target]
	{
		return target->GetComponentLocation().Y;
	}), FLTweenDoubleSetterFunction::CreateWeakLambda(target, [target, &sweepHitResult, sweep, teleport](auto value) {
		auto location = target->GetComponentLocation();
		location.Y = value;
		target->SetWorldLocation(location, teleport, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport));
	}), endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
	}
	return Tweener;
}
ULTweener* ULTweenBPLibrary::WorldLocationZTo_Sweep(USceneComponent* target, double endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::WorldLocationZTo_Sweep] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto Tweener = ULTweenManager::To(target, FLTweenDoubleGetterFunction::CreateWeakLambda(target, [target] 
	{
		return target->GetComponentLocation().Z;
	}), FLTweenDoubleSetterFunction::CreateWeakLambda(target, [target, &sweepHitResult, sweep, teleport](auto value) {
		auto location = target->GetComponentLocation();
		location.Z = value;
		target->SetWorldLocation(location, teleport, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport));
	}), endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
	}
	return Tweener;
}
#pragma endregion LocationXYZ




#pragma region Location
ULTweener* ULTweenBPLibrary::RelativeLocationTo(USceneComponent* target, FVector endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::RelativeLocationTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto Tweener = ULTweenManager::To(target
	, FLTweenPositionGetterFunction::CreateUObject(target, &USceneComponent::GetRelativeLocation)
	, FLTweenPositionSetterFunction::CreateUObject(target, &USceneComponent::SetRelativeLocation)
	, endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
	}
	return Tweener;
}
ULTweener* ULTweenBPLibrary::WorldLocationTo(USceneComponent* target, FVector endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::WorldLocationTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto Tweener = ULTweenManager::To(target
	, FLTweenPositionGetterFunction::CreateUObject(target, &USceneComponent::GetComponentLocation)
	, FLTweenPositionSetterFunction::CreateUObject(target, &USceneComponent::SetWorldLocation)
	, endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
	}
	return Tweener;
}
ULTweener* ULTweenBPLibrary::RelativeLocationTo_Sweep(USceneComponent* target, FVector endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::RelativeLocationTo_Sweep] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto Tweener = ULTweenManager::To(target
	, FLTweenVectorGetterFunction::CreateUObject(target, &USceneComponent::GetRelativeLocation)
	, FLTweenPositionSetterFunction::CreateUObject(target, &USceneComponent::SetRelativeLocation)
	, endValue, duration, sweep, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport));
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
	}
	return Tweener;
}
ULTweener* ULTweenBPLibrary::WorldLocationTo_Sweep(USceneComponent* target, FVector endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::WorldLocationTo_Sweep] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto Tweener = ULTweenManager::To(target
	, FLTweenPositionGetterFunction::CreateUObject(target, &USceneComponent::GetComponentLocation)
	, FLTweenPositionSetterFunction::CreateUObject(target, &USceneComponent::SetRelativeLocation)
	, endValue, duration, sweep, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport));
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
	}
	return Tweener;
}
#pragma endregion Location



ULTweener* ULTweenBPLibrary::RelativeScaleTo(USceneComponent* target, FVector endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::RelativeScaleTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto Tweener = ULTweenManager::To(target
	, FLTweenVectorGetterFunction::CreateUObject(target, &USceneComponent::GetRelativeScale3D)
	, FLTweenVectorSetterFunction::CreateUObject(target, &USceneComponent::SetRelativeScale3D)
	, endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
	}
	return Tweener;
}


#pragma region Rotation
ULTweener* ULTweenBPLibrary::RelativeRotateEulerAngleTo(USceneComponent* target, FVector eulerAngle, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::RelativeRotateEulerAngleTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto Tweener = ULTweenManager::To(target, FLTweenRotationQuatGetterFunction::CreateWeakLambda(target, [target]
	{ 
		return target->GetRelativeRotationCache().GetCachedQuat();
	}), FLTweenRotationQuatSetterFunction::CreateUObject(target, &USceneComponent::SetRelativeRotation)
	, eulerAngle, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
	}
	return Tweener;
}
ULTweener* ULTweenBPLibrary::RelativeRotationQuaternionTo(USceneComponent* target, const FQuat& endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::RelativeRotationQuaternionTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto Tweener = ULTweenManager::To(target, FLTweenRotationQuatGetterFunction::CreateWeakLambda(target, [target]
	{
		return target->GetRelativeRotationCache().GetCachedQuat();
	}), FLTweenRotationQuatSetterFunction::CreateUObject(target, &USceneComponent::SetRelativeRotation)
	, endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
	}
	return Tweener;
}
ULTweener* ULTweenBPLibrary::RelativeRotateEulerAngleTo_Sweep(USceneComponent* target, FVector eulerAngle, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::RelativeRotateEulerAngleTo_Sweep] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto Tweener = ULTweenManager::To(target, FLTweenRotationQuatGetterFunction::CreateWeakLambda(target, [target]
	{ 
		return target->GetRelativeRotationCache().GetCachedQuat();
	}), FLTweenRotationQuatSetterFunction::CreateUObject(target, &USceneComponent::SetRelativeRotation)
	, eulerAngle, duration, sweep, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport));
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
	}
	return Tweener;
}
ULTweener* ULTweenBPLibrary::RelativeRotationQuaternionTo_Sweep(USceneComponent* target, const FQuat& endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::RelativeRotationQuaternionTo_Sweep] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto Tweener = ULTweenManager::To(target, FLTweenRotationQuatGetterFunction::CreateWeakLambda(target, [target]
	{
		return target->GetRelativeRotationCache().GetCachedQuat();
	}), FLTweenRotationQuatSetterFunction::CreateUObject(target, &USceneComponent::SetRelativeRotation)
	, endValue, duration, sweep, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport));
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
	}
	return Tweener;
}
ULTweener* ULTweenBPLibrary::RelativeRotatorTo(USceneComponent* target, FRotator endValue, bool shortestPath, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::RelativeRotatorTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	if (shortestPath)
	{
		return RelativeRotationQuaternionTo(target, endValue.Quaternion(), duration, delay, ease);
	}
	else
	{
		auto Tweener = ULTweenManager::To(target
		, FLTweenRotatorGetterFunction::CreateUObject(target, &USceneComponent::GetRelativeRotation)
		, FLTweenRotatorSetterFunction::CreateWeakLambda(target, [target] (FRotator value)
		{
			target->SetRelativeRotation(value);
		}), endValue, duration);
		if (Tweener)
		{
			Tweener->SetDelay(delay)->SetEase(ease);
		}
		return Tweener;
	}
}
ULTweener* ULTweenBPLibrary::RelativeRotatorTo_Sweep(USceneComponent* target, FRotator endValue, bool shortestPath, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::RelativeRotatorTo_Sweep] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	if (shortestPath)
	{
		return RelativeRotationQuaternionTo_Sweep(target, endValue.Quaternion(), sweepHitResult, sweep, teleport, duration, delay, ease);
	}
	else
	{
		auto Tweener = ULTweenManager::To(target
		, FLTweenRotatorGetterFunction::CreateUObject(target, &USceneComponent::GetRelativeRotation)
		, FLTweenRotatorSetterFunction::CreateWeakLambda(target, [target, &sweepHitResult, sweep, teleport](FRotator value)
		{
			target->SetRelativeRotation(value, sweep, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport));
		}), endValue, duration);
		if (Tweener)
		{
			Tweener->SetDelay(delay)->SetEase(ease);
		}
		return Tweener;
	}
}



ULTweener* ULTweenBPLibrary::WorldRotateEulerAngleTo(USceneComponent* target, FVector eulerAngle, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::WorldRotateEulerAngleTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto Tweener = ULTweenManager::To(target, FLTweenRotationQuatGetterFunction::CreateWeakLambda(target, [target]
	{
		return target->GetComponentRotation().Quaternion();
	}), FLTweenRotationQuatSetterFunction::CreateUObject(target, &USceneComponent::SetWorldRotation)
	, eulerAngle, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
	}
	return Tweener;
}
ULTweener* ULTweenBPLibrary::WorldRotationQuaternionTo(USceneComponent* target, const FQuat& endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::WorldRotationTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto Tweener = ULTweenManager::To(target, FLTweenRotationQuatGetterFunction::CreateWeakLambda(target, [target]
	{
		return target->GetComponentRotation().Quaternion();
	}), FLTweenRotationQuatSetterFunction::CreateUObject(target, &USceneComponent::SetWorldRotation)
	, endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
	}
	return Tweener;
}
ULTweener* ULTweenBPLibrary::WorldRotateEulerAngleTo_Sweep(USceneComponent* target, FVector eulerAngle, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::WorldRotateEulerAngleTo_Sweep] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto Tweener = ULTweenManager::To(target, FLTweenRotationQuatGetterFunction::CreateWeakLambda(target, [target]
	{
		return target->GetComponentRotation().Quaternion();
	}), FLTweenRotationQuatSetterFunction::CreateUObject(target, &USceneComponent::SetWorldRotation)
	, eulerAngle, duration, sweep, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport));
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
	}
	return Tweener;
}
ULTweener* ULTweenBPLibrary::WorldRotationQuaternionTo_Sweep(USceneComponent* target, const FQuat& endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::WorldRotationTo_Sweep] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto Tweener = ULTweenManager::To(target, FLTweenRotationQuatGetterFunction::CreateWeakLambda(target, [target]
	{
		return target->GetComponentRotation().Quaternion();
	}), FLTweenRotationQuatSetterFunction::CreateUObject(target, &USceneComponent::SetWorldRotation)
	, endValue, duration, sweep, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport));
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
	}
	return Tweener;
}
ULTweener* ULTweenBPLibrary::WorldRotatorTo(USceneComponent* target, FRotator endValue, bool shortestPath, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::WorldRotatorTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	if (shortestPath)
	{
		return WorldRotationQuaternionTo(target, endValue.Quaternion(), duration, delay, ease);
	}
	else
	{
		auto Tweener = ULTweenManager::To(target
		, FLTweenRotatorGetterFunction::CreateUObject(target, &USceneComponent::GetComponentRotation)
		, FLTweenRotatorSetterFunction::CreateWeakLambda(target, [target](FRotator value)
		{
			target->SetWorldRotation(value);
		}), endValue, duration);
		if (Tweener)
		{
			Tweener->SetDelay(delay)->SetEase(ease);
		}
		return Tweener;
	}
}
ULTweener* ULTweenBPLibrary::WorldRotatorTo_Sweep(USceneComponent* target, FRotator endValue, bool shortestPath, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::WorldRotatorTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	if (shortestPath)
	{
		return WorldRotationQuaternionTo_Sweep(target, endValue.Quaternion(), sweepHitResult, sweep, teleport, duration, delay, ease);
	}
	else
	{
		auto Tweener = ULTweenManager::To(target
		, FLTweenRotatorGetterFunction::CreateUObject(target, &USceneComponent::GetComponentRotation)
		, FLTweenRotatorSetterFunction::CreateWeakLambda(target, [target, &sweepHitResult, sweep, teleport](FRotator value)
		{
			target->SetWorldRotation(value, sweep, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport));
		}), endValue, duration);
		if (Tweener)
		{
			Tweener->SetDelay(delay)->SetEase(ease);
		}
		return Tweener;
	}
}
#pragma endregion Rotation


#pragma region Deprecated
ULTweener* ULTweenBPLibrary::LocalPositionXTo(USceneComponent* target, double endValue, float duration, float delay, ELTweenEase ease)
{
	return RelativeLocationXTo(target, endValue, duration, delay, ease);
}
ULTweener* ULTweenBPLibrary::LocalPositionYTo(USceneComponent* target, double endValue, float duration, float delay, ELTweenEase ease)
{
	return RelativeLocationYTo(target, endValue, duration, delay, ease);
}
ULTweener* ULTweenBPLibrary::LocalPositionZTo(USceneComponent* target, double endValue, float duration, float delay, ELTweenEase ease)
{
	return RelativeLocationZTo(target, endValue, duration, delay, ease);
}
ULTweener* ULTweenBPLibrary::LocalPositionXTo_Sweep(USceneComponent* target, double endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, ELTweenEase ease)
{
	return RelativeLocationXTo_Sweep(target, endValue, sweepHitResult, sweep, teleport, duration, delay, ease);
}
ULTweener* ULTweenBPLibrary::LocalPositionYTo_Sweep(USceneComponent* target, double endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, ELTweenEase ease)
{
	return RelativeLocationYTo_Sweep(target, endValue, sweepHitResult, sweep, teleport, duration, delay, ease);
}
ULTweener* ULTweenBPLibrary::LocalPositionZTo_Sweep(USceneComponent* target, double endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, ELTweenEase ease)
{
	return RelativeLocationZTo_Sweep(target, endValue, sweepHitResult, sweep, teleport, duration, delay, ease);
}
ULTweener* ULTweenBPLibrary::WorldPositionXTo(USceneComponent* target, double endValue, float duration, float delay, ELTweenEase ease)
{
	return WorldLocationXTo(target, endValue, duration, delay, ease);
}
ULTweener* ULTweenBPLibrary::WorldPositionYTo(USceneComponent* target, double endValue, float duration, float delay, ELTweenEase ease)
{
	return WorldLocationYTo(target, endValue, duration, delay, ease);
}
ULTweener* ULTweenBPLibrary::WorldPositionZTo(USceneComponent* target, double endValue, float duration, float delay, ELTweenEase ease)
{
	return WorldLocationZTo(target, endValue, duration, delay, ease);
}
ULTweener* ULTweenBPLibrary::WorldPositionXTo_Sweep(USceneComponent* target, double endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, ELTweenEase ease)
{
	return WorldLocationXTo_Sweep(target, endValue, sweepHitResult, sweep, teleport, duration, delay, ease);
}
ULTweener* ULTweenBPLibrary::WorldPositionYTo_Sweep(USceneComponent* target, double endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, ELTweenEase ease)
{
	return WorldLocationYTo_Sweep(target, endValue, sweepHitResult, sweep, teleport, duration, delay, ease);
}
ULTweener* ULTweenBPLibrary::WorldPositionZTo_Sweep(USceneComponent* target, double endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, ELTweenEase ease)
{
	return WorldLocationZTo_Sweep(target, endValue, sweepHitResult, sweep, teleport, duration, delay, ease);
}
ULTweener* ULTweenBPLibrary::LocalPositionTo(USceneComponent* target, FVector endValue, float duration, float delay, ELTweenEase ease)
{
	return RelativeLocationTo(target, endValue, duration, delay, ease);
}
ULTweener* ULTweenBPLibrary::LocalPositionTo_Sweep(USceneComponent* target, FVector endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, ELTweenEase ease)
{
	return RelativeLocationTo_Sweep(target, endValue, sweepHitResult, sweep, teleport, duration, delay, ease);
}
ULTweener* ULTweenBPLibrary::WorldPositionTo(USceneComponent* target, FVector endValue, float duration, float delay, ELTweenEase ease)
{
	return WorldLocationTo(target, endValue, duration, delay, ease);
}
ULTweener* ULTweenBPLibrary::WorldPositionTo_Sweep(USceneComponent* target, FVector endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, ELTweenEase ease)
{
	return WorldLocationTo_Sweep(target, endValue, sweepHitResult, sweep, teleport, duration, delay, ease);
}
ULTweener* ULTweenBPLibrary::LocalScaleTo(USceneComponent* target, FVector endValue, float duration, float delay, ELTweenEase ease)
{
	return RelativeScaleTo(target, endValue, duration, delay, ease);
}
ULTweener* ULTweenBPLibrary::LocalRotateEulerAngleTo(USceneComponent* target, FVector eulerAngle, float duration, float delay, ELTweenEase ease)
{
	return RelativeRotateEulerAngleTo(target, eulerAngle, duration, delay, ease);
}
ULTweener* ULTweenBPLibrary::LocalRotateEulerAngleTo_Sweep(USceneComponent* target, FVector eulerAngle, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, ELTweenEase ease)
{
	return RelativeRotateEulerAngleTo_Sweep(target, eulerAngle, sweepHitResult, sweep, teleport, duration, delay, ease);
}
ULTweener* ULTweenBPLibrary::LocalRotationQuaternionTo(USceneComponent* target, const FQuat& endValue, float duration, float delay, ELTweenEase ease)
{
	return RelativeRotationQuaternionTo(target, endValue, duration, delay, ease);
}
ULTweener* ULTweenBPLibrary::LocalRotationQuaternionTo_Sweep(USceneComponent* target, const FQuat& endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, ELTweenEase ease)
{
	return RelativeRotationQuaternionTo_Sweep(target, endValue, sweepHitResult, sweep, teleport, duration, delay, ease);
}
ULTweener* ULTweenBPLibrary::LocalRotatorTo(USceneComponent* target, FRotator endValue, bool shortestPath, float duration, float delay, ELTweenEase ease)
{
	return RelativeRotatorTo(target, endValue, shortestPath, duration, delay, ease);
}
ULTweener* ULTweenBPLibrary::LocalRotatorTo_Sweep(USceneComponent* target, FRotator endValue, bool shortestPath, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, ELTweenEase ease)
{
	return RelativeRotatorTo_Sweep(target, endValue, shortestPath, sweepHitResult, sweep, teleport, duration, delay, ease);
}
#pragma endregion Deprecated


#pragma region Material
ULTweener* ULTweenBPLibrary::MaterialScalarParameterTo(UObject* WorldContextObject, UMaterialInstanceDynamic* target, FName parameterName, float endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(WorldContextObject))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::MaterialScalarParameterTo] WorldContextObject is not valid!"));
		return nullptr;
	}
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::MaterialScalarParameterTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	float startValue = 0;
	int32 parameterIndex = 0;
	if (target->GetScalarParameterValue(parameterName, startValue))
	{
		target->InitializeScalarParameterAndGetIndex(parameterName, startValue, parameterIndex);
	}
	else
	{
		UE_LOG(LTween, Warning, TEXT("[ULTweenBPLibrary::MaterialScalarParameterTo]GetScalarParameterValue:%s error!"), *(parameterName.ToString()));
		return nullptr;
	}
	auto Tweener = ULTweenManager::To(WorldContextObject, FLTweenMaterialScalarGetterFunction::CreateWeakLambda(target, [target, parameterName](float& result)
	{
		return target->GetScalarParameterValue(parameterName, result);
	}), FLTweenMaterialScalarSetterFunction::CreateUObject(target, &UMaterialInstanceDynamic::SetScalarParameterByIndex)
	, endValue, duration, parameterIndex);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
	}
	return Tweener;
}
ULTweener* ULTweenBPLibrary::MaterialVectorParameterTo(UObject* WorldContextObject, UMaterialInstanceDynamic* target, FName parameterName, FLinearColor endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(WorldContextObject))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::MaterialVectorParameterTo] WorldContextObject is not valid!"));
		return nullptr;
	}
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::MaterialVectorParameterTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	FLinearColor startValue = FLinearColor();
	int32 parameterIndex = 0;
	if (target->GetVectorParameterValue(parameterName, startValue))
	{
		target->InitializeVectorParameterAndGetIndex(parameterName, startValue, parameterIndex);
	}
	else
	{
		UE_LOG(LTween, Warning, TEXT("[ULTweenBPLibrary::MaterialVectorParameterTo]GetVectorParameterValue:%s error!"), *(parameterName.ToString()));
		return nullptr;
	}
	auto Tweener = ULTweenManager::To(WorldContextObject, FLTweenMaterialVectorGetterFunction::CreateWeakLambda(target, [target, parameterName](FLinearColor& result)
	{
		return target->GetVectorParameterValue(parameterName, result);
	}), FLTweenMaterialVectorSetterFunction::CreateUObject(target, &UMaterialInstanceDynamic::SetVectorParameterByIndex)
	, endValue, duration, parameterIndex);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
	}
	return Tweener;
}

ULTweener* ULTweenBPLibrary::MeshMaterialScalarParameterTo(UPrimitiveComponent* target, int materialIndex, FName parameterName, float endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::MeshMaterialScalarParameterTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	float startValue = 0;
	auto material = target->CreateAndSetMaterialInstanceDynamic(materialIndex);
	int32 parameterIndex = 0;
	if (material->GetScalarParameterValue(parameterName, startValue))
	{
		material->InitializeScalarParameterAndGetIndex(parameterName, startValue, parameterIndex);
	}
	else
	{
		UE_LOG(LTween, Warning, TEXT("[ULTweenBPLibrary::MaterialScalarParameterTo]GetScalarParameterValue:%s error!"), *(parameterName.ToString()));
		return nullptr;
	}
	auto Tweener = ULTweenManager::To(material, FLTweenMaterialScalarGetterFunction::CreateWeakLambda(material, [material, parameterName](float& result)
	{
		return material->GetScalarParameterValue(parameterName, result);
	}), FLTweenMaterialScalarSetterFunction::CreateUObject(material, &UMaterialInstanceDynamic::SetScalarParameterByIndex)
	, endValue, duration, parameterIndex);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
	}
	return Tweener;
}
ULTweener* ULTweenBPLibrary::MeshMaterialVectorParameterTo(UPrimitiveComponent* target, int materialIndex, FName parameterName, FLinearColor endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::MeshMaterialVectorParameterTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	FLinearColor startValue = FLinearColor();
	auto material = target->CreateAndSetMaterialInstanceDynamic(materialIndex);
	int32 parameterIndex = 0;
	if (material->GetVectorParameterValue(parameterName, startValue))
	{
		material->InitializeVectorParameterAndGetIndex(parameterName, startValue, parameterIndex);
	}
	else
	{
		UE_LOG(LTween, Warning, TEXT("[ULTweenBPLibrary::MaterialVectorParameterTo]GetVectorParameterValue:%s error!"), *(parameterName.ToString()));
		return nullptr;
	}
	auto Tweener = ULTweenManager::To(material, FLTweenMaterialVectorGetterFunction::CreateWeakLambda(material, [material, parameterName](FLinearColor& result)
	{
		return material->GetVectorParameterValue(parameterName, result);
	}), FLTweenMaterialVectorSetterFunction::CreateUObject(material, &UMaterialInstanceDynamic::SetVectorParameterByIndex)
	, endValue, duration, parameterIndex);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
	}
	return Tweener;
}
#pragma endregion

#pragma region UMG
#include "Components/CanvasPanelSlot.h"
ULTweener* ULTweenBPLibrary::UMG_CanvasPanelSlot_PositionTo(UObject* WorldContextObject, UCanvasPanelSlot* target, const FVector2D& endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[%s] target is not valid:%s"), ANSI_TO_TCHAR(__FUNCTION__), *(target->GetPathName()));
		return nullptr;
	}
	auto Tweener = ULTweenManager::To(WorldContextObject
	, FLTweenVector2DGetterFunction::CreateUObject(target, &UCanvasPanelSlot::GetPosition)
	, FLTweenVector2DSetterFunction::CreateUObject(target, &UCanvasPanelSlot::SetPosition)
	, endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease)->SetAffectByGamePause(false)->SetAffectByTimeDilation(false);
	}
	return Tweener;
}
ULTweener* ULTweenBPLibrary::UMG_CanvasPanelSlot_SizeTo(UObject* WorldContextObject, class UCanvasPanelSlot* target, const FVector2D& endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[%s] target is not valid:%s"), ANSI_TO_TCHAR(__FUNCTION__), *(target->GetPathName()));
		return nullptr;
	}
	auto Tweener = ULTweenManager::To(WorldContextObject
	, FLTweenVector2DGetterFunction::CreateUObject(target, &UCanvasPanelSlot::GetSize)
	, FLTweenVector2DSetterFunction::CreateUObject(target, &UCanvasPanelSlot::SetSize)
	, endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease)->SetAffectByGamePause(false)->SetAffectByTimeDilation(false);
	}
	return Tweener;
}
#include "Components/HorizontalBoxSlot.h"
ULTweener* ULTweenBPLibrary::UMG_HorizontalBoxSlot_PaddingTo(UObject* WorldContextObject, UHorizontalBoxSlot* target, const FMargin& endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[%s] target is not valid:%s"), ANSI_TO_TCHAR(__FUNCTION__), *(target->GetPathName()));
		return nullptr;
	}
	auto endValueVector4 = FVector4(endValue.Left, endValue.Top, endValue.Right, endValue.Bottom);
	auto Tweener = ULTweenManager::To(WorldContextObject, FLTweenVector4GetterFunction::CreateWeakLambda(target, [=] 
	{
		auto padding = target->GetPadding();
		return FVector4(padding.Left, padding.Top, padding.Right, padding.Bottom);
	}), FLTweenVector4SetterFunction::CreateWeakLambda(target, [=](const FVector4& value) {
		target->SetPadding(FMargin(value.X, value.Y, value.Z, value.W));
	}), endValueVector4, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease)->SetAffectByGamePause(false)->SetAffectByTimeDilation(false);
	}
	return Tweener;
}
#include "Components/VerticalBoxSlot.h"
ULTweener* ULTweenBPLibrary::UMG_VerticalBoxSlot_PaddingTo(UObject* WorldContextObject, UVerticalBoxSlot* target, const FMargin& endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[%s] target is not valid:%s"), ANSI_TO_TCHAR(__FUNCTION__), *(target->GetPathName()));
		return nullptr;
	}
	auto endValueVector4 = FVector4(endValue.Left, endValue.Top, endValue.Right, endValue.Bottom);
	auto Tweener = ULTweenManager::To(WorldContextObject, FLTweenVector4GetterFunction::CreateWeakLambda(target, [=] 
	{
		auto padding = target->GetPadding();
		return FVector4(padding.Left, padding.Top, padding.Right, padding.Bottom);
	}), FLTweenVector4SetterFunction::CreateWeakLambda(target, [=](const FVector4& value) {
		target->SetPadding(FMargin(value.X, value.Y, value.Z, value.W));
	}), endValueVector4, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease)->SetAffectByGamePause(false)->SetAffectByTimeDilation(false);
	}
	return Tweener;
}
#include "Components/OverlaySlot.h"
ULTweener* ULTweenBPLibrary::UMG_OverlaySlot_PaddingTo(UObject* WorldContextObject, UOverlaySlot* target, const FMargin& endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[%s] target is not valid:%s"), ANSI_TO_TCHAR(__FUNCTION__), *(target->GetPathName()));
		return nullptr;
	}
	auto endValueVector4 = FVector4(endValue.Left, endValue.Top, endValue.Right, endValue.Bottom);
	auto Tweener = ULTweenManager::To(WorldContextObject, FLTweenVector4GetterFunction::CreateWeakLambda(target, [=]
	{
		auto padding = target->GetPadding();
		return FVector4(padding.Left, padding.Top, padding.Right, padding.Bottom);
	}), FLTweenVector4SetterFunction::CreateWeakLambda(target, [=](const FVector4& value) {
		target->SetPadding(FMargin(value.X, value.Y, value.Z, value.W));
	}), endValueVector4, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease)->SetAffectByGamePause(false)->SetAffectByTimeDilation(false);
	}
	return Tweener;
}
#include "Components/ButtonSlot.h"
ULTweener* ULTweenBPLibrary::UMG_ButtonSlot_PaddingTo(UObject* WorldContextObject, UButtonSlot* target, const FMargin& endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[%s] target is not valid:%s"), ANSI_TO_TCHAR(__FUNCTION__), *(target->GetPathName()));
		return nullptr;
	}
	auto endValueVector4 = FVector4(endValue.Left, endValue.Top, endValue.Right, endValue.Bottom);
	auto Tweener = ULTweenManager::To(WorldContextObject, FLTweenVector4GetterFunction::CreateWeakLambda(target, [=]
	{
		auto padding = target->GetPadding();
		return FVector4(padding.Left, padding.Top, padding.Right, padding.Bottom);
	}), FLTweenVector4SetterFunction::CreateWeakLambda(target, [=](const FVector4& value) {
		target->SetPadding(FMargin(value.X, value.Y, value.Z, value.W));
	}), endValueVector4, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease)->SetAffectByGamePause(false)->SetAffectByTimeDilation(false);
	}
	return Tweener;
}
#include "Components/BorderSlot.h"
ULTweener* ULTweenBPLibrary::UMG_BorderSlot_PaddingTo(UObject* WorldContextObject, UBorderSlot* target, const FMargin& endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[%s] target is not valid:%s"), ANSI_TO_TCHAR(__FUNCTION__), *(target->GetPathName()));
		return nullptr;
	}
	auto endValueVector4 = FVector4(endValue.Left, endValue.Top, endValue.Right, endValue.Bottom);
	auto Tweener = ULTweenManager::To(WorldContextObject, FLTweenVector4GetterFunction::CreateWeakLambda(target, [=]
	{
		auto padding = target->GetPadding();
		return FVector4(padding.Left, padding.Top, padding.Right, padding.Bottom);
	}), FLTweenVector4SetterFunction::CreateWeakLambda(target, [=](const FVector4& value) {
		target->SetPadding(FMargin(value.X, value.Y, value.Z, value.W));
	}), endValueVector4, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease)->SetAffectByGamePause(false)->SetAffectByTimeDilation(false);
	}
	return Tweener;
}

#include "Components/Widget.h"
ULTweener* ULTweenBPLibrary::UMG_RenderTransform_TranslationTo(UObject* WorldContextObject, class UWidget* target, const FVector2D& endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[%s] target is not valid:%s"), ANSI_TO_TCHAR(__FUNCTION__), *(target->GetPathName()));
		return nullptr;
	}
	auto Tweener = ULTweenManager::To(WorldContextObject, FLTweenVector2DGetterFunction::CreateWeakLambda(target, [=] 
	{
		return target->GetRenderTransform().Translation;
	}), FLTweenVector2DSetterFunction::CreateUObject(target, &UWidget::SetRenderTranslation)
	, endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease)->SetAffectByGamePause(false)->SetAffectByTimeDilation(false);
	}
	return Tweener;
}
ULTweener* ULTweenBPLibrary::UMG_RenderTransform_AngleTo(UObject* WorldContextObject, UWidget* target, float endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[%s] target is not valid:%s"), ANSI_TO_TCHAR(__FUNCTION__), *(target->GetPathName()));
		return nullptr;
	}
	auto Tweener = ULTweenManager::To(WorldContextObject
	, FLTweenFloatGetterFunction::CreateUObject(target, &UWidget::GetRenderTransformAngle)
	, FLTweenFloatSetterFunction::CreateUObject(target, &UWidget::SetRenderTransformAngle)
	, endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease)->SetAffectByGamePause(false)->SetAffectByTimeDilation(false);
	}
	return Tweener;
}
ULTweener* ULTweenBPLibrary::UMG_RenderTransform_ScaleTo(UObject* WorldContextObject, UWidget* target, const FVector2D& endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[%s] target is not valid:%s"), ANSI_TO_TCHAR(__FUNCTION__), *(target->GetPathName()));
		return nullptr;
	}
	auto Tweener = ULTweenManager::To(WorldContextObject, FLTweenVector2DGetterFunction::CreateWeakLambda(target, [=] 
	{
		return target->GetRenderTransform().Scale;
	}), FLTweenVector2DSetterFunction::CreateUObject(target, &UWidget::SetRenderScale)
	, endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease)->SetAffectByGamePause(false)->SetAffectByTimeDilation(false);
	}
	return Tweener;
}
ULTweener* ULTweenBPLibrary::UMG_RenderTransform_ShearTo(UObject* WorldContextObject, UWidget* target, const FVector2D& endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[%s] target is not valid:%s"), ANSI_TO_TCHAR(__FUNCTION__), *(target->GetPathName()));
		return nullptr;
	}
	auto Tweener = ULTweenManager::To(WorldContextObject, FLTweenVector2DGetterFunction::CreateWeakLambda(target, [=] 
	{
		return target->GetRenderTransform().Shear;
	}), FLTweenVector2DSetterFunction::CreateUObject(target, &UWidget::SetRenderShear)
	, endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease)->SetAffectByGamePause(false)->SetAffectByTimeDilation(false);
	}
	return Tweener;
}
ULTweener* ULTweenBPLibrary::UMG_RenderOpacityTo(UObject* WorldContextObject, UWidget* target, float endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[%s] target is not valid:%s"), ANSI_TO_TCHAR(__FUNCTION__), *(target->GetPathName()));
		return nullptr;
	}
	auto Tweener = ULTweenManager::To(WorldContextObject
	, FLTweenFloatGetterFunction::CreateUObject(target, &UWidget::GetRenderOpacity)
	, FLTweenFloatSetterFunction::CreateUObject(target, &UWidget::SetRenderOpacity)
	, endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease)->SetAffectByGamePause(false)->SetAffectByTimeDilation(false);
	}
	return Tweener;
}
#include "Blueprint/UserWidget.h"
ULTweener* ULTweenBPLibrary::UMG_UserWidget_ColorAndOpacityTo(UObject* WorldContextObject, UUserWidget* target, const FLinearColor& endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[%s] target is not valid:%s"), ANSI_TO_TCHAR(__FUNCTION__), *(target->GetPathName()));
		return nullptr;
	}
	auto Tweener = ULTweenManager::To(WorldContextObject, FLTweenLinearColorGetterFunction::CreateWeakLambda(target, [=] 
	{
		return target->GetColorAndOpacity();
	}), FLTweenLinearColorSetterFunction::CreateUObject(target, &UUserWidget::SetColorAndOpacity)
	, endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease)->SetAffectByGamePause(false)->SetAffectByTimeDilation(false);
	}
	return Tweener;
}
#include "Components/Image.h"
ULTweener* ULTweenBPLibrary::UMG_Image_ColorAndOpacityTo(UObject* WorldContextObject, UImage* target, const FLinearColor& endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[%s] target is not valid:%s"), ANSI_TO_TCHAR(__FUNCTION__), *(target->GetPathName()));
		return nullptr;
	}
	auto Tweener = ULTweenManager::To(WorldContextObject, FLTweenLinearColorGetterFunction::CreateWeakLambda(target, [=] 
	{
		return target->GetColorAndOpacity();
	}), FLTweenLinearColorSetterFunction::CreateUObject(target, &UImage::SetColorAndOpacity)
	, endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease)->SetAffectByGamePause(false)->SetAffectByTimeDilation(false);
	}
	return Tweener;
}
#include "Components/Button.h"
ULTweener* ULTweenBPLibrary::UMG_Button_ColorAndOpacityTo(UObject* WorldContextObject, UButton* target, const FLinearColor& endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[%s] target is not valid:%s"), ANSI_TO_TCHAR(__FUNCTION__), *(target->GetPathName()));
		return nullptr;
	}
	auto Tweener = ULTweenManager::To(WorldContextObject, FLTweenLinearColorGetterFunction::CreateWeakLambda(target, [=] 
	{
		return target->GetColorAndOpacity();
	}), FLTweenLinearColorSetterFunction::CreateUObject(target, &UButton::SetColorAndOpacity)
	, endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease)->SetAffectByGamePause(false)->SetAffectByTimeDilation(false);
	}
	return Tweener;
}
#include "Components/Border.h"
ULTweener* ULTweenBPLibrary::UMG_Border_ContentColorAndOpacityTo(UObject* WorldContextObject, UBorder* target, const FLinearColor& endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[%s] target is not valid:%s"), ANSI_TO_TCHAR(__FUNCTION__), *(target->GetPathName()));
		return nullptr;
	}
	auto Tweener = ULTweenManager::To(WorldContextObject
	, FLTweenLinearColorGetterFunction::CreateUObject(target, &UBorder::GetContentColorAndOpacity)
	, FLTweenLinearColorSetterFunction::CreateUObject(target, &UBorder::SetContentColorAndOpacity)
	, endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease)->SetAffectByGamePause(false)->SetAffectByTimeDilation(false);
	}
	return Tweener;
}
#pragma endregion
