// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UIText.h"
#include "LGUI.h"
#include "Core/UIGeometry.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Materials/MaterialInterface.h"
#include "Core/LGUIFontData.h"


#if WITH_EDITORONLY_DATA
TWeakObjectPtr<ULGUIFontData> UUIText::CurrentUsingFontData = nullptr;
#endif
UUIText::UUIText(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
#if WITH_EDITORONLY_DATA
	if (UUIText::CurrentUsingFontData.IsValid())
	{
		font = CurrentUsingFontData.Get();
	}
#endif
}
void UUIText::ApplyFontTextureScaleUp()
{
	auto& vertices = geometry->vertices;
	if (vertices.Num() != 0)
	{
		for (int i = 0; i < vertices.Num(); i++)
		{
			auto& uv = vertices[i].TextureCoordinate[0];
			uv *= 0.5f;
		}
		MarkUVDirty();
	}
	geometry->texture = font->texture;
	if (CheckRenderCanvas())
	{
		RenderCanvas->SetDrawcallTexture(geometry->drawcallIndex, font->texture, true);
	}
}

void UUIText::ApplyFontTextureChange()
{
	if (IsValid(font))
	{
		cachedTextPropertyList.Reset();
		MarkTriangleDirty();
		MarkTextureDirty();
		geometry->texture = font->texture;
		if (CheckRenderCanvas())
		{
			RenderCanvas->SetDrawcallTexture(geometry->drawcallIndex, font->texture, true);
		}
	}
}

void UUIText::ApplyRecreateText()
{
	if (IsValid(font))
	{
		cachedTextPropertyList.Reset();
		MarkVertexPositionDirty();
	}
}

void UUIText::BeginPlay()
{
	Super::BeginPlay();
	if (IsValid(font))
	{
		font->InitFreeType();
		font->AddUIText(this);
	}
	visibleCharCount = VisibleCharCountInString(text);
}

void UUIText::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );
}

void UUIText::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (IsValid(font))
	{
		font->RemoveUIText(this);
	}
}

void UUIText::OnRegister()
{
	Super::OnRegister();
#if WITH_EDITOR
	if (this->GetWorld() && !this->GetWorld()->IsGameWorld())
	{
		if (IsValid(font))
		{
			font->AddUIText(this);
		}
	}
#endif
}
void UUIText::OnUnregister()
{
	Super::OnUnregister();
#if WITH_EDITOR
	if (this->GetWorld() && !this->GetWorld()->IsGameWorld())
	{
		if (IsValid(font))
		{
			font->RemoveUIText(this);
		}
	}
#endif
}

void UUIText::WidthChanged()
{
	MarkVertexPositionDirty();
	MarkUVDirty();
}
void UUIText::HeightChanged()
{
	MarkVertexPositionDirty();
	MarkUVDirty();
}
void UUIText::UpdateCachedData()
{
	Super::UpdateCachedData();
}
void UUIText::UpdateBasePrevData()
{
	Super::UpdateBasePrevData();
}

UTexture* UUIText::GetTextureToCreateGeometry()
{
	if (!IsValid(font))
	{
		font = ULGUIFontData::GetDefaultFont();
	}
	font->InitFreeType();
	return font->texture;
}
bool UUIText::HaveDataToCreateGeometry()
{
	return visibleCharCount > 0 && IsValid(font);
}

void UUIText::OnBeforeCreateOrUpdateGeometry()
{
	if (visibleCharCount == -1)
	{
		visibleCharCount = VisibleCharCountInString(text);
	}
	CacheTextGeometry();
}
void UUIText::OnCreateGeometry()
{
	geometry->isFontTexture = true;
	float width = widget.width;
	float height = widget.height;
	UIGeometry::FromUIText(text, visibleCharCount, width, height, widget.pivot, GetFinalColor(), space, geometry, size, hAlign, vAlign, overflowType, adjustWidth, adjustHeight, fontStyle, textRealSize, RenderCanvas, cachedTextPropertyList, font);
	SetWidth(width);
	SetHeight(height);
}
void UUIText::OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	if (InVertexColorChanged)
	{
		UIGeometry::UpdateUIColor(geometry, GetFinalColor());
	}
	if (InVertexPositionChanged || InVertexUVChanged)
	{
		float width = widget.width;
		float height = widget.height;
		UIGeometry::UpdateUITextVertexOrUV(text, visibleCharCount, width, height, widget.pivot, space, geometry, size, hAlign, vAlign, overflowType, adjustWidth, adjustHeight, fontStyle, textRealSize, RenderCanvas, cachedTextPropertyList, font);
		SetWidth(width);
		SetHeight(height);
	}
}


