// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/Components/LexText.h"
#include "Core/LexUIGeometry.h"
#include "Materials/MaterialInterface.h"
#include "Core/LexUIFontData_BaseObject.h"
#include "Core/LexUIRichTextImageData_BaseObject.h"
#include "Core/LexUIRichTextCustomStyleData.h"
#include "Core/LexUIFontEmojiData.h"
#include "Core/LexUIManager.h"
#include "Core/Components/LexCanvas.h"
#include "Core/Components/LexWidget.h"
#include "Utils/LexUIUtils.h"
#include "Engine/Texture2D.h"
#include "Engine/Texture2DArray.h"


#define LOCTEXT_NAMESPACE "UIText"

#if WITH_EDITORONLY_DATA
TWeakObjectPtr<ULexUIFontData_BaseObject> ULexText::CurrentUsingFontData = nullptr;
#endif
ULexText::ULexText(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
#if WITH_EDITOR
	if (ULexText::CurrentUsingFontData.IsValid())
	{
		Font = CurrentUsingFontData.Get();
	}
	else
#endif
	{
		Font = ULexUIFontData_BaseObject::GetDefaultFont();
	}
	CacheTextGeometryData = FLexUITextGeometryCache(this);
	UIGeometry->bIsFont = true;
}

void ULexText::ApplyFontTextureChange()
{
	if (IsValid(Font))
	{
		MarkVerticesDirty(true, true, true, true);
		MarkTextureDirty();
		UIGeometry->Texture = GetTextureToCreateGeometry();
	}
}

void ULexText::ApplyFontMaterialChange()
{
	if (IsValid(Font))
	{
		MarkVerticesDirty(true, true, true, true);
		MarkMaterialDirty();
		UIGeometry->Material = GetMaterialToCreateGeometry();
	}
}

void ULexText::ApplyRecreateText()
{
	if (IsValid(Font))
	{
		CacheTextGeometryData.MarkDirty();
		MarkVertexPositionDirty();
	}
}

void ULexText::ApplyFontEmojiChange()
{
	this->MarkVerticesDirty(false, true, true, false);
}

void ULexText::BeginPlay()
{
	Super::BeginPlay();
	if (IsValid(Font))
	{
		Font->InitFont();
		RegisterFont();
	}
	if (IsValid(RichTextImageData))
	{
		this->RegisterOnRichTextImageDataChange();
	}
	if (IsValid(RichTextCustomStyleData))
	{
		this->RegisterOnRichTextCustomStyleDataChange();
	}
}

void ULexText::EndPlay()
{
	Super::EndPlay();
	if (IsValid(Font))
	{
		UnregisterFont();
	}
	if (IsValid(RichTextImageData))
	{
		this->UnregisterOnRichTextImageDataChange();
	}
	if (IsValid(RichTextCustomStyleData))
	{
		this->UnregisterOnRichTextCustomStyleDataChange();
	}

	for (int i = 0; i < CreatedRichTextImageObjectArray.Num(); i++)
	{
		auto item = CreatedRichTextImageObjectArray[i];
		if (IsValid(item))
		{
			item->DestroyWidget();
		}
	}
	CreatedRichTextImageObjectArray.Empty();
}

void ULexText::OnRegister()
{
	Super::OnRegister();
	if (auto World = this->GetWorld())
	{
#if WITH_EDITOR
		if (!World->IsGameWorld())
		{
			if (IsValid(Font))
			{
				RegisterFont();
			}
			if (!RichTextImageDataChangedDelegateHandle.IsValid())
			{
				if (IsValid(RichTextImageData))
				{
					this->RegisterOnRichTextImageDataChange();
				}
			}
			if (!RichTextCustomStyleDataChangedDelegateHandle.IsValid())
			{
				if (IsValid(RichTextCustomStyleData))
				{
					this->RegisterOnRichTextCustomStyleDataChange();
				}
			}
		}
		else
#endif
		{
			ULexUIManagerWorldSubsystem::RegisterLexUICultureChangedEvent(this);
		}
	}
}
void ULexText::OnUnregister()
{
	Super::OnUnregister();
	if (auto World = this->GetWorld())
	{
#if WITH_EDITOR
		if (!World->IsGameWorld())
		{
			if (IsValid(Font))
			{
				UnregisterFont();
			}
			if (IsValid(RichTextImageData))
			{
				if (RichTextImageDataChangedDelegateHandle.IsValid())
				{
					this->UnregisterOnRichTextImageDataChange();
				}
			}
			if (IsValid(RichTextCustomStyleData))
			{
				if (RichTextCustomStyleDataChangedDelegateHandle.IsValid())
				{
					this->UnregisterOnRichTextCustomStyleDataChange();
				}
			}
		}
		else
#endif
		{
			ULexUIManagerWorldSubsystem::UnregisterLexUICultureChangedEvent(this);
		}
	}
}

void ULexText::BeginDestroy()
{
	Super::BeginDestroy();
	UnregisterFont();
}

void ULexText::OnTransformChanged(bool InPositionChanged, bool InScaleChanged)
{
	Super::OnTransformChanged(InPositionChanged, InScaleChanged);
	if (IsValid(Font) && Font->GetNeedObjectScale())//some font need object scale (SDF font), so detect scale change and mark update
	{
		if (InScaleChanged)
		{
			MarkVertexUVDirty();//object scale value is stored in uv2. @todo: font should tell
		}
	}
}

void ULexText::OnDimensionChanged(bool InPivotChange, bool InWidthChange, bool InHeightChange)
{
	Super::OnDimensionChanged(InPivotChange, InWidthChange, InHeightChange);
	MarkVertexPositionDirty();
	MarkVertexUVDirty();
}

UTexture* ULexText::GetTextureToCreateGeometry()
{
	if (!IsValid(Font))
	{
		Font = ULexUIFontData_BaseObject::GetDefaultFont();
	}
	Font->InitFont();
	return Font->GetFontTexture();
}

