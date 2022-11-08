// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UIText.h"
#include "LGUI.h"
#include "Core/UIGeometry.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Materials/MaterialInterface.h"
#include "Core/LGUIFontData_BaseObject.h"
#include "Core/UIDrawcall.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "Utils/LGUIUtils.h"
#include "Core/ActorComponent/UICanvasGroup.h"

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_DISABLE_OPTIMIZATION
#endif

#define LOCTEXT_NAMESPACE "UIText"

#if WITH_EDITORONLY_DATA
TWeakObjectPtr<ULGUIFontData_BaseObject> UUIText::CurrentUsingFontData = nullptr;
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
	CacheTextGeometryData = FTextGeometryCache(this);
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
	}
	geometry->texture = GetTextureToCreateGeometry();
	if (RenderCanvas.IsValid())
	{
		if (drawcall.IsValid())
		{
			drawcall->Texture = geometry->texture;
			drawcall->bTextureChanged = true;
			drawcall->bNeedToUpdateVertex = true;
		}
	}
	MarkVerticesDirty(false, true, true, false);
	MarkCanvasUpdate(true, true, false);
}

void UUIText::ApplyFontTextureChange()
{
	if (IsValid(font))
	{
		MarkVerticesDirty(true, true, true, true);
		MarkTextureDirty();
		geometry->texture = GetTextureToCreateGeometry();
		if (RenderCanvas.IsValid())
		{
			if (drawcall.IsValid())
			{
				drawcall->Texture = geometry->texture;
				drawcall->bTextureChanged = true;
				drawcall->bNeedToUpdateVertex = true;
			}
		}
	}
}

void UUIText::ApplyFontMaterialChange()
{
	if (IsValid(font))
	{
		MarkVerticesDirty(true, true, true, true);
		MarkMaterialDirty();
		geometry->material = GetMaterialToCreateGeometry();
		if (RenderCanvas.IsValid())
		{
			if (drawcall.IsValid())
			{
				drawcall->Material = geometry->material;
				drawcall->bMaterialChanged = true;
				drawcall->bMaterialNeedToReassign = true;
				drawcall->bNeedToUpdateVertex = true;
			}
		}
	}
}

void UUIText::ApplyRecreateText()
{
	if (IsValid(font))
	{
		CacheTextGeometryData.MarkDirty();
		MarkVertexPositionDirty();
	}
}

void UUIText::BeginPlay()
{
	Super::BeginPlay();
	if (IsValid(font))
	{
		font->InitFont();
		CheckFontAdditionalShaderChannels();
		if (!bHasAddToFont)
		{
			font->AddUIText(this);
			bHasAddToFont = true;
		}
	}
	visibleCharCount = VisibleCharCountInString(text.ToString());
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
		bHasAddToFont = false;
	}
}

void UUIText::OnRegister()
{
	Super::OnRegister();
	if (auto World = this->GetWorld())
	{
#if WITH_EDITOR
		if (!World->IsGameWorld())
		{
			if (!bHasAddToFont)
			{
				if (IsValid(font))
				{
					font->AddUIText(this);
					bHasAddToFont = true;
				}
			}
			ULGUIEditorManagerObject::RegisterLGUILayout(this);
		}
		else
#endif
		{
			ALGUIManagerActor::RegisterLGUICultureChangedEvent(this);
			ALGUIManagerActor::RegisterLGUILayout(this);
		}
	}
}
void UUIText::OnUnregister()
{
	Super::OnUnregister();
	if (auto World = this->GetWorld())
	{
#if WITH_EDITOR
		if (!World->IsGameWorld())
		{
			if (IsValid(font))
			{
				font->RemoveUIText(this);
				bHasAddToFont = false;
			}
			ULGUIEditorManagerObject::UnregisterLGUILayout(this);
		}
		else
#endif
		{
			ALGUIManagerActor::UnregisterLGUICultureChangedEvent(this);
			ALGUIManagerActor::UnregisterLGUILayout(this);
		}
	}
}

