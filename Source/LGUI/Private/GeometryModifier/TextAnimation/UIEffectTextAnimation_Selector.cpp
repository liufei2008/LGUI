// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "GeometryModifier/TextAnimation/UIEffectTextAnimation_Selector.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIText.h"
#include "Curves/CurveFloat.h"
#include "Core/ActorComponent/UIText.h"


void UUIEffectTextAnimation_RangeSelector::CheckAndInitSelector()
{
	if (easeFunc.IsBound())return;
	switch (easeLerp)
	{
	case LTweenEase::Linear:
		easeFunc.BindStatic(&ULTweener::Linear);
		break;
	case LTweenEase::InQuad:
		easeFunc.BindStatic(&ULTweener::InQuad);
		break;
	case LTweenEase::OutQuad:
		easeFunc.BindStatic(&ULTweener::OutQuad);
		break;
	case LTweenEase::InOutQuad:
		easeFunc.BindStatic(&ULTweener::InOutQuad);
		break;
	case LTweenEase::InCubic:
		easeFunc.BindStatic(&ULTweener::InCubic);
		break;
	case LTweenEase::OutCubic:
		easeFunc.BindStatic(&ULTweener::OutCubic);
		break;
	case LTweenEase::InOutCubic:
		easeFunc.BindStatic(&ULTweener::InOutCubic);
		break;
	case LTweenEase::InQuart:
		easeFunc.BindStatic(&ULTweener::InQuart);
		break;
	case LTweenEase::OutQuart:
		easeFunc.BindStatic(&ULTweener::OutQuart);
		break;
	case LTweenEase::InOutQuart:
		easeFunc.BindStatic(&ULTweener::InOutQuart);
		break;
	case LTweenEase::InSine:
		easeFunc.BindStatic(&ULTweener::InSine);
		break;
	case LTweenEase::OutSine:
		easeFunc.BindStatic(&ULTweener::OutSine);
		break;
	default:
	case LTweenEase::InOutSine:
		easeFunc.BindStatic(&ULTweener::InOutSine);
		break;
	case LTweenEase::InExpo:
		easeFunc.BindStatic(&ULTweener::InExpo);
		break;
	case LTweenEase::OutExpo:
		easeFunc.BindStatic(&ULTweener::OutExpo);
		break;
	case LTweenEase::InOutExpo:
		easeFunc.BindStatic(&ULTweener::InOutExpo);
		break;
	case LTweenEase::InCirc:
		easeFunc.BindStatic(&ULTweener::InCirc);
		break;
	case LTweenEase::OutCirc:
		easeFunc.BindStatic(&ULTweener::OutCirc);
		break;
	case LTweenEase::InOutCirc:
		easeFunc.BindStatic(&ULTweener::InOutCirc);
		break;
	case LTweenEase::InElastic:
		easeFunc.BindStatic(&ULTweener::InElastic);
		break;
	case LTweenEase::OutElastic:
		easeFunc.BindStatic(&ULTweener::OutElastic);
		break;
	case LTweenEase::InOutElastic:
		easeFunc.BindStatic(&ULTweener::InOutElastic);
		break;
	case LTweenEase::InBack:
		easeFunc.BindStatic(&ULTweener::InBack);
		break;
	case LTweenEase::OutBack:
		easeFunc.BindStatic(&ULTweener::OutBack);
		break;
	case LTweenEase::InOutBack:
		easeFunc.BindStatic(&ULTweener::InOutBack);
		break;
	case LTweenEase::InBounce:
		easeFunc.BindStatic(&ULTweener::InBounce);
		break;
	case LTweenEase::OutBounce:
		easeFunc.BindStatic(&ULTweener::OutBounce);
		break;
	case LTweenEase::InOutBounce:
		easeFunc.BindStatic(&ULTweener::InOutBounce);
		break;
	case LTweenEase::CurveFloat:
		easeFunc.BindUObject(this, &UUIEffectTextAnimation_RangeSelector::CurveFloat);
		break;
	}
}
float UUIEffectTextAnimation_RangeSelector::CurveFloat(float c, float b, float t, float d)
{
	if (curveFloat != nullptr)
	{
		return curveFloat->GetFloatValue(t / d) * c + b;
	}
	else
	{
		return ULTweener::Linear(c, b, t, d);
	}
}
#if WITH_EDITOR
void UUIEffectTextAnimation_RangeSelector::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.Property)
	{
		auto propertyName = Property->GetFName();
		if (propertyName == GET_MEMBER_NAME_CHECKED(UUIEffectTextAnimation_RangeSelector, easeLerp))
		{
			easeFunc.Unbind();
		}
	}
}
#endif
bool UUIEffectTextAnimation_RangeSelector::Select(UUIText* InUIText, TArray<FUIEffectTextAnimation_SelectResult>& OutSelection)
{
	CheckAndInitSelector();
	if (FMath::Abs(range) < KINDA_SMALL_NUMBER)return false;
	auto& charProperties = InUIText->GetCharPropertyArray();
	float interval = 1.0f / charProperties.Num();
	float calculatedOffset = offset * (1.0f + range) - range;
	float value = -calculatedOffset;
	OutSelection.Reset(charProperties.Num());
	for (int i = 0; i < charProperties.Num(); i++)
	{
		auto clampedValue = FMath::Clamp(value, 0.0f, range);
		auto lerpValue = easeFunc.Execute(1.0f, 0.0f, clampedValue, range);
		auto selectItem = FUIEffectTextAnimation_SelectResult();
		selectItem.lerpValue = flipDirection ? 1.0f - lerpValue : lerpValue;
		OutSelection.Add(selectItem);
		value += interval;
	}
	return true;
}
void UUIEffectTextAnimation_RangeSelector::SetRange(float value)
{
	if (range != value)
	{
		range = value;
		if (auto uiText = GetUIText())
		{
			uiText->MarkVertexPositionDirty();
		}
	}
}
void UUIEffectTextAnimation_RangeSelector::SetOffset(float value)
{
	if (offset != value)
	{
		offset = value;
		if (auto uiText = GetUIText())
		{
			uiText->MarkVertexPositionDirty();
		}
	}
}
void UUIEffectTextAnimation_RangeSelector::SetFlipDirection(bool value)
{
	if (flipDirection != value)
	{
		flipDirection = value;
		if (auto uiText = GetUIText())
		{
			uiText->MarkVertexPositionDirty();
		}
	}
}
void UUIEffectTextAnimation_RangeSelector::SetEaseLerp(LTweenEase value)
{
	if (easeLerp != value)
	{
		easeLerp = value;
		easeFunc.Unbind();
		if (auto uiText = GetUIText())
		{
			uiText->MarkVertexPositionDirty();
		}
	}
}
void UUIEffectTextAnimation_RangeSelector::SetCurveFloat(UCurveFloat* value)
{
	if (curveFloat != value)
	{
		curveFloat = value;
		if (easeLerp == LTweenEase::CurveFloat)
		{
			if (auto uiText = GetUIText())
			{
				uiText->MarkVertexPositionDirty();
			}
		}
	}
}

bool UUIEffectTextAnimation_RandomSelector::Select(UUIText* InUIText, TArray<FUIEffectTextAnimation_SelectResult>& OutSelection)
{
	FMath::RandInit(seed);
	auto& charProperties = InUIText->GetCharPropertyArray();
	OutSelection.Reset(charProperties.Num());
	for (int i = 0; i < charProperties.Num(); i++)
	{
		auto selectItem = FUIEffectTextAnimation_SelectResult();
		selectItem.lerpValue = FMath::FRand() * strength;
		OutSelection.Add(selectItem);
	}
	return true;
}
void UUIEffectTextAnimation_RandomSelector::SetSeed(int value)
{
	if (seed != value)
	{
		seed = value;
		if (auto uiText = GetUIText())
		{
			uiText->MarkVertexPositionDirty();
		}
	}
}
void UUIEffectTextAnimation_RandomSelector::SetStrength(float value)
{
	if (strength != value)
	{
		strength = value;
		if (auto uiText = GetUIText())
		{
			uiText->MarkVertexPositionDirty();
		}
	}
}