UMaterialInterface* ULexText::GetMaterialToCreateGeometry()
{
	if (IsValid(OverrideMaterial))
	{
		return OverrideMaterial;
	}
	if (!IsValid(Font))
	{
		Font = ULexUIFontData_BaseObject::GetDefaultFont();
	}
	Font->InitFont();
	return Font->GetFontMaterial();
}

void ULexText::OnBeforeCreateOrUpdateGeometry()
{
	if (IsValid(Font))
	{
		RegisterFont();
	}
	if (bRichText && !RichTextImageDataChangedDelegateHandle.IsValid())
	{
		if (IsValid(RichTextImageData))
		{
			this->RegisterOnRichTextImageDataChange();
		}
	}
	if (bRichText && !RichTextCustomStyleDataChangedDelegateHandle.IsValid())
	{
		if (IsValid(RichTextCustomStyleData))
		{
			this->RegisterOnRichTextCustomStyleDataChange();
		}
	}
}

bool ULexText::GetShouldAffectByPixelSnapping()const
{
	if (IsValid(Font))
	{
		return Font->GetShouldAffectByPixelSnapping();
	}
	return Super::GetShouldAffectByPixelSnapping();
}

void ULexText::OnUpdateGeometry(FLexUIGeometry& InGeo, bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	if (InTriangleChanged || InVertexPositionChanged || InVertexUVChanged || InVertexColorChanged)
	{
		CheckRequireNormalAndTangent();
		UpdateCacheTextGeometry();
	}
}

uint8 ULexText::GetFontMark_WidgetPropertyDataForMaterial()
{
	return static_cast<uint8>(this->Font->GetFontTextureMark());
}

void ULexText::OnCultureChanged_Implementation()
{
	auto originText = Text;
	Text = FText::GetEmpty();//just make it work, because SetText will compare text value
	SetText(originText);
}

void ULexText::CheckRequireNormalAndTangent()
{
	if (IsValid(Font) && Font->GetRequireNormalAndTangent())
	{
		if (auto Canvas = GetWidget()->GetRenderCanvas())
		{
			if (auto RootCanvas = Canvas->GetRootCanvas())
			{
				RootCanvas->SetRequireNormalAndTangent(true);
			}
		}
	}
}


#if WITH_EDITOR
void ULexText::PreEditChange(FProperty* PropertyAboutToChange)
{
	Super::PreEditChange(PropertyAboutToChange);
	auto PropertyName = PropertyAboutToChange->GetFName();
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ULexText, Font))
	{
		if (IsValid(Font))
		{
			UnregisterFont();
		}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ULexText, RichTextImageData))
	{
		if (IsValid(RichTextImageData))//unregister event from prev
		{
			UnregisterOnRichTextImageDataChange();
		}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ULexText, RichTextCustomStyleData))
	{
		if (IsValid(RichTextCustomStyleData))
		{
			UnregisterOnRichTextCustomStyleDataChange();
		}
	}
}
void ULexText::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	auto MemberProperty = PropertyChangedEvent.MemberProperty;
	auto Property = PropertyChangedEvent.Property;
	if (MemberProperty != nullptr && Property != nullptr)
	{
		if (!this->GetName().StartsWith("Default__"))
		{
			auto MemberPropertyName = MemberProperty->GetFName();
			if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(ULexText, Text))
			{
				if (IsValid(Font))
				{
					RegisterFont();
				}
				ConditionalUpdateCacheTextGeometry();
			}
			else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(ULexText, Font))
			{
				ULexText::CurrentUsingFontData = Font;
				ClearEmojiObject();
				ConditionalUpdateCacheTextGeometry();
			}
			else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(ULexText, bUseKerning))
			{
				MarkVertexPositionDirty();
				CacheTextGeometryData.MarkDirty();
			}
			else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(ULexText, bRichText))
			{
				if (bRichText)
				{
					ConditionalUpdateCacheTextGeometry();
				}
				else
				{
					ClearCreatedRichTextImageObject();
				}
			}
			else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(ULexText, RichTextImageData))
			{
				UnregisterOnRichTextImageDataChange();
				if (!IsValid(RichTextImageData))//clear richTextImageData, then need to delete created object
				{
					ClearCreatedRichTextImageObject();
				}
				else
				{
					ConditionalUpdateCacheTextGeometry();
				}
			}
			else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(ULexText, RichTextTagFilterFlags))
			{
				if (!(RichTextTagFilterFlags & (1 << (int)ELexUIText_RichTextTagFilterFlags::Image)))
				{
					ClearCreatedRichTextImageObject();
				}
				else
				{
					ConditionalUpdateCacheTextGeometry();
				}
			}
			else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(ULexText, RichTextCustomStyleData))
			{
				if (IsValid(RichTextCustomStyleData))
				{
					RegisterOnRichTextCustomStyleDataChange();
				}
			}
			ULexWidget::MarkLayoutForRebuild(GetWidget());
		}
	}
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

#endif
void ULexText::RegisterOnRichTextImageDataChange()
{
	RichTextImageDataChangedDelegateHandle = RichTextImageData->OnDataChange.AddWeakLambda(this, [this] {
		this->MarkVerticesDirty(true, true, true, false);
		});
}
void ULexText::UnregisterOnRichTextImageDataChange()
{
	RichTextImageData->OnDataChange.Remove(RichTextImageDataChangedDelegateHandle);
	RichTextImageDataChangedDelegateHandle.Reset();
}

void ULexText::RegisterOnRichTextCustomStyleDataChange()
{
	RichTextCustomStyleDataChangedDelegateHandle = RichTextCustomStyleData->OnDataChange.AddWeakLambda(this, [this] {
		this->MarkVerticesDirty(true, true, true, false);
		});
}
void ULexText::UnregisterOnRichTextCustomStyleDataChange()
{
	RichTextCustomStyleData->OnDataChange.Remove(RichTextCustomStyleDataChangedDelegateHandle);
	RichTextCustomStyleDataChangedDelegateHandle.Reset();
}

