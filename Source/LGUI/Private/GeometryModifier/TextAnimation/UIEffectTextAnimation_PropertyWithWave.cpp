// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "GeometryModifier/TextAnimation/UIEffectTextAnimation_PropertyWithWave.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIText.h"

void UUIEffectTextAnimation_PropertyWithWave::SetFrequency(float value)
{
	if (frequency != value)
	{
		frequency = value;
		MarkUITextPositionDirty();
	}
}

void UUIEffectTextAnimation_PositionWaveProperty::ApplyProperty(UUIText* InUIText, const FUIEffectTextAnimation_SelectResult& InSelection, TSharedPtr<UIGeometry> OutGeometry)
{
	auto& vertPositions = OutGeometry->originPositions;
	auto& charProperties = InUIText->GetCharPropertyArray();
	float PIx2xFreq = PI * 2.0f * GetFrequency();
	for (int charIndex = InSelection.startCharIndex; charIndex < InSelection.endCharIndex; charIndex++)
	{
		auto charPropertyItem = charProperties[charIndex];
		int startVertIndex = charPropertyItem.StartVertIndex;
		int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;
		float lerpValue = InSelection.lerpValueArray[charIndex];
		lerpValue = FMath::Sin(lerpValue * PIx2xFreq);
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& pos = vertPositions[vertIndex];
			pos = FMath::Lerp(pos, pos + position, lerpValue);
		}
	}
}
void UUIEffectTextAnimation_PositionWaveProperty::SetPosition(FVector value)
{
	if (position != value)
	{
		position = value;
		MarkUITextPositionDirty();
	}
}

void UUIEffectTextAnimation_RotationWaveProperty::ApplyProperty(UUIText* InUIText, const FUIEffectTextAnimation_SelectResult& InSelection, TSharedPtr<UIGeometry> OutGeometry)
{
	auto& vertPositions = OutGeometry->originPositions;
	auto& charProperties = InUIText->GetCharPropertyArray();
	float PIx2xFreq = PI * 2.0f * GetFrequency();
	for (int charIndex = InSelection.startCharIndex; charIndex < InSelection.endCharIndex; charIndex++)
	{
		auto charPropertyItem = charProperties[charIndex];
		int startVertIndex = charPropertyItem.StartVertIndex;
		int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;
		FVector charCenterPos = vertPositions[startVertIndex];
		for (int vertIndex = startVertIndex + 1; vertIndex < endVertIndex; vertIndex++)
		{
			charCenterPos += vertPositions[vertIndex];
		}
		charCenterPos /= charPropertyItem.VertCount;
		float lerpValue = InSelection.lerpValueArray[charIndex];
		lerpValue = FMath::Sin(lerpValue * PIx2xFreq);
		auto calcRotationMatrix = FRotationMatrix(rotator * lerpValue);
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& pos = vertPositions[vertIndex];
			auto vector = pos - charCenterPos;
			pos = charCenterPos + calcRotationMatrix.TransformPosition(vector);
		}
	}
}
void UUIEffectTextAnimation_RotationWaveProperty::SetRotator(FRotator value)
{
	if (rotator != value)
	{
		rotator = value;
		MarkUITextPositionDirty();
	}
}

void UUIEffectTextAnimation_ScaleWaveProperty::ApplyProperty(UUIText* InUIText, const FUIEffectTextAnimation_SelectResult& InSelection, TSharedPtr<UIGeometry> OutGeometry)
{
	auto& vertPositions = OutGeometry->originPositions;
	auto& charProperties = InUIText->GetCharPropertyArray();
	float PIx2xFreq = PI * 2.0f * GetFrequency();
	for (int charIndex = InSelection.startCharIndex; charIndex < InSelection.endCharIndex; charIndex++)
	{
		auto charPropertyItem = charProperties[charIndex];
		int startVertIndex = charPropertyItem.StartVertIndex;
		int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;
		FVector charCenterPos = vertPositions[startVertIndex];
		for (int vertIndex = startVertIndex + 1; vertIndex < endVertIndex; vertIndex++)
		{
			charCenterPos += vertPositions[vertIndex];
		}
		charCenterPos /= charPropertyItem.VertCount;
		float lerpValue = InSelection.lerpValueArray[charIndex];
		lerpValue = FMath::Sin(lerpValue * PIx2xFreq);
		auto calcScale = FMath::Lerp(FVector::OneVector, scale, lerpValue);
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& pos = vertPositions[vertIndex];
			auto vector = pos - charCenterPos;
			pos = charCenterPos + vector * calcScale;
		}
	}
}
void UUIEffectTextAnimation_ScaleWaveProperty::SetScale(FVector value)
{
	if (scale != value)
	{
		scale = value;
		MarkUITextPositionDirty();
	}
}
