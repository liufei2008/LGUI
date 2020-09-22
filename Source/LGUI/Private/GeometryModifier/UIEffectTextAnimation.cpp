// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "GeometryModifier/UIEffectTextAnimation.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIText.h"
#include "Curves/CurveFloat.h"


UUIEffectTextAnimation::UUIEffectTextAnimation()
{
	PrimaryComponentTick.bCanEverTick = false;
}
bool UUIEffectTextAnimation::CheckUIText()
{
	if (IsValid(uiText))return true;
	if (auto uiRenderable = GetRenderableUIItem())
	{
		uiText = Cast<UUIText>(uiRenderable);
		if (IsValid(uiText))
		{
			return true;
		}
	}
	return false;
}
void UUIEffectTextAnimation::ModifyUIGeometry(TSharedPtr<UIGeometry>& InGeometry, int32& InOutOriginVerticesCount, int32& InOutOriginTriangleIndicesCount, bool& OutTriangleChanged)
{
	if (!CheckUIText())return;
	if (IsValid(selector))
	{
		if (selector->Select(uiText, selection))
		{
			for (auto propertyItem : properties)
			{
				if (IsValid(propertyItem))
				{
					propertyItem->ApplyProperty(uiText, selection, InGeometry);
				}
			}
		}
	}
}
UUIText* UUIEffectTextAnimation::GetUIText()
{
	CheckUIText();
	return uiText;
}
UUIEffectTextAnimation_Property* UUIEffectTextAnimation::GetProperty(int index)const
{
	if (index >= properties.Num())
	{
		UE_LOG(LGUI, Error, TEXT("[UUIEffectTextAnimation::GetProperty]index:%d out of range:%d"), index, properties.Num());
		return nullptr;
	}
	return properties[index];
}
void UUIEffectTextAnimation::SetSelector(UUIEffectTextAnimation_Selector* value)
{
	if (selector != value)
	{
		selector = value;
		if (CheckUIText())
		{
			uiText->MarkTriangleDirty();
		}
	}
}
void UUIEffectTextAnimation::SetProperties(const TArray<UUIEffectTextAnimation_Property*>& value)
{
	if (properties != value)
	{
		properties = value;
		if (CheckUIText())
		{
			uiText->MarkTriangleDirty();
		}
	}
}
void UUIEffectTextAnimation::SetProperty(int index, UUIEffectTextAnimation_Property* value)
{
	if (index >= properties.Num())
	{
		UE_LOG(LGUI, Error, TEXT("[UUIEffectTextAnimation::SetProperty]index:%d out of range:%d"), index, properties.Num());
		return;
	}
	if (properties[index] != value)
	{
		properties[index] = value;
		if (CheckUIText())
		{
			uiText->MarkTriangleDirty();
		}
	}
}

UUIText* UUIEffectTextAnimation_Selector::GetUIText()
{
	if (auto outter = this->GetOuter())
	{
		if (auto uiTextAnimation = Cast<UUIEffectTextAnimation>(outter))
		{
			return uiTextAnimation->GetUIText();
		}
	}
	return nullptr;
}
UUIText* UUIEffectTextAnimation_Property::GetUIText()
{
	if (auto outter = this->GetOuter())
	{
		if (auto uiTextAnimation = Cast<UUIEffectTextAnimation>(outter))
		{
			return uiTextAnimation->GetUIText();
		}
	}
	return nullptr;
}
const FLTweenFunction& UUIEffectTextAnimation_Property::GetEaseFunction()
{
	if (easeFunc.IsBound())return easeFunc;
	switch (easeType)
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
		easeFunc.BindUObject(this, &UUIEffectTextAnimation_Property::EaseCurveFunction);
		break;
	}
	return easeFunc;
}
float UUIEffectTextAnimation_Property::EaseCurveFunction(float c, float b, float t, float d)
{
	if (easeCurve != nullptr)
	{
		return easeCurve->GetFloatValue(t / d) * c + b;
	}
	else
	{
		return ULTweener::Linear(c, b, t, d);
	}
}
#if WITH_EDITOR
void UUIEffectTextAnimation_Property::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.Property)
	{
		auto propertyName = Property->GetFName();
		if (propertyName == GET_MEMBER_NAME_CHECKED(UUIEffectTextAnimation_Property, easeType))
		{
			easeFunc.Unbind();
		}
	}
}
#endif

void UUIEffectTextAnimation_Property::SetEaseType(LTweenEase value)
{
	if (easeType != value)
	{
		easeType = value;
		easeFunc.Unbind();
		if (auto uiText = GetUIText())
		{
			uiText->MarkVertexPositionDirty();
		}
	}
}
void UUIEffectTextAnimation_Property::SetEaseCurve(UCurveFloat* value)
{
	if (easeCurve != value)
	{
		easeCurve = value;
		if (easeType == LTweenEase::CurveFloat)
		{
			if (auto uiText = GetUIText())
			{
				uiText->MarkVertexPositionDirty();
			}
		}
	}
}