bool ULexText::IsTextTruncated()const
{
	UpdateCacheTextGeometry();
	return CacheTextGeometryData.textTruncated;
}



void ULexText::SetFont(ULexUIFontData_BaseObject* Value) {
	if (Font != Value)
	{
		//remove from old
		if (IsValid(Font))
		{
			UnregisterFont();
		}
		Font = Value;

		MarkTextureDirty();
		//add to new
		if (IsValid(Font))
		{
			RegisterFont();
		}
	}
}
void ULexText::SetText(const FText& Value) {
	if (!Text.EqualTo(Value))
	{
		Text = Value;
		MarkVerticesDirty(true, true, true, false);
		if (GetWidget()->GetLayoutSelf())//text size may change by LayoutSelf
		{
			ULexWidget::MarkLayoutForRebuild(GetWidget());
		}
		ConditionalUpdateCacheTextGeometry();
	}
}


void ULexText::SetFontSize(float Value) {
	if (FontSize != Value)
	{
		FontSize = Value;
		MarkVertexPositionDirty();
		if (GetWidget()->GetLayoutSelf())//text size may change by LayoutSelf
		{
			ULexWidget::MarkLayoutForRebuild(GetWidget());
		}
	}
}
void ULexText::SetUseKerning(bool Value)
{
	if (bUseKerning != Value)
	{
		bUseKerning = Value;
		MarkVertexPositionDirty();
		if (GetWidget()->GetLayoutSelf())//text size may change by LayoutSelf
		{
			ULexWidget::MarkLayoutForRebuild(GetWidget());
		}
	}
}
void ULexText::SetFontSpace(FVector2D Value) {
	if (FontSpace != Value)
	{
		MarkVertexPositionDirty();
		FontSpace = Value;
		if (GetWidget()->GetLayoutSelf())//text size may change by LayoutSelf
		{
			ULexWidget::MarkLayoutForRebuild(GetWidget());
		}
	}
}
void ULexText::SetParagraphHorizontalAlignment(ELexUITextParagraphHorizontalAlign Value) {
	if (HAlign != Value)
	{
		MarkVertexPositionDirty();
		HAlign = Value;
		if (GetWidget()->GetLayoutSelf())//text size may change by LayoutSelf
		{
			ULexWidget::MarkLayoutForRebuild(GetWidget());
		}
	}
}
void ULexText::SetParagraphVerticalAlignment(ELexUITextParagraphVerticalAlign Value) {
	if (VAlign != Value)
	{
		MarkVertexPositionDirty();
		VAlign = Value;
		if (GetWidget()->GetLayoutSelf())//text size may change by LayoutSelf
		{
			ULexWidget::MarkLayoutForRebuild(GetWidget());
		}
	}
}
void ULexText::SetOverflowType(ELexUITextOverflowType Value) {
	if (OverflowType != Value)
	{
		if (OverflowType == ELexUITextOverflowType::Truncate
			|| Value == ELexUITextOverflowType::Truncate
			)
			MarkVerticesDirty(true, true, true, true);
		else
			MarkVertexPositionDirty();
		OverflowType = Value;
		if (GetWidget()->GetLayoutSelf())//text size may change by LayoutSelf
		{
			ULexWidget::MarkLayoutForRebuild(GetWidget());
		}
	}
}

void ULexText::SetWrappingPolicy(ETextWrappingPolicy Value)
{
	if (WrappingPolicy != Value)
	{
		WrappingPolicy = Value;
		MarkVertexPositionDirty();
		if (GetWidget()->GetLayoutSelf())//text size may change by LayoutSelf
		{
			ULexWidget::MarkLayoutForRebuild(GetWidget());
		}
	}
}

void ULexText::SetFontStyle(ELexUITextFontStyle Value) {
	if (FontStyle != Value)
	{
		if ((FontStyle == ELexUITextFontStyle::None || FontStyle == ELexUITextFontStyle::Italic)
			&& (Value == ELexUITextFontStyle::None || Value == ELexUITextFontStyle::Italic))//these only affect vertex position
		{
			MarkVertexPositionDirty();
		}
		else
		{
			MarkVerticesDirty(true, true, true, true);
		}
		FontStyle = Value;
		if (GetWidget()->GetLayoutSelf())//text size may change by LayoutSelf
		{
			ULexWidget::MarkLayoutForRebuild(GetWidget());
		}
	}
}
void ULexText::SetRichText(bool Value)
{
	if (bRichText != Value)
	{
		MarkVerticesDirty(true, true, true, true);
		bRichText = Value;
		if (!bRichText)
		{
			ClearCreatedRichTextImageObject();
		}
		if (GetWidget()->GetLayoutSelf())//text size may change by LayoutSelf
		{
			ULexWidget::MarkLayoutForRebuild(GetWidget());
		}
	}
}
void ULexText::SetRichTextTagFilterFlags(int32 Value)
{
	if (RichTextTagFilterFlags != Value)
	{
		MarkVerticesDirty(true, true, true, true);
		RichTextTagFilterFlags = Value;
		if (!(RichTextTagFilterFlags & (1 << (int)ELexUIText_RichTextTagFilterFlags::Image)))
		{
			ClearCreatedRichTextImageObject();
		}
		if (GetWidget()->GetLayoutSelf())//text size may change by LayoutSelf
		{
			ULexWidget::MarkLayoutForRebuild(GetWidget());
		}
	}
}
void ULexText::SetRichTextImageData(ULexUIRichTextImageData_BaseObject* Value)
{
	if (RichTextImageData != Value)
	{
		MarkVerticesDirty(true, true, true, true);
		RichTextImageData = Value;
		if (!IsValid(RichTextImageData))//clear richTextImageData, then need to delete created object
		{
			ClearCreatedRichTextImageObject();
		}
		if (GetWidget()->GetLayoutSelf())//text size may change by LayoutSelf
		{
			ULexWidget::MarkLayoutForRebuild(GetWidget());
		}
	}
}
void ULexText::SetRichTextCustomStyleData(ULexUIRichTextCustomStyleData* Value)
{
	if (RichTextCustomStyleData != Value)
	{
		MarkVerticesDirty(true, true, true, true);
		RichTextCustomStyleData = Value;
		if (GetWidget()->GetLayoutSelf())//text size may change by LayoutSelf
		{
			ULexWidget::MarkLayoutForRebuild(GetWidget());
		}
	}
}