#if WITH_EDITOR
void UUIText::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	visibleCharCount = VisibleCharCountInString(text);
}
void UUIText::EditorForceUpdateImmediately()
{
	Super::EditorForceUpdateImmediately();
#if WITH_EDITORONLY_DATA
	UUIText::CurrentUsingFontData = font;
#endif
	visibleCharCount = VisibleCharCountInString(text);
	if (!IsValid(font))
	{
		font = ULGUIFontData::GetDefaultFont();
		if (IsValid(font))
		{
			font->AddUIText(this);
		}
	}
	cachedTextPropertyList.Reset();
}
void UUIText::OnPreChangeFontProperty()
{
	if (IsValid(font))
	{
		font->RemoveUIText(this);
	}
}
void UUIText::OnPostChangeFontProperty()
{
	if (IsValid(font))
	{
		font->AddUIText(this);
	}
}
#endif

FVector2D UUIText::GetRealSize()
{
	if (cachedTextPropertyList.Num() == 0)//no cache data yet
	{
		if (geometry->vertices.Num() == 0)//no geometry yet
		{
			if (visibleCharCount > 0)
			{
				CreateGeometry();
			}
		}
		else//if geometry is created then update
		{
			float width = widget.width;
			float height = widget.height;
			UIGeometry::UpdateUITextVertexOrUV(text, visibleCharCount, width, height, widget.pivot, space, geometry, size, hAlign, vAlign, overflowType, adjustWidth, adjustHeight, fontStyle, textRealSize, RenderCanvas, cachedTextPropertyList, font);
			SetWidth(width);
			SetHeight(height);
		}
		return textRealSize;
	}
	else//if have valid cache data
	{
		return textRealSize;
	}
}



