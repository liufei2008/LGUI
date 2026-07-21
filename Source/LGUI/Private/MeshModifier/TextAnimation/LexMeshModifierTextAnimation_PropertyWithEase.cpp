// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LGUI/Public/MeshModifier/TextAnimation/LexMeshModifierTextAnimation_PropertyWithEase.h"
#include "LGUI.h"
#include "Core/Components/LexText.h"
#include "Curves/CurveFloat.h"
#include "Utils/LexUIUtils.h"

const FLTweenFunction& ULexMeshModifierTextAnimation_PropertyWithEase::GetEaseFunction()
{
	if (EaseFunc.IsBound())return EaseFunc;
	switch (EaseType)
	{
	case ELTweenEase::Linear:
		EaseFunc.BindStatic(&ULTweener::Linear);
		break;
	case ELTweenEase::InQuad:
		EaseFunc.BindStatic(&ULTweener::InQuad);
		break;
	case ELTweenEase::OutQuad:
		EaseFunc.BindStatic(&ULTweener::OutQuad);
		break;
	case ELTweenEase::InOutQuad:
		EaseFunc.BindStatic(&ULTweener::InOutQuad);
		break;
	case ELTweenEase::InCubic:
		EaseFunc.BindStatic(&ULTweener::InCubic);
		break;
	case ELTweenEase::OutCubic:
		EaseFunc.BindStatic(&ULTweener::OutCubic);
		break;
	case ELTweenEase::InOutCubic:
		EaseFunc.BindStatic(&ULTweener::InOutCubic);
		break;
	case ELTweenEase::InQuart:
		EaseFunc.BindStatic(&ULTweener::InQuart);
		break;
	case ELTweenEase::OutQuart:
		EaseFunc.BindStatic(&ULTweener::OutQuart);
		break;
	case ELTweenEase::InOutQuart:
		EaseFunc.BindStatic(&ULTweener::InOutQuart);
		break;
	case ELTweenEase::InSine:
		EaseFunc.BindStatic(&ULTweener::InSine);
		break;
	case ELTweenEase::OutSine:
		EaseFunc.BindStatic(&ULTweener::OutSine);
		break;
	default:
	case ELTweenEase::InOutSine:
		EaseFunc.BindStatic(&ULTweener::InOutSine);
		break;
	case ELTweenEase::InExpo:
		EaseFunc.BindStatic(&ULTweener::InExpo);
		break;
	case ELTweenEase::OutExpo:
		EaseFunc.BindStatic(&ULTweener::OutExpo);
		break;
	case ELTweenEase::InOutExpo:
		EaseFunc.BindStatic(&ULTweener::InOutExpo);
		break;
	case ELTweenEase::InCirc:
		EaseFunc.BindStatic(&ULTweener::InCirc);
		break;
	case ELTweenEase::OutCirc:
		EaseFunc.BindStatic(&ULTweener::OutCirc);
		break;
	case ELTweenEase::InOutCirc:
		EaseFunc.BindStatic(&ULTweener::InOutCirc);
		break;
	case ELTweenEase::InElastic:
		EaseFunc.BindStatic(&ULTweener::InElastic);
		break;
	case ELTweenEase::OutElastic:
		EaseFunc.BindStatic(&ULTweener::OutElastic);
		break;
	case ELTweenEase::InOutElastic:
		EaseFunc.BindStatic(&ULTweener::InOutElastic);
		break;
	case ELTweenEase::InBack:
		EaseFunc.BindStatic(&ULTweener::InBack);
		break;
	case ELTweenEase::OutBack:
		EaseFunc.BindStatic(&ULTweener::OutBack);
		break;
	case ELTweenEase::InOutBack:
		EaseFunc.BindStatic(&ULTweener::InOutBack);
		break;
	case ELTweenEase::InBounce:
		EaseFunc.BindStatic(&ULTweener::InBounce);
		break;
	case ELTweenEase::OutBounce:
		EaseFunc.BindStatic(&ULTweener::OutBounce);
		break;
	case ELTweenEase::InOutBounce:
		EaseFunc.BindStatic(&ULTweener::InOutBounce);
		break;
	case ELTweenEase::CurveFloat:
		EaseFunc.BindUObject(this, &ULexMeshModifierTextAnimation_PropertyWithEase::EaseCurveFunction);
		break;
	}
	return EaseFunc;
}
float ULexMeshModifierTextAnimation_PropertyWithEase::EaseCurveFunction(float c, float b, float t, float d)
{
	if (EaseCurve != nullptr)
	{
		return EaseCurve->GetFloatValue(t / d) * c + b;
	}
	else
	{
		return ULTweener::Linear(c, b, t, d);
	}
}
#if WITH_EDITOR
void ULexMeshModifierTextAnimation_PropertyWithEase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.Property)
	{
		auto propertyName = Property->GetFName();
		if (propertyName == GET_MEMBER_NAME_CHECKED(ULexMeshModifierTextAnimation_PropertyWithEase, EaseType))
		{
			EaseFunc.Unbind();
		}
	}
}
#endif

