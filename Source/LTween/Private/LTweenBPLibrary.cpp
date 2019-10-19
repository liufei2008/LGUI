// Copyright 2019 LexLiu. All Rights Reserved.

#include "LTweenBPLibrary.h"


#pragma region PositionXYZ
ULTweener* ULTweenBPLibrary::LocalPositionXTo(USceneComponent* target, float endValue, float duration, float delay, LTweenEase ease)
{
	if (!target->IsValidLowLevel() || target->IsPendingKill())
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::LocalPositionXTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto targetWeakPtr = TWeakObjectPtr<USceneComponent>(target);
	return ALTweenActor::To(FLTweenFloatGetterFunction::CreateLambda([targetWeakPtr] {
		if (targetWeakPtr.IsValid())
		{
			return targetWeakPtr->RelativeLocation.X;
		}
		return 0.0f;
	}), FLTweenFloatSetterFunction::CreateLambda([targetWeakPtr](float value) {
		if (targetWeakPtr.IsValid())
		{
			auto location = targetWeakPtr->RelativeLocation;
			location.X = value;
			targetWeakPtr->SetRelativeLocation(location);
		}
	}), endValue, duration)->SetEase(ease);
}
ULTweener* ULTweenBPLibrary::LocalPositionYTo(USceneComponent* target, float endValue, float duration, float delay, LTweenEase ease)
{
	if (!target->IsValidLowLevel() || target->IsPendingKill())
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::LocalPositionYTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto targetWeakPtr = TWeakObjectPtr<USceneComponent>(target);
	return ALTweenActor::To(FLTweenFloatGetterFunction::CreateLambda([targetWeakPtr] {
		if (targetWeakPtr.IsValid())
		{
			return targetWeakPtr->RelativeLocation.Y;
		}
		return 0.0f;
	}), FLTweenFloatSetterFunction::CreateLambda([targetWeakPtr](float value) {
		if (targetWeakPtr.IsValid())
		{
			auto location = targetWeakPtr->RelativeLocation;
			location.Y = value;
			targetWeakPtr->SetRelativeLocation(location);
		}
	}), endValue, duration)->SetEase(ease);
}
ULTweener* ULTweenBPLibrary::LocalPositionZTo(USceneComponent* target, float endValue, float duration, float delay, LTweenEase ease)
{
	if (!target->IsValidLowLevel() || target->IsPendingKill())
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::LocalPositionZTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto targetWeakPtr = TWeakObjectPtr<USceneComponent>(target);
	return ALTweenActor::To(FLTweenFloatGetterFunction::CreateLambda([targetWeakPtr] {
		if (targetWeakPtr.IsValid())
		{
			return targetWeakPtr->RelativeLocation.Z;
		}
		return 0.0f;
	}), FLTweenFloatSetterFunction::CreateLambda([targetWeakPtr](float value) {
		if (targetWeakPtr.IsValid())
		{
			auto location = targetWeakPtr->RelativeLocation;
			location.Z = value;
			targetWeakPtr->SetRelativeLocation(location);
		}
	}), endValue, duration)->SetEase(ease);
}
ULTweener* ULTweenBPLibrary::LocalPositionXTo_Sweep(USceneComponent* target, float endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, LTweenEase ease)
{
	if (!target->IsValidLowLevel() || target->IsPendingKill())
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::LocalPositionXTo_Sweep] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto targetWeakPtr = TWeakObjectPtr<USceneComponent>(target);
	return ALTweenActor::To(FLTweenFloatGetterFunction::CreateLambda([targetWeakPtr] {
		if (targetWeakPtr.IsValid())
		{
			return targetWeakPtr->RelativeLocation.X;
		}
		return 0.0f;
	}), FLTweenFloatSetterFunction::CreateLambda([targetWeakPtr, &sweepHitResult, sweep, teleport](float value) {
		if (targetWeakPtr.IsValid())
		{
			auto location = targetWeakPtr->RelativeLocation;
			location.X = value;
			targetWeakPtr->SetRelativeLocation(location, teleport, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport));
		}
	}), endValue, duration)->SetEase(ease);
}
ULTweener* ULTweenBPLibrary::LocalPositionYTo_Sweep(USceneComponent* target, float endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, LTweenEase ease)
{
	if (!target->IsValidLowLevel() || target->IsPendingKill())
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::LocalPositionYTo_Sweep] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto targetWeakPtr = TWeakObjectPtr<USceneComponent>(target);
	return ALTweenActor::To(FLTweenFloatGetterFunction::CreateLambda([targetWeakPtr] {
		if (targetWeakPtr.IsValid())
		{
			return targetWeakPtr->RelativeLocation.Y;
		}
		return 0.0f;
	}), FLTweenFloatSetterFunction::CreateLambda([targetWeakPtr, &sweepHitResult, sweep, teleport](float value) {
		if (targetWeakPtr.IsValid())
		{
			auto location = targetWeakPtr->RelativeLocation;
			location.Y = value;
			targetWeakPtr->SetRelativeLocation(location, teleport, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport));
		}
	}), endValue, duration)->SetEase(ease);
}
ULTweener* ULTweenBPLibrary::LocalPositionZTo_Sweep(USceneComponent* target, float endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, LTweenEase ease)
{
	if (!target->IsValidLowLevel() || target->IsPendingKill())
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::LocalPositionZTo_Sweep] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto targetWeakPtr = TWeakObjectPtr<USceneComponent>(target);
	return ALTweenActor::To(FLTweenFloatGetterFunction::CreateLambda([targetWeakPtr] {
		if (targetWeakPtr.IsValid())
		{
			return targetWeakPtr->RelativeLocation.Z;
		}
		return 0.0f;
	}), FLTweenFloatSetterFunction::CreateLambda([targetWeakPtr, &sweepHitResult, sweep, teleport](float value) {
		if (targetWeakPtr.IsValid())
		{
			auto location = targetWeakPtr->RelativeLocation;
			location.Z = value;
			targetWeakPtr->SetRelativeLocation(location, teleport, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport));
		}
	}), endValue, duration)->SetEase(ease);
}



