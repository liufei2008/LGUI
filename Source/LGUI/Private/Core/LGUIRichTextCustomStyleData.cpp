// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/LGUIRichTextCustomStyleData.h"
#include "LGUI.h"

void FLGUIRichTextCustomStyleItemData::ApplyToRichTextParseResult(LGUIRichTextParser::RichTextParseResult& value)const
{
	value.bold = this->bold;
	value.italic = this->italic;
	value.underline = this->underline;
	value.strikethrough = this->strikethrough;
	switch (this->sizeType)
	{
	default:
	case ELGUIRichTextCustomStyleData_SizeType::KeepOrigin:
		break;
	case ELGUIRichTextCustomStyleData_SizeType::SizeValue:
		value.size = this->size;
		break;
	case ELGUIRichTextCustomStyleData_SizeType::SizeValueAsAdditional:
		value.size += this->size;
		break;
	}
	switch (this->colorType)
	{
	default:
	case ELGUIRichTextCustomStyleData_ColorType::KeepOrigin:
		break;
	case ELGUIRichTextCustomStyleData_ColorType::Replace:
		value.color = this->color;
		break;
	case ELGUIRichTextCustomStyleData_ColorType::Multiply:
		value.color = LGUIUtils::MultiplyColor(value.color, this->color);
		break;
	}
	switch (this->supOrSub)
	{
	default:
	case ELGUIRichTextCustomStyleData_SupOrSubType::KeepOrigin:
		break;
	case ELGUIRichTextCustomStyleData_SupOrSubType::None:
		value.supOrSubMode = LGUIRichTextParser::SupOrSubMode::None;
		break;
	case ELGUIRichTextCustomStyleData_SupOrSubType::Superscript:
		value.supOrSubMode = LGUIRichTextParser::SupOrSubMode::Sup;
		value.size *= 0.8f;
		break;
	case ELGUIRichTextCustomStyleData_SupOrSubType::Subscript:
		value.supOrSubMode = LGUIRichTextParser::SupOrSubMode::Sub;
		value.size *= 0.8f;
		break;
	}
}

#if WITH_EDITOR
void ULGUIRichTextCustomStyleData::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	OnDataChange.Broadcast();
}
#endif
