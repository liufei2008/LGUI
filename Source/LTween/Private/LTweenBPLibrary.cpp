// Copyright 2019 LexLiu. All Rights Reserved.

#include "LTweenBPLibrary.h"


ULTweener* ULTweenBPLibrary::FloatTo(FLTweenFloatGetterDynamic getter, FLTweenFloatSetterDynamic setter, float endValue, float duration)
{
	return ALTweenActor::To(FLTweenFloatGetterFunction::CreateLambda([getter]
	{
		float value = 0.0f;
		if (getter.IsBound())
			getter.Execute(value);
		return value;
	}), FLTweenFloatSetterFunction::CreateLambda([setter](float value)
	{
		if (setter.IsBound())
			setter.Execute(value);
	}), endValue, duration);
}
ULTweener* ULTweenBPLibrary::IntTo(FLTweenIntGetterDynamic getter, FLTweenIntSetterDynamic setter, int endValue, float duration /* = 0.5f */)
{
	return ALTweenActor::To(FLTweenIntGetterFunction::CreateLambda([getter]
	{
		int value = 0;
		if (getter.IsBound())
			getter.Execute(value);
		return value;
	}), FLTweenIntSetterFunction::CreateLambda([setter](int value)
	{
		if (setter.IsBound())
			setter.Execute(value);
	}), endValue, duration);
}
ULTweener* ULTweenBPLibrary::Vector2To(FLTweenVector2GetterDynamic getter, FLTweenVector2SetterDynamic setter, const FVector2D& endValue, float duration /* = 0.5f */)
{
	return ALTweenActor::To(FLTweenVector2DGetterFunction::CreateLambda([getter]
	{
		FVector2D value = FVector2D::ZeroVector;
		if (getter.IsBound())
			getter.Execute(value);
		return value;
	}), FLTweenVector2DSetterFunction::CreateLambda([setter](const FVector2D& value)
	{
		if (setter.IsBound())
			setter.Execute(value);
	}), endValue, duration);
}
ULTweener* ULTweenBPLibrary::Vector3To(FLTweenVector3GetterDynamic getter, FLTweenVector3SetterDynamic setter, const FVector& endValue, float duration /* = 0.5f */)
{
	return ALTweenActor::To(FLTweenVectorGetterFunction::CreateLambda([getter]
	{
		FVector value = FVector::ZeroVector;
		if (getter.IsBound())
			getter.Execute(value);
		return value;
	}), FLTweenVectorSetterFunction::CreateLambda([setter](const FVector& value)
	{
		if (setter.IsBound())
			setter.Execute(value);
	}), endValue, duration);
}
ULTweener* ULTweenBPLibrary::Vector4To(FLTweenVector4GetterDynamic getter, FLTweenVector4SetterDynamic setter, const FVector4& endValue, float duration /* = 0.5f */)
{
	return ALTweenActor::To(FLTweenVector4GetterFunction::CreateLambda([getter]
	{
		FVector4 value = FVector4(EForceInit::ForceInitToZero);
		if (getter.IsBound())
			getter.Execute(value);
		return value;
	}), FLTweenVector4SetterFunction::CreateLambda([setter](const FVector4& value)
	{
		if (setter.IsBound())
			setter.Execute(value);
	}), endValue, duration);
}
ULTweener* ULTweenBPLibrary::ColorTo(FLTweenColorGetterDynamic getter, FLTweenColorSetterDynamic setter, const FColor& endValue, float duration /* = 0.5f */)
{
	return ALTweenActor::To(FLTweenColorGetterFunction::CreateLambda([getter]
	{
		FColor value = FColor::Black;
		if (getter.IsBound())
			getter.Execute(value);
		return value;
	}), FLTweenColorSetterFunction::CreateLambda([setter](const FColor& value)
	{
		if (setter.IsBound())
			setter.Execute(value);
	}), endValue, duration);
}
ULTweener* ULTweenBPLibrary::LinearColorTo(FLTweenLinearColorGetterDynamic getter, FLTweenLinearColorSetterDynamic setter, const FLinearColor& endValue, float duration /* = 0.5f */)
{
	return ALTweenActor::To(FLTweenLinearColorGetterFunction::CreateLambda([getter]
	{
		FLinearColor value = FLinearColor::Black;
		if (getter.IsBound())
			getter.Execute(value);
		return value;
	}), FLTweenLinearColorSetterFunction::CreateLambda([setter](const FLinearColor& value)
	{
		if (setter.IsBound())
			setter.Execute(value);
	}), endValue, duration);
}
ULTweener* ULTweenBPLibrary::QuaternionTo(FLTweenQuaternionGetterDynamic getter, FLTweenQuaternionSetterDynamic setter, const FQuat& endValue, float duration /* = 0.5f */)
{
	return ALTweenActor::To(FLTweenQuaternionGetterFunction::CreateLambda([getter]
	{
		FQuat value = FQuat::Identity;
		if (getter.IsBound())
			getter.Execute(value);
		return value;
	}), FLTweenQuaternionSetterFunction::CreateLambda([setter](const FQuat& value)
	{
		if (setter.IsBound())
			setter.Execute(value);
	}), endValue, duration);
}
ULTweener* ULTweenBPLibrary::RotatorTo(FLTweenRotatorGetterDynamic getter, FLTweenRotatorSetterDynamic setter, const FRotator& endValue, float duration /* = 0.5f */)
{
	return ALTweenActor::To(FLTweenRotatorGetterFunction::CreateLambda([getter]
	{
		FRotator value = FRotator::ZeroRotator;
		if (getter.IsBound())
			getter.Execute(value);
		return value;
	}), FLTweenRotatorSetterFunction::CreateLambda([setter](const FRotator& value)
	{
		if (setter.IsBound())
			setter.Execute(value);
	}), endValue, duration);
}