ULTweener* ULTweenBPLibrary::WorldPositionXTo(USceneComponent* target, float endValue, float duration, float delay, LTweenEase ease)
{
	if (!target->IsValidLowLevel() || target->IsPendingKill())
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::WorldPositionXTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto targetWeakPtr = TWeakObjectPtr<USceneComponent>(target);
	return ALTweenActor::To(FLTweenFloatGetterFunction::CreateLambda([targetWeakPtr] {
		if (targetWeakPtr.IsValid())
		{
			return targetWeakPtr->GetComponentLocation().X;
		}
		return 0.0f;
	}), FLTweenFloatSetterFunction::CreateLambda([targetWeakPtr](float value) {
		if (targetWeakPtr.IsValid())
		{
			auto location = targetWeakPtr->GetComponentLocation();
			location.X = value;
			targetWeakPtr->SetWorldLocation(location);
		}
	}), endValue, duration)->SetEase(ease);
}
ULTweener* ULTweenBPLibrary::WorldPositionYTo(USceneComponent* target, float endValue, float duration, float delay, LTweenEase ease)
{
	if (!target->IsValidLowLevel() || target->IsPendingKill())
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::WorldPositionYTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto targetWeakPtr = TWeakObjectPtr<USceneComponent>(target);
	return ALTweenActor::To(FLTweenFloatGetterFunction::CreateLambda([targetWeakPtr] {
		if (targetWeakPtr.IsValid())
		{
			return targetWeakPtr->GetComponentLocation().Y;
		}
		return 0.0f;
	}), FLTweenFloatSetterFunction::CreateLambda([targetWeakPtr](float value) {
		if (targetWeakPtr.IsValid())
		{
			auto location = targetWeakPtr->GetComponentLocation();
			location.Y = value;
			targetWeakPtr->SetWorldLocation(location);
		}
	}), endValue, duration)->SetEase(ease);
}
ULTweener* ULTweenBPLibrary::WorldPositionZTo(USceneComponent* target, float endValue, float duration, float delay, LTweenEase ease)
{
	if (!target->IsValidLowLevel() || target->IsPendingKill())
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::WorldPositionZTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto targetWeakPtr = TWeakObjectPtr<USceneComponent>(target);
	return ALTweenActor::To(FLTweenFloatGetterFunction::CreateLambda([targetWeakPtr] {
		if (targetWeakPtr.IsValid())
		{
			return targetWeakPtr->GetComponentLocation().Z;
		}
		return 0.0f;
	}), FLTweenFloatSetterFunction::CreateLambda([targetWeakPtr](float value) {
		if (targetWeakPtr.IsValid())
		{
			auto location = targetWeakPtr->GetComponentLocation();
			location.Z = value;
			targetWeakPtr->SetWorldLocation(location);
		}
	}), endValue, duration)->SetEase(ease);
}
ULTweener* ULTweenBPLibrary::WorldPositionXTo_Sweep(USceneComponent* target, float endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, LTweenEase ease)
{
	if (!target->IsValidLowLevel() || target->IsPendingKill())
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::WorldPositionXTo_Sweep] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto targetWeakPtr = TWeakObjectPtr<USceneComponent>(target);
	return ALTweenActor::To(FLTweenFloatGetterFunction::CreateLambda([targetWeakPtr] {
		if (targetWeakPtr.IsValid())
		{
			return targetWeakPtr->GetComponentLocation().X;
		}
		return 0.0f;
	}), FLTweenFloatSetterFunction::CreateLambda([targetWeakPtr, &sweepHitResult, sweep, teleport](float value) {
		if (targetWeakPtr.IsValid())
		{
			auto location = targetWeakPtr->GetComponentLocation();
			location.X = value;
			targetWeakPtr->SetWorldLocation(location, teleport, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport));
		}
	}), endValue, duration)->SetEase(ease);
}
ULTweener* ULTweenBPLibrary::WorldPositionYTo_Sweep(USceneComponent* target, float endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, LTweenEase ease)
{
	if (!target->IsValidLowLevel() || target->IsPendingKill())
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::WorldPositionYTo_Sweep] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto targetWeakPtr = TWeakObjectPtr<USceneComponent>(target);
	return ALTweenActor::To(FLTweenFloatGetterFunction::CreateLambda([targetWeakPtr] {
		if (targetWeakPtr.IsValid())
		{
			return targetWeakPtr->GetComponentLocation().Y;
		}
		return 0.0f;
	}), FLTweenFloatSetterFunction::CreateLambda([targetWeakPtr, &sweepHitResult, sweep, teleport](float value) {
		if (targetWeakPtr.IsValid())
		{
			auto location = targetWeakPtr->GetComponentLocation();
			location.Y = value;
			targetWeakPtr->SetWorldLocation(location, teleport, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport));
		}
	}), endValue, duration)->SetEase(ease);
}
ULTweener* ULTweenBPLibrary::WorldPositionZTo_Sweep(USceneComponent* target, float endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, LTweenEase ease)
{
	if (!target->IsValidLowLevel() || target->IsPendingKill())
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::WorldPositionZTo_Sweep] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto targetWeakPtr = TWeakObjectPtr<USceneComponent>(target);
	return ALTweenActor::To(FLTweenFloatGetterFunction::CreateLambda([targetWeakPtr] {
		if (targetWeakPtr.IsValid())
		{
			return targetWeakPtr->GetComponentLocation().Z;
		}
		return 0.0f;
	}), FLTweenFloatSetterFunction::CreateLambda([targetWeakPtr, &sweepHitResult, sweep, teleport](float value) {
		if (targetWeakPtr.IsValid())
		{
			auto location = targetWeakPtr->GetComponentLocation();
			location.Z = value;
			targetWeakPtr->SetWorldLocation(location, teleport, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport));
		}
	}), endValue, duration)->SetEase(ease);
}
#pragma endregion PositionXYZ




