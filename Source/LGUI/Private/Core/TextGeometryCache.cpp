// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Core/TextGeometryCache.h"
#include "Core/UIGeometry.h"
#include "Core/ActorComponent/UIText.h"

FTextGeometryCache::FTextGeometryCache(UUIText* InUIText)
{
	this->UIText = InUIText;
}
bool FTextGeometryCache::SetInputParameters(
	const FString& InContent,
	int32 InVisibleCharCount,
	float InWidth,
	float InHeight,
	FVector2D InPivot,
	FColor InColor,
	float InCanvasGroupAlpha,
	FVector2D InFontSpace,
	float InFontSize,
	UITextParagraphHorizontalAlign InParagraphHAlign,
	UITextParagraphVerticalAlign InParagraphVAlign,
	UITextOverflowType InOverflowType,
	bool InAdjustWidth,
	bool InAdjustHeight,
	bool InUseKerning,
	UITextFontStyle InFontStyle,
	bool InRichText,
	TWeakObjectPtr<ULGUIFontData_BaseObject> InFont
)
{
	if (!this->content.Equals(InContent))
	{
		this->content = InContent;
		bIsDirty = true;
	}
	if (this->visibleCharCount != InVisibleCharCount)
	{
		this->visibleCharCount = InVisibleCharCount;
		bIsDirty = true;
	}
	if (this->width != InWidth)
	{
		this->width = InWidth;
		bIsDirty = true;
	}
	if (this->height != InHeight)
	{
		this->height = InHeight;
		bIsDirty = true;
	}
	if (this->pivot != InPivot)
	{
		this->pivot = InPivot;
		bIsDirty = true;
	}
	if (this->color != InColor)
	{
		this->color = InColor;
		bIsColorDirty = true;
	}
	if (this->richText != InRichText)
	{
		this->richText = InRichText;
		bIsDirty = true;
	}
	if (this->canvasGroupAlpha != InCanvasGroupAlpha)
	{
		this->canvasGroupAlpha = InCanvasGroupAlpha;
		if (this->richText)//CanvasGroupAlpha only affect rich text alpha
		{
			bIsColorDirty = true;
		}
	}
	if (this->fontSpace != InFontSpace)
	{
		this->fontSpace = InFontSpace;
		bIsDirty = true;
	}
	if (this->fontSize != InFontSize)
	{
		this->fontSize = InFontSize;
		bIsDirty = true;
	}
	if (this->paragraphHAlign != InParagraphHAlign)
	{
		this->paragraphHAlign = InParagraphHAlign;
		bIsDirty = true;
	}
	if (this->paragraphVAlign != InParagraphVAlign)
	{
		this->paragraphVAlign = InParagraphVAlign;
		bIsDirty = true;
	}
	if (this->overflowType != InOverflowType)
	{
		this->overflowType = InOverflowType;
		bIsDirty = true;
	}
	if (this->adjustWidth != InAdjustWidth)
	{
		this->adjustWidth = InAdjustWidth;
		bIsDirty = true;
	}
	if (this->adjustHeight != InAdjustHeight)
	{
		this->adjustHeight = InAdjustHeight;
		bIsDirty = true;
	}
	if (this->useKerning != InUseKerning)
	{
		this->useKerning = InUseKerning;
		bIsDirty = true;
	}
	if (this->fontStyle != InFontStyle)
	{
		this->fontStyle = InFontStyle;
		bIsDirty = true;
	}
	if (this->font != InFont)
	{
		this->font = InFont;
		bIsDirty = true;
	}
	return bIsDirty;
}

void FTextGeometryCache::MarkDirty()
{
	bIsDirty = true;
}

void FTextGeometryCache::ConditaionalCalculateGeometry()
{
	if (bIsColorDirty && !bIsDirty)
	{
		bIsColorDirty = false;
		UIGeometry::UpdateUIColor(this->UIText->GetGeometry(), this->color);
	}
	else if (bIsDirty)
	{
		bIsDirty = false;
		bIsColorDirty = false;
		UIGeometry::UpdateUIText(
			this->content
			, this->visibleCharCount
			, this->width
			, this->height
			, this->pivot
			, this->color
			, (uint8)(this->canvasGroupAlpha * 255)
			, this->fontSpace
			, this->UIText->GetGeometry()
			, this->fontSize
			, this->paragraphHAlign
			, this->paragraphVAlign
			, this->overflowType
			, this->adjustWidth
			, this->adjustHeight
			, this->useKerning
			, this->fontStyle
			, this->textRealSize
			, this->UIText->GetRenderCanvas()
			, this->UIText.Get()
			, this->cacheTextPropertyArray
			, this->cacheCharPropertyArray
			, this->cacheRichTextCustomTagArray
			, this->font.Get()
			, this->richText
		);
	}
}