#pragma region PositionXYZ
ULTweener* ULTweenBPLibrary::LocalPositionXTo(USceneComponent* target, float endValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::LocalPositionXTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FLTweenFloatGetterFunction::CreateLambda([target] {
		if (IsValid(target))
		{
			return target->GetRelativeLocation().X;
		}
		return 0.0f;
	}), FLTweenFloatSetterFunction::CreateLambda([target](float value) {
		if (IsValid(target))
		{
			auto location = target->GetRelativeLocation();
			location.X = value;
			target->SetRelativeLocation(location);
		}
	}), endValue, duration)->SetEase(ease);
}
ULTweener* ULTweenBPLibrary::LocalPositionYTo(USceneComponent* target, float endValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::LocalPositionYTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FLTweenFloatGetterFunction::CreateLambda([target] {
		if (IsValid(target))
		{
			return target->GetRelativeLocation().Y;
		}
		return 0.0f;
	}), FLTweenFloatSetterFunction::CreateLambda([target](float value) {
		if (IsValid(target))
		{
			auto location = target->GetRelativeLocation();
			location.Y = value;
			target->SetRelativeLocation(location);
		}
	}), endValue, duration)->SetEase(ease);
}
ULTweener* ULTweenBPLibrary::LocalPositionZTo(USceneComponent* target, float endValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::LocalPositionZTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FLTweenFloatGetterFunction::CreateLambda([target] {
		if (IsValid(target))
		{
			return target->GetRelativeLocation().Z;
		}
		return 0.0f;
	}), FLTweenFloatSetterFunction::CreateLambda([target](float value) {
		if (IsValid(target))
		{
			auto location = target->GetRelativeLocation();
			location.Z = value;
			target->SetRelativeLocation(location);
		}
	}), endValue, duration)->SetEase(ease);
}
ULTweener* ULTweenBPLibrary::LocalPositionXTo_Sweep(USceneComponent* target, float endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::LocalPositionXTo_Sweep] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FLTweenFloatGetterFunction::CreateLambda([target] {
		if (IsValid(target))
		{
			return target->GetRelativeLocation().X;
		}
		return 0.0f;
	}), FLTweenFloatSetterFunction::CreateLambda([target, &sweepHitResult, sweep, teleport](float value) {
		if (IsValid(target))
		{
			auto location = target->GetRelativeLocation();
			location.X = value;
			target->SetRelativeLocation(location, teleport, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport));
		}
	}), endValue, duration)->SetEase(ease);
}
ULTweener* ULTweenBPLibrary::LocalPositionYTo_Sweep(USceneComponent* target, float endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::LocalPositionYTo_Sweep] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FLTweenFloatGetterFunction::CreateLambda([target] {
		if (IsValid(target))
		{
			return target->GetRelativeLocation().Y;
		}
		return 0.0f;
	}), FLTweenFloatSetterFunction::CreateLambda([target, &sweepHitResult, sweep, teleport](float value) {
		if (IsValid(target))
		{
			auto location = target->GetRelativeLocation();
			location.Y = value;
			target->SetRelativeLocation(location, teleport, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport));
		}
	}), endValue, duration)->SetEase(ease);
}
ULTweener* ULTweenBPLibrary::LocalPositionZTo_Sweep(USceneComponent* target, float endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::LocalPositionZTo_Sweep] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FLTweenFloatGetterFunction::CreateLambda([target] {
		if (IsValid(target))
		{
			return target->GetRelativeLocation().Z;
		}
		return 0.0f;
	}), FLTweenFloatSetterFunction::CreateLambda([target, &sweepHitResult, sweep, teleport](float value) {
		if (IsValid(target))
		{
			auto location = target->GetRelativeLocation();
			location.Z = value;
			target->SetRelativeLocation(location, teleport, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport));
		}
	}), endValue, duration)->SetEase(ease);
}