void UUIText::SetFont(ULGUIFontData* newFont) {
	if (font != newFont)
	{
		//remove from old
		if (IsValid(font))
		{
			font->RemoveUIText(this);
		}
		font = newFont;
		cachedTextPropertyList.Reset();
		MarkTextureDirty();
		//add to new
		if (IsValid(font))
		{
			font->AddUIText(this);
		}
	}
}
void UUIText::SetText(const FString& newText) {
	if (text != newText)
	{
		if (CheckRenderCanvas()) RenderCanvas->MarkCanvasUpdate();
		text = newText;
		text.ReplaceInline(TEXT("\r\n"), TEXT("\n"));
		cachedTextPropertyList.Reset();

		int newVisibleCharCount = VisibleCharCountInString(text);
		if (newVisibleCharCount != visibleCharCount)//visible char count change
		{
			MarkTriangleDirty();
			visibleCharCount = newVisibleCharCount;
		}
		else//visible char count not change, just mark update vertex and uv
		{
			MarkVertexPositionDirty();
			MarkUVDirty();
		}
	}
}
void UUIText::SetFontSize(float newSize) {
	newSize = FMath::Clamp(newSize, 0.0f, 200.0f);
	if (size != newSize)
	{
		MarkVertexPositionDirty();
		size = newSize;
		cachedTextPropertyList.Reset();
	}
}
void UUIText::SetFontSpace(FVector2D newSpace) {
	if (space != newSpace)
	{
		MarkVertexPositionDirty();
		space = newSpace;
		cachedTextPropertyList.Reset();
	}
}
void UUIText::SetParagraphHorizontalAlignment(UITextParagraphHorizontalAlign newHAlign) {
	if (hAlign != newHAlign)
	{
		MarkVertexPositionDirty();
		hAlign = newHAlign;
		cachedTextPropertyList.Reset();
	}
}
void UUIText::SetParagraphVerticalAlignment(UITextParagraphVerticalAlign newVAlign) {
	if (vAlign != newVAlign)
	{
		MarkVertexPositionDirty();
		vAlign = newVAlign;
		cachedTextPropertyList.Reset();
	}
}
void UUIText::SetOverflowType(UITextOverflowType newOverflowType) {
	if (overflowType != newOverflowType)
	{
		if (overflowType == UITextOverflowType::ClampContent || newOverflowType == UITextOverflowType::ClampContent)
			MarkTriangleDirty();
		else
			MarkVertexPositionDirty();
		overflowType = newOverflowType;
		cachedTextPropertyList.Reset();
	}
}
void UUIText::SetAdjustWidth(bool newAdjustWidth) {
	if (adjustWidth != newAdjustWidth)
	{
		adjustWidth = newAdjustWidth;
		MarkVertexPositionDirty();
		cachedTextPropertyList.Reset();
	}
}
void UUIText::SetAdjustHeight(bool newAdjustHeight) {
	if (adjustHeight != newAdjustHeight)
	{
		adjustHeight = newAdjustHeight;
		MarkVertexPositionDirty();
		cachedTextPropertyList.Reset();
	}
}
void UUIText::SetFontStyle(UITextFontStyle newFontStyle) {
	if (fontStyle != newFontStyle)
	{
		if ((fontStyle == UITextFontStyle::None || fontStyle == UITextFontStyle::Italic)
			&& (newFontStyle == UITextFontStyle::None || newFontStyle == UITextFontStyle::Italic))//these only affect vertex position
		{
			MarkVertexPositionDirty();
		}
		else
		{
			MarkTriangleDirty();
		}
		fontStyle = newFontStyle;
		cachedTextPropertyList.Reset();
	}
}
void UUIText::CacheTextGeometry()
{
	if (visibleCharCount == -1)visibleCharCount = VisibleCharCountInString(text);
}
void UUIText::MarkAllDirtyRecursive()
{
	cachedTextPropertyList.Reset();
	Super::MarkAllDirtyRecursive();
}
FVector2D UUIText::GetCharSize(TCHAR character)
{
	if (character == '\n' || character == '\r')return FVector2D::ZeroVector;
	if (character == ' ')
	{
		return FVector2D(size * 0.5f, size);
	}
	else if (character == '\t')
	{
		return FVector2D(size + size, size);
	}
	else
	{
		bool bold = fontStyle == UITextFontStyle::Bold || fontStyle == UITextFontStyle::BoldAndItalic;
		bool italic = fontStyle == UITextFontStyle::Italic || fontStyle == UITextFontStyle::BoldAndItalic;
		auto charData = font->GetCharData(character, size, bold, italic);
		return FVector2D(charData->xadvance, size);
	}
}
int UUIText::VisibleCharCountInString(const FString& srcStr)
{
	int count = srcStr.Len();
	if (count == 0)return 0;
	int result = 0;
	for (int i = 0; i < count; i++)
	{
		auto charIndexItem = srcStr[i];
		if (IsVisibleChar(charIndexItem) == false)
		{
			continue;
		}
		result++;
	}
	return result;
}




