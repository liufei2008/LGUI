﻿// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "GeometryModifier/TextAnimation/UIEffectTextAnimation_PropertyWithWave.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIText.h"
#include "LTweenBPLibrary.h"
#include "Engine/World.h"

void UUIEffectTextAnimation_PropertyWithWave::Init()
{
	uiText = GetUIText();
	updateTweener = ULTweenBPLibrary::UpdateCall(this, FLTweenUpdateDelegate::CreateUObject(this, &UUIEffectTextAnimation_PropertyWithWave::OnUpdate));
}
void UUIEffectTextAnimation_PropertyWithWave::Deinit()
{
	ULTweenBPLibrary::KillIfIsTweening(this, updateTweener.Get());
}
void UUIEffectTextAnimation_PropertyWithWave::SetFrequency(float value)
{
	if (speed != value)
	{
		speed = value;
		MarkUITextPositionDirty();
	}
}
void UUIEffectTextAnimation_PropertyWithWave::OnUpdate(float deltaTime)
{
	if (IsValid(uiText))
	{
		uiText->MarkVertexPositionDirty();
	}
}

void UUIEffectTextAnimation_PositionWaveProperty::ApplyProperty(UUIText* InUIText, const FUIEffectTextAnimation_SelectResult& InSelection, UIGeometry* InGeometry)
{
	auto& originVertices = InGeometry->originVertices;
	auto& charProperties = InUIText->GetCharPropertyArray();
	float PIxFreq = this->GetWorld()->TimeSeconds * PI * speed;
	PIxFreq = flipDirection ? -PIxFreq : PIxFreq;
	for (int charIndex = InSelection.startCharIndex; charIndex < InSelection.endCharCount; charIndex++)
	{
		auto charPropertyItem = charProperties[charIndex];
		int startVertIndex = charPropertyItem.StartVertIndex;
		int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;
		float lerpValue = FMath::Clamp(InSelection.lerpValueArray[charIndex - InSelection.startCharIndex], 0.0f, 1.0f);
		auto wavePosition = (FVector3f)position * FMath::Sin(PIxFreq + charIndex * frequency);
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& pos = originVertices[vertIndex].Position;
			pos = FMath::Lerp(pos, pos + wavePosition, lerpValue);
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

void UUIEffectTextAnimation_RotationWaveProperty::ApplyProperty(UUIText* InUIText, const FUIEffectTextAnimation_SelectResult& InSelection, UIGeometry* InGeometry)
{
	auto& originVertices = InGeometry->originVertices;
	auto& charProperties = InUIText->GetCharPropertyArray();
	float PIxFreq = this->GetWorld()->TimeSeconds * PI * speed;
	PIxFreq = flipDirection ? -PIxFreq : PIxFreq;
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
		auto waveRotator = (FRotator3f)rotator * FMath::Sin(PIxFreq + charIndex * frequency);
		auto calcRotationMatrix = FRotationMatrix44f(waveRotator * lerpValue);
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& pos = originVertices[vertIndex].Position;
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

void UUIEffectTextAnimation_ScaleWaveProperty::ApplyProperty(UUIText* InUIText, const FUIEffectTextAnimation_SelectResult& InSelection, UIGeometry* InGeometry)
{
	auto& originVertices = InGeometry->originVertices;
	auto& charProperties = InUIText->GetCharPropertyArray();
	float PIxFreq = this->GetWorld()->TimeSeconds * PI * speed;
	PIxFreq = flipDirection ? -PIxFreq : PIxFreq;
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
		auto waveScale = FVector3f::OneVector + ((FVector3f)scale - FVector3f::OneVector) * FMath::Sin(PIxFreq + charIndex * frequency);
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& pos = originVertices[vertIndex].Position;
			auto vector = pos - charCenterPos;
			pos = charCenterPos + vector * waveScale;
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
