// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "GeometryModifier/TextAnimation/UIEffectTextAnimation_Property.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIText.h"

void UUIEffectTextAnimation_PositionProperty::ApplyProperty(UUIText* InUIText, const TArray<FUIEffectTextAnimation_SelectResult>& InSelection, TSharedPtr<UIGeometry> OutGeometry)
{
	auto easeFunction = GetEaseFunction();
	auto& vertPositions = OutGeometry->originPositions;
	auto& charProperties = InUIText->GetCharPropertyArray();
	for (int charIndex = 0; charIndex < charProperties.Num(); charIndex++)
	{
		auto charPropertyItem = charProperties[charIndex];
		int startVertIndex = charPropertyItem.StartVertIndex;
		int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;
		auto lerpValue = easeFunction.Execute(1.0f, 0.0f, InSelection[charIndex].lerpValue, 1.0f);
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& pos = vertPositions[vertIndex];
			pos = FMath::Lerp(pos, pos + position, lerpValue);
		}
	}
}

void UUIEffectTextAnimation_PositionRandomProperty::ApplyProperty(UUIText* InUIText, const TArray<FUIEffectTextAnimation_SelectResult>& InSelection, TSharedPtr<UIGeometry> OutGeometry)
{
	FMath::RandInit(seed);
	auto easeFunction = GetEaseFunction();
	auto& vertPositions = OutGeometry->originPositions;
	auto& charProperties = InUIText->GetCharPropertyArray();
	for (int charIndex = 0; charIndex < charProperties.Num(); charIndex++)
	{
		auto charPropertyItem = charProperties[charIndex];
		int startVertIndex = charPropertyItem.StartVertIndex;
		int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;
		auto lerpValue = easeFunction.Execute(1.0f, 0.0f, InSelection[charIndex].lerpValue, 1.0f);
		auto position = FVector(FMath::FRandRange(min.X, max.X), FMath::FRandRange(min.Y, max.Y), FMath::FRandRange(min.Z, max.Z));
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& pos = vertPositions[vertIndex];
			pos = FMath::Lerp(pos, pos + position, lerpValue);
		}
	}
}

void UUIEffectTextAnimation_RotationProperty::ApplyProperty(UUIText* InUIText, const TArray<FUIEffectTextAnimation_SelectResult>& InSelection, TSharedPtr<UIGeometry> OutGeometry)
{
	auto easeFunction = GetEaseFunction();
	auto& vertPositions = OutGeometry->originPositions;
	auto& charProperties = InUIText->GetCharPropertyArray();
	for (int charIndex = 0; charIndex < charProperties.Num(); charIndex++)
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
		auto lerpValue = easeFunction.Execute(1.0f, 0.0f, InSelection[charIndex].lerpValue, 1.0f);
		auto calcRotationMatrix = FRotationMatrix(rotator * lerpValue);
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& pos = vertPositions[vertIndex];
			auto vector = pos - charCenterPos;
			pos = charCenterPos + calcRotationMatrix.TransformPosition(vector);
		}
	}
}

void UUIEffectTextAnimation_RotationRandomProperty::ApplyProperty(UUIText* InUIText, const TArray<FUIEffectTextAnimation_SelectResult>& InSelection, TSharedPtr<UIGeometry> OutGeometry)
{
	FMath::RandInit(seed);
	auto easeFunction = GetEaseFunction();
	auto& vertPositions = OutGeometry->originPositions;
	auto& charProperties = InUIText->GetCharPropertyArray();
	for (int charIndex = 0; charIndex < charProperties.Num(); charIndex++)
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
		auto lerpValue = easeFunction.Execute(1.0f, 0.0f, InSelection[charIndex].lerpValue, 1.0f);
		auto rotator = FRotator(FMath::FRandRange(min.Pitch, max.Pitch), FMath::FRandRange(min.Yaw, max.Yaw), FMath::FRandRange(min.Roll, max.Roll));
		auto calcRotationMatrix = FRotationMatrix(rotator * lerpValue);
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& pos = vertPositions[vertIndex];
			auto vector = pos - charCenterPos;
			pos = charCenterPos + calcRotationMatrix.TransformPosition(vector);
		}
	}
}