void ULexText::SetOverrideMaterial(UMaterialInterface* Value)
{
	if (OverrideMaterial != Value)
	{
		OverrideMaterial = Value;
		MarkMaterialDirty();
	}
}

void ULexText::SetExpandMeshSize(float Value)
{
	if (ExpandMeshSize != Value)
	{
		ExpandMeshSize = Value;
		MarkVerticesDirty(false, true, true, false);
	}
}
void ULexText::SetDynamicPixelsPerUnit(float Value)
{
	if (DynamicPixelsPerUnit != Value)
	{
		DynamicPixelsPerUnit = Value;
		MarkVerticesDirty(false, true, true, false);
	}
}

void ULexText::ClearCreatedRichTextImageObject()
{
	for (auto& ImageObj : CreatedRichTextImageObjectArray)
	{
		if (IsValid(ImageObj))
		{
			ImageObj->DestroyWidget();
		}
	}
	CreatedRichTextImageObjectArray.Empty();
}

void ULexText::ClearEmojiObject()
{
	for (auto& ItemObj : CreatedEmojiObjectArray)
	{
		if (IsValid(ItemObj))
		{
			ItemObj->DestroyWidget();
		}
	}
	CreatedEmojiObjectArray.Empty();
}

void ULexText::RegisterFont()
{
	if (!bHasAddToFont)
	{
		bHasAddToFont = true;
		Font->AddUIText(this);
		EmojiDataChangedDelegateHandle = Font->OnEmojiDataChanged.AddWeakLambda(this, [=, this]()
		{
			ClearEmojiObject();
			MarkVerticesDirty(true, true, true, true);
		});
	}
}

void ULexText::UnregisterFont()
{
	if (bHasAddToFont)
	{
		bHasAddToFont = false;
		Font->RemoveUIText(this);
		Font->OnEmojiDataChanged.Remove(EmojiDataChangedDelegateHandle);
		EmojiDataChangedDelegateHandle.Reset();
	}
}

void ULexText::UpdateCacheTextGeometry()const
{
	if (!IsValid(this->GetFont()))return;
	auto Widget = GetWidget();
	
	auto RenderOpacityForRichText = this->GetRichText() ? Widget->GetFinalRenderOpacity() : 1.0f;
	CacheTextGeometryData.SetInputParameters(
		this->Text.ToString()
		, Widget->GetWidth()
		, Widget->GetHeight()
		, FVector2f(Widget->GetPivot())
		, this->GetFinalColor()
		, RenderOpacityForRichText
		, FVector2f(this->GetFontSpace())
		, this->GetFontSize()
		, this->GetParagraphHorizontalAlignment()
		, this->GetParagraphVerticalAlignment()
		, this->GetOverflowType()
		, this->GetWrappingPolicy()
		, this->GetUseKerning()
		, this->GetFontStyle()
		, this->GetRichText()
		, this->GetRichTextTagFilterFlags()
		, this->GetFont()
	);
	if (UIGeometry->Vertices.Num() == 0)//@todo: geometry is cleared before OnUpdateGeometry, consider use a cached UIGeometry
	{
		CacheTextGeometryData.MarkDirty();
	}
	CacheTextGeometryData.ConditionalCalculateGeometry();
}

void ULexText::ConditionalUpdateCacheTextGeometry() const
{
	/**
	 * RichTextImageData and EmojiData could cause create or delete widget, so we should make it happen before Canvas-Update,
	 * because unexpected thing will happed if we create or delete widget during Canvas-Update.
	 */
	if (IsValid(RichTextImageData) || (IsValid(Font) && IsValid(Font->GetEmojiData())))
	{
		UpdateCacheTextGeometry();
	}
}

void ULexText::MarkVerticesDirty(bool InTriangleDirty, bool InVertexPositionDirty, bool InVertexUVDirty, bool InVertexColorDirty)
{
	CacheTextGeometryData.MarkDirty();
	Super::MarkVerticesDirty(InTriangleDirty, InVertexPositionDirty, InVertexUVDirty, InVertexColorDirty);
}
void ULexText::MarkTextureDirty()
{
	CacheTextGeometryData.MarkDirty();
	Super::MarkTextureDirty();
}

void ULexText::MarkAllDirty()
{
	CacheTextGeometryData.MarkDirty();
	Super::MarkAllDirty();
}
int ULexText::VisibleCharCountInString(const FString& srcStr)
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

const TArray<FLexUITextCharProperty>& ULexText::GetCharPropertyArray()const
{
	UpdateCacheTextGeometry();
	return CacheTextGeometryData.cacheCharPropertyArray;
}
int32 ULexText::GetVisibleCharCount()const
{
	UpdateCacheTextGeometry();
	return CacheTextGeometryData.cacheCharPropertyArray.Num();
}
const TArray<FLexUIText_RichTextCustomTag>& ULexText::GetRichTextCustomTagArray()const
{
	UpdateCacheTextGeometry();
	return CacheTextGeometryData.cacheRichTextCustomTagArray;
}
const TArray<FLexUIText_RichTextImageTag>& ULexText::GetRichTextImageTagArray()const
{
	UpdateCacheTextGeometry();
	return CacheTextGeometryData.cacheRichTextImageTagArray;
}

void ULexText::GenerateRichTextImageObject()
{
	if (!IsValid(RichTextImageData))return;
	RichTextImageData->CreateOrUpdateObject(this->GetWidget(), CacheTextGeometryData.cacheRichTextImageTagArray, CreatedRichTextImageObjectArray);
}

