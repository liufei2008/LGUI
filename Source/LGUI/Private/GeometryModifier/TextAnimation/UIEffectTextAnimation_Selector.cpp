// Copyright 2019-2022 LexLiu. All Rights Reserved.

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
	OutSelection.startCharIndex = charProperties.Num() * start;
	OutSelection.endCharCount = charProperties.Num() * end;
	int count = OutSelection.endCharCount - OutSelection.startCharIndex;
	auto& lerpValueArray = OutSelection.lerpValueArray;
	lerpValueArray.Reset(count);
	lerpValueArray.AddDefaulted(count);
	float rangeInv = 1.0f / range;
	for (int startIndex = OutSelection.startCharIndex, endIndex = OutSelection.endCharCount; startIndex < endIndex; startIndex++)
	{
		float lerpValue = value * rangeInv;
		//lerpValue = FMath::Clamp(value, 0.0f, 1.0f);
		int lerpValueIndex = startIndex - OutSelection.startCharIndex;
		lerpValueArray[flipDirection ? endIndex - startIndex - 1 : lerpValueIndex] = 1.0f - lerpValue;
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
void UUIEffectTextAnimation_RangeSelector::SetStart(float value)
{
	if (start != value)
	{
		start = value;
		if (auto uiText = GetUIText())
		{
			uiText->MarkVertexPositionDirty();
		}
	}
}
void UUIEffectTextAnimation_RangeSelector::SetEnd(float value)
{
	if (end != value)
	{
		end = value;
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
	OutSelection.startCharIndex = charProperties.Num() * start;
	OutSelection.endCharCount = charProperties.Num() * end;
	int count = OutSelection.endCharCount - OutSelection.startCharIndex;
	auto& lerpValueArray = OutSelection.lerpValueArray;
	lerpValueArray.Reset(count);
	lerpValueArray.AddDefaulted(count);
	for (int startIndex = OutSelection.startCharIndex, endIndex = OutSelection.endCharCount; startIndex < endIndex; startIndex++)
	{
		float lerpValue = FMath::FRand() + calculatedOffset;
		//lerpValue = FMath::Clamp(lerpValue, 0.0f, 1.0f);
		int lerpValueIndex = startIndex - OutSelection.startCharIndex;
		lerpValueArray[lerpValueIndex] = lerpValue;
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
void UUIEffectTextAnimation_RandomSelector::SetStart(float value)
{
	if (start != value)
	{
		start = value;
		if (auto uiText = GetUIText())
		{
			uiText->MarkVertexPositionDirty();
		}
	}
}
void UUIEffectTextAnimation_RandomSelector::SetEnd(float value)
{
	if (end != value)
	{
		end = value;
		if (auto uiText = GetUIText())
		{
			uiText->MarkVertexPositionDirty();
		}
	}
}

bool UUIEffectTextAnimation_RichTextTagSelector::Select(class UUIText* InUIText, FUIEffectTextAnimation_SelectResult& OutSelection)
{
	if (FMath::Abs(range) < KINDA_SMALL_NUMBER)return false;
	auto& charProperties = InUIText->GetCharPropertyArray();
	auto& richTextCustomTagArray = InUIText->GetRichTextCustomTagArray();
	int foundIndex = richTextCustomTagArray.IndexOfByPredicate([=](const FUIText_RichTextCustomTag& A) {
		return A.TagName == tagName;
		});
	if (foundIndex == -1)return false;
	auto customTag = richTextCustomTagArray[foundIndex];

	float calculatedOffset = offset * (1.0f + range) - range;
	float value = -calculatedOffset;
	OutSelection.startCharIndex = customTag.CharIndexStart;
	OutSelection.endCharCount = customTag.CharIndexEnd + 1;
	int count = OutSelection.endCharCount - OutSelection.startCharIndex;
	auto& lerpValueArray = OutSelection.lerpValueArray;
	lerpValueArray.Reset(count);
	lerpValueArray.AddDefaulted(count);
	float interval = 1.0f / (count - 1);
	float rangeInv = 1.0f / range;
	for (int startIndex = OutSelection.startCharIndex, endIndex = OutSelection.endCharCount; startIndex < endIndex; startIndex++)
	{
		float lerpValue = value * rangeInv;
		//lerpValue = FMath::Clamp(lerpValue, 0.0f, 1.0f);
		int lerpValueIndex = startIndex - OutSelection.startCharIndex;
		lerpValueArray[flipDirection ? endIndex - startIndex - 1 : lerpValueIndex] = 1.0f - lerpValue;
		value += interval;
	}
	return true;
}
void UUIEffectTextAnimation_RichTextTagSelector::SetTagName(const FName& value)
{
	if (tagName != value)
	{
		tagName = value;
		if (auto uiText = GetUIText())
		{
			uiText->MarkVertexPositionDirty();
		}
	}
}
void UUIEffectTextAnimation_RichTextTagSelector::SetRange(float value)
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
void UUIEffectTextAnimation_RichTextTagSelector::SetFlipDirection(bool value)
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