void ULexMeshModifierTextAnimation_PropertyWithEase::SetEaseType(ELTweenEase Value)
{
	if (EaseType != Value)
	{
		EaseType = Value;
		EaseFunc.Unbind();
		if (auto LexText = GetLexText())
		{
			LexText->MarkVertexPositionDirty();
		}
	}
}
void ULexMeshModifierTextAnimation_PropertyWithEase::SetEaseCurve(UCurveFloat* Value)
{
	if (EaseCurve != Value)
	{
		EaseCurve = Value;
		if (EaseType == ELTweenEase::CurveFloat)
		{
			if (auto LexText = GetLexText())
			{
				LexText->MarkVertexPositionDirty();
			}
		}
	}
}

void ULexMeshModifierTextAnimation_PositionProperty::ApplyProperty(ULexText* InUIText, const FLexMeshModifierTextAnimation_SelectResult& InSelection, FLexUIGeometry* InGeometry)
{
	auto easeFunction = GetEaseFunction();
	auto& originVertices = InGeometry->OriginVertices;
	auto& charProperties = InUIText->GetCharPropertyArray();
	for (int charIndex = InSelection.StartCharIndex; charIndex < InSelection.EndCharCount; charIndex++)
	{
		auto charPropertyItem = charProperties[charIndex];
		int startVertIndex = charPropertyItem.StartVertIndex;
		int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;
		float lerpValue = FMath::Clamp(InSelection.LerpValueArray[charIndex - InSelection.StartCharIndex], 0.0f, 1.0f);
		lerpValue = easeFunction.Execute(1.0f, 0.0f, lerpValue, 1.0f);
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& pos = originVertices[vertIndex].Position;
			pos = FMath::Lerp(pos, pos + (FVector3f)Position, lerpValue);
		}
	}
}

void ULexMeshModifierTextAnimation_PositionRandomProperty::ApplyProperty(ULexText* InUIText, const FLexMeshModifierTextAnimation_SelectResult& InSelection, FLexUIGeometry* InGeometry)
{
	FMath::RandInit(Seed);
	auto easeFunction = GetEaseFunction();
	auto& originVertices = InGeometry->OriginVertices;
	auto& charProperties = InUIText->GetCharPropertyArray();
	for (int charIndex = InSelection.StartCharIndex; charIndex < InSelection.EndCharCount; charIndex++)
	{
		auto charPropertyItem = charProperties[charIndex];
		int startVertIndex = charPropertyItem.StartVertIndex;
		int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;
		float lerpValue = FMath::Clamp(InSelection.LerpValueArray[charIndex - InSelection.StartCharIndex], 0.0f, 1.0f);
		lerpValue = easeFunction.Execute(1.0f, 0.0f, lerpValue, 1.0f);
		auto position = FVector3f(FMath::FRandRange(Min.X, Max.X), FMath::FRandRange(Min.Y, Max.Y), FMath::FRandRange(Min.Z, Max.Z));
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& pos = originVertices[vertIndex].Position;
			pos = FMath::Lerp(pos, pos + position, lerpValue);
		}
	}
}