ULTweener* ULTweenBPLibrary::WorldPositionXTo(USceneComponent* target, float endValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::WorldPositionXTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FLTweenFloatGetterFunction::CreateLambda([target] {
		if (IsValid(target))
		{
			return target->GetComponentLocation().X;
		}
		return 0.0f;
	}), FLTweenFloatSetterFunction::CreateLambda([target](float value) {
		if (IsValid(target))
		{
			auto location = target->GetComponentLocation();
			location.X = value;
			target->SetWorldLocation(location);
		}
	}), endValue, duration)->SetEase(ease);
}
ULTweener* ULTweenBPLibrary::WorldPositionYTo(USceneComponent* target, float endValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::WorldPositionYTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FLTweenFloatGetterFunction::CreateLambda([target] {
		if (IsValid(target))
		{
			return target->GetComponentLocation().Y;
		}
		return 0.0f;
	}), FLTweenFloatSetterFunction::CreateLambda([target](float value) {
		if (IsValid(target))
		{
			auto location = target->GetComponentLocation();
			location.Y = value;
			target->SetWorldLocation(location);
		}
	}), endValue, duration)->SetEase(ease);
}
ULTweener* ULTweenBPLibrary::WorldPositionZTo(USceneComponent* target, float endValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::WorldPositionZTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FLTweenFloatGetterFunction::CreateLambda([target] {
		if (IsValid(target))
		{
			return target->GetComponentLocation().Z;
		}
		return 0.0f;
	}), FLTweenFloatSetterFunction::CreateLambda([target](float value) {
		if (IsValid(target))
		{
			auto location = target->GetComponentLocation();
			location.Z = value;
			target->SetWorldLocation(location);
		}
	}), endValue, duration)->SetEase(ease);
}
ULTweener* ULTweenBPLibrary::WorldPositionXTo_Sweep(USceneComponent* target, float endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::WorldPositionXTo_Sweep] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FLTweenFloatGetterFunction::CreateLambda([target] {
		if (IsValid(target))
		{
			return target->GetComponentLocation().X;
		}
		return 0.0f;
	}), FLTweenFloatSetterFunction::CreateLambda([target, &sweepHitResult, sweep, teleport](float value) {
		if (IsValid(target))
		{
			auto location = target->GetComponentLocation();
			location.X = value;
			target->SetWorldLocation(location, teleport, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport));
		}
	}), endValue, duration)->SetEase(ease);
}
ULTweener* ULTweenBPLibrary::WorldPositionYTo_Sweep(USceneComponent* target, float endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::WorldPositionYTo_Sweep] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FLTweenFloatGetterFunction::CreateLambda([target] {
		if (IsValid(target))
		{
			return target->GetComponentLocation().Y;
		}
		return 0.0f;
	}), FLTweenFloatSetterFunction::CreateLambda([target, &sweepHitResult, sweep, teleport](float value) {
		if (IsValid(target))
		{
			auto location = target->GetComponentLocation();
			location.Y = value;
			target->SetWorldLocation(location, teleport, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport));
		}
	}), endValue, duration)->SetEase(ease);
}
ULTweener* ULTweenBPLibrary::WorldPositionZTo_Sweep(USceneComponent* target, float endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::WorldPositionZTo_Sweep] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FLTweenFloatGetterFunction::CreateLambda([target] {
		if (IsValid(target))
		{
			return target->GetComponentLocation().Z;
		}
		return 0.0f;
	}), FLTweenFloatSetterFunction::CreateLambda([target, &sweepHitResult, sweep, teleport](float value) {
		if (IsValid(target))
		{
			auto location = target->GetComponentLocation();
			location.Z = value;
			target->SetWorldLocation(location, teleport, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport));
		}
	}), endValue, duration)->SetEase(ease);
}
#pragma endregion PositionXYZ