void UUIText::OnAnchorChange(bool InPivotChange, bool InSizeChange, bool InDiscardCache)
{
    Super::OnAnchorChange(InPivotChange, InSizeChange, InDiscardCache);
    if (InPivotChange || InSizeChange)
    {
        MarkVertexPositionDirty();
        MarkUVDirty();
    }
}

UTexture* UUIText::GetTextureToCreateGeometry()
{
	if (!IsValid(font))
	{
		font = ULGUIFontData_BaseObject::GetDefaultFont();
	}
	font->InitFont();
	CheckFontAdditionalShaderChannels();
	return font->GetFontTexture();
}

UMaterialInterface* UUIText::GetMaterialToCreateGeometry()
{
	if (IsValid(CustomUIMaterial))
	{
		return CustomUIMaterial;
	}
	else
	{
		if (!IsValid(font))
		{
			font = ULGUIFontData_BaseObject::GetDefaultFont();
		}
		font->InitFont();
		CheckFontAdditionalShaderChannels();
		return font->GetFontMaterial(this->GetRenderCanvas() != nullptr ? this->GetRenderCanvas()->GetActualClipType() : ELGUICanvasClipType::None);
	}
}

void UUIText::CheckFontAdditionalShaderChannels()
{
	if (RenderCanvas.IsValid())
	{
		auto flags = font->GetRequireAdditionalShaderChannels();
#if WITH_EDITOR
		auto originFlags = RenderCanvas->GetActualAdditionalShaderChannelFlags();
		if (flags != 0 && (originFlags & flags) == 0)
		{
			auto MsgText = FText::Format(LOCTEXT("FontChangeAddtionalShaderChannels"
				, "Automatically change 'AdditionalShaderChannels' property for LGUICanvas because font need it, font object: '{0}'")
				, FText::FromString(font->GetPathName()));
			//LGUIUtils::EditorNotification(MsgText);
			UE_LOG(LGUI, Log, TEXT("%s"), *MsgText.ToString());
		}
#endif
		RenderCanvas->SetActualRequireAdditionalShaderChannels(flags);
	}
}

void UUIText::OnBeforeCreateOrUpdateGeometry()
{
	if (!bHasAddToFont)
	{
		if (IsValid(font))
		{
			font->AddUIText(this);
			bHasAddToFont = true;
		}
	}
	if (visibleCharCount == -1)visibleCharCount = VisibleCharCountInString(text.ToString());
}

void UUIText::OnUpdateGeometry(UIGeometry& InGeo, bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	if (InTriangleChanged || InVertexPositionChanged || InVertexUVChanged || InVertexColorChanged)
	{
		UpdateCacheTextGeometry();
	}
}

void UUIText::UpdateMaterialClipType()
{
	geometry->material = GetMaterialToCreateGeometry();
	if (drawcall.IsValid())
	{
		drawcall->bMaterialChanged = true;
		drawcall->bMaterialNeedToReassign = true;
		drawcall->bNeedToUpdateVertex = true;
	}
}

void UUIText::OnCultureChanged_Implementation()
{
	static auto emptyText = FText();
	auto originText = text;
	text = emptyText;//just make it work, because SetText will compare text value
	SetText(originText);
}


#if WITH_EDITOR
void UUIText::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (auto Property = PropertyChangedEvent.Property)
	{
		auto PropertyName = Property->GetFName();
		if (PropertyName == GET_MEMBER_NAME_CHECKED(UUIText, text))
		{
			
		}
		else if (PropertyName == GET_MEMBER_NAME_CHECKED(UUIText, font))
		{
			UUIText::CurrentUsingFontData = font;
		}
		else if (PropertyName == GET_MEMBER_NAME_CHECKED(UUIText, useKerning))
		{
			MarkVertexPositionDirty();
			CacheTextGeometryData.MarkDirty();
		}
	}
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
void UUIText::EditorForceUpdate()
{
	Super::EditorForceUpdate();

	visibleCharCount = VisibleCharCountInString(text.ToString());
	if (!IsValid(font))
	{
		font = ULGUIFontData_BaseObject::GetDefaultFont();
		if (IsValid(font))
		{
			font->AddUIText(this);
			bHasAddToFont = true;
		}
	}
}
void UUIText::OnPreChangeFontProperty()
{
	if (IsValid(font))
	{
		font->RemoveUIText(this);
		bHasAddToFont = false;
	}
}
void UUIText::OnPostChangeFontProperty()
{
	if (IsValid(font))
	{
		font->AddUIText(this);
		bHasAddToFont = true;
	}
}
#endif