void ULexMeshModifierTextAnimation_RotationProperty::ApplyProperty(ULexText* InUIText, const FLexMeshModifierTextAnimation_SelectResult& InSelection, FLexUIGeometry* InGeometry)
{
	auto easeFunction = GetEaseFunction();
	auto& originVertices = InGeometry->OriginVertices;
	auto& charProperties = InUIText->GetCharPropertyArray();
	for (int charIndex = InSelection.StartCharIndex; charIndex < InSelection.EndCharCount; charIndex++)
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
		float lerpValue = FMath::Clamp(InSelection.LerpValueArray[charIndex - InSelection.StartCharIndex], 0.0f, 1.0f);
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

void ULexMeshModifierTextAnimation_RotationRandomProperty::ApplyProperty(ULexText* InUIText, const FLexMeshModifierTextAnimation_SelectResult& InSelection, FLexUIGeometry* InGeometry)
{
	FMath::RandInit(Seed);
	auto easeFunction = GetEaseFunction();
	auto& originVertices = InGeometry->OriginVertices;
	auto& charProperties = InUIText->GetCharPropertyArray();
	for (int charIndex = InSelection.StartCharIndex; charIndex < InSelection.EndCharCount; charIndex++)
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
		float lerpValue = FMath::Clamp(InSelection.LerpValueArray[charIndex - InSelection.StartCharIndex], 0.0f, 1.0f);
		lerpValue = easeFunction.Execute(1.0f, 0.0f, lerpValue, 1.0f);
		auto rotator = FRotator3f(FMath::FRandRange(Min.Pitch, Max.Pitch), FMath::FRandRange(Min.Yaw, Max.Yaw), FMath::FRandRange(Min.Roll, Max.Roll));
		auto calcRotationMatrix = FRotationMatrix44f(rotator * lerpValue);
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& pos = originVertices[vertIndex].Position;
			auto vector = pos - charCenterPos;
			pos = charCenterPos + calcRotationMatrix.TransformPosition(vector);
		}
	}
}

void ULexMeshModifierTextAnimation_ScaleProperty::ApplyProperty(ULexText* InUIText, const FLexMeshModifierTextAnimation_SelectResult& InSelection, FLexUIGeometry* InGeometry)
{
	auto easeFunction = GetEaseFunction();
	auto& originVertices = InGeometry->OriginVertices;
	auto& charProperties = InUIText->GetCharPropertyArray();
	for (int charIndex = InSelection.StartCharIndex; charIndex < InSelection.EndCharCount; charIndex++)
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
		float lerpValue = FMath::Clamp(InSelection.LerpValueArray[charIndex - InSelection.StartCharIndex], 0.0f, 1.0f);
		lerpValue = easeFunction.Execute(1.0f, 0.0f, lerpValue, 1.0f);
		auto calcScale = FMath::Lerp(FVector3f::OneVector, (FVector3f)Scale, lerpValue);
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& pos = originVertices[vertIndex].Position;
			auto vector = pos - charCenterPos;
			pos = charCenterPos + vector * calcScale;
		}
	}
}

void ULexMeshModifierTextAnimation_ScaleRandomProperty::ApplyProperty(ULexText* InUIText, const FLexMeshModifierTextAnimation_SelectResult& InSelection, FLexUIGeometry* InGeometry)
{
	FMath::RandInit(Seed);
	auto easeFunction = GetEaseFunction();
	auto& originVertices = InGeometry->OriginVertices;
	auto& charProperties = InUIText->GetCharPropertyArray();
	for (int charIndex = InSelection.StartCharIndex; charIndex < InSelection.EndCharCount; charIndex++)
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
		float lerpValue = FMath::Clamp(InSelection.LerpValueArray[charIndex - InSelection.StartCharIndex], 0.0f, 1.0f);
		lerpValue = easeFunction.Execute(1.0f, 0.0f, lerpValue, 1.0f);
		auto scale = FVector3f(FMath::FRandRange(Min.X, Max.X), FMath::FRandRange(Min.Y, Max.Y), FMath::FRandRange(Min.Z, Max.Z));
		auto calcScale = FMath::Lerp(FVector3f::OneVector, scale, lerpValue);
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& pos = originVertices[vertIndex].Position;
			auto vector = pos - charCenterPos;
			pos = charCenterPos + vector * calcScale;
		}
	}
}