void UUIText::CheckCachedTextPropertyList()
{
	auto cacheCount = cachedTextPropertyList.Num();
	if (cacheCount == 0)//no cache ye
	{
		CacheTextGeometry();
		float width = widget.width;
		float height = widget.height;
		UIGeometry::UpdateUITextVertexOrUV(text, visibleCharCount, width, height, widget.pivot, space, geometry, size, hAlign, vAlign, overflowType, adjustWidth, adjustHeight, fontStyle, textRealSize, RenderCanvas, cachedTextPropertyList, font);
		SetWidth(width);
		SetHeight(height);
	}
}
FString UUIText::GetSubStringByLine(const FString& inString, int32& inOutLineStartIndex, int32& inOutLineEndIndex, int32& inOutCharStartIndex, int32& inOutCharEndIndex)
{
	if (inString.Len() == 0)//no text
		return inString;
	SetText(inString);
	CheckCachedTextPropertyList();
	int lineCount = inOutLineEndIndex - inOutLineStartIndex;
	if (inOutLineEndIndex + 1 >= cachedTextPropertyList.Num())
	{
		inOutLineEndIndex = cachedTextPropertyList.Num() - 1;
		if (lineCount < cachedTextPropertyList.Num())
		{
			inOutLineStartIndex = inOutLineEndIndex - lineCount;
		}
	}
	if (inOutLineStartIndex < 0)
	{
		inOutLineStartIndex = 0;
		if (lineCount < cachedTextPropertyList.Num())
		{
			inOutLineEndIndex = inOutLineStartIndex + lineCount;
		}
	}
	inOutCharStartIndex = cachedTextPropertyList[inOutLineStartIndex].charPropertyList[0].charIndex;
	auto& endLine = cachedTextPropertyList[inOutLineEndIndex];
	inOutCharEndIndex = endLine.charPropertyList[endLine.charPropertyList.Num() - 1].charIndex;
	return inString.Mid(inOutCharStartIndex, inOutCharEndIndex - inOutCharStartIndex);
}
//caret is at left side of char
void UUIText::FindCaretByIndex(int32 caretPositionIndex, FVector2D& outCaretPosition, int32& outCaretPositionLineIndex)
{
	outCaretPosition.X = outCaretPosition.Y = 0;
	outCaretPositionLineIndex = 0;
	if (text.Len() == 0)
	{
		float pivotOffsetX = widget.width * (0.5f - widget.pivot.X);
		float pivotOffsetY = widget.height * (0.5f - widget.pivot.Y);
		switch (hAlign)
		{
		case UITextParagraphHorizontalAlign::Left:
		{
			outCaretPosition.X = pivotOffsetX - widget.width * 0.5f;
		}
			break;
		case UITextParagraphHorizontalAlign::Center:
		{
			outCaretPosition.X = pivotOffsetX;
		}
			break;
		case UITextParagraphHorizontalAlign::Right:
		{
			outCaretPosition.X = pivotOffsetX + widget.width * 0.5f;
		}
			break;
		}
		switch (vAlign)
		{
		case UITextParagraphVerticalAlign::Top:
		{
			outCaretPosition.Y = pivotOffsetY + widget.height * 0.5f - size * 0.5f;//fixed offset
		}
			break;
		case UITextParagraphVerticalAlign::Middle:
		{
			outCaretPosition.Y = pivotOffsetY;
		}
			break;
		case UITextParagraphVerticalAlign::Bottom:
		{
			outCaretPosition.Y = pivotOffsetY - widget.height * 0.5f + size * 0.5f;//fixed offset
		}
			break;
		}
	}
	else
	{
		CheckCachedTextPropertyList();

		if (caretPositionIndex == 0)//first char
		{
			outCaretPosition = cachedTextPropertyList[0].charPropertyList[0].caretPosition;
		}
		else//not first char
		{
			int lineCount = cachedTextPropertyList.Num();//line count
			if (lineCount == 1)//only one line
			{
				auto& firstLine = cachedTextPropertyList[0];
				auto& charProperty = firstLine.charPropertyList[caretPositionIndex];
				outCaretPosition = charProperty.caretPosition;
				outCaretPositionLineIndex = 0;
				return;
			}
			//search all lines, find charIndex == caretPositionIndex
			for (int lineIndex = 0; lineIndex < lineCount; lineIndex ++)
			{
				auto& lineItem = cachedTextPropertyList[lineIndex];
				int lineCharCount = lineItem.charPropertyList.Num();
				for (int lineCharIndex = 0; lineCharIndex < lineCharCount; lineCharIndex++)
				{
					auto& charItem = lineItem.charPropertyList[lineCharIndex];
					if (caretPositionIndex == charItem.charIndex)//found it
					{
						outCaretPosition = charItem.caretPosition;
						outCaretPositionLineIndex = lineIndex;
						return;
					}
				}
			}
		}
	}
}
void UUIText::FindCaretUp(FVector2D& inOutCaretPosition, int32 inCaretPositionLineIndex, int32& outCaretPositionIndex)
{
	if (text.Len() == 0)//no text
		return;
	CheckCachedTextPropertyList();
	auto lineCount = cachedTextPropertyList.Num();//line count
	if (lineCount == 1)//only one line
		return;
	outCaretPositionIndex = 0;

	//find nearest char to caret from this line
	auto& lineItem = cachedTextPropertyList[inCaretPositionLineIndex];
	int charCount = lineItem.charPropertyList.Num();//char count of this line
	float nearestDistance = MAX_FLT;
	int32 nearestIndex = -1;
	for (int charIndex = 0; charIndex < charCount; charIndex++)
	{
		auto& charItem = lineItem.charPropertyList[charIndex];
		float distance = FMath::Abs(charItem.caretPosition.X - inOutCaretPosition.X);
		if (distance <= nearestDistance)
		{
			nearestDistance = distance;
			nearestIndex = charIndex;
			outCaretPositionIndex = charItem.charIndex;
		}
	}
	inOutCaretPosition = lineItem.charPropertyList[nearestIndex].caretPosition;
}
void UUIText::FindCaretDown(FVector2D& inOutCaretPosition, int32 inCaretPositionLineIndex, int32& outCaretPositionIndex)
{
	if (text.Len() == 0)//no text
		return;
	CheckCachedTextPropertyList();
	auto lineCount = cachedTextPropertyList.Num();//line count
	if (lineCount == 1)//only one line
		return;
	outCaretPositionIndex = 0;

	//find nearest char to caret from this line
	auto& lineItem = cachedTextPropertyList[inCaretPositionLineIndex];
	int charPropertyCount = lineItem.charPropertyList.Num();//char count of this line
	float nearestDistance = MAX_FLT;
	int32 nearestIndex = -1;
	for (int charPropertyIndex = 0; charPropertyIndex < charPropertyCount; charPropertyIndex++)
	{
		auto& charItem = lineItem.charPropertyList[charPropertyIndex];
		float distance = FMath::Abs(charItem.caretPosition.X - inOutCaretPosition.X);
		if (distance <= nearestDistance)
		{
			nearestDistance = distance;
			nearestIndex = charPropertyIndex;
			outCaretPositionIndex = charItem.charIndex;
		}
	}
	inOutCaretPosition = lineItem.charPropertyList[nearestIndex].caretPosition;
}
//find caret by position, caret is on left side of char
void UUIText::FindCaretByPosition(FVector inWorldPosition, FVector2D& outCaretPosition, int32& outCaretPositionLineIndex, int32& outCaretPositionIndex)
{
	if (text.Len() == 0)//no text
	{
		outCaretPositionIndex = 0;
		FindCaretByIndex(0, outCaretPosition, outCaretPositionLineIndex);
	}
	else
	{
		CheckCachedTextPropertyList();

		auto localPosition = this->GetComponentTransform().InverseTransformPosition(inWorldPosition);
		auto localPosition2D = FVector2D(localPosition);

		float nearestDistance = MAX_FLT;
		int lineCount = cachedTextPropertyList.Num();
		//find the nearest line, only need to compare Y
		for (int lineIndex = 0; lineIndex < lineCount; lineIndex++)
		{
			auto& lineItem = cachedTextPropertyList[lineIndex];
			float distance = FMath::Abs(lineItem.charPropertyList[0].caretPosition.Y - localPosition2D.Y);
			if (distance <= nearestDistance)
			{
				nearestDistance = distance;
				outCaretPositionLineIndex = lineIndex;
			}
		}
		//then find nearest char, only need to compare X
		nearestDistance = MAX_FLT;
		auto& nearestLine = cachedTextPropertyList[outCaretPositionLineIndex];
		int charCount = nearestLine.charPropertyList.Num();
		for (int charIndex = 0; charIndex < charCount; charIndex++)
		{
			auto& charItem = nearestLine.charPropertyList[charIndex];
			float distance = FMath::Abs(charItem.caretPosition.X - localPosition2D.X);
			if (distance <= nearestDistance)
			{
				nearestDistance = distance;
				outCaretPositionIndex = charItem.charIndex;
				outCaretPosition = charItem.caretPosition;
			}
		}
	}
}

