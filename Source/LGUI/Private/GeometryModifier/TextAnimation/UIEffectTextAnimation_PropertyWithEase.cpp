﻿// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "GeometryModifier/TextAnimation/UIEffectTextAnimation_PropertyWithEase.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIText.h"
#include "Curves/CurveFloat.h"
#include "Utils/LGUIUtils.h"

const FLTweenFunction& UUIEffectTextAnimation_PropertyWithEase::GetEaseFunction()
{
	if (easeFunc.IsBound())return easeFunc;
	switch (easeType)
	{
	case ELTweenEase::Linear:
		easeFunc.BindStatic(&ULTweener::Linear);
		break;
	case ELTweenEase::InQuad:
		easeFunc.BindStatic(&ULTweener::InQuad);
		break;
	case ELTweenEase::OutQuad:
		easeFunc.BindStatic(&ULTweener::OutQuad);
		break;
	case ELTweenEase::InOutQuad:
		easeFunc.BindStatic(&ULTweener::InOutQuad);
		break;
	case ELTweenEase::InCubic:
		easeFunc.BindStatic(&ULTweener::InCubic);
		break;
	case ELTweenEase::OutCubic:
		easeFunc.BindStatic(&ULTweener::OutCubic);
		break;
	case ELTweenEase::InOutCubic:
		easeFunc.BindStatic(&ULTweener::InOutCubic);
		break;
	case ELTweenEase::InQuart:
		easeFunc.BindStatic(&ULTweener::InQuart);
		break;
	case ELTweenEase::OutQuart:
		easeFunc.BindStatic(&ULTweener::OutQuart);
		break;
	case ELTweenEase::InOutQuart:
		easeFunc.BindStatic(&ULTweener::InOutQuart);
		break;
	case ELTweenEase::InSine:
		easeFunc.BindStatic(&ULTweener::InSine);
		break;
	case ELTweenEase::OutSine:
		easeFunc.BindStatic(&ULTweener::OutSine);
		break;
	default:
	case ELTweenEase::InOutSine:
		easeFunc.BindStatic(&ULTweener::InOutSine);
		break;
	case ELTweenEase::InExpo:
		easeFunc.BindStatic(&ULTweener::InExpo);
		break;
	case ELTweenEase::OutExpo:
		easeFunc.BindStatic(&ULTweener::OutExpo);
		break;
	case ELTweenEase::InOutExpo:
		easeFunc.BindStatic(&ULTweener::InOutExpo);
		break;
	case ELTweenEase::InCirc:
		easeFunc.BindStatic(&ULTweener::InCirc);
		break;
	case ELTweenEase::OutCirc:
		easeFunc.BindStatic(&ULTweener::OutCirc);
		break;
	case ELTweenEase::InOutCirc:
		easeFunc.BindStatic(&ULTweener::InOutCirc);
		break;
	case ELTweenEase::InElastic:
		easeFunc.BindStatic(&ULTweener::InElastic);
		break;
	case ELTweenEase::OutElastic:
		easeFunc.BindStatic(&ULTweener::OutElastic);
		break;
	case ELTweenEase::InOutElastic:
		easeFunc.BindStatic(&ULTweener::InOutElastic);
		break;
	case ELTweenEase::InBack:
		easeFunc.BindStatic(&ULTweener::InBack);
		break;
	case ELTweenEase::OutBack:
		easeFunc.BindStatic(&ULTweener::OutBack);
		break;
	case ELTweenEase::InOutBack:
		easeFunc.BindStatic(&ULTweener::InOutBack);
		break;
	case ELTweenEase::InBounce:
		easeFunc.BindStatic(&ULTweener::InBounce);
		break;
	case ELTweenEase::OutBounce:
		easeFunc.BindStatic(&ULTweener::OutBounce);
		break;
	case ELTweenEase::InOutBounce:
		easeFunc.BindStatic(&ULTweener::InOutBounce);
		break;
	case ELTweenEase::CurveFloat:
		easeFunc.BindUObject(this, &UUIEffectTextAnimation_PropertyWithEase::EaseCurveFunction);
		break;
	}
	return easeFunc;
}
float UUIEffectTextAnimation_PropertyWithEase::EaseCurveFunction(float c, float b, float t, float d)
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
void UUIEffectTextAnimation_PropertyWithEase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.Property)
	{
		auto propertyName = Property->GetFName();
		if (propertyName == GET_MEMBER_NAME_CHECKED(UUIEffectTextAnimation_PropertyWithEase, easeType))
		{
			easeFunc.Unbind();
		}
	}
}
#endif