void ULexMeshModifierTextAnimation_AlphaProperty::ApplyProperty(ULexText* InUIText, const FLexMeshModifierTextAnimation_SelectResult& InSelection, FLexUIGeometry* InGeometry)
{
	auto easeFunction = GetEaseFunction();
	auto& vertices = InGeometry->Vertices;
	auto& charProperties = InUIText->GetCharPropertyArray();
	for (int charIndex = InSelection.StartCharIndex; charIndex < InSelection.EndCharCount; charIndex++)
	{
		auto charPropertyItem = charProperties[charIndex];
		int startVertIndex = charPropertyItem.StartVertIndex;
		int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;
		float lerpValue = FMath::Clamp(InSelection.LerpValueArray[charIndex - InSelection.StartCharIndex], 0.0f, 1.0f);
		lerpValue = easeFunction.Execute(1.0f, 0.0f, lerpValue, 1.0f);
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& vert = vertices[vertIndex];
			vert.Color.A = FMath::Lerp(vert.Color.A, (uint8)(vert.Color.A * Alpha), lerpValue);
		}
	}
}

void ULexMeshModifierTextAnimation_ColorProperty::ApplyProperty(ULexText* InUIText, const FLexMeshModifierTextAnimation_SelectResult& InSelection, FLexUIGeometry* InGeometry)
{
	auto easeFunction = GetEaseFunction();
	auto& vertices = InGeometry->Vertices;
	auto& charProperties = InUIText->GetCharPropertyArray();
	FVector colorHsv;
	if (bUseHSV)
	{
		colorHsv = FLexUIUtils::ColorRGBToColorHSVData(Color);
	}
	for (int charIndex = InSelection.StartCharIndex; charIndex < InSelection.EndCharCount; charIndex++)
	{
		auto charPropertyItem = charProperties[charIndex];
		int startVertIndex = charPropertyItem.StartVertIndex;
		int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;
		float lerpValue = FMath::Clamp(InSelection.LerpValueArray[charIndex - InSelection.StartCharIndex], 0.0f, 1.0f);
		lerpValue = easeFunction.Execute(1.0f, 0.0f, lerpValue, 1.0f);
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& vert = vertices[vertIndex];
			if (bUseHSV)
			{
				auto vertColorHsv = FLexUIUtils::ColorRGBToColorHSVData(vert.Color);
				vertColorHsv = FMath::Lerp(vertColorHsv, colorHsv, lerpValue);
				auto vertColor = FLexUIUtils::ColorHSVDataToColorRGB(vertColorHsv);
				vert.Color.R = vertColor.R;
				vert.Color.G = vertColor.G;
				vert.Color.B = vertColor.B;
			}
			else
			{
				vert.Color.R = FMath::Lerp(vert.Color.R, Color.R, lerpValue);
				vert.Color.G = FMath::Lerp(vert.Color.G, Color.G, lerpValue);
				vert.Color.B = FMath::Lerp(vert.Color.B, Color.B, lerpValue);
			}
			vert.Color.A = FMath::Lerp(vert.Color.A, Color.A, lerpValue);
		}
	}
}
void ULexMeshModifierTextAnimation_ColorProperty::SetUseHSV(bool Value)
{
	if (bUseHSV != Value)
	{
		bUseHSV = Value;
		if (auto LexText = GetLexText())
		{
			LexText->MarkColorDirty();
		}
	}
}