#pragma region Position
ULTweener* ULTweenBPLibrary::LocalPositionTo(USceneComponent* target, FVector endValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::LocalPositionTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FLTweenPositionGetterFunction::CreateLambda([target] 
	{
		if (!IsValid(target))
		{
			UE_LOG(LTween, Warning, TEXT("[ULTweenBPLibrary::LocalPositionZTo_Sweep]PositionGetterFunction target is not valid:%s"), *(target->GetPathName()));
			return FVector();
		}
		return target->GetRelativeLocation();
	}), FLTweenPositionSetterFunction::CreateUObject(target, &USceneComponent::SetRelativeLocation), endValue, duration)
		->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULTweenBPLibrary::WorldPositionTo(USceneComponent* target, FVector endValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::WorldPositionTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FLTweenPositionGetterFunction::CreateUObject(target, &USceneComponent::GetComponentLocation), 
		FLTweenPositionSetterFunction::CreateUObject(target, &USceneComponent::SetWorldLocation), endValue, duration)
		->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULTweenBPLibrary::LocalPositionTo_Sweep(USceneComponent* target, FVector endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::LocalPositionTo_Sweep] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FLTweenVectorGetterFunction::CreateLambda([target]
	{
		if (!IsValid(target))
		{
			UE_LOG(LTween, Warning, TEXT("[ULTweenBPLibrary::LocalPositionTo_Sweep]PositionGetterFunction target is not valid:%s"), *(target->GetPathName()));
			return FVector();
		}
		return target->GetRelativeLocation();
	}),
		FLTweenPositionSetterFunction::CreateUObject(target, &USceneComponent::SetRelativeLocation), endValue, duration, sweep, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport))
		->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULTweenBPLibrary::WorldPositionTo_Sweep(USceneComponent* target, FVector endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::WorldPositionTo_Sweep] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FLTweenPositionGetterFunction::CreateUObject(target, &USceneComponent::GetComponentLocation),
		FLTweenPositionSetterFunction::CreateUObject(target, &USceneComponent::SetRelativeLocation), endValue, duration, sweep, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport))
		->SetEase(ease)->SetDelay(delay);
}
#pragma endregion Position



ULTweener* ULTweenBPLibrary::LocalScaleTo(USceneComponent* target, FVector endValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::LocalScaleTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FLTweenVectorGetterFunction::CreateLambda([target] 
	{
		if (!IsValid(target))
		{
			UE_LOG(LTween, Warning, TEXT("[ULTweenBPLibrary::LocalScaleTo]VectorGetterFunction target is not valid:%s"), *(target->GetPathName()));
			return FVector();
		}
		return target->GetRelativeScale3D(); 
	}), FLTweenVectorSetterFunction::CreateUObject(target, &USceneComponent::SetRelativeScale3D), endValue, duration)
		->SetEase(ease)->SetDelay(delay);
}