FVector2D UUIText::GetTextRealSize()
{
	UpdateCacheTextGeometry();
	return CacheTextGeometryData.textRealSize;
}



void UUIText::SetFont(ULGUIFontData_BaseObject* newFont) {
	if (font != newFont)
	{
		//remove from old
		if (IsValid(font))
		{
			font->RemoveUIText(this);
			bHasAddToFont = false;
		}
		font = newFont;

		MarkTextureDirty();
		//add to new
		if (IsValid(font))
		{
			font->AddUIText(this);
			bHasAddToFont = true;
		}
	}
}
void UUIText::SetText(const FText& newText) {
	if (!text.EqualTo(newText))
	{
		text = newText;

		int newVisibleCharCount = VisibleCharCountInString(text.ToString());
		if (newVisibleCharCount != visibleCharCount)//visible char count change
		{
			MarkVerticesDirty(true, true, true, true);
			visibleCharCount = newVisibleCharCount;
		}
		else//visible char count not change, just mark update vertex and uv
		{
			MarkVerticesDirty(false, true, true, false);
		}
	}
}


void UUIText::SetFontSize(float newSize) {
	if (size != newSize)
	{
		MarkVertexPositionDirty();
		size = newSize;
	}
}
void UUIText::SetUseKerning(bool value)
{
	if (useKerning != value)
	{
		useKerning = value;
		MarkVertexPositionDirty();
	}
}
void UUIText::SetFontSpace(FVector2D newSpace) {
	if (space != newSpace)
	{
		MarkVertexPositionDirty();
		space = newSpace;
	}
}
void UUIText::SetParagraphHorizontalAlignment(UITextParagraphHorizontalAlign newHAlign) {
	if (hAlign != newHAlign)
	{
		MarkVertexPositionDirty();
		hAlign = newHAlign;
	}
}
void UUIText::SetParagraphVerticalAlignment(UITextParagraphVerticalAlign newVAlign) {
	if (vAlign != newVAlign)
	{
		MarkVertexPositionDirty();
		vAlign = newVAlign;
	}
}
void UUIText::SetOverflowType(UITextOverflowType newOverflowType) {
	if (overflowType != newOverflowType)
	{
		if (overflowType == UITextOverflowType::ClampContent
			|| newOverflowType == UITextOverflowType::ClampContent
			)
			MarkVerticesDirty(true, true, true, true);
		else
			MarkVertexPositionDirty();
		overflowType = newOverflowType;
	}
}
void UUIText::SetAdjustWidth(bool newAdjustWidth) {
	if (adjustWidth != newAdjustWidth)
	{
		adjustWidth = newAdjustWidth;
		MarkVertexPositionDirty();
	}
}
void UUIText::SetAdjustHeight(bool newAdjustHeight) {
	if (adjustHeight != newAdjustHeight)
	{
		adjustHeight = newAdjustHeight;
		MarkVertexPositionDirty();
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
			MarkVerticesDirty(true, true, true, true);
		}
		fontStyle = newFontStyle;
	}
}
void UUIText::SetRichText(bool newRichText)
{
	if (richText != newRichText)
	{
		MarkVerticesDirty(true, true, true, true);
		richText = newRichText;
	}
}

void UUIText::MarkTextLayoutDirty()
{
	bTextLayoutDirty = true;
	if (auto World = this->GetWorld())
	{
#if WITH_EDITOR
		if (!World->IsGameWorld())
		{

		}
		else
#endif
		{
			ALGUIManagerActor::MarkUpdateLayout(World);
		}
	}
}