void ULexText::GenerateEmojiObject()
{
	if (auto EmojiData = Font->GetEmojiData())
	{
		EmojiData->CreateOrUpdateObject(this->GetWidget(), CacheTextGeometryData.cacheEmojiArray, CreatedEmojiObjectArray);
	}
}

float ULexText::GetPreferredWidth() const
{
	UpdateCacheTextGeometry();
	return CacheTextGeometryData.textPreferredSize.X;
}

float ULexText::GetPreferredHeight() const
{
	UpdateCacheTextGeometry();
	return CacheTextGeometryData.textPreferredSize.Y;
}


bool ULexText::MoveCaret(int32 moveType, int32& inOutCaretPositionIndex, int32& inOutCaretPositionLineIndex, FVector2f& inOutCaretPosition)
{
	auto originCaretPositionIndex = inOutCaretPositionIndex;
	auto originCaretPositionLineIndex = inOutCaretPositionLineIndex;

	UpdateCacheTextGeometry();
	auto& cacheLinePropertyArray = CacheTextGeometryData.cacheLinePropertyArray;
	//moveType 0-left, 1-right, 2-up, 3-down, 4-start, 5-end
	switch (moveType)
	{
	case 0:
	case 1:
	{
		if (moveType == 0)
		{
			if (inOutCaretPositionIndex > 0)
			{
				inOutCaretPositionIndex--;
			}
		}
		else
		{
			inOutCaretPositionIndex++;
		}

		bool foundCaret = false;
		int totalCaretIndex = 0;
		for (int lineIndex = 0; lineIndex < cacheLinePropertyArray.Num(); lineIndex++)
		{
			auto& lineProperty = cacheLinePropertyArray[lineIndex];
			for (int caretIndex = 0; caretIndex < lineProperty.CaretPropertyList.Num(); caretIndex++)
			{
				if (totalCaretIndex == inOutCaretPositionIndex)//find caret
				{
					inOutCaretPositionLineIndex = lineIndex;
					inOutCaretPosition = lineProperty.CaretPropertyList[caretIndex].CaretPosition;
					//stop loop
					foundCaret = true;
					caretIndex = lineProperty.CaretPropertyList.Num();
					lineIndex = cacheLinePropertyArray.Num();
				}
				else
				{
					totalCaretIndex++;
				}
			}
		}
		if (!foundCaret)//could be out of range, use last caret
		{
			inOutCaretPositionIndex = totalCaretIndex - 1;
			inOutCaretPositionLineIndex = cacheLinePropertyArray.Num() - 1;
			auto& lastLineProperty = cacheLinePropertyArray[cacheLinePropertyArray.Num() - 1];
			inOutCaretPosition = lastLineProperty.CaretPropertyList[lastLineProperty.CaretPropertyList.Num() - 1].CaretPosition;
		}
	}
	break;
	case 2:
	case 3:
	{
		if (moveType == 2)
		{
			if (inOutCaretPositionLineIndex > 0)
			{
				inOutCaretPositionLineIndex--;
			}
		}
		else
		{
			if (inOutCaretPositionLineIndex < cacheLinePropertyArray.Num() - 1)
			{
				inOutCaretPositionLineIndex++;
			}
		}
		auto& lineProperty = cacheLinePropertyArray[inOutCaretPositionLineIndex];
		float minDistance = MAX_FLT;
		int accumulatedCaretIndex = 0;
		for (int lineIndex = 0; lineIndex < inOutCaretPositionLineIndex; lineIndex++)
		{
			accumulatedCaretIndex += cacheLinePropertyArray[lineIndex].CaretPropertyList.Num();
		}
		auto originCaretPosition = inOutCaretPosition;
		for (int caretIndex = 0; caretIndex < lineProperty.CaretPropertyList.Num(); caretIndex++)
		{
			auto& caretProperty = lineProperty.CaretPropertyList[caretIndex];
			auto distance = FMath::Abs(originCaretPosition.X - caretProperty.CaretPosition.X);
			if (distance < minDistance)
			{
				minDistance = distance;
				inOutCaretPositionIndex = accumulatedCaretIndex;
				inOutCaretPosition = caretProperty.CaretPosition;
			}
			else//found min distance at prev
			{
				break;
			}
			accumulatedCaretIndex++;
		}
	}
	break;
	case 4:
	{
		inOutCaretPositionIndex = 0;
		inOutCaretPositionLineIndex = 0;
		inOutCaretPosition = cacheLinePropertyArray[0].CaretPropertyList[0].CaretPosition;
	}
	break;
	case 5:
	{
		int32 accumulatedCaretIndex = 0;
		for (int lineIndex = 0; lineIndex < cacheLinePropertyArray.Num(); lineIndex++)
		{
			accumulatedCaretIndex += cacheLinePropertyArray[lineIndex].CaretPropertyList.Num();
		}
		inOutCaretPositionIndex = accumulatedCaretIndex - 1;
		inOutCaretPositionLineIndex = cacheLinePropertyArray.Num() - 1;
		auto& lastLineProperty = cacheLinePropertyArray[cacheLinePropertyArray.Num() - 1];
		inOutCaretPosition = lastLineProperty.CaretPropertyList[lastLineProperty.CaretPropertyList.Num() - 1].CaretPosition;
	}
	break;
	}
	if (originCaretPositionIndex != inOutCaretPositionIndex || originCaretPositionLineIndex != inOutCaretPositionLineIndex)
	{
		return true;
	}
	return false;
}