#pragma region Position
ULTweener* ULTweenBPLibrary::LocalPositionTo(USceneComponent* target, FVector endValue, float duration, float delay, LTweenEase ease)
{
	if (!target->IsValidLowLevel() || target->IsPendingKill())
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::LocalPositionTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FLTweenPositionGetterFunction::CreateLambda([target] 
	{
		if (!target->IsValidLowLevel() || target->IsPendingKill())
		{
			UE_LOG(LTween, Warning, TEXT("[ULTweenBPLibrary::LocalPositionZTo_Sweep]PositionGetterFunction target is not valid:%s"), *(target->GetPathName()));
			return FVector();
		}
		return target->RelativeLocation; 
	}), FLTweenPositionSetterFunction::CreateUObject(target, &USceneComponent::SetRelativeLocation), endValue, duration)
		->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULTweenBPLibrary::WorldPositionTo(USceneComponent* target, FVector endValue, float duration, float delay, LTweenEase ease)
{
	if (!target->IsValidLowLevel() || target->IsPendingKill())
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
	if (!target->IsValidLowLevel() || target->IsPendingKill())
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::LocalPositionTo_Sweep] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FLTweenPositionGetterFunction::CreateLambda([target]
	{
		if (!target->IsValidLowLevel() || target->IsPendingKill())
		{
			UE_LOG(LTween, Warning, TEXT("[ULTweenBPLibrary::LocalPositionTo_Sweep]PositionGetterFunction target is not valid:%s"), *(target->GetPathName()));
			return FVector();
		}
		return target->RelativeLocation;
	}),
		FLTweenPositionSetterFunction::CreateUObject(target, &USceneComponent::SetRelativeLocation), endValue, duration, sweep, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport))
		->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULTweenBPLibrary::WorldPositionTo_Sweep(USceneComponent* target, FVector endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, LTweenEase ease)
{
	if (!target->IsValidLowLevel() || target->IsPendingKill())
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
	if (!target->IsValidLowLevel() || target->IsPendingKill())
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::LocalScaleTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FLTweenVectorGetterFunction::CreateLambda([target] 
	{
		if (!target->IsValidLowLevel() || target->IsPendingKill())
		{
			UE_LOG(LTween, Warning, TEXT("[ULTweenBPLibrary::LocalScaleTo]VectorGetterFunction target is not valid:%s"), *(target->GetPathName()));
			return FVector();
		}
		return target->RelativeScale3D; 
	}), FLTweenVectorSetterFunction::CreateUObject(target, &USceneComponent::SetRelativeScale3D), endValue, duration)
		->SetEase(ease)->SetDelay(delay);
}