void UUIText::OnUpdateLayout_Implementation()
{
	if (!this->RenderCanvas.IsValid())return;

	if (bTextLayoutDirty)
	{
		if (UpdateCacheTextGeometry())
		{
			bTextLayoutDirty = false;
			if (overflowType == UITextOverflowType::HorizontalOverflow)
			{
				if (adjustWidth) SetWidth(CacheTextGeometryData.textRealSize.X);
			}
			else if (overflowType == UITextOverflowType::VerticalOverflow)
			{
				if (adjustHeight) SetHeight(CacheTextGeometryData.textRealSize.Y);
			}
		}
	}
}
bool UUIText::GetCanLayoutControlAnchor_Implementation(class UUIItem* InUIItem, FLGUICanLayoutControlAnchor& OutResult)const
{
	if (this == InUIItem)
	{
		if (overflowType == UITextOverflowType::HorizontalOverflow)
		{
			if (adjustWidth)
			{
				OutResult.bCanControlHorizontalSizeDelta = true;
				return true;
			}
		}
		else if (overflowType == UITextOverflowType::VerticalOverflow)
		{
			if (adjustHeight)
			{
				OutResult.bCanControlVerticalSizeDelta = true;
				return true;
			}
		}
	}
	return false;
}

bool UUIText::UpdateCacheTextGeometry()const
{
	if (!IsValid(this->GetFont()))return false;

	if (visibleCharCount == -1)visibleCharCount = VisibleCharCountInString(text.ToString());
	auto CanvasGroupAlpha = (this->GetRichText() && CanvasGroup.IsValid()) ? CanvasGroup->GetFinalAlpha() : 1.0f;
	CacheTextGeometryData.SetInputParameters(
		this->text.ToString()
		, this->visibleCharCount
		, this->GetWidth()
		, this->GetHeight()
		, this->GetPivot()
		, this->GetFinalColor()
		, CanvasGroupAlpha
		, this->GetFontSpace()
		, this->GetFontSize()
		, this->GetParagraphHorizontalAlignment()
		, this->GetParagraphVerticalAlignment()
		, this->GetOverflowType()
		, this->GetAdjustWidth()
		, this->GetAdjustHeight()
		, this->GetUseKerning()
		, this->GetFontStyle()
		, this->GetRichText()
		, this->GetFont()
	);
	if (geometry->vertices.Num() == 0)//@todo: geometry is cleared before OnUpdateGeometry, consider use a cached UIGeometry
	{
		CacheTextGeometryData.MarkDirty();
	}
	CacheTextGeometryData.ConditaionalCalculateGeometry();
	return true;
}

void UUIText::MarkVerticesDirty(bool InTriangleDirty, bool InVertexPositionDirty, bool InVertexUVDirty, bool InVertexColorDirty)
{
	MarkTextLayoutDirty();
	CacheTextGeometryData.MarkDirty();
	Super::MarkVerticesDirty(InTriangleDirty, InVertexPositionDirty, InVertexUVDirty, InVertexColorDirty);
}
void UUIText::MarkTextureDirty()
{
	MarkTextLayoutDirty();
	CacheTextGeometryData.MarkDirty();
	Super::MarkTextureDirty();
}