int ULexText::GetCharIndexByCaretIndex(int32 inCaretPositionIndex)
{
	UpdateCacheTextGeometry();
	auto& cacheLinePropertyArray = CacheTextGeometryData.cacheLinePropertyArray;
	int accumulatedCaretIndex = 0;
	for (int lineIndex = 0; lineIndex < cacheLinePropertyArray.Num(); lineIndex++)
	{
		auto& lineProperty = cacheLinePropertyArray[lineIndex];
		for (int caretIndex = 0; caretIndex < lineProperty.CaretPropertyList.Num(); caretIndex++)
		{
			if (accumulatedCaretIndex == inCaretPositionIndex)//find caret
			{
				return lineProperty.CaretPropertyList[caretIndex].CharIndex;
			}
			accumulatedCaretIndex++;
		}
	}
	//not found caret, use last one
	auto& lastLineProperty = cacheLinePropertyArray[cacheLinePropertyArray.Num() - 1];
	return lastLineProperty.CaretPropertyList[lastLineProperty.CaretPropertyList.Num() - 1].CharIndex;
}
int ULexText::GetLastCaret()
{
	UpdateCacheTextGeometry();
	auto& cacheLinePropertyArray = CacheTextGeometryData.cacheLinePropertyArray;
	int totalCaretIndex = 0;
	for (int lineIndex = 0; lineIndex < cacheLinePropertyArray.Num(); lineIndex++)
	{
		auto& lineProperty = cacheLinePropertyArray[lineIndex];
		totalCaretIndex += lineProperty.CaretPropertyList.Num();
	}
	return totalCaretIndex - 1;
}
//caret is at left side of char
void ULexText::FindCaretByIndex(int32& inOutCaretPositionIndex, FVector2f& outCaretPosition, int32& outCaretPositionLineIndex, int32& outVisibleCaretStartIndex)
{
	UpdateCacheTextGeometry();
	auto& cacheLinePropertyArray = CacheTextGeometryData.cacheLinePropertyArray;

	auto Widget = GetWidget();
	if (inOutCaretPositionIndex < 0)inOutCaretPositionIndex = 0;
	outCaretPosition.X = outCaretPosition.Y = 0;
	outCaretPositionLineIndex = 0;
	outVisibleCaretStartIndex = 0;
	if (cacheLinePropertyArray.Num() == 0)
	{
		float pivotOffsetX = Widget->GetWidth() * (0.5f - Widget->GetPivot().X);
		float pivotOffsetY = Widget->GetHeight() * (0.5f - Widget->GetPivot().Y);
		switch (HAlign)
		{
		case ELexUITextParagraphHorizontalAlign::Left:
		{
			outCaretPosition.X = pivotOffsetX - Widget->GetWidth() * 0.5f;
		}
			break;
		case ELexUITextParagraphHorizontalAlign::Center:
		{
			outCaretPosition.X = pivotOffsetX;
		}
			break;
		case ELexUITextParagraphHorizontalAlign::Right:
		{
			outCaretPosition.X = pivotOffsetX + Widget->GetWidth() * 0.5f;
		}
			break;
		}
		switch (VAlign)
		{
		case ELexUITextParagraphVerticalAlign::Top:
		{
			outCaretPosition.Y = pivotOffsetY + Widget->GetHeight() * 0.5f - FontSize * 0.5f;//fixed offset
		}
			break;
		case ELexUITextParagraphVerticalAlign::Middle:
		{
			outCaretPosition.Y = pivotOffsetY;
		}
			break;
		case ELexUITextParagraphVerticalAlign::Bottom:
		{
			outCaretPosition.Y = pivotOffsetY - Widget->GetHeight() * 0.5f + FontSize * 0.5f;//fixed offset
		}
			break;
		}
	}
	else
	{
		if (inOutCaretPositionIndex == 0)//first char
		{
			outCaretPosition = cacheLinePropertyArray[0].CaretPropertyList[0].CaretPosition;
			outCaretPositionLineIndex = 0;
			outVisibleCaretStartIndex = 0;
		}
		else//not first char
		{
			bool foundCaret = false;
			int accumulatedCaretIndex = 0;
			for (int lineIndex = 0; lineIndex < cacheLinePropertyArray.Num(); lineIndex++)
			{
				auto& lineProperty = cacheLinePropertyArray[lineIndex];
				for (int caretIndex = 0; caretIndex < lineProperty.CaretPropertyList.Num(); caretIndex++)
				{
					if (accumulatedCaretIndex == inOutCaretPositionIndex)//find caret
					{
						outCaretPositionLineIndex = lineIndex;
						outCaretPosition = lineProperty.CaretPropertyList[caretIndex].CaretPosition;
						outVisibleCaretStartIndex = accumulatedCaretIndex;
						//stop loop
						foundCaret = true;
						caretIndex = lineProperty.CaretPropertyList.Num();
						lineIndex = cacheLinePropertyArray.Num();
					}
					else
					{
						accumulatedCaretIndex++;
					}
				}
			}
			if (!foundCaret)//could be out of range
			{
				auto& lastLineProperty = cacheLinePropertyArray[cacheLinePropertyArray.Num() - 1];
				inOutCaretPositionIndex = accumulatedCaretIndex - 1;
				outCaretPosition = lastLineProperty.CaretPropertyList[lastLineProperty.CaretPropertyList.Num() - 1].CaretPosition;
				outCaretPositionLineIndex = cacheLinePropertyArray.Num() - 1;
				outVisibleCaretStartIndex = 0;
			}
		}
	}
}
void ULexText::FindCaret(FVector2f& inOutCaretPosition, int32 inCaretPositionLineIndex, int32& outCaretPositionIndex)
{
	if (Text.ToString().Len() == 0)//no text
		return;
	UpdateCacheTextGeometry();
	auto& cacheTextPropertyArray = CacheTextGeometryData.cacheLinePropertyArray;
	auto lineCount = cacheTextPropertyArray.Num();//line count
	outCaretPositionIndex = 0;

	//find nearest char to caret from this line
	auto& lineItem = cacheTextPropertyArray[inCaretPositionLineIndex];
	int charPropertyCount = lineItem.CaretPropertyList.Num();//char count of this line
	float nearestDistance = MAX_FLT;
	int32 nearestIndex = -1;
	for (int charPropertyIndex = 0; charPropertyIndex < charPropertyCount; charPropertyIndex++)
	{
		auto& charItem = lineItem.CaretPropertyList[charPropertyIndex];
		float distance = FMath::Abs(charItem.CaretPosition.X - inOutCaretPosition.X);
		if (distance <= nearestDistance)
		{
			nearestDistance = distance;
			nearestIndex = charPropertyIndex;
			outCaretPositionIndex = charItem.CharIndex;
		}
	}
	inOutCaretPosition = lineItem.CaretPropertyList[nearestIndex].CaretPosition;
}
//find caret by position, caret is on left side of char
void ULexText::FindCaretByWorldPosition(FVector inWorldPosition, FVector2f& outCaretPosition, int32& outCaretPositionLineIndex, int32& outCaretPositionIndex)
{
	if (Text.ToString().Len() == 0)//no text
	{
		outCaretPositionIndex = 0;
		int tempVisibleCharStartIndex = 0;
		FindCaretByIndex(outCaretPositionIndex, outCaretPosition, outCaretPositionLineIndex, tempVisibleCharStartIndex);
	}
	else
	{
		UpdateCacheTextGeometry();
		auto& cacheLinePropertyArray = CacheTextGeometryData.cacheLinePropertyArray;

		auto localPosition = GetWidget()->GetWorldTransform().InverseTransformPosition(inWorldPosition);
		auto localPosition2D = FVector2f(localPosition.Y, localPosition.Z);

		float nearestDistance = MAX_FLT;
		int accumulatedCaretIndex = 0;
		//find the nearest line, only need to compare Y
		int foundLineIndex = -1;
		for (int lineIndex = 0; lineIndex < cacheLinePropertyArray.Num(); lineIndex++)
		{
			auto& lineItem = cacheLinePropertyArray[lineIndex];
			float distance = FMath::Abs(lineItem.CaretPropertyList[0].CaretPosition.Y - localPosition2D.Y);
			if (distance <= nearestDistance)
			{
				nearestDistance = distance;
				outCaretPositionLineIndex = lineIndex;
				accumulatedCaretIndex += lineItem.CaretPropertyList.Num();
			}
			else
			{
				foundLineIndex = lineIndex - 1;
				break;
			}
		}
		if (foundLineIndex == -1)
		{
			foundLineIndex = cacheLinePropertyArray.Num() - 1;
		}
		accumulatedCaretIndex -= cacheLinePropertyArray[foundLineIndex].CaretPropertyList.Num();//remove prev line's caret count, because we need to add it when compare X pos
		//then find nearest char, only need to compare X
		nearestDistance = MAX_FLT;
		auto& nearestLine = cacheLinePropertyArray[outCaretPositionLineIndex];
		for (int caretIndex = 0; caretIndex < nearestLine.CaretPropertyList.Num(); caretIndex++)
		{
			auto& caretItem = nearestLine.CaretPropertyList[caretIndex];
			float distance = FMath::Abs(caretItem.CaretPosition.X - localPosition2D.X);
			if (distance <= nearestDistance)
			{
				nearestDistance = distance;
				outCaretPositionIndex = accumulatedCaretIndex + caretIndex;
				outCaretPosition = caretItem.CaretPosition;
			}
			else
			{
				break;
			}
		}
	}
}