void UUIEffectTextAnimation_PropertyWithEase::SetEaseType(ELTweenEase value)
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
void UUIEffectTextAnimation_PropertyWithEase::SetEaseCurve(UCurveFloat* value)
{
	if (easeCurve != value)
	{
		easeCurve = value;
		if (easeType == ELTweenEase::CurveFloat)
		{
			if (auto uiText = GetUIText())
			{
				uiText->MarkVertexPositionDirty();
			}
		}
	}
}

void UUIEffectTextAnimation_PositionProperty::ApplyProperty(UUIText* InUIText, const FUIEffectTextAnimation_SelectResult& InSelection, UIGeometry* InGeometry)
{
	auto easeFunction = GetEaseFunction();
	auto& originVertices = InGeometry->originVertices;
	auto& charProperties = InUIText->GetCharPropertyArray();
	for (int charIndex = InSelection.startCharIndex; charIndex < InSelection.endCharCount; charIndex++)
	{
		auto charPropertyItem = charProperties[charIndex];
		int startVertIndex = charPropertyItem.StartVertIndex;
		int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;
		float lerpValue = FMath::Clamp(InSelection.lerpValueArray[charIndex - InSelection.startCharIndex], 0.0f, 1.0f);
		lerpValue = easeFunction.Execute(1.0f, 0.0f, lerpValue, 1.0f);
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& pos = originVertices[vertIndex].Position;
			pos = FMath::Lerp(pos, pos + (FVector3f)position, lerpValue);
		}
	}
}

void UUIEffectTextAnimation_PositionRandomProperty::ApplyProperty(UUIText* InUIText, const FUIEffectTextAnimation_SelectResult& InSelection, UIGeometry* InGeometry)
{
	FMath::RandInit(seed);
	auto easeFunction = GetEaseFunction();
	auto& originVertices = InGeometry->originVertices;
	auto& charProperties = InUIText->GetCharPropertyArray();
	for (int charIndex = InSelection.startCharIndex; charIndex < InSelection.endCharCount; charIndex++)
	{
		auto charPropertyItem = charProperties[charIndex];
		int startVertIndex = charPropertyItem.StartVertIndex;
		int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;
		float lerpValue = FMath::Clamp(InSelection.lerpValueArray[charIndex - InSelection.startCharIndex], 0.0f, 1.0f);
		lerpValue = easeFunction.Execute(1.0f, 0.0f, lerpValue, 1.0f);
		auto position = FVector3f(FMath::FRandRange(min.X, max.X), FMath::FRandRange(min.Y, max.Y), FMath::FRandRange(min.Z, max.Z));
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& pos = originVertices[vertIndex].Position;
			pos = FMath::Lerp(pos, pos + position, lerpValue);
		}
	}
}

void UUIEffectTextAnimation_RotationProperty::ApplyProperty(UUIText* InUIText, const FUIEffectTextAnimation_SelectResult& InSelection, UIGeometry* InGeometry)
{
	auto easeFunction = GetEaseFunction();
	auto& originVertices = InGeometry->originVertices;
	auto& charProperties = InUIText->GetCharPropertyArray();
	for (int charIndex = InSelection.startCharIndex; charIndex < InSelection.endCharCount; charIndex++)
	{
		auto charPropertyItem = charProperties[charIndex];
		int startVertIndex = charPropertyItem.StartVertIndex;
		int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;
		auto charCenterPos = originVertices[startVertIndex].Position;
		for (int vertIndex = startVertIndex + 1; vertIndex < endVertIndex; vertIndex++)
		{
			charCenterPos += originVertices[vertIndex].Position;
		}
		charCenterPos /= charPropertyItem.VertCount;
		float lerpValue = FMath::Clamp(InSelection.lerpValueArray[charIndex - InSelection.startCharIndex], 0.0f, 1.0f);
		lerpValue = easeFunction.Execute(1.0f, 0.0f, lerpValue, 1.0f);
		auto calcRotationMatrix = FRotationMatrix44f(((FRotator3f)rotator) * lerpValue);
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& pos = originVertices[vertIndex].Position;
			auto vector = pos - charCenterPos;
			pos = charCenterPos + calcRotationMatrix.TransformPosition(vector);
		}
	}
}