#pragma region Rotation
ULTweener* ULTweenBPLibrary::LocalRotateEulerAngleTo(USceneComponent* target, FVector eulerAngle, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::LocalRotateEulerAngleTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FLTweenRotationQuatGetterFunction::CreateLambda([target]
	{ 
		if (IsValid(target))
			return target->GetRelativeRotationCache().GetCachedQuat();
		else
			return FQuat();
	}), FLTweenRotationQuatSetterFunction::CreateUObject(target, &USceneComponent::SetRelativeRotation), eulerAngle, duration)
		->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULTweenBPLibrary::LocalRotationQuaternionTo(USceneComponent* target, const FQuat& endValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::LocalRotationTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FLTweenRotationQuatGetterFunction::CreateLambda([target]
	{
		if (IsValid(target))
			return target->GetRelativeRotationCache().GetCachedQuat();
		else
			return FQuat();
	}), FLTweenRotationQuatSetterFunction::CreateUObject(target, &USceneComponent::SetRelativeRotation), endValue, duration)
		->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULTweenBPLibrary::LocalRotateEulerAngleTo_Sweep(USceneComponent* target, FVector eulerAngle, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::LocalRotateEulerAngleTo_Sweep] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FLTweenRotationQuatGetterFunction::CreateLambda([target]
	{ 
		if (IsValid(target))
			return target->GetRelativeRotationCache().GetCachedQuat();
		else
			return FQuat();
	}), FLTweenRotationQuatSetterFunction::CreateUObject(target, &USceneComponent::SetRelativeRotation), eulerAngle, duration, sweep, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport))
		->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULTweenBPLibrary::LocalRotationQuaternionTo_Sweep(USceneComponent* target, const FQuat& endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::LocalRotationTo_Sweep] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FLTweenRotationQuatGetterFunction::CreateLambda([target]
	{
		if (IsValid(target))
			return target->GetRelativeRotationCache().GetCachedQuat();
		else
			return FQuat();
	}), FLTweenRotationQuatSetterFunction::CreateUObject(target, &USceneComponent::SetRelativeRotation), endValue, duration, sweep, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport))
		->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULTweenBPLibrary::LocalRotatorTo(USceneComponent* target, FRotator endValue, bool shortestPath, float duration, float delay, LTweenEase ease)
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
		return ALTweenActor::To(FLTweenRotatorGetterFunction::CreateLambda([target]
		{
			if (IsValid(target))
				return target->GetRelativeRotation();
			else
				return FRotator();
		}), FLTweenRotatorSetterFunction::CreateLambda([target] (FRotator value)
		{
			if (IsValid(target))
				target->SetRelativeRotation(value);
		}), endValue, duration)->SetEase(ease)->SetDelay(delay);
	}
}
ULTweener* ULTweenBPLibrary::LocalRotatorTo_Sweep(USceneComponent* target, FRotator endValue, bool shortestPath, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, LTweenEase ease)
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
		return ALTweenActor::To(FLTweenRotatorGetterFunction::CreateLambda([target]
		{
			if (IsValid(target))
				return target->GetRelativeRotation();
			else
				return FRotator();
		}), FLTweenRotatorSetterFunction::CreateLambda([target, &sweepHitResult, sweep, teleport](FRotator value)
		{
			if(IsValid(target))
				target->SetRelativeRotation(value, sweep, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport));
		}), endValue, duration)->SetEase(ease)->SetDelay(delay);
	}
}