int ULexText::GetCaretIndexByCharIndex(int32 inCharIndex)
{
	UpdateCacheTextGeometry();
	int accumulatedCaretIndex = 0;
	auto& cacheLinePropertyArray = CacheTextGeometryData.cacheLinePropertyArray;
	for (int lineIndex = 0; lineIndex < cacheLinePropertyArray.Num(); lineIndex++)
	{
		auto& lineProperty = cacheLinePropertyArray[lineIndex];
		for (int caretIndex = 0; caretIndex < lineProperty.CaretPropertyList.Num(); caretIndex++)
		{
			if (lineProperty.CaretPropertyList[caretIndex].CharIndex == inCharIndex)//find char
			{
				return accumulatedCaretIndex;
			}
			else
			{
				accumulatedCaretIndex++;
			}
		}
	}
	return accumulatedCaretIndex - 1;//not found, return last one
}

bool ULexText::GetVisibleCharRangeForMultiLine(int32& inOutCaretPositionIndex, int32& inOutCaretPositionLineIndex, int32& inOutVisibleCaretStartLineIndex, int32& inOutVisibleCaretStartIndex, int inMaxLineCount, int32& outVisibleCharStartIndex, int32& outVisibleCharCount)
{
	UpdateCacheTextGeometry();
	auto& cacheLinePropertyArray = CacheTextGeometryData.cacheLinePropertyArray;
	int accumulatedCaretIndex = 0;
	bool foundCaret = false;
	for (int lineIndex = 0; lineIndex < cacheLinePropertyArray.Num(); lineIndex++)
	{
		auto& lineProperty = cacheLinePropertyArray[lineIndex];
		for (int caretIndex = 0; caretIndex < lineProperty.CaretPropertyList.Num(); caretIndex++)
		{
			if (inOutCaretPositionIndex == accumulatedCaretIndex)//find caret
			{
				inOutCaretPositionLineIndex = lineIndex;
				lineIndex = cacheLinePropertyArray.Num();
				foundCaret = true;
				break;
			}
			else
			{
				accumulatedCaretIndex++;
			}
		}
	}
	if (!foundCaret)//could be last caret
	{
		inOutCaretPositionLineIndex = cacheLinePropertyArray.Num() - 1;
	}

	inOutCaretPositionLineIndex = FMath::Clamp(inOutCaretPositionLineIndex, 0, cacheLinePropertyArray.Num() - 1);

	if (inOutVisibleCaretStartLineIndex > inOutCaretPositionLineIndex)
	{
		inOutVisibleCaretStartLineIndex = inOutCaretPositionLineIndex;
	}
	if (inOutVisibleCaretStartLineIndex + (inMaxLineCount - 1) < inOutCaretPositionLineIndex)
	{
		inOutVisibleCaretStartLineIndex = inOutCaretPositionLineIndex - (inMaxLineCount - 1);
	}

	int calculatedLineCount = 0;
	bool outOfRange = false;
	int VisibleCaretEndLineIndex = inOutCaretPositionLineIndex;
	//check from CaretLineIndex to VisibleCaretStartLineIndex
	for (int lineIndex = inOutCaretPositionLineIndex; lineIndex >= 0 && lineIndex >= inOutVisibleCaretStartLineIndex; lineIndex--)
	{
		auto& lineProperty = cacheLinePropertyArray[lineIndex];
		calculatedLineCount++;
		if (calculatedLineCount >= inMaxLineCount)
		{
			outOfRange = true;
			inOutVisibleCaretStartLineIndex = lineIndex;
			break;
		}
	}
	if (!outOfRange)
	{
		//check from CaretLineIndex to bottom end
		for (int lineIndex = inOutCaretPositionLineIndex + 1; lineIndex < cacheLinePropertyArray.Num(); lineIndex++)
		{
			auto& lineProperty = cacheLinePropertyArray[lineIndex];
			calculatedLineCount++;
			VisibleCaretEndLineIndex++;
			if (calculatedLineCount >= inMaxLineCount)
			{
				outOfRange = true;
				break;
			}
		}

		if (!outOfRange)
		{
			//check from VisibleCaretStartLineIndex to top
			for (int lineIndex = inOutVisibleCaretStartLineIndex - 1; lineIndex >= 0 && lineIndex < cacheLinePropertyArray.Num(); lineIndex--)
			{
				auto& lineProperty = cacheLinePropertyArray[lineIndex];
				calculatedLineCount++;
				if (calculatedLineCount >= inMaxLineCount)
				{
					outOfRange = true;
					break;
				}
				inOutVisibleCaretStartLineIndex--;
			}
		}
	}
	inOutVisibleCaretStartIndex = 0;
	for (int lineIndex = 0; lineIndex < inOutVisibleCaretStartLineIndex; lineIndex++)
	{
		auto& lineProperty = cacheLinePropertyArray[lineIndex];
		inOutVisibleCaretStartIndex += lineProperty.CaretPropertyList.Num();
	}
	auto& startLineProperty = cacheLinePropertyArray[inOutVisibleCaretStartLineIndex];
	auto& endLineProperty = cacheLinePropertyArray[VisibleCaretEndLineIndex];
	outVisibleCharStartIndex = startLineProperty.CaretPropertyList[0].CharIndex;
	auto lastIndex = endLineProperty.CaretPropertyList.Num() - 1;
	auto lastCharIndex = endLineProperty.CaretPropertyList[lastIndex].CharIndex;
	if (lastCharIndex == -1)//-1 means newline break, so use next caret's char index
	{
		lastCharIndex = endLineProperty.CaretPropertyList[lastIndex - 1].CharIndex + 1;
	}
	outVisibleCharCount = lastCharIndex - outVisibleCharStartIndex;
	return outOfRange;
}