void UUIText::GetSelectionProperty(int32 InSelectionStartCaretIndex, int32 InSelectionEndCaretIndex, TArray<FUITextSelectionProperty>& OutSelectionProeprtyArray)
{
	OutSelectionProeprtyArray.Reset();
	if (text.Len() == 0)return;
	CheckCachedTextPropertyList();
	//start
	FVector2D startCaretPosition;
	int32 startCaretPositionLineIndex;
	FindCaretByIndex(InSelectionStartCaretIndex, startCaretPosition, startCaretPositionLineIndex);
	//end
	FVector2D endCaretPosition;
	int32 endCaretPositionLineIndex;
	FindCaretByIndex(InSelectionEndCaretIndex, endCaretPosition, endCaretPositionLineIndex);
	//if select from down to up, then convert it from up to down
	if (startCaretPositionLineIndex > endCaretPositionLineIndex)
	{
		auto tempInt = endCaretPositionLineIndex;
		endCaretPositionLineIndex = startCaretPositionLineIndex;
		startCaretPositionLineIndex = tempInt;
		auto tempV2 = endCaretPosition;
		endCaretPosition = startCaretPosition;
		startCaretPosition = tempV2;
	}
	
	if (startCaretPositionLineIndex == endCaretPositionLineIndex)//same line
	{
		FUITextSelectionProperty selectionProperty;
		selectionProperty.Pos = startCaretPosition;
		selectionProperty.Size = endCaretPosition.X - startCaretPosition.X;
		OutSelectionProeprtyArray.Add(selectionProperty);
	}
	else//different line
	{
		//first line
		FUITextSelectionProperty selectionProperty;
		selectionProperty.Pos = startCaretPosition;
		auto& firstLineCharPropertyList = cachedTextPropertyList[startCaretPositionLineIndex].charPropertyList;
		auto& firstLineLastCharProperty = firstLineCharPropertyList[firstLineCharPropertyList.Num() - 1];
		selectionProperty.Size = FMath::RoundToInt(firstLineLastCharProperty.caretPosition.X - startCaretPosition.X);
		//selectionProperty.Size = (1.0f - widget.pivot.X) * widget.width - startCaretPosition.X;
		OutSelectionProeprtyArray.Add(selectionProperty);
		//middle line, use widget.width as size
		int middleLineCount = endCaretPositionLineIndex - startCaretPositionLineIndex - 1;
		for (int i = 0; i < middleLineCount; i++)
		{
			auto& charPropertyList = cachedTextPropertyList[startCaretPositionLineIndex + i + 1].charPropertyList;
			auto& firstPosition = charPropertyList[0].caretPosition;
			auto& lasPosition = charPropertyList[charPropertyList.Num() - 1].caretPosition;
			selectionProperty.Pos = firstPosition;
			selectionProperty.Size = FMath::RoundToInt(lasPosition.X - firstPosition.X);
			OutSelectionProeprtyArray.Add(selectionProperty);
		}
		//end line
		auto& firstPosition = cachedTextPropertyList[endCaretPositionLineIndex].charPropertyList[0].caretPosition;
		selectionProperty.Pos = firstPosition;
		selectionProperty.Size = FMath::RoundToInt(endCaretPosition.X - firstPosition.X);
		OutSelectionProeprtyArray.Add(selectionProperty);
	}
}
int UUIText::GetLineCount()
{
	if (text.Len() == 0)return 0;
	CheckCachedTextPropertyList();
	return cachedTextPropertyList.Num();
}