// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "GeometryModifier/TextAnimation/UIEffectTextAnimation_Selector.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIText.h"

bool UUIEffectTextAnimation_RangeSelector::Select(UUIText* InUIText, FUIEffectTextAnimation_SelectResult& OutSelection)
{
	if (FMath::Abs(range) < KINDA_SMALL_NUMBER)return false;
	if (end <= start)return false;
	auto& charProperties = InUIText->GetCharPropertyArray();
	float interval = 1.0f / (charProperties.Num() * (end - start));
	float calculatedOffset = offset * (1.0f + range) - range;
	float value = -calculatedOffset;
	auto& lerpValueArray = OutSelection.lerpValueArray;
	lerpValueArray.Reset(charProperties.Num());
	lerpValueArray.AddDefaulted(charProperties.Num());
	OutSelection.startCharIndex = charProperties.Num() * start;
	OutSelection.endCharIndex = charProperties.Num() * end;
	for (int i = OutSelection.startCharIndex, count = OutSelection.endCharIndex; i < count; i++)
	{
		auto lerpValue = FMath::Clamp(value, 0.0f, range);
		lerpValue /= range;
		lerpValueArray[flipDirection ? count - i - 1 : i] = 1.0f - lerpValue;
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

bool UUIEffectTextAnimation_RandomSelector::Select(UUIText* InUIText, FUIEffectTextAnimation_SelectResult& OutSelection)
{
	if (end <= start)return false;
	FMath::RandInit(seed);
	auto& charProperties = InUIText->GetCharPropertyArray();
	float calculatedOffset = offset * 2.0f - 1.0f;
	auto& lerpValueArray = OutSelection.lerpValueArray;
	lerpValueArray.Reset(charProperties.Num());
	lerpValueArray.AddDefaulted(charProperties.Num());
	OutSelection.startCharIndex = charProperties.Num() * start;
	OutSelection.endCharIndex = charProperties.Num() * end;
	for (int i = OutSelection.startCharIndex, count = OutSelection.endCharIndex; i < count; i++)
	{
		float lerpValue = FMath::FRand() + calculatedOffset;
		lerpValue = FMath::Clamp(lerpValue, 0.0f, 1.0f);
		lerpValueArray[i] = lerpValue;
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
