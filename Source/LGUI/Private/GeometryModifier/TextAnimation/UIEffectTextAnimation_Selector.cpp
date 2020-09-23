// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "GeometryModifier/TextAnimation/UIEffectTextAnimation_Selector.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIText.h"

bool UUIEffectTextAnimation_RangeSelector::Select(UUIText* InUIText, TArray<FUIEffectTextAnimation_SelectResult>& OutSelection)
{
	if (FMath::Abs(range) < KINDA_SMALL_NUMBER)return false;
	auto& charProperties = InUIText->GetCharPropertyArray();
	float interval = 1.0f / charProperties.Num();
	float calculatedOffset = offset * (1.0f + range) - range;
	float value = -calculatedOffset;
	OutSelection.Reset(charProperties.Num());
	OutSelection.AddUninitialized(charProperties.Num());
	for (int i = 0; i < charProperties.Num(); i++)
	{
		auto lerpValue = FMath::Clamp(value, 0.0f, range);
		lerpValue /= range;
		auto& selectItem = OutSelection[flipDirection ? charProperties.Num() - i - 1 : i];
		selectItem.lerpValue = 1.0f - lerpValue;
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

bool UUIEffectTextAnimation_RandomSelector::Select(UUIText* InUIText, TArray<FUIEffectTextAnimation_SelectResult>& OutSelection)
{
	FMath::RandInit(seed);
	auto& charProperties = InUIText->GetCharPropertyArray();
	float calculatedOffset = offset * 2.0f - 1.0f;
	OutSelection.Reset(charProperties.Num());
	for (int i = 0; i < charProperties.Num(); i++)
	{
		float lerpValue = FMath::FRand() + calculatedOffset;
		lerpValue = FMath::Clamp(lerpValue, 0.0f, 1.0f);
		auto selectItem = FUIEffectTextAnimation_SelectResult();
		selectItem.lerpValue = lerpValue;
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
void UUIEffectTextAnimation_RandomSelector::SetOffset(float value)
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
