// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LTweenBPLibrary.h"
#include "LTween.h"
#include "Components/SceneComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/MeshComponent.h"


ULTweener* ULTweenBPLibrary::FloatTo(UObject* WorldContextObject, FLTweenFloatSetterDynamic setter, float startValue, float endValue, float duration, float delay, ELTweenEase ease)
{
	return ULTweenManager::To(WorldContextObject, FLTweenFloatGetterFunction::CreateLambda([startValue]
	{
		return startValue;
	}), FLTweenFloatSetterFunction::CreateLambda([setter](float value)
	{
		if (setter.IsBound())
			setter.Execute(value);
	}), endValue, duration)->SetDelay(delay)->SetEase(ease);
}
ULTweener* ULTweenBPLibrary::DoubleTo(UObject* WorldContextObject, FLTweenDoubleSetterDynamic setter, double startValue, double endValue, float duration, float delay, ELTweenEase ease)
{
	return ULTweenManager::To(WorldContextObject, FLTweenDoubleGetterFunction::CreateLambda([startValue]
		{
			return startValue;
		}), FLTweenDoubleSetterFunction::CreateLambda([setter](auto value)
			{
				if (setter.IsBound())
					setter.Execute(value);
			}), endValue, duration)->SetDelay(delay)->SetEase(ease);
}
ULTweener* ULTweenBPLibrary::IntTo(UObject* WorldContextObject, FLTweenIntSetterDynamic setter, int startValue, int endValue, float duration, float delay, ELTweenEase ease)
{
	return ULTweenManager::To(WorldContextObject, FLTweenIntGetterFunction::CreateLambda([startValue]
	{
		return startValue;
	}), FLTweenIntSetterFunction::CreateLambda([setter](int value)
	{
		if (setter.IsBound())
			setter.Execute(value);
	}), endValue, duration)->SetDelay(delay)->SetEase(ease);
}
ULTweener* ULTweenBPLibrary::Vector2To(UObject* WorldContextObject, FLTweenVector2SetterDynamic setter, FVector2D startValue, FVector2D endValue, float duration, float delay, ELTweenEase ease)
{
	return ULTweenManager::To(WorldContextObject, FLTweenVector2DGetterFunction::CreateLambda([startValue]
	{
		return startValue;
	}), FLTweenVector2DSetterFunction::CreateLambda([setter](const FVector2D& value)
	{
		if (setter.IsBound())
			setter.Execute(value);
	}), endValue, duration)->SetDelay(delay)->SetEase(ease);
}
ULTweener* ULTweenBPLibrary::Vector3To(UObject* WorldContextObject, FLTweenVector3SetterDynamic setter, FVector startValue, FVector endValue, float duration, float delay, ELTweenEase ease)
{
	return ULTweenManager::To(WorldContextObject, FLTweenVectorGetterFunction::CreateLambda([startValue]
	{
		return startValue;
	}), FLTweenVectorSetterFunction::CreateLambda([setter](const FVector& value)
	{
		if (setter.IsBound())
			setter.Execute(value);
	}), endValue, duration)->SetDelay(delay)->SetEase(ease);
}
ULTweener* ULTweenBPLibrary::Vector4To(UObject* WorldContextObject, FLTweenVector4SetterDynamic setter, FVector4 startValue, FVector4 endValue, float duration, float delay, ELTweenEase ease)
{
	return ULTweenManager::To(WorldContextObject, FLTweenVector4GetterFunction::CreateLambda([startValue]
	{
		return startValue;
	}), FLTweenVector4SetterFunction::CreateLambda([setter](const FVector4& value)
	{
		if (setter.IsBound())
			setter.Execute(value);
	}), endValue, duration)->SetDelay(delay)->SetEase(ease);
}
ULTweener* ULTweenBPLibrary::ColorTo(UObject* WorldContextObject, FLTweenColorSetterDynamic setter, FColor startValue, FColor endValue, float duration, float delay, ELTweenEase ease)
{
	return ULTweenManager::To(WorldContextObject, FLTweenColorGetterFunction::CreateLambda([startValue]
	{
		return startValue;
	}), FLTweenColorSetterFunction::CreateLambda([setter](const FColor& value)
	{
		if (setter.IsBound())
			setter.Execute(value);
	}), endValue, duration)->SetDelay(delay)->SetEase(ease);
}
ULTweener* ULTweenBPLibrary::LinearColorTo(UObject* WorldContextObject, FLTweenLinearColorSetterDynamic setter, FLinearColor startValue, FLinearColor endValue, float duration, float delay, ELTweenEase ease)
{
	return ULTweenManager::To(WorldContextObject, FLTweenLinearColorGetterFunction::CreateLambda([startValue]
	{
		return startValue;
	}), FLTweenLinearColorSetterFunction::CreateLambda([setter](const FLinearColor& value)
	{
		if (setter.IsBound())
			setter.Execute(value);
	}), endValue, duration)->SetDelay(delay)->SetEase(ease);
}
ULTweener* ULTweenBPLibrary::QuaternionTo(UObject* WorldContextObject, FLTweenQuaternionSetterDynamic setter, FQuat startValue, FQuat endValue, float duration, float delay, ELTweenEase ease)
{
	return ULTweenManager::To(WorldContextObject, FLTweenQuaternionGetterFunction::CreateLambda([startValue]
	{
		return startValue;
	}), FLTweenQuaternionSetterFunction::CreateLambda([setter](const FQuat& value)
	{
		if (setter.IsBound())
			setter.Execute(value);
	}), endValue, duration)->SetDelay(delay)->SetEase(ease);
}
ULTweener* ULTweenBPLibrary::RotatorTo(UObject* WorldContextObject, FLTweenRotatorSetterDynamic setter, FRotator startValue, FRotator endValue, float duration, float delay, ELTweenEase ease)
{
	return ULTweenManager::To(WorldContextObject, FLTweenRotatorGetterFunction::CreateLambda([startValue]
	{
		return startValue;
	}), FLTweenRotatorSetterFunction::CreateLambda([setter](const FRotator& value)
	{
		if (setter.IsBound())
			setter.Execute(value);
	}), endValue, duration)->SetDelay(delay)->SetEase(ease);
}

