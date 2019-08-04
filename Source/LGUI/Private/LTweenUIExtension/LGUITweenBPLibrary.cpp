// Copyright 2019 LexLiu. All Rights Reserved.

#include "LTweenUIExtension/LGUITweenBPLibrary.h"

#pragma region QuickEntry
ULTweener* ULGUITweenBPLibrary::AlphaFrom(AActor* target, float startValue, float duration, float delay, LTweenEase ease)
{
	if (target == nullptr)
	{
		UE_LOG(LGUI, Error, TEXT("ULGUITweenBPLibrary::AlphaFrom target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	if (auto uiItem = Cast<UUIItem>(target->GetRootComponent()))
	{
		return AlphaFrom(uiItem, startValue, duration, delay, ease);
	}
	return nullptr;
}
ULTweener* ULGUITweenBPLibrary::AlphaTo(AActor* target, float endValue, float duration, float delay, LTweenEase ease)
{
	if (target == nullptr)
	{
		UE_LOG(LGUI, Error, TEXT("ULGUITweenBPLibrary::AlphaTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	if (auto uiItem = Cast<UUIItem>(target->GetRootComponent()))
	{
		return AlphaTo(uiItem, endValue, duration, delay, ease);
	}
	return nullptr;
}
#pragma endregion
#pragma region UIItem
ULTweener* ULGUITweenBPLibrary::WidthTo(UUIItem* target, float endValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUITweenBPLibrary::WidthTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FloatGetterFunction::CreateUObject(target, &UUIItem::GetWidth), FloatSetterFunction::CreateUObject(target, &UUIItem::SetWidth), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULGUITweenBPLibrary::HeightTo(UUIItem* target, float endValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUITweenBPLibrary::HeightTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FloatGetterFunction::CreateUObject(target, &UUIItem::GetHeight), FloatSetterFunction::CreateUObject(target, &UUIItem::SetHeight), endValue, duration)->SetEase(ease)->SetDelay(delay);
}

ULTweener* ULGUITweenBPLibrary::ColorTo(UUIItem* target, FColor endValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUITweenBPLibrary::ColorTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(ColorGetterFunction::CreateUObject(target, &UUIItem::GetColor), ColorSetterFunction::CreateUObject(target, &UUIItem::SetColor), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULGUITweenBPLibrary::ColorFrom(UUIItem* target, FColor startValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUITweenBPLibrary::ColorFrom target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto endValue = target->GetColor();
	target->SetColor(startValue);
	return ALTweenActor::To(ColorGetterFunction::CreateUObject(target, &UUIItem::GetColor), ColorSetterFunction::CreateUObject(target, &UUIItem::SetColor), endValue, duration)->SetEase(ease)->SetDelay(delay);
}

ULTweener* ULGUITweenBPLibrary::AlphaTo(UUIItem* target, float endValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUITweenBPLibrary::AlphaTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FloatGetterFunction::CreateUObject(target, &UUIItem::GetAlpha), FloatSetterFunction::CreateUObject(target, &UUIItem::SetAlpha), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULGUITweenBPLibrary::AlphaFrom(UUIItem* target, float startValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUITweenBPLibrary::AlphaFrom target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto endValue = target->GetAlpha();
	target->SetAlpha(startValue);
	return ALTweenActor::To(FloatGetterFunction::CreateUObject(target, &UUIItem::GetAlpha), FloatSetterFunction::CreateUObject(target, &UUIItem::SetAlpha), endValue, duration)->SetEase(ease)->SetDelay(delay);
}

ULTweener* ULGUITweenBPLibrary::AnchorOffsetXTo(UUIItem* target, float endValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUITweenBPLibrary::AnchorOffsetXTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FloatGetterFunction::CreateUObject(target, &UUIItem::GetAnchorOffsetX), FloatSetterFunction::CreateUObject(target, &UUIItem::SetAnchorOffsetX), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULGUITweenBPLibrary::AnchorOffsetYTo(UUIItem* target, float endValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUITweenBPLibrary::AnchorOffsetYTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FloatGetterFunction::CreateUObject(target, &UUIItem::GetAnchorOffsetY), FloatSetterFunction::CreateUObject(target, &UUIItem::SetAnchorOffsetY), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULGUITweenBPLibrary::PivotTo(UUIItem* target, FVector2D endValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUITweenBPLibrary::PivotTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(Vector2DGetterFunction::CreateUObject(target, &UUIItem::GetPivot), Vector2DSetterFunction::CreateUObject(target, &UUIItem::SetPivot), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULGUITweenBPLibrary::StretchLeftTo(UUIItem* target, float endValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUITweenBPLibrary::StretchLeftTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FloatGetterFunction::CreateUObject(target, &UUIItem::GetStretchLeft), FloatSetterFunction::CreateUObject(target, &UUIItem::SetStretchLeft), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULGUITweenBPLibrary::StretchRightTo(UUIItem* target, float endValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUITweenBPLibrary::StretchRightTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FloatGetterFunction::CreateUObject(target, &UUIItem::GetStretchRight), FloatSetterFunction::CreateUObject(target, &UUIItem::SetStretchRight), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULGUITweenBPLibrary::StretchTopTo(UUIItem* target, float endValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUITweenBPLibrary::StretchTopTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FloatGetterFunction::CreateUObject(target, &UUIItem::GetStretchTop), FloatSetterFunction::CreateUObject(target, &UUIItem::SetStretchTop), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULGUITweenBPLibrary::StretchBottomTo(UUIItem* target, float endValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUITweenBPLibrary::StretchBottomTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FloatGetterFunction::CreateUObject(target, &UUIItem::GetStretchBottom), FloatSetterFunction::CreateUObject(target, &UUIItem::SetStretchBottom), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
#pragma endregion


#pragma region UISector
ULTweener* ULGUITweenBPLibrary::StartAngleTo(UUISector* target, float endValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUITweenBPLibrary::StartAngleTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FloatGetterFunction::CreateUObject(target, &UUISector::GetStartAngle), FloatSetterFunction::CreateUObject(target, &UUISector::SetStartAngle), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULGUITweenBPLibrary::EndAngleTo(UUISector* target, float endValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUITweenBPLibrary::EndAngleTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FloatGetterFunction::CreateUObject(target, &UUISector::GetEndAngle), FloatSetterFunction::CreateUObject(target, &UUISector::SetEndAngle), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
#pragma endregion