void UUIText::MarkAllDirty()
{
	MarkTextLayoutDirty();
	CacheTextGeometryData.MarkDirty();
	Super::MarkAllDirty();
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

const TArray<FUITextCharProperty>& UUIText::GetCharPropertyArray()const
{
	UpdateCacheTextGeometry();
	return CacheTextGeometryData.cacheCharPropertyArray;
}
int32 UUIText::GetVisibleCharCount()const
{
	UpdateCacheTextGeometry();
	return CacheTextGeometryData.cacheCharPropertyArray.Num();
}
const TArray<FUIText_RichTextCustomTag>& UUIText::GetRichTextCustomTagArray()const
{
	UpdateCacheTextGeometry();
	return CacheTextGeometryData.cacheRichTextCustomTagArray;
}





FString UUIText::GetSubStringByLine(const FString& inString, int32& inOutLineStartIndex, int32& inOutLineEndIndex, int32& inOutCharStartIndex, int32& inOutCharEndIndex)
{
	if (inString.Len() == 0)//no text
		return inString;
	SetText(FText::FromString(inString));
	UpdateCacheTextGeometry();
	int lineCount = inOutLineEndIndex - inOutLineStartIndex;
	auto& cacheTextPropertyArray = CacheTextGeometryData.cacheTextPropertyArray;
	if (inOutLineEndIndex + 1 >= cacheTextPropertyArray.Num())
	{
		inOutLineEndIndex = cacheTextPropertyArray.Num() - 1;
		if (lineCount < cacheTextPropertyArray.Num())
		{
			inOutLineStartIndex = inOutLineEndIndex - lineCount;
		}
	}
	if (inOutLineStartIndex < 0)
	{
		inOutLineStartIndex = 0;
		if (lineCount < cacheTextPropertyArray.Num())
		{
			inOutLineEndIndex = inOutLineStartIndex + lineCount;
		}
	}
	inOutCharStartIndex = cacheTextPropertyArray[inOutLineStartIndex].charPropertyList[0].charIndex;
	auto& endLine = cacheTextPropertyArray[inOutLineEndIndex];
	inOutCharEndIndex = endLine.charPropertyList[endLine.charPropertyList.Num() - 1].charIndex;
	return inString.Mid(inOutCharStartIndex, inOutCharEndIndex - inOutCharStartIndex);
}
//caret is at left side of char
void UUIText::FindCaretByIndex(int32 caretPositionIndex, FVector2D& outCaretPosition, int32& outCaretPositionLineIndex)
{
	outCaretPosition.X = outCaretPosition.Y = 0;
	outCaretPositionLineIndex = 0;
	if (text.ToString().Len() == 0)
	{
		float pivotOffsetX = this->GetWidth() * (0.5f - this->GetPivot().X);
		float pivotOffsetY = this->GetHeight() * (0.5f - this->GetPivot().Y);
		switch (hAlign)
		{
		case UITextParagraphHorizontalAlign::Left:
		{
			outCaretPosition.X = pivotOffsetX - this->GetWidth() * 0.5f;
		}
			break;
		case UITextParagraphHorizontalAlign::Center:
		{
			outCaretPosition.X = pivotOffsetX;
		}
			break;
		case UITextParagraphHorizontalAlign::Right:
		{
			outCaretPosition.X = pivotOffsetX + this->GetWidth() * 0.5f;
		}
			break;
		}
		switch (vAlign)
		{
		case UITextParagraphVerticalAlign::Top:
		{
			outCaretPosition.Y = pivotOffsetY + this->GetHeight() * 0.5f - size * 0.5f;//fixed offset
		}
			break;
		case UITextParagraphVerticalAlign::Middle:
		{
			outCaretPosition.Y = pivotOffsetY;
		}
			break;
		case UITextParagraphVerticalAlign::Bottom:
		{
			outCaretPosition.Y = pivotOffsetY - this->GetHeight() * 0.5f + size * 0.5f;//fixed offset
		}
			break;
		}
	}
	else
	{
		UpdateCacheTextGeometry();
		auto& cacheTextPropertyArray = CacheTextGeometryData.cacheTextPropertyArray;

		if (caretPositionIndex == 0)//first char
		{
			outCaretPosition = cacheTextPropertyArray[0].charPropertyList[0].caretPosition;
		}
		else//not first char
		{
			int lineCount = cacheTextPropertyArray.Num();//line count
			if (lineCount == 1)//only one line
			{
				auto& firstLine = cacheTextPropertyArray[0];
				auto& charProperty = firstLine.charPropertyList[caretPositionIndex];
				outCaretPosition = charProperty.caretPosition;
				outCaretPositionLineIndex = 0;
				return;
			}
			//search all lines, find charIndex == caretPositionIndex
			for (int lineIndex = 0; lineIndex < lineCount; lineIndex ++)
			{
				auto& lineItem = cacheTextPropertyArray[lineIndex];
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
	if (text.ToString().Len() == 0)//no text
		return;
	UpdateCacheTextGeometry();
	auto& cacheTextPropertyArray = CacheTextGeometryData.cacheTextPropertyArray;
	auto lineCount = cacheTextPropertyArray.Num();//line count
	if (lineCount == 1)//only one line
		return;
	outCaretPositionIndex = 0;

	//find nearest char to caret from this line
	auto& lineItem = cacheTextPropertyArray[inCaretPositionLineIndex];
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
	if (text.ToString().Len() == 0)//no text
		return;
	UpdateCacheTextGeometry();
	auto& cacheTextPropertyArray = CacheTextGeometryData.cacheTextPropertyArray;
	auto lineCount = cacheTextPropertyArray.Num();//line count
	if (lineCount == 1)//only one line
		return;
	outCaretPositionIndex = 0;

	//find nearest char to caret from this line
	auto& lineItem = cacheTextPropertyArray[inCaretPositionLineIndex];
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
	if (text.ToString().Len() == 0)//no text
	{
		outCaretPositionIndex = 0;
		FindCaretByIndex(0, outCaretPosition, outCaretPositionLineIndex);
	}
	else
	{
		UpdateCacheTextGeometry();
		auto& cacheTextPropertyArray = CacheTextGeometryData.cacheTextPropertyArray;

		auto localPosition = this->GetComponentTransform().InverseTransformPosition(inWorldPosition);
		auto localPosition2D = FVector2D(localPosition.Y, localPosition.Z);

		float nearestDistance = MAX_FLT;
		int lineCount = cacheTextPropertyArray.Num();
		//find the nearest line, only need to compare Y
		for (int lineIndex = 0; lineIndex < lineCount; lineIndex++)
		{
			auto& lineItem = cacheTextPropertyArray[lineIndex];
			float distance = FMath::Abs(lineItem.charPropertyList[0].caretPosition.Y - localPosition2D.Y);
			if (distance <= nearestDistance)
			{
				nearestDistance = distance;
				outCaretPositionLineIndex = lineIndex;
			}
		}
		//then find nearest char, only need to compare X
		nearestDistance = MAX_FLT;
		auto& nearestLine = cacheTextPropertyArray[outCaretPositionLineIndex];
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
	if (text.ToString().Len() == 0)return;
	UpdateCacheTextGeometry();
	auto& cacheTextPropertyArray = CacheTextGeometryData.cacheTextPropertyArray;
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
		auto& firstLineCharPropertyList = cacheTextPropertyArray[startCaretPositionLineIndex].charPropertyList;
		auto& firstLineLastCharProperty = firstLineCharPropertyList[firstLineCharPropertyList.Num() - 1];
		selectionProperty.Size = FMath::RoundToInt(firstLineLastCharProperty.caretPosition.X - startCaretPosition.X);
		//selectionProperty.Size = (1.0f - this->GetPivot().X) * this->GetWidth() - startCaretPosition.X;
		OutSelectionProeprtyArray.Add(selectionProperty);
		//middle line, use this->GetWidth() as size
		int middleLineCount = endCaretPositionLineIndex - startCaretPositionLineIndex - 1;
		for (int i = 0; i < middleLineCount; i++)
		{
			auto& charPropertyList = cacheTextPropertyArray[startCaretPositionLineIndex + i + 1].charPropertyList;
			auto& firstPosition = charPropertyList[0].caretPosition;
			auto& lasPosition = charPropertyList[charPropertyList.Num() - 1].caretPosition;
			selectionProperty.Pos = firstPosition;
			selectionProperty.Size = FMath::RoundToInt(lasPosition.X - firstPosition.X);
			OutSelectionProeprtyArray.Add(selectionProperty);
		}
		//end line
		auto& firstPosition = cacheTextPropertyArray[endCaretPositionLineIndex].charPropertyList[0].caretPosition;
		selectionProperty.Pos = firstPosition;
		selectionProperty.Size = FMath::RoundToInt(endCaretPosition.X - firstPosition.X);
		OutSelectionProeprtyArray.Add(selectionProperty);
	}
}

#undef LOCTEXT_NAMESPACE

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_ENABLE_OPTIMIZATION
#endif