void ULexMeshModifierTextAnimation_ColorRandomProperty::ApplyProperty(ULexText* InUIText, const FLexMeshModifierTextAnimation_SelectResult& InSelection, FLexUIGeometry* InGeometry)
{
	FMath::RandInit(Seed);
	auto easeFunction = GetEaseFunction();
	auto& vertices = InGeometry->Vertices;
	auto& charProperties = InUIText->GetCharPropertyArray();
	for (int charIndex = InSelection.StartCharIndex; charIndex < InSelection.EndCharCount; charIndex++)
	{
		auto charPropertyItem = charProperties[charIndex];
		int startVertIndex = charPropertyItem.StartVertIndex;
		int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;
		auto color = FColor((uint8)FMath::RandRange(Min.R, Max.R), (uint8)FMath::RandRange(Min.G, Max.G), (uint8)FMath::RandRange(Min.B, Max.B), (uint8)FMath::RandRange(Min.A, Max.A));
		float lerpValue = FMath::Clamp(InSelection.LerpValueArray[charIndex - InSelection.StartCharIndex], 0.0f, 1.0f);
		lerpValue = easeFunction.Execute(1.0f, 0.0f, lerpValue, 1.0f);
		FVector colorHsv;
		if (bUseHSV)
		{
			colorHsv = FLexUIUtils::ColorRGBToColorHSVData(color);
		}
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& vert = vertices[vertIndex];
			if (bUseHSV)
			{
				auto vertColorHsv = FLexUIUtils::ColorRGBToColorHSVData(vert.Color);
				vertColorHsv = FMath::Lerp(vertColorHsv, colorHsv, lerpValue);
				auto vertColor = FLexUIUtils::ColorHSVDataToColorRGB(vertColorHsv);
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
void ULexMeshModifierTextAnimation_ColorRandomProperty::SetUseHSV(bool Value)
{
	if (bUseHSV != Value)
	{
		bUseHSV = Value;
		if (auto LexText = GetLexText())
		{
			LexText->MarkColorDirty();
		}
	}
}

void ULexMeshModifierTextAnimation_PositionProperty::SetPosition(FVector Value)
{
	if (Position != Value)
	{
		Position = Value;
		MarkUITextPositionDirty();
	}
}
void ULexMeshModifierTextAnimation_PositionRandomProperty::SetSeed(int Value)
{
	if (Seed != Value)
	{
		Seed = Value;
		MarkUITextPositionDirty();
	}
}
void ULexMeshModifierTextAnimation_PositionRandomProperty::SetMin(FVector Value)
{
	if (Min != Value)
	{
		Min = Value;
		MarkUITextPositionDirty();
	}
}
void ULexMeshModifierTextAnimation_PositionRandomProperty::SetMax(FVector Value)
{
	if (Max != Value)
	{
		Max = Value;
		MarkUITextPositionDirty();
	}
}
void ULexMeshModifierTextAnimation_RotationProperty::SetRotator(FRotator value)
{
	if (rotator != value)
	{
		rotator = value;
		MarkUITextPositionDirty();
	}
}
void ULexMeshModifierTextAnimation_RotationRandomProperty::SetSeed(int Value)
{
	if (Seed != Value)
	{
		Seed = Value;
		MarkUITextPositionDirty();
	}
}
void ULexMeshModifierTextAnimation_RotationRandomProperty::SetMin(FRotator Value)
{
	if (Min != Value)
	{
		Min = Value;
		MarkUITextPositionDirty();
	}
}
void ULexMeshModifierTextAnimation_RotationRandomProperty::SetMax(FRotator Value)
{
	if (Max != Value)
	{
		Max = Value;
		MarkUITextPositionDirty();
	}
}
void ULexMeshModifierTextAnimation_ScaleProperty::SetScale(FVector Value)
{
	if (Scale != Value)
	{
		Scale = Value;
		MarkUITextPositionDirty();
	}
}
void ULexMeshModifierTextAnimation_ScaleRandomProperty::SetSeed(int Value)
{
	if (Seed != Value)
	{
		Seed = Value;
		MarkUITextPositionDirty();
	}
}
void ULexMeshModifierTextAnimation_ScaleRandomProperty::SetMin(FVector Value)
{
	if (Min != Value)
	{
		Min = Value;
		MarkUITextPositionDirty();
	}
}
void ULexMeshModifierTextAnimation_ScaleRandomProperty::SetMax(FVector Value)
{
	if (Max != Value)
	{
		Max = Value;
		MarkUITextPositionDirty();
	}
}
void ULexMeshModifierTextAnimation_AlphaProperty::SetAlpha(float Value)
{
	if (Alpha != Value)
	{
		Alpha = Value;
		MarkUITextPositionDirty();
	}
}
void ULexMeshModifierTextAnimation_ColorProperty::SetColor(FColor value)
{
	if (Color != value)
	{
		Color = value;
		MarkUITextPositionDirty();
	}
}
void ULexMeshModifierTextAnimation_ColorRandomProperty::SetSeed(int Value)
{
	if (Seed != Value)
	{
		Seed = Value;
		MarkUITextPositionDirty();
	}
}
void ULexMeshModifierTextAnimation_ColorRandomProperty::SetMin(FColor Value)
{
	if (Min != Value)
	{
		Min = Value;
		MarkUITextPositionDirty();
	}
}
void ULexMeshModifierTextAnimation_ColorRandomProperty::SetMax(FColor Value)
{
	if (Max != Value)
	{
		Max = Value;
		MarkUITextPositionDirty();
	}
}