#pragma region Rotation
ULTweener* ULTweenBPLibrary::LocalRotateEulerAngleTo(USceneComponent* target, FVector eulerAngle, float duration, float delay, LTweenEase ease)
{
	if (!target->IsValidLowLevel() || target->IsPendingKill())
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::LocalRotateEulerAngleTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto targetWeakPtr = TWeakObjectPtr<USceneComponent>(target);
	return ALTweenActor::To(FLTweenRotationQuatGetterFunction::CreateLambda([targetWeakPtr]
	{ 
		if (targetWeakPtr.IsValid())
			return targetWeakPtr->GetRelativeRotationCache().GetCachedQuat();
		else
			return FQuat();
	}), FLTweenRotationQuatSetterFunction::CreateUObject(target, &USceneComponent::SetRelativeRotation), eulerAngle, duration)
		->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULTweenBPLibrary::LocalRotationQuaternionTo(USceneComponent* target, FQuat endValue, float duration, float delay, LTweenEase ease)
{
	if (!target->IsValidLowLevel() || target->IsPendingKill())
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::LocalRotationTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto targetWeakPtr = TWeakObjectPtr<USceneComponent>(target);
	return ALTweenActor::To(FLTweenRotationQuatGetterFunction::CreateLambda([targetWeakPtr]
	{
		if (targetWeakPtr.IsValid())
			return targetWeakPtr->GetRelativeRotationCache().GetCachedQuat();
		else
			return FQuat();
	}), FLTweenRotationQuatSetterFunction::CreateUObject(target, &USceneComponent::SetRelativeRotation), endValue, duration)
		->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULTweenBPLibrary::LocalRotateEulerAngleTo_Sweep(USceneComponent* target, FVector eulerAngle, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, LTweenEase ease)
{
	if (!target->IsValidLowLevel() || target->IsPendingKill())
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::LocalRotateEulerAngleTo_Sweep] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto targetWeakPtr = TWeakObjectPtr<USceneComponent>(target);
	return ALTweenActor::To(FLTweenRotationQuatGetterFunction::CreateLambda([targetWeakPtr]
	{ 
		if (targetWeakPtr.IsValid())
			return targetWeakPtr->GetRelativeRotationCache().GetCachedQuat();
		else
			return FQuat();
	}), FLTweenRotationQuatSetterFunction::CreateUObject(target, &USceneComponent::SetRelativeRotation), eulerAngle, duration, sweep, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport))
		->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULTweenBPLibrary::LocalRotationQuaternionTo_Sweep(USceneComponent* target, FQuat endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, LTweenEase ease)
{
	if (!target->IsValidLowLevel() || target->IsPendingKill())
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::LocalRotationTo_Sweep] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto targetWeakPtr = TWeakObjectPtr<USceneComponent>(target);
	return ALTweenActor::To(FLTweenRotationQuatGetterFunction::CreateLambda([targetWeakPtr]
	{
		if (targetWeakPtr.IsValid())
			return targetWeakPtr->GetRelativeRotationCache().GetCachedQuat();
		else
			return FQuat();
	}), FLTweenRotationQuatSetterFunction::CreateUObject(target, &USceneComponent::SetRelativeRotation), endValue, duration, sweep, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport))
		->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULTweenBPLibrary::LocalRotatorTo(USceneComponent* target, FRotator endValue, bool shortestPath, float duration, float delay, LTweenEase ease)
{
	if (!target->IsValidLowLevel() || target->IsPendingKill())
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
		auto targetWeakPtr = TWeakObjectPtr<USceneComponent>(target);
		return ALTweenActor::To(FLTweenRotatorGetterFunction::CreateLambda([targetWeakPtr]
		{
			if (targetWeakPtr.IsValid())
				return targetWeakPtr->RelativeRotation;
			else
				return FRotator();
		}), FLTweenRotatorSetterFunction::CreateLambda([targetWeakPtr] (FRotator value)
		{
			if (targetWeakPtr.IsValid())
				targetWeakPtr->SetRelativeRotation(value);
		}), endValue, duration)->SetEase(ease)->SetDelay(delay);
	}
}
ULTweener* ULTweenBPLibrary::LocalRotatorTo_Sweep(USceneComponent* target, FRotator endValue, bool shortestPath, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, LTweenEase ease)
{
	if (!target->IsValidLowLevel() || target->IsPendingKill())
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
		auto targetWeakPtr = TWeakObjectPtr<USceneComponent>(target);
		return ALTweenActor::To(FLTweenRotatorGetterFunction::CreateLambda([targetWeakPtr]
		{
			if (targetWeakPtr.IsValid())
				return targetWeakPtr->RelativeRotation;
			else
				return FRotator();
		}), FLTweenRotatorSetterFunction::CreateLambda([targetWeakPtr, &sweepHitResult, sweep, teleport](FRotator value)
		{
			if(targetWeakPtr.IsValid())
				targetWeakPtr->SetRelativeRotation(value, sweep, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport));
		}), endValue, duration)->SetEase(ease)->SetDelay(delay);
	}
}