#pragma region PositionXYZ
ULTweener* ULTweenBPLibrary::LocalPositionXTo(USceneComponent* target, double endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::LocalPositionXTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ULTweenManager::To(target, FLTweenDoubleGetterFunction::CreateWeakLambda(target, [target] {
		return target->GetRelativeLocation().X;
	}), FLTweenDoubleSetterFunction::CreateWeakLambda(target, [target](auto value) {
		auto location = target->GetRelativeLocation();
		location.X = value;
		target->SetRelativeLocation(location);
	}), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULTweenBPLibrary::LocalPositionYTo(USceneComponent* target, double endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::LocalPositionYTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ULTweenManager::To(target, FLTweenDoubleGetterFunction::CreateWeakLambda(target, [target] {
		return target->GetRelativeLocation().Y;
	}), FLTweenDoubleSetterFunction::CreateWeakLambda(target, [target](auto value) {
		auto location = target->GetRelativeLocation();
		location.Y = value;
		target->SetRelativeLocation(location);
	}), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULTweenBPLibrary::LocalPositionZTo(USceneComponent* target, double endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::LocalPositionZTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ULTweenManager::To(target, FLTweenDoubleGetterFunction::CreateWeakLambda(target, [target] {
		return target->GetRelativeLocation().Z;
	}), FLTweenDoubleSetterFunction::CreateWeakLambda(target, [=](auto value) {
		auto location = target->GetRelativeLocation();
		location.Z = value;
		target->SetRelativeLocation(location);
	}), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULTweenBPLibrary::LocalPositionXTo_Sweep(USceneComponent* target, double endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::LocalPositionXTo_Sweep] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ULTweenManager::To(target, FLTweenDoubleGetterFunction::CreateWeakLambda(target, [target] {
		return target->GetRelativeLocation().X;
	}), FLTweenDoubleSetterFunction::CreateWeakLambda(target, [target, &sweepHitResult, sweep, teleport](auto value) {
		auto location = target->GetRelativeLocation();
		location.X = value;
		target->SetRelativeLocation(location, teleport, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport));
	}), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULTweenBPLibrary::LocalPositionYTo_Sweep(USceneComponent* target, double endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::LocalPositionYTo_Sweep] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ULTweenManager::To(target, FLTweenDoubleGetterFunction::CreateWeakLambda(target, [target] {
		return target->GetRelativeLocation().Y;
	}), FLTweenDoubleSetterFunction::CreateWeakLambda(target, [target, &sweepHitResult, sweep, teleport](auto value) {
		auto location = target->GetRelativeLocation();
		location.Y = value;
		target->SetRelativeLocation(location, teleport, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport));
	}), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULTweenBPLibrary::LocalPositionZTo_Sweep(USceneComponent* target, double endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::LocalPositionZTo_Sweep] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ULTweenManager::To(target, FLTweenDoubleGetterFunction::CreateWeakLambda(target, [target] {
		return target->GetRelativeLocation().Z;
	}), FLTweenDoubleSetterFunction::CreateWeakLambda(target, [target, &sweepHitResult, sweep, teleport](auto value) {
		auto location = target->GetRelativeLocation();
		location.Z = value;
		target->SetRelativeLocation(location, teleport, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport));
	}), endValue, duration)->SetEase(ease)->SetDelay(delay);
}



ULTweener* ULTweenBPLibrary::WorldPositionXTo(USceneComponent* target, double endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::WorldPositionXTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ULTweenManager::To(target, FLTweenDoubleGetterFunction::CreateWeakLambda(target, [target] {
		return target->GetComponentLocation().X;
	}), FLTweenDoubleSetterFunction::CreateWeakLambda(target, [target](auto value) {
		auto location = target->GetComponentLocation();
		location.X = value;
		target->SetWorldLocation(location);
	}), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULTweenBPLibrary::WorldPositionYTo(USceneComponent* target, double endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::WorldPositionYTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ULTweenManager::To(target, FLTweenDoubleGetterFunction::CreateWeakLambda(target, [target] {
		return target->GetComponentLocation().Y;
	}), FLTweenDoubleSetterFunction::CreateWeakLambda(target, [target](auto value) {
		auto location = target->GetComponentLocation();
		location.Y = value;
		target->SetWorldLocation(location);
	}), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULTweenBPLibrary::WorldPositionZTo(USceneComponent* target, double endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::WorldPositionZTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ULTweenManager::To(target, FLTweenDoubleGetterFunction::CreateWeakLambda(target, [target] {
		return target->GetComponentLocation().Z;
	}), FLTweenDoubleSetterFunction::CreateWeakLambda(target, [target](auto value) {
		auto location = target->GetComponentLocation();
		location.Z = value;
		target->SetWorldLocation(location);
	}), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULTweenBPLibrary::WorldPositionXTo_Sweep(USceneComponent* target, double endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::WorldPositionXTo_Sweep] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ULTweenManager::To(target, FLTweenDoubleGetterFunction::CreateWeakLambda(target, [target] {
		return target->GetComponentLocation().X;
	}), FLTweenDoubleSetterFunction::CreateWeakLambda(target, [target, &sweepHitResult, sweep, teleport](auto value) {
		auto location = target->GetComponentLocation();
		location.X = value;
		target->SetWorldLocation(location, teleport, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport));
	}), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULTweenBPLibrary::WorldPositionYTo_Sweep(USceneComponent* target, double endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::WorldPositionYTo_Sweep] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ULTweenManager::To(target, FLTweenDoubleGetterFunction::CreateWeakLambda(target, [target] {
		return target->GetComponentLocation().Y;
	}), FLTweenDoubleSetterFunction::CreateWeakLambda(target, [target, &sweepHitResult, sweep, teleport](auto value) {
		auto location = target->GetComponentLocation();
		location.Y = value;
		target->SetWorldLocation(location, teleport, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport));
	}), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULTweenBPLibrary::WorldPositionZTo_Sweep(USceneComponent* target, double endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::WorldPositionZTo_Sweep] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ULTweenManager::To(target, FLTweenDoubleGetterFunction::CreateWeakLambda(target, [target] {
		return target->GetComponentLocation().Z;
	}), FLTweenDoubleSetterFunction::CreateWeakLambda(target, [target, &sweepHitResult, sweep, teleport](auto value) {
		auto location = target->GetComponentLocation();
		location.Z = value;
		target->SetWorldLocation(location, teleport, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport));
	}), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
#pragma endregion PositionXYZ




#pragma region Position
ULTweener* ULTweenBPLibrary::LocalPositionTo(USceneComponent* target, FVector endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::LocalPositionTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ULTweenManager::To(target, FLTweenPositionGetterFunction::CreateUObject(target, &USceneComponent::GetRelativeLocation), FLTweenPositionSetterFunction::CreateUObject(target, &USceneComponent::SetRelativeLocation), endValue, duration)
		->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULTweenBPLibrary::WorldPositionTo(USceneComponent* target, FVector endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::WorldPositionTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ULTweenManager::To(target, FLTweenPositionGetterFunction::CreateUObject(target, &USceneComponent::GetComponentLocation),
		FLTweenPositionSetterFunction::CreateUObject(target, &USceneComponent::SetWorldLocation), endValue, duration)
		->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULTweenBPLibrary::LocalPositionTo_Sweep(USceneComponent* target, FVector endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::LocalPositionTo_Sweep] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ULTweenManager::To(target, FLTweenVectorGetterFunction::CreateUObject(target, &USceneComponent::GetRelativeLocation), FLTweenPositionSetterFunction::CreateUObject(target, &USceneComponent::SetRelativeLocation), endValue, duration, sweep, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport))
		->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULTweenBPLibrary::WorldPositionTo_Sweep(USceneComponent* target, FVector endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::WorldPositionTo_Sweep] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ULTweenManager::To(target, FLTweenPositionGetterFunction::CreateUObject(target, &USceneComponent::GetComponentLocation),
		FLTweenPositionSetterFunction::CreateUObject(target, &USceneComponent::SetRelativeLocation), endValue, duration, sweep, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport))
		->SetEase(ease)->SetDelay(delay);
}
#pragma endregion Position



ULTweener* ULTweenBPLibrary::LocalScaleTo(USceneComponent* target, FVector endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::LocalScaleTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ULTweenManager::To(target, FLTweenVectorGetterFunction::CreateUObject(target, &USceneComponent::GetRelativeScale3D), FLTweenVectorSetterFunction::CreateUObject(target, &USceneComponent::SetRelativeScale3D), endValue, duration)
		->SetEase(ease)->SetDelay(delay);
}


#pragma region Rotation
ULTweener* ULTweenBPLibrary::LocalRotateEulerAngleTo(USceneComponent* target, FVector eulerAngle, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::LocalRotateEulerAngleTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ULTweenManager::To(target, FLTweenRotationQuatGetterFunction::CreateWeakLambda(target, [target]
	{ 
		return target->GetRelativeRotationCache().GetCachedQuat();
	}), FLTweenRotationQuatSetterFunction::CreateUObject(target, &USceneComponent::SetRelativeRotation), eulerAngle, duration)
		->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULTweenBPLibrary::LocalRotationQuaternionTo(USceneComponent* target, const FQuat& endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::LocalRotationTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ULTweenManager::To(target, FLTweenRotationQuatGetterFunction::CreateWeakLambda(target, [target]
	{
		return target->GetRelativeRotationCache().GetCachedQuat();
	}), FLTweenRotationQuatSetterFunction::CreateUObject(target, &USceneComponent::SetRelativeRotation), endValue, duration)
		->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULTweenBPLibrary::LocalRotateEulerAngleTo_Sweep(USceneComponent* target, FVector eulerAngle, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::LocalRotateEulerAngleTo_Sweep] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ULTweenManager::To(target, FLTweenRotationQuatGetterFunction::CreateWeakLambda(target, [target]
	{ 
		return target->GetRelativeRotationCache().GetCachedQuat();
	}), FLTweenRotationQuatSetterFunction::CreateUObject(target, &USceneComponent::SetRelativeRotation), eulerAngle, duration, sweep, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport))
		->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULTweenBPLibrary::LocalRotationQuaternionTo_Sweep(USceneComponent* target, const FQuat& endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::LocalRotationTo_Sweep] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ULTweenManager::To(target, FLTweenRotationQuatGetterFunction::CreateWeakLambda(target, [target]
	{
		return target->GetRelativeRotationCache().GetCachedQuat();
	}), FLTweenRotationQuatSetterFunction::CreateUObject(target, &USceneComponent::SetRelativeRotation), endValue, duration, sweep, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport))
		->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULTweenBPLibrary::LocalRotatorTo(USceneComponent* target, FRotator endValue, bool shortestPath, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::LocalRotatorTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	if (shortestPath)
	{
		return LocalRotationQuaternionTo(target, endValue.Quaternion(), duration, delay, ease);
	}
	else
	{
		return ULTweenManager::To(target, FLTweenRotatorGetterFunction::CreateUObject(target, &USceneComponent::GetRelativeRotation),
		FLTweenRotatorSetterFunction::CreateWeakLambda(target, [target] (FRotator value)
		{
			target->SetRelativeRotation(value);
		}), endValue, duration)->SetEase(ease)->SetDelay(delay);
	}
}
ULTweener* ULTweenBPLibrary::LocalRotatorTo_Sweep(USceneComponent* target, FRotator endValue, bool shortestPath, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::LocalRotatorTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	if (shortestPath)
	{
		return LocalRotationQuaternionTo_Sweep(target, endValue.Quaternion(), sweepHitResult, sweep, teleport, duration, delay, ease);
	}
	else
	{
		return ULTweenManager::To(target, FLTweenRotatorGetterFunction::CreateUObject(target, &USceneComponent::GetRelativeRotation), 
		FLTweenRotatorSetterFunction::CreateWeakLambda(target, [target, &sweepHitResult, sweep, teleport](FRotator value)
		{
			target->SetRelativeRotation(value, sweep, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport));
		}), endValue, duration)->SetEase(ease)->SetDelay(delay);
	}
}



ULTweener* ULTweenBPLibrary::WorldRotateEulerAngleTo(USceneComponent* target, FVector eulerAngle, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::WorldRotateEulerAngleTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ULTweenManager::To(target, FLTweenRotationQuatGetterFunction::CreateWeakLambda(target, [target]
	{
		return target->GetComponentRotation().Quaternion();
	}), FLTweenRotationQuatSetterFunction::CreateUObject(target, &USceneComponent::SetWorldRotation), eulerAngle, duration)
		->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULTweenBPLibrary::WorldRotationQuaternionTo(USceneComponent* target, const FQuat& endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::WorldRotationTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ULTweenManager::To(target, FLTweenRotationQuatGetterFunction::CreateWeakLambda(target, [target]
	{
		return target->GetComponentRotation().Quaternion();
	}), FLTweenRotationQuatSetterFunction::CreateUObject(target, &USceneComponent::SetWorldRotation), endValue, duration)
		->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULTweenBPLibrary::WorldRotateEulerAngleTo_Sweep(USceneComponent* target, FVector eulerAngle, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::WorldRotateEulerAngleTo_Sweep] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ULTweenManager::To(target, FLTweenRotationQuatGetterFunction::CreateWeakLambda(target, [target]
	{
		return target->GetComponentRotation().Quaternion();
	}), FLTweenRotationQuatSetterFunction::CreateUObject(target, &USceneComponent::SetWorldRotation), eulerAngle, duration, sweep, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport))
		->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULTweenBPLibrary::WorldRotationQuaternionTo_Sweep(USceneComponent* target, const FQuat& endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::WorldRotationTo_Sweep] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ULTweenManager::To(target, FLTweenRotationQuatGetterFunction::CreateWeakLambda(target, [target]
	{
		return target->GetComponentRotation().Quaternion();
	}), FLTweenRotationQuatSetterFunction::CreateUObject(target, &USceneComponent::SetWorldRotation), endValue, duration, sweep, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport))
		->SetEase(ease)->SetDelay(delay);
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
		return ULTweenManager::To(target, FLTweenRotatorGetterFunction::CreateUObject(target, &USceneComponent::GetComponentRotation), 
		FLTweenRotatorSetterFunction::CreateWeakLambda(target, [target](FRotator value)
		{
			target->SetWorldRotation(value);
		}), endValue, duration)->SetEase(ease)->SetDelay(delay);
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
		return ULTweenManager::To(target, FLTweenRotatorGetterFunction::CreateUObject(target, &USceneComponent::GetComponentRotation),
		FLTweenRotatorSetterFunction::CreateWeakLambda(target, [target, &sweepHitResult, sweep, teleport](FRotator value)
		{
			target->SetWorldRotation(value, sweep, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport));
		}), endValue, duration)->SetEase(ease)->SetDelay(delay);
	}
}
#pragma endregion Rotation


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
	return ULTweenManager::To(WorldContextObject, FLTweenMaterialScalarGetterFunction::CreateWeakLambda(target, [target, parameterName](float& result)
	{
		return target->GetScalarParameterValue(parameterName, result);
	}), FLTweenMaterialScalarSetterFunction::CreateUObject(target, &UMaterialInstanceDynamic::SetScalarParameterByIndex), endValue, duration, parameterIndex)->SetEase(ease)->SetDelay(delay);
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
	return ULTweenManager::To(WorldContextObject, FLTweenMaterialVectorGetterFunction::CreateWeakLambda(target, [target, parameterName](FLinearColor& result)
	{
		return target->GetVectorParameterValue(parameterName, result);
	}), FLTweenMaterialVectorSetterFunction::CreateUObject(target, &UMaterialInstanceDynamic::SetVectorParameterByIndex), endValue, duration, parameterIndex)->SetEase(ease)->SetDelay(delay);
}

ULTweener* ULTweenBPLibrary::MeshMaterialScalarParameterTo(UPrimitiveComponent* target, int materialIndex, FName parameterName, float endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::MeshMaterialScalarParameterTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	float startValue = 0;
	int32 parameterIndex = 0;
	auto material = target->CreateAndSetMaterialInstanceDynamic(parameterIndex);
	if (material->GetScalarParameterValue(parameterName, startValue))
	{
		material->InitializeScalarParameterAndGetIndex(parameterName, startValue, parameterIndex);
	}
	else
	{
		UE_LOG(LTween, Warning, TEXT("[ULTweenBPLibrary::MaterialScalarParameterTo]GetScalarParameterValue:%s error!"), *(parameterName.ToString()));
		return nullptr;
	}
	return ULTweenManager::To(material, FLTweenMaterialScalarGetterFunction::CreateWeakLambda(material, [material, parameterName](float& result)
		{
			return material->GetScalarParameterValue(parameterName, result);
		}), FLTweenMaterialScalarSetterFunction::CreateUObject(material, &UMaterialInstanceDynamic::SetScalarParameterByIndex), endValue, duration, parameterIndex)->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULTweenBPLibrary::MeshMaterialVectorParameterTo(UPrimitiveComponent* target, int materialIndex, FName parameterName, FLinearColor endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::MeshMaterialVectorParameterTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	FLinearColor startValue = FLinearColor();
	int32 parameterIndex = 0;
	auto material = target->CreateAndSetMaterialInstanceDynamic(parameterIndex);
	if (material->GetVectorParameterValue(parameterName, startValue))
	{
		material->InitializeVectorParameterAndGetIndex(parameterName, startValue, parameterIndex);
	}
	else
	{
		UE_LOG(LTween, Warning, TEXT("[ULTweenBPLibrary::MaterialVectorParameterTo]GetVectorParameterValue:%s error!"), *(parameterName.ToString()));
		return nullptr;
	}
	return ULTweenManager::To(material, FLTweenMaterialVectorGetterFunction::CreateWeakLambda(material, [material, parameterName](FLinearColor& result)
		{
			return material->GetVectorParameterValue(parameterName, result);
		}), FLTweenMaterialVectorSetterFunction::CreateUObject(material, &UMaterialInstanceDynamic::SetVectorParameterByIndex), endValue, duration, parameterIndex)->SetEase(ease)->SetDelay(delay);
}
#pragma endregion