void UUIEffectTextAnimation_ScaleProperty::ApplyProperty(UUIText* InUIText, const TArray<FUIEffectTextAnimation_SelectResult>& InSelection, TSharedPtr<UIGeometry> OutGeometry)
{
	auto easeFunction = GetEaseFunction();
	auto& vertPositions = OutGeometry->originPositions;
	auto& charProperties = InUIText->GetCharPropertyArray();
	for (int charIndex = 0; charIndex < charProperties.Num(); charIndex++)
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
		auto lerpValue = easeFunction.Execute(1.0f, 0.0f, InSelection[charIndex].lerpValue, 1.0f);
		auto calcScale = FMath::Lerp(FVector::OneVector, scale, lerpValue);
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& pos = vertPositions[vertIndex];
			auto vector = pos - charCenterPos;
			pos = charCenterPos + vector * calcScale;
		}
	}
}

void UUIEffectTextAnimation_ScaleRandomProperty::ApplyProperty(UUIText* InUIText, const TArray<FUIEffectTextAnimation_SelectResult>& InSelection, TSharedPtr<UIGeometry> OutGeometry)
{
	FMath::RandInit(seed);
	auto easeFunction = GetEaseFunction();
	auto& vertPositions = OutGeometry->originPositions;
	auto& charProperties = InUIText->GetCharPropertyArray();
	for (int charIndex = 0; charIndex < charProperties.Num(); charIndex++)
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
		auto lerpValue = easeFunction.Execute(1.0f, 0.0f, InSelection[charIndex].lerpValue, 1.0f);
		auto scale = FVector(FMath::FRandRange(min.X, max.X), FMath::FRandRange(min.Y, max.Y), FMath::FRandRange(min.Z, max.Z));
		auto calcScale = FMath::Lerp(FVector::OneVector, scale, lerpValue);
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& pos = vertPositions[vertIndex];
			auto vector = pos - charCenterPos;
			pos = charCenterPos + vector * calcScale;
		}
	}
}

void UUIEffectTextAnimation_AlphaProperty::ApplyProperty(UUIText* InUIText, const TArray<FUIEffectTextAnimation_SelectResult>& InSelection, TSharedPtr<UIGeometry> OutGeometry)
{
	auto easeFunction = GetEaseFunction();
	auto& vertices = OutGeometry->vertices;
	auto& charProperties = InUIText->GetCharPropertyArray();
	for (int charIndex = 0; charIndex < charProperties.Num(); charIndex++)
	{
		auto charPropertyItem = charProperties[charIndex];
		int startVertIndex = charPropertyItem.StartVertIndex;
		int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;
		auto lerpValue = easeFunction.Execute(1.0f, 0.0f, InSelection[charIndex].lerpValue, 1.0f);
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& vert = vertices[vertIndex];
			vert.Color.A = FMath::Lerp(vert.Color.A, (uint8)(vert.Color.A * alpha), lerpValue);
		}
	}
}

void UUIEffectTextAnimation_ColorProperty::ApplyProperty(UUIText* InUIText, const TArray<FUIEffectTextAnimation_SelectResult>& InSelection, TSharedPtr<UIGeometry> OutGeometry)
{
	auto easeFunction = GetEaseFunction();
	auto& vertices = OutGeometry->vertices;
	auto& charProperties = InUIText->GetCharPropertyArray();
	for (int charIndex = 0; charIndex < charProperties.Num(); charIndex++)
	{
		auto charPropertyItem = charProperties[charIndex];
		int startVertIndex = charPropertyItem.StartVertIndex;
		int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;
		auto lerpValue = easeFunction.Execute(1.0f, 0.0f, InSelection[charIndex].lerpValue, 1.0f);
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& vert = vertices[vertIndex];
			vert.Color.R = FMath::Lerp(vert.Color.R, color.R, lerpValue);
			vert.Color.G = FMath::Lerp(vert.Color.G, color.G, lerpValue);
			vert.Color.B = FMath::Lerp(vert.Color.B, color.B, lerpValue);
			vert.Color.A = FMath::Lerp(vert.Color.A, color.A, lerpValue);
		}
	}
}

