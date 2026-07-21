// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LGUI/Public/MeshModifier/TextAnimation/LexMeshModifierTextAnimation_Selector.h"
#include "LGUI.h"
#include "Core/Components/LexText.h"

bool ULexMeshModifierTextAnimation_RangeSelector::Select(ULexText* InUIText, FLexMeshModifierTextAnimation_SelectResult& OutSelection)
{
	if (FMath::Abs(Range) < KINDA_SMALL_NUMBER)return false;
	if (End <= Start)return false;
	auto& charProperties = InUIText->GetCharPropertyArray();
	float interval = 1.0f / (charProperties.Num() * (End - Start));
	float calculatedOffset = Offset * (1.0f + Range) - Range;
	float value = -calculatedOffset;
	OutSelection.StartCharIndex = charProperties.Num() * Start;
	OutSelection.EndCharCount = charProperties.Num() * End;
	int count = OutSelection.EndCharCount - OutSelection.StartCharIndex;
	auto& lerpValueArray = OutSelection.LerpValueArray;
	lerpValueArray.Reset(count);
	lerpValueArray.AddDefaulted(count);
	float rangeInv = 1.0f / Range;
	for (int startIndex = OutSelection.StartCharIndex, endIndex = OutSelection.EndCharCount; startIndex < endIndex; startIndex++)
	{
		float lerpValue = value * rangeInv;
		//lerpValue = FMath::Clamp(value, 0.0f, 1.0f);
		int lerpValueIndex = startIndex - OutSelection.StartCharIndex;
		lerpValueArray[bFlipDirection ? endIndex - startIndex - 1 : lerpValueIndex] = 1.0f - lerpValue;
		value += interval;
	}
	return true;
}
void ULexMeshModifierTextAnimation_RangeSelector::SetRange(float Value)
{
	if (Range != Value)
	{
		Range = Value;
		if (auto uiText = GetLexText())
		{
			uiText->MarkVertexPositionDirty();
		}
	}
}
void ULexMeshModifierTextAnimation_RangeSelector::SetFlipDirection(bool Value)
{
	if (bFlipDirection != Value)
	{
		bFlipDirection = Value;
		if (auto uiText = GetLexText())
		{
			uiText->MarkVertexPositionDirty();
		}
	}
}
void ULexMeshModifierTextAnimation_RangeSelector::SetStart(float Value)
{
	if (Start != Value)
	{
		Start = Value;
		if (auto uiText = GetLexText())
		{
			uiText->MarkVertexPositionDirty();
		}
	}
}
void ULexMeshModifierTextAnimation_RangeSelector::SetEnd(float Value)
{
	if (End != Value)
	{
		End = Value;
		if (auto uiText = GetLexText())
		{
			uiText->MarkVertexPositionDirty();
		}
	}
}

bool ULexMeshModifierTextAnimation_RandomSelector::Select(ULexText* InUIText, FLexMeshModifierTextAnimation_SelectResult& OutSelection)
{
	if (End <= Start)return false;
	FMath::RandInit(Seed);
	auto& charProperties = InUIText->GetCharPropertyArray();
	float calculatedOffset = Offset * 2.0f - 1.0f;
	OutSelection.StartCharIndex = charProperties.Num() * Start;
	OutSelection.EndCharCount = charProperties.Num() * End;
	int count = OutSelection.EndCharCount - OutSelection.StartCharIndex;
	auto& lerpValueArray = OutSelection.LerpValueArray;
	lerpValueArray.Reset(count);
	lerpValueArray.AddDefaulted(count);
	for (int startIndex = OutSelection.StartCharIndex, endIndex = OutSelection.EndCharCount; startIndex < endIndex; startIndex++)
	{
		float lerpValue = FMath::FRand() + calculatedOffset;
		//lerpValue = FMath::Clamp(lerpValue, 0.0f, 1.0f);
		int lerpValueIndex = startIndex - OutSelection.StartCharIndex;
		lerpValueArray[lerpValueIndex] = lerpValue;
	}
	return true;
}
void ULexMeshModifierTextAnimation_RandomSelector::SetSeed(int Value)
{
	if (Seed != Value)
	{
		Seed = Value;
		if (auto uiText = GetLexText())
		{
			uiText->MarkVertexPositionDirty();
		}
	}
}
void ULexMeshModifierTextAnimation_RandomSelector::SetStart(float Value)
{
	if (Start != Value)
	{
		Start = Value;
		if (auto uiText = GetLexText())
		{
			uiText->MarkVertexPositionDirty();
		}
	}
}
void ULexMeshModifierTextAnimation_RandomSelector::SetEnd(float Value)
{
	if (End != Value)
	{
		End = Value;
		if (auto uiText = GetLexText())
		{
			uiText->MarkVertexPositionDirty();
		}
	}
}

bool ULexMeshModifierTextAnimation_RichTextTagSelector::Select(ULexText* InUIText, FLexMeshModifierTextAnimation_SelectResult& OutSelection)
{
	if (FMath::Abs(Range) < KINDA_SMALL_NUMBER)return false;
	auto& charProperties = InUIText->GetCharPropertyArray();
	auto& richTextCustomTagArray = InUIText->GetRichTextCustomTagArray();
	int foundIndex = richTextCustomTagArray.IndexOfByPredicate([this](const FLexUIText_RichTextCustomTag& A) {
		return A.TagName == TagName;
		});
	if (foundIndex == -1)return false;
	auto customTag = richTextCustomTagArray[foundIndex];

	float calculatedOffset = Offset * (1.0f + Range) - Range;
	float value = -calculatedOffset;
	OutSelection.StartCharIndex = customTag.CharIndexStart;
	OutSelection.EndCharCount = customTag.CharIndexEnd + 1;
	int count = OutSelection.EndCharCount - OutSelection.StartCharIndex;
	auto& lerpValueArray = OutSelection.LerpValueArray;
	lerpValueArray.Reset(count);
	lerpValueArray.AddDefaulted(count);
	float interval = 1.0f / (count - 1);
	float rangeInv = 1.0f / Range;
	for (int startIndex = OutSelection.StartCharIndex, endIndex = OutSelection.EndCharCount; startIndex < endIndex; startIndex++)
	{
		float lerpValue = value * rangeInv;
		//lerpValue = FMath::Clamp(lerpValue, 0.0f, 1.0f);
		int lerpValueIndex = startIndex - OutSelection.StartCharIndex;
		lerpValueArray[bFlipDirection ? endIndex - startIndex - 1 : lerpValueIndex] = 1.0f - lerpValue;
		value += interval;
	}
	return true;
}
void ULexMeshModifierTextAnimation_RichTextTagSelector::SetTagName(const FName& Value)
{
	if (TagName != Value)
	{
		TagName = Value;
		if (auto uiText = GetLexText())
		{
			uiText->MarkVertexPositionDirty();
		}
	}
}
void ULexMeshModifierTextAnimation_RichTextTagSelector::SetRange(float Value)
{
	if (Range != Value)
	{
		Range = Value;
		if (auto uiText = GetLexText())
		{
			uiText->MarkVertexPositionDirty();
		}
	}
}
void ULexMeshModifierTextAnimation_RichTextTagSelector::SetFlipDirection(bool Value)
{
	if (bFlipDirection != Value)
	{
		bFlipDirection = Value;
		if (auto uiText = GetLexText())
		{
			uiText->MarkVertexPositionDirty();
		}
	}
}