ULTweener* ULTweenBPLibrary::WorldRotateEulerAngleTo(USceneComponent* target, FVector eulerAngle, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::WorldRotateEulerAngleTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FLTweenRotationQuatGetterFunction::CreateLambda([target]
	{
		if (IsValid(target))
			return target->GetComponentRotation().Quaternion();
		else
			return FQuat();
	}), FLTweenRotationQuatSetterFunction::CreateUObject(target, &USceneComponent::SetWorldRotation), eulerAngle, duration)
		->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULTweenBPLibrary::WorldRotationQuaternionTo(USceneComponent* target, const FQuat& endValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::WorldRotationTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FLTweenRotationQuatGetterFunction::CreateLambda([target]
	{
		if (IsValid(target))
			return target->GetComponentRotation().Quaternion();
		else
			return FQuat();
	}), FLTweenRotationQuatSetterFunction::CreateUObject(target, &USceneComponent::SetWorldRotation), endValue, duration)
		->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULTweenBPLibrary::WorldRotateEulerAngleTo_Sweep(USceneComponent* target, FVector eulerAngle, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::WorldRotateEulerAngleTo_Sweep] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FLTweenRotationQuatGetterFunction::CreateLambda([target]
	{
		if (IsValid(target))
			return target->GetComponentRotation().Quaternion();
		else
			return FQuat();
	}), FLTweenRotationQuatSetterFunction::CreateUObject(target, &USceneComponent::SetWorldRotation), eulerAngle, duration, sweep, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport))
		->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULTweenBPLibrary::WorldRotationQuaternionTo_Sweep(USceneComponent* target, const FQuat& endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::WorldRotationTo_Sweep] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FLTweenRotationQuatGetterFunction::CreateLambda([target]
	{
		if (IsValid(target))
			return target->GetComponentRotation().Quaternion();
		else
			return FQuat();
	}), FLTweenRotationQuatSetterFunction::CreateUObject(target, &USceneComponent::SetWorldRotation), endValue, duration, sweep, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport))
		->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULTweenBPLibrary::WorldRotatorTo(USceneComponent* target, FRotator endValue, bool shortestPath, float duration, float delay, LTweenEase ease)
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
		return ALTweenActor::To(FLTweenRotatorGetterFunction::CreateLambda([target]
		{
			if (IsValid(target))
				return target->GetComponentRotation();
			else
				return FRotator();
		}), FLTweenRotatorSetterFunction::CreateLambda([target](FRotator value)
		{
			if (IsValid(target))
				target->SetWorldRotation(value);
		}), endValue, duration)->SetEase(ease)->SetDelay(delay);
	}
}
ULTweener* ULTweenBPLibrary::WorldRotatorTo_Sweep(USceneComponent* target, FRotator endValue, bool shortestPath, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, LTweenEase ease)
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
		return ALTweenActor::To(FLTweenRotatorGetterFunction::CreateLambda([target]
		{
			if (IsValid(target))
				return target->GetComponentRotation();
			else
				return FRotator();
		}), FLTweenRotatorSetterFunction::CreateLambda([target, &sweepHitResult, sweep, teleport](FRotator value)
		{
			if (IsValid(target))
				target->SetWorldRotation(value, sweep, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport));
		}), endValue, duration)->SetEase(ease)->SetDelay(delay);
	}
}
#pragma endregion Rotation



ULTweener* ULTweenBPLibrary::MaterialScalarParameterTo(UMaterialInstanceDynamic* target, FName parameterName, float endValue, float duration, float delay, LTweenEase ease)
{
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
	return ALTweenActor::To(FLTweenMaterialScalarGetterFunction::CreateLambda([target, parameterName](float& result)
	{
		if (IsValid(target))
			return target->GetScalarParameterValue(parameterName, result);
		else
			return false;
	}), FLTweenMaterialScalarSetterFunction::CreateUObject(target, &UMaterialInstanceDynamic::SetScalarParameterByIndex), endValue, duration, parameterIndex)->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULTweenBPLibrary::MaterialVectorParameterTo(UMaterialInstanceDynamic* target, FName parameterName, FLinearColor endValue, float duration, float delay, LTweenEase ease)
{
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
	return ALTweenActor::To(FLTweenMaterialVectorGetterFunction::CreateLambda([target, parameterName](FLinearColor& result)
	{
		if (IsValid(target))
			return target->GetVectorParameterValue(parameterName, result);
		else
			return false;
	}), FLTweenMaterialVectorSetterFunction::CreateUObject(target, &UMaterialInstanceDynamic::SetVectorParameterByIndex), endValue, duration, parameterIndex)->SetEase(ease)->SetDelay(delay);
}