void UUIEffectTextAnimation_ColorRandomProperty::ApplyProperty(UUIText* InUIText, const TArray<FUIEffectTextAnimation_SelectResult>& InSelection, TSharedPtr<UIGeometry> OutGeometry)
{
	FMath::RandInit(seed);
	auto easeFunction = GetEaseFunction();
	auto& vertices = OutGeometry->vertices;
	auto& charProperties = InUIText->GetCharPropertyArray();
	for (int charIndex = 0; charIndex < charProperties.Num(); charIndex++)
	{
		auto charPropertyItem = charProperties[charIndex];
		int startVertIndex = charPropertyItem.StartVertIndex;
		int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;
		auto color = FColor(FMath::FRandRange(min.R, max.R), FMath::FRandRange(min.G, max.G), FMath::FRandRange(min.B, max.B), FMath::FRandRange(min.A, max.A));
		auto lerpValue = easeFunction.Execute(1.0f, 0.0f, InSelection[charIndex].lerpValue, 1.0f);
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& vert = vertices[vertIndex];
			vert.Color.R = FMath::Lerp(vert.Color.R, color.R, lerpValue);
			vert.Color.G = FMath::Lerp(vert.Color.G, color.G, lerpValue);
			vert.Color.B = FMath::Lerp(vert.Color.B, color.B, lerpValue);
			vert.Color.A = FMath::Lerp(vert.Color.A, color.A, lerpValue);
		}
	}
}

void UUIEffectTextAnimation_PositionProperty::SetPosition(FVector value)
{
	if (position != value)
	{
		position = value;
		if (auto uiText = GetUIText())
		{
			uiText->MarkVertexPositionDirty();
		}
	}
}
void UUIEffectTextAnimation_PositionRandomProperty::SetSeed(int value)
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
void UUIEffectTextAnimation_PositionRandomProperty::SetMin(FVector value)
{
	if (min != value)
	{
		min = value;
		if (auto uiText = GetUIText())
		{
			uiText->MarkVertexPositionDirty();
		}
	}
}
void UUIEffectTextAnimation_PositionRandomProperty::SetMax(FVector value)
{
	if (max != value)
	{
		max = value;
		if (auto uiText = GetUIText())
		{
			uiText->MarkVertexPositionDirty();
		}
	}
}
void UUIEffectTextAnimation_RotationProperty::SetRotator(FRotator value)
{
	if (rotator != value)
	{
		rotator = value;
		if (auto uiText = GetUIText())
		{
			uiText->MarkVertexPositionDirty();
		}
	}
}
void UUIEffectTextAnimation_RotationRandomProperty::SetSeed(int value)
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
void UUIEffectTextAnimation_RotationRandomProperty::SetMin(FRotator value)
{
	if (min != value)
	{
		min = value;
		if (auto uiText = GetUIText())
		{
			uiText->MarkVertexPositionDirty();
		}
	}
}
void UUIEffectTextAnimation_RotationRandomProperty::SetMax(FRotator value)
{
	if (max != value)
	{
		max = value;
		if (auto uiText = GetUIText())
		{
			uiText->MarkVertexPositionDirty();
		}
	}
}
void UUIEffectTextAnimation_ScaleProperty::SetScale(FVector value)
{
	if (scale != value)
	{
		scale = value;
		if (auto uiText = GetUIText())
		{
			uiText->MarkVertexPositionDirty();
		}
	}
}
void UUIEffectTextAnimation_ScaleRandomProperty::SetSeed(int value)
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
void UUIEffectTextAnimation_ScaleRandomProperty::SetMin(FVector value)
{
	if (min != value)
	{
		min = value;
		if (auto uiText = GetUIText())
		{
			uiText->MarkVertexPositionDirty();
		}
	}
}
void UUIEffectTextAnimation_ScaleRandomProperty::SetMax(FVector value)
{
	if (max != value)
	{
		max = value;
		if (auto uiText = GetUIText())
		{
			uiText->MarkVertexPositionDirty();
		}
	}
}
void UUIEffectTextAnimation_AlphaProperty::SetAlpha(float value)
{
	if (alpha != value)
	{
		alpha = value;
		if (auto uiText = GetUIText())
		{
			uiText->MarkVertexPositionDirty();
		}
	}
}
void UUIEffectTextAnimation_ColorProperty::SetColor(FColor value)
{
	if (color != value)
	{
		color = value;
		if (auto uiText = GetUIText())
		{
			uiText->MarkVertexPositionDirty();
		}
	}
}
void UUIEffectTextAnimation_ColorRandomProperty::SetSeed(int value)
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
void UUIEffectTextAnimation_ColorRandomProperty::SetMin(FColor value)
{
	if (min != value)
	{
		min = value;
		if (auto uiText = GetUIText())
		{
			uiText->MarkVertexPositionDirty();
		}
	}
}
void UUIEffectTextAnimation_ColorRandomProperty::SetMax(FColor value)
{
	if (max != value)
	{
		max = value;
		if (auto uiText = GetUIText())
		{
			uiText->MarkVertexPositionDirty();
		}
	}
}