ULTweener* ULTweenBPLibrary::WorldRotateEulerAngleTo(USceneComponent* target, FVector eulerAngle, float duration, float delay, LTweenEase ease)
{
	if (!target->IsValidLowLevel() || target->IsPendingKill())
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::WorldRotateEulerAngleTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto targetWeakPtr = TWeakObjectPtr<USceneComponent>(target);
	return ALTweenActor::To(FLTweenRotationQuatGetterFunction::CreateLambda([targetWeakPtr]
	{
		if (targetWeakPtr.IsValid())
			return targetWeakPtr->GetComponentRotation().Quaternion();
		else
			return FQuat();
	}), FLTweenRotationQuatSetterFunction::CreateUObject(target, &USceneComponent::SetWorldRotation), eulerAngle, duration)
		->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULTweenBPLibrary::WorldRotationQuaternionTo(USceneComponent* target, FQuat endValue, float duration, float delay, LTweenEase ease)
{
	if (!target->IsValidLowLevel() || target->IsPendingKill())
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::WorldRotationTo] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto targetWeakPtr = TWeakObjectPtr<USceneComponent>(target);
	return ALTweenActor::To(FLTweenRotationQuatGetterFunction::CreateLambda([targetWeakPtr]
	{
		if (targetWeakPtr.IsValid())
			return targetWeakPtr->GetComponentRotation().Quaternion();
		else
			return FQuat();
	}), FLTweenRotationQuatSetterFunction::CreateUObject(target, &USceneComponent::SetWorldRotation), endValue, duration)
		->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULTweenBPLibrary::WorldRotateEulerAngleTo_Sweep(USceneComponent* target, FVector eulerAngle, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, LTweenEase ease)
{
	if (!target->IsValidLowLevel() || target->IsPendingKill())
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::WorldRotateEulerAngleTo_Sweep] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto targetWeakPtr = TWeakObjectPtr<USceneComponent>(target);
	return ALTweenActor::To(FLTweenRotationQuatGetterFunction::CreateLambda([targetWeakPtr]
	{
		if (targetWeakPtr.IsValid())
			return targetWeakPtr->GetComponentRotation().Quaternion();
		else
			return FQuat();
	}), FLTweenRotationQuatSetterFunction::CreateUObject(target, &USceneComponent::SetWorldRotation), eulerAngle, duration, sweep, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport))
		->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULTweenBPLibrary::WorldRotationQuaternionTo_Sweep(USceneComponent* target, FQuat endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, LTweenEase ease)
{
	if (!target->IsValidLowLevel() || target->IsPendingKill())
	{
		UE_LOG(LTween, Error, TEXT("[ULTweenBPLibrary::WorldRotationTo_Sweep] target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto targetWeakPtr = TWeakObjectPtr<USceneComponent>(target);
	return ALTweenActor::To(FLTweenRotationQuatGetterFunction::CreateLambda([targetWeakPtr]
	{
		if (targetWeakPtr.IsValid())
			return targetWeakPtr->GetComponentRotation().Quaternion();
		else
			return FQuat();
	}), FLTweenRotationQuatSetterFunction::CreateUObject(target, &USceneComponent::SetWorldRotation), endValue, duration, sweep, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport))
		->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULTweenBPLibrary::WorldRotatorTo(USceneComponent* target, FRotator endValue, bool shortestPath, float duration, float delay, LTweenEase ease)
{
	if (!target->IsValidLowLevel() || target->IsPendingKill())
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
		auto targetWeakPtr = TWeakObjectPtr<USceneComponent>(target);
		return ALTweenActor::To(FLTweenRotatorGetterFunction::CreateLambda([targetWeakPtr]
		{
			if (targetWeakPtr.IsValid())
				return targetWeakPtr->GetComponentRotation();
			else
				return FRotator();
		}), FLTweenRotatorSetterFunction::CreateLambda([targetWeakPtr](FRotator value)
		{
			if (targetWeakPtr.IsValid())
				targetWeakPtr->SetWorldRotation(value);
		}), endValue, duration)->SetEase(ease)->SetDelay(delay);
	}
}
ULTweener* ULTweenBPLibrary::WorldRotatorTo_Sweep(USceneComponent* target, FRotator endValue, bool shortestPath, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration, float delay, LTweenEase ease)
{
	if (!target->IsValidLowLevel() || target->IsPendingKill())
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
		auto targetWeakPtr = TWeakObjectPtr<USceneComponent>(target);
		return ALTweenActor::To(FLTweenRotatorGetterFunction::CreateLambda([targetWeakPtr]
		{
			if (targetWeakPtr.IsValid())
				return targetWeakPtr->GetComponentRotation();
			else
				return FRotator();
		}), FLTweenRotatorSetterFunction::CreateLambda([targetWeakPtr, &sweepHitResult, sweep, teleport](FRotator value)
		{
			if (targetWeakPtr.IsValid())
				targetWeakPtr->SetWorldRotation(value, sweep, sweep ? &sweepHitResult : nullptr, TeleportFlagToEnum(teleport));
		}), endValue, duration)->SetEase(ease)->SetDelay(delay);
	}
}
#pragma endregion Rotation



ULTweener* ULTweenBPLibrary::MaterialScalarParameterTo(UMaterialInstanceDynamic* target, FName parameterName, float endValue, float duration, float delay, LTweenEase ease)
{
	if (!target->IsValidLowLevel() || target->IsPendingKill())
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
	auto targetWeakPtr = TWeakObjectPtr<UMaterialInstanceDynamic>(target);
	return ALTweenActor::To(FLTweenMaterialScalarGetterFunction::CreateLambda([targetWeakPtr, parameterName](float& result)
	{
		if (targetWeakPtr.IsValid())
			return targetWeakPtr->GetScalarParameterValue(parameterName, result);
		else
			return false;
	}), FLTweenMaterialScalarSetterFunction::CreateUObject(target, &UMaterialInstanceDynamic::SetScalarParameterByIndex), endValue, duration, parameterIndex)->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULTweenBPLibrary::MaterialVectorParameterTo(UMaterialInstanceDynamic* target, FName parameterName, FLinearColor endValue, float duration, float delay, LTweenEase ease)
{
	if (!target->IsValidLowLevel() || target->IsPendingKill())
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
	auto targetWeakPtr = TWeakObjectPtr<UMaterialInstanceDynamic>(target);
	return ALTweenActor::To(FLTweenMaterialVectorGetterFunction::CreateLambda([targetWeakPtr, parameterName](FLinearColor& result)
	{
		if (targetWeakPtr.IsValid())
			return targetWeakPtr->GetVectorParameterValue(parameterName, result);
		else
			return false;
	}), FLTweenMaterialVectorSetterFunction::CreateUObject(target, &UMaterialInstanceDynamic::SetVectorParameterByIndex), endValue, duration, parameterIndex)->SetEase(ease)->SetDelay(delay);
}