void UUIEffectTextAnimation_RotationRandomProperty::ApplyProperty(UUIText* InUIText, const FUIEffectTextAnimation_SelectResult& InSelection, UIGeometry* InGeometry)
{
	FMath::RandInit(seed);
	auto easeFunction = GetEaseFunction();
	auto& originVertices = InGeometry->originVertices;
	auto& charProperties = InUIText->GetCharPropertyArray();
	for (int charIndex = InSelection.startCharIndex; charIndex < InSelection.endCharCount; charIndex++)
	{
		auto charPropertyItem = charProperties[charIndex];
		int startVertIndex = charPropertyItem.StartVertIndex;
		int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;
		auto charCenterPos = originVertices[startVertIndex].Position;
		for (int vertIndex = startVertIndex + 1; vertIndex < endVertIndex; vertIndex++)
		{
			charCenterPos += originVertices[vertIndex].Position;
		}
		charCenterPos /= charPropertyItem.VertCount;
		float lerpValue = FMath::Clamp(InSelection.lerpValueArray[charIndex - InSelection.startCharIndex], 0.0f, 1.0f);
		lerpValue = easeFunction.Execute(1.0f, 0.0f, lerpValue, 1.0f);
		auto rotator = FRotator3f(FMath::FRandRange(min.Pitch, max.Pitch), FMath::FRandRange(min.Yaw, max.Yaw), FMath::FRandRange(min.Roll, max.Roll));
		auto calcRotationMatrix = FRotationMatrix44f(rotator * lerpValue);
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& pos = originVertices[vertIndex].Position;
			auto vector = pos - charCenterPos;
			pos = charCenterPos + calcRotationMatrix.TransformPosition(vector);
		}
	}
}

void UUIEffectTextAnimation_ScaleProperty::ApplyProperty(UUIText* InUIText, const FUIEffectTextAnimation_SelectResult& InSelection, UIGeometry* InGeometry)
{
	auto easeFunction = GetEaseFunction();
	auto& originVertices = InGeometry->originVertices;
	auto& charProperties = InUIText->GetCharPropertyArray();
	for (int charIndex = InSelection.startCharIndex; charIndex < InSelection.endCharCount; charIndex++)
	{
		auto charPropertyItem = charProperties[charIndex];
		int startVertIndex = charPropertyItem.StartVertIndex;
		int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;
		auto charCenterPos = originVertices[startVertIndex].Position;
		for (int vertIndex = startVertIndex + 1; vertIndex < endVertIndex; vertIndex++)
		{
			charCenterPos += originVertices[vertIndex].Position;
		}
		charCenterPos /= charPropertyItem.VertCount;
		float lerpValue = FMath::Clamp(InSelection.lerpValueArray[charIndex - InSelection.startCharIndex], 0.0f, 1.0f);
		lerpValue = easeFunction.Execute(1.0f, 0.0f, lerpValue, 1.0f);
		auto calcScale = FMath::Lerp(FVector3f::OneVector, (FVector3f)scale, lerpValue);
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& pos = originVertices[vertIndex].Position;
			auto vector = pos - charCenterPos;
			pos = charCenterPos + vector * calcScale;
		}
	}
}

void UUIEffectTextAnimation_ScaleRandomProperty::ApplyProperty(UUIText* InUIText, const FUIEffectTextAnimation_SelectResult& InSelection, UIGeometry* InGeometry)
{
	FMath::RandInit(seed);
	auto easeFunction = GetEaseFunction();
	auto& originVertices = InGeometry->originVertices;
	auto& charProperties = InUIText->GetCharPropertyArray();
	for (int charIndex = InSelection.startCharIndex; charIndex < InSelection.endCharCount; charIndex++)
	{
		auto charPropertyItem = charProperties[charIndex];
		int startVertIndex = charPropertyItem.StartVertIndex;
		int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;
		auto charCenterPos = originVertices[startVertIndex].Position;
		for (int vertIndex = startVertIndex + 1; vertIndex < endVertIndex; vertIndex++)
		{
			charCenterPos += originVertices[vertIndex].Position;
		}
		charCenterPos /= charPropertyItem.VertCount;
		float lerpValue = FMath::Clamp(InSelection.lerpValueArray[charIndex - InSelection.startCharIndex], 0.0f, 1.0f);
		lerpValue = easeFunction.Execute(1.0f, 0.0f, lerpValue, 1.0f);
		auto scale = FVector3f(FMath::FRandRange(min.X, max.X), FMath::FRandRange(min.Y, max.Y), FMath::FRandRange(min.Z, max.Z));
		auto calcScale = FMath::Lerp(FVector3f::OneVector, scale, lerpValue);
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& pos = originVertices[vertIndex].Position;
			auto vector = pos - charCenterPos;
			pos = charCenterPos + vector * calcScale;
		}
	}
}

void UUIEffectTextAnimation_AlphaProperty::ApplyProperty(UUIText* InUIText, const FUIEffectTextAnimation_SelectResult& InSelection, UIGeometry* InGeometry)
{
	auto easeFunction = GetEaseFunction();
	auto& vertices = InGeometry->vertices;
	auto& charProperties = InUIText->GetCharPropertyArray();
	for (int charIndex = InSelection.startCharIndex; charIndex < InSelection.endCharCount; charIndex++)
	{
		auto charPropertyItem = charProperties[charIndex];
		int startVertIndex = charPropertyItem.StartVertIndex;
		int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;
		float lerpValue = FMath::Clamp(InSelection.lerpValueArray[charIndex - InSelection.startCharIndex], 0.0f, 1.0f);
		lerpValue = easeFunction.Execute(1.0f, 0.0f, lerpValue, 1.0f);
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& vert = vertices[vertIndex];
			vert.Color.A = FMath::Lerp(vert.Color.A, (uint8)(vert.Color.A * alpha), lerpValue);
		}
	}
}

void UUIEffectTextAnimation_ColorProperty::ApplyProperty(UUIText* InUIText, const FUIEffectTextAnimation_SelectResult& InSelection, UIGeometry* InGeometry)
{
	auto easeFunction = GetEaseFunction();
	auto& vertices = InGeometry->vertices;
	auto& charProperties = InUIText->GetCharPropertyArray();
	FVector colorHsv;
	if (useHSV)
	{
		colorHsv = LGUIUtils::ColorRGBToColorHSVData(color);
	}
	for (int charIndex = InSelection.startCharIndex; charIndex < InSelection.endCharCount; charIndex++)
	{
		auto charPropertyItem = charProperties[charIndex];
		int startVertIndex = charPropertyItem.StartVertIndex;
		int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;
		float lerpValue = FMath::Clamp(InSelection.lerpValueArray[charIndex - InSelection.startCharIndex], 0.0f, 1.0f);
		lerpValue = easeFunction.Execute(1.0f, 0.0f, lerpValue, 1.0f);
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& vert = vertices[vertIndex];
			if (useHSV)
			{
				auto vertColorHsv = LGUIUtils::ColorRGBToColorHSVData(vert.Color);
				vertColorHsv = FMath::Lerp(vertColorHsv, colorHsv, lerpValue);
				auto vertColor = LGUIUtils::ColorHSVDataToColorRGB(vertColorHsv);
				vert.Color.R = vertColor.R;
				vert.Color.G = vertColor.G;
				vert.Color.B = vertColor.B;
			}
			else
			{
				vert.Color.R = FMath::Lerp(vert.Color.R, color.R, lerpValue);
				vert.Color.G = FMath::Lerp(vert.Color.G, color.G, lerpValue);
				vert.Color.B = FMath::Lerp(vert.Color.B, color.B, lerpValue);
			}
			vert.Color.A = FMath::Lerp(vert.Color.A, color.A, lerpValue);
		}
	}
}
void UUIEffectTextAnimation_ColorProperty::SetUseHSV(bool value)
{
	if (useHSV != value)
	{
		useHSV = value;
		if (auto uiText = GetUIText())
		{
			uiText->MarkColorDirty();
		}
	}
}

void UUIEffectTextAnimation_ColorRandomProperty::ApplyProperty(UUIText* InUIText, const FUIEffectTextAnimation_SelectResult& InSelection, UIGeometry* InGeometry)
{
	FMath::RandInit(seed);
	auto easeFunction = GetEaseFunction();
	auto& vertices = InGeometry->vertices;
	auto& charProperties = InUIText->GetCharPropertyArray();
	for (int charIndex = InSelection.startCharIndex; charIndex < InSelection.endCharCount; charIndex++)
	{
		auto charPropertyItem = charProperties[charIndex];
		int startVertIndex = charPropertyItem.StartVertIndex;
		int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;
		auto color = FColor((uint8)FMath::RandRange(min.R, max.R), (uint8)FMath::RandRange(min.G, max.G), (uint8)FMath::RandRange(min.B, max.B), (uint8)FMath::RandRange(min.A, max.A));
		float lerpValue = FMath::Clamp(InSelection.lerpValueArray[charIndex - InSelection.startCharIndex], 0.0f, 1.0f);
		lerpValue = easeFunction.Execute(1.0f, 0.0f, lerpValue, 1.0f);
		FVector colorHsv;
		if (useHSV)
		{
			colorHsv = LGUIUtils::ColorRGBToColorHSVData(color);
		}
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& vert = vertices[vertIndex];
			if (useHSV)
			{
				auto vertColorHsv = LGUIUtils::ColorRGBToColorHSVData(vert.Color);
				vertColorHsv = FMath::Lerp(vertColorHsv, colorHsv, lerpValue);
				auto vertColor = LGUIUtils::ColorHSVDataToColorRGB(vertColorHsv);
				vert.Color.R = vertColor.R;
				vert.Color.G = vertColor.G;
				vert.Color.B = vertColor.B;
			}
			else
			{
				vert.Color.R = FMath::Lerp(vert.Color.R, color.R, lerpValue);
				vert.Color.G = FMath::Lerp(vert.Color.G, color.G, lerpValue);
				vert.Color.B = FMath::Lerp(vert.Color.B, color.B, lerpValue);
			}
			vert.Color.A = FMath::Lerp(vert.Color.A, color.A, lerpValue);
		}
	}
}
void UUIEffectTextAnimation_ColorRandomProperty::SetUseHSV(bool value)
{
	if (useHSV != value)
	{
		useHSV = value;
		if (auto uiText = GetUIText())
		{
			uiText->MarkColorDirty();
		}
	}
}

void UUIEffectTextAnimation_PositionProperty::SetPosition(FVector value)
{
	if (position != value)
	{
		position = value;
		MarkUITextPositionDirty();
	}
}
void UUIEffectTextAnimation_PositionRandomProperty::SetSeed(int value)
{
	if (seed != value)
	{
		seed = value;
		MarkUITextPositionDirty();
	}
}
void UUIEffectTextAnimation_PositionRandomProperty::SetMin(FVector value)
{
	if (min != value)
	{
		min = value;
		MarkUITextPositionDirty();
	}
}
void UUIEffectTextAnimation_PositionRandomProperty::SetMax(FVector value)
{
	if (max != value)
	{
		max = value;
		MarkUITextPositionDirty();
	}
}
void UUIEffectTextAnimation_RotationProperty::SetRotator(FRotator value)
{
	if (rotator != value)
	{
		rotator = value;
		MarkUITextPositionDirty();
	}
}
void UUIEffectTextAnimation_RotationRandomProperty::SetSeed(int value)
{
	if (seed != value)
	{
		seed = value;
		MarkUITextPositionDirty();
	}
}
void UUIEffectTextAnimation_RotationRandomProperty::SetMin(FRotator value)
{
	if (min != value)
	{
		min = value;
		MarkUITextPositionDirty();
	}
}
void UUIEffectTextAnimation_RotationRandomProperty::SetMax(FRotator value)
{
	if (max != value)
	{
		max = value;
		MarkUITextPositionDirty();
	}
}
void UUIEffectTextAnimation_ScaleProperty::SetScale(FVector value)
{
	if (scale != value)
	{
		scale = value;
		MarkUITextPositionDirty();
	}
}
void UUIEffectTextAnimation_ScaleRandomProperty::SetSeed(int value)
{
	if (seed != value)
	{
		seed = value;
		MarkUITextPositionDirty();
	}
}
void UUIEffectTextAnimation_ScaleRandomProperty::SetMin(FVector value)
{
	if (min != value)
	{
		min = value;
		MarkUITextPositionDirty();
	}
}
void UUIEffectTextAnimation_ScaleRandomProperty::SetMax(FVector value)
{
	if (max != value)
	{
		max = value;
		MarkUITextPositionDirty();
	}
}
void UUIEffectTextAnimation_AlphaProperty::SetAlpha(float value)
{
	if (alpha != value)
	{
		alpha = value;
		MarkUITextPositionDirty();
	}
}
void UUIEffectTextAnimation_ColorProperty::SetColor(FColor value)
{
	if (color != value)
	{
		color = value;
		MarkUITextPositionDirty();
	}
}
void UUIEffectTextAnimation_ColorRandomProperty::SetSeed(int value)
{
	if (seed != value)
	{
		seed = value;
		MarkUITextPositionDirty();
	}
}
void UUIEffectTextAnimation_ColorRandomProperty::SetMin(FColor value)
{
	if (min != value)
	{
		min = value;
		MarkUITextPositionDirty();
	}
}
void UUIEffectTextAnimation_ColorRandomProperty::SetMax(FColor value)
{
	if (max != value)
	{
		max = value;
		MarkUITextPositionDirty();
	}
}
