// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LGUI/Public/MeshModifier/TextAnimation/LexMeshModifierTextAnimation_PropertyWithWave.h"
#include "LGUI.h"
#include "Core/Components/LexText.h"
#include "LTweenBPLibrary.h"
#include "Engine/World.h"

void ULexMeshModifierTextAnimation_PropertyWithWave::Init()
{
	TextObject = GetLexText();
	UpdateTweener = ULTweenBPLibrary::UpdateCall(this, FLTweenUpdateDelegate::CreateUObject(this, &ULexMeshModifierTextAnimation_PropertyWithWave::OnUpdate));
}
void ULexMeshModifierTextAnimation_PropertyWithWave::Deinit()
{
	ULTweenBPLibrary::KillIfIsTweening(this, UpdateTweener.Get());
}
void ULexMeshModifierTextAnimation_PropertyWithWave::SetFrequency(float Value)
{
	if (Speed != Value)
	{
		Speed = Value;
		MarkUITextPositionDirty();
	}
}
void ULexMeshModifierTextAnimation_PropertyWithWave::OnUpdate(float deltaTime)
{
	if (IsValid(TextObject))
	{
		TextObject->MarkVertexPositionDirty();
	}
}

void ULexMeshModifierTextAnimation_PositionWaveProperty::ApplyProperty(ULexText* InUIText, const FLexMeshModifierTextAnimation_SelectResult& InSelection, FLexUIGeometry* InGeometry)
{
	auto& originVertices = InGeometry->OriginVertices;
	auto& charProperties = InUIText->GetCharPropertyArray();
	float PIxFreq = this->GetWorld()->TimeSeconds * PI * Speed;
	PIxFreq = FlipDirection ? -PIxFreq : PIxFreq;
	for (int charIndex = InSelection.StartCharIndex; charIndex < InSelection.EndCharCount; charIndex++)
	{
		auto charPropertyItem = charProperties[charIndex];
		int startVertIndex = charPropertyItem.StartVertIndex;
		int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;
		float lerpValue = FMath::Clamp(InSelection.LerpValueArray[charIndex - InSelection.StartCharIndex], 0.0f, 1.0f);
		auto wavePosition = (FVector3f)Position * FMath::Sin(PIxFreq + charIndex * Frequency);
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& pos = originVertices[vertIndex].Position;
			pos = FMath::Lerp(pos, pos + wavePosition, lerpValue);
		}
	}
}
void ULexMeshModifierTextAnimation_PositionWaveProperty::SetPosition(FVector Value)
{
	if (Position != Value)
	{
		Position = Value;
		MarkUITextPositionDirty();
	}
}

void ULexMeshModifierTextAnimation_RotationWaveProperty::ApplyProperty(ULexText* InUIText, const FLexMeshModifierTextAnimation_SelectResult& InSelection, FLexUIGeometry* InGeometry)
{
	auto& originVertices = InGeometry->OriginVertices;
	auto& charProperties = InUIText->GetCharPropertyArray();
	float PIxFreq = this->GetWorld()->TimeSeconds * PI * Speed;
	PIxFreq = FlipDirection ? -PIxFreq : PIxFreq;
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
		auto waveRotator = (FRotator3f)Rotator * FMath::Sin(PIxFreq + charIndex * Frequency);
		auto calcRotationMatrix = FRotationMatrix44f(waveRotator * lerpValue);
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& pos = originVertices[vertIndex].Position;
			auto vector = pos - charCenterPos;
			pos = charCenterPos + calcRotationMatrix.TransformPosition(vector);
		}
	}
}
void ULexMeshModifierTextAnimation_RotationWaveProperty::SetRotator(FRotator Value)
{
	if (Rotator != Value)
	{
		Rotator = Value;
		MarkUITextPositionDirty();
	}
}

void ULexMeshModifierTextAnimation_ScaleWaveProperty::ApplyProperty(ULexText* InUIText, const FLexMeshModifierTextAnimation_SelectResult& InSelection, FLexUIGeometry* InGeometry)
{
	auto& originVertices = InGeometry->OriginVertices;
	auto& charProperties = InUIText->GetCharPropertyArray();
	float PIxFreq = this->GetWorld()->TimeSeconds * PI * Speed;
	PIxFreq = FlipDirection ? -PIxFreq : PIxFreq;
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
		auto waveScale = FVector3f::OneVector + ((FVector3f)Scale - FVector3f::OneVector) * FMath::Sin(PIxFreq + charIndex * Frequency);
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& pos = originVertices[vertIndex].Position;
			auto vector = pos - charCenterPos;
			pos = charCenterPos + vector * waveScale;
		}
	}
}
void ULexMeshModifierTextAnimation_ScaleWaveProperty::SetScale(FVector Value)
{
	if (Scale != Value)
	{
		Scale = Value;
		MarkUITextPositionDirty();
	}
}