void ULexText::GetSelectionProperty(int32 InSelectionStartCaretIndex, int32 InSelectionEndCaretIndex, TArray<FLexUITextSelectionProperty>& OutSelectionProeprtyArray)
{
	OutSelectionProeprtyArray.Reset();
	UpdateCacheTextGeometry();
	auto& cacheTextPropertyArray = CacheTextGeometryData.cacheLinePropertyArray;
	//start
	FVector2f startCaretPosition;
	int32 startCaretPositionLineIndex;
	int visibleCharStartIndex = 0;
	FindCaretByIndex(InSelectionStartCaretIndex, startCaretPosition, startCaretPositionLineIndex, visibleCharStartIndex);
	//end
	FVector2f endCaretPosition;
	int32 endCaretPositionLineIndex;
	FindCaretByIndex(InSelectionEndCaretIndex, endCaretPosition, endCaretPositionLineIndex, visibleCharStartIndex);
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
		FLexUITextSelectionProperty selectionProperty;
		selectionProperty.Pos = startCaretPosition;
		selectionProperty.Size = endCaretPosition.X - startCaretPosition.X;
		OutSelectionProeprtyArray.Add(selectionProperty);
	}
	else//different line
	{
		//first line
		FLexUITextSelectionProperty selectionProperty;
		selectionProperty.Pos = startCaretPosition;
		auto& firstLineCharPropertyList = cacheTextPropertyArray[startCaretPositionLineIndex].CaretPropertyList;
		auto& firstLineLastCharProperty = firstLineCharPropertyList[firstLineCharPropertyList.Num() - 1];
		selectionProperty.Size = FMath::RoundToInt(firstLineLastCharProperty.CaretPosition.X - startCaretPosition.X);
		//selectionProperty.Size = (1.0f - this->GetPivot().X) * this->GetWidth() - startCaretPosition.X;
		OutSelectionProeprtyArray.Add(selectionProperty);
		//middle line, use this->GetWidth() as size
		int middleLineCount = endCaretPositionLineIndex - startCaretPositionLineIndex - 1;
		for (int i = 0; i < middleLineCount; i++)
		{
			auto& charPropertyList = cacheTextPropertyArray[startCaretPositionLineIndex + i + 1].CaretPropertyList;
			auto& firstPosition = charPropertyList[0].CaretPosition;
			auto& lasPosition = charPropertyList[charPropertyList.Num() - 1].CaretPosition;
			selectionProperty.Pos = firstPosition;
			selectionProperty.Size = FMath::RoundToInt(lasPosition.X - firstPosition.X);
			OutSelectionProeprtyArray.Add(selectionProperty);
		}
		//end line
		auto& firstPosition = cacheTextPropertyArray[endCaretPositionLineIndex].CaretPropertyList[0].CaretPosition;
		selectionProperty.Pos = firstPosition;
		selectionProperty.Size = FMath::RoundToInt(endCaretPosition.X - firstPosition.X);
		OutSelectionProeprtyArray.Add(selectionProperty);
	}
}

#undef LOCTEXT_NAMESPACE


