// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UIText.h"
#include "LGUI.h"
#include "Core/UIGeometry.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Materials/MaterialInterface.h"
#include "Core/LGUIFontData_BaseObject.h"
#include "Core/LGUIRichTextImageData_BaseObject.h"
#include "Core/LGUIRichTextCustomStyleData.h"
#include "Core/UIDrawcall.h"
#include "Core/LGUIManager.h"
#include "PrefabSystem/LGUIPrefabManager.h"
#include "Utils/LGUIUtils.h"
#include "Core/ActorComponent/UICanvasGroup.h"

#if LGUI_CAN_DISABLE_OPTIMIZATION
UE_DISABLE_OPTIMIZATION
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
		CheckAdditionalShaderChannels();//@todo: looks this line is not necessary
		if (!bHasAddToFont)
		{
			font->AddUIText(this);
			bHasAddToFont = true;
		}
	}
	if (IsValid(richTextImageData))
	{
		this->RegisterOnRichTextImageDataChange();
	}
	if (IsValid(richTextCustomStyleData))
	{
		this->RegisterOnRichTextCustomStyleDataChange();
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
	if (IsValid(richTextImageData))
	{
		this->UnregisterOnRichTextImageDataChange();
	}
	if (IsValid(richTextCustomStyleData))
	{
		this->UnregisterOnRichTextCustomStyleDataChange();
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
			if (!onRichTextImageDataChangedDelegateHandle.IsValid())
			{
				if (IsValid(richTextImageData))
				{
					this->RegisterOnRichTextImageDataChange();
				}
			}
			if (!onRichTextCustomStyleDataChangedDelegateHandle.IsValid())
			{
				if (IsValid(richTextCustomStyleData))
				{
					this->RegisterOnRichTextCustomStyleDataChange();
				}
			}
		}
		else
#endif
		{
			ULGUIManagerWorldSubsystem::RegisterLGUICultureChangedEvent(this);
		}
		ULGUIManagerWorldSubsystem::RegisterLGUILayout(this);
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
			if (IsValid(richTextImageData))
			{
				if (onRichTextImageDataChangedDelegateHandle.IsValid())
				{
					this->UnregisterOnRichTextImageDataChange();
				}
			}
			if (IsValid(richTextCustomStyleData))
			{
				if (onRichTextCustomStyleDataChangedDelegateHandle.IsValid())
				{
					this->UnregisterOnRichTextCustomStyleDataChange();
				}
			}
		}
		else
#endif
		{
			ULGUIManagerWorldSubsystem::UnregisterLGUICultureChangedEvent(this);
		}
		ULGUIManagerWorldSubsystem::UnregisterLGUILayout(this);
	}
}
void UUIText::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	Super::OnComponentDestroyed(bDestroyingHierarchy);
	for (int i = 0; i < createdRichTextImageObjectArray.Num(); i++)
	{
		auto item = createdRichTextImageObjectArray[i];
		if (IsValid(item) && IsValid(item->GetOwner()))
		{
			LGUIUtils::DestroyActorWithHierarchy(item->GetOwner());
		}
	}
	createdRichTextImageObjectArray.Empty();
}
void UUIText::OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport)
{
	Super::OnUpdateTransform(UpdateTransformFlags, Teleport);
	if (IsValid(font) && font->GetNeedObjectScale())//some font need object scale (SDF font), so detect scale change and mark update
	{
		auto CompScale3D = this->GetComponentScale();
		auto CompScale2D = FVector2f(CompScale3D.Y, CompScale3D.Z);
		if (!PrevScale2DForUIText.Equals(CompScale2D))
		{
			PrevScale2DForUIText = CompScale2D;
			MarkUVDirty();//object scale value is stored in uv2. @todo: this should defined in font
		}
	}
}

void UUIText::OnAnchorChange(bool InPivotChange, bool InWidthChange, bool InHeightChange, bool InDiscardCache)
{
    Super::OnAnchorChange(InPivotChange, InWidthChange, InHeightChange, InDiscardCache);
    if (InPivotChange || InWidthChange || InHeightChange)
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
	CheckAdditionalShaderChannels();
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
		CheckAdditionalShaderChannels();
		auto CanvasClipType = ELGUICanvasClipType::None;
		if (this->GetRenderCanvas() != nullptr)
		{
			CanvasClipType = this->GetRenderCanvas()->GetActualClipType();
			if (CanvasClipType == ELGUICanvasClipType::Custom)
			{
				CanvasClipType = ELGUICanvasClipType::None;
			}
		}
		return font->GetFontMaterial(CanvasClipType);
	}
}

void UUIText::CheckAdditionalShaderChannels()
{
	if (RenderCanvas.IsValid())
	{
		auto flags = font->GetRequireAdditionalShaderChannels();
#if WITH_EDITOR
		auto originFlags = RenderCanvas->GetActualAdditionalShaderChannelFlags();
		if (flags != 0 && (originFlags & flags) == 0)
		{
			auto MsgText = FText::Format(LOCTEXT("FontChangeAddtionalShaderChannels"
				, "{0} Automatically change 'AdditionalShaderChannels' property for LGUICanvas because font need it, font object: '{1}'")
				, FText::FromString(FString::Printf(TEXT("[%s].%d"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__))
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
	if (richText && !onRichTextImageDataChangedDelegateHandle.IsValid())
	{
		if (IsValid(richTextImageData))
		{
			this->RegisterOnRichTextImageDataChange();
		}
	}
	if (richText && !onRichTextCustomStyleDataChangedDelegateHandle.IsValid())
	{
		if (IsValid(richTextCustomStyleData))
		{
			this->RegisterOnRichTextCustomStyleDataChange();
		}
	}
	if (visibleCharCount == -1)visibleCharCount = VisibleCharCountInString(text.ToString());
}

bool UUIText::GetShouldAffectByPixelPerfect()const
{
	if (IsValid(font))
	{
		return font->GetShouldAffectByPixelPerfect();
	}
	return Super::GetShouldAffectByPixelPerfect();
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
	auto originText = text;
	text = FText::GetEmpty();//just make it work, because SetText will compare text value
	SetText(originText);
}


#if WITH_EDITOR
void UUIText::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	auto MemberProperty = PropertyChangedEvent.MemberProperty;
	auto Property = PropertyChangedEvent.Property;
	if (MemberProperty != nullptr && Property != nullptr)
	{
		auto PropertyName = Property->GetFName();
		auto MemberPropertyName = MemberProperty->GetFName();
		if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UUIText, text))
		{
			
		}
		else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UUIText, font))
		{
			UUIText::CurrentUsingFontData = font;
		}
		else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UUIText, useKerning))
		{
			MarkVertexPositionDirty();
			CacheTextGeometryData.MarkDirty();
		}
		else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UUIText, adjustWidthRange))
		{
			if (PropertyName == "X")
			{
				if (adjustWidthRange.Y < adjustWidthRange.X)
				{
					adjustWidthRange.Y = adjustWidthRange.X;
				}
			}
			else if (PropertyName == "Y")
			{
				if (adjustWidthRange.X > adjustWidthRange.Y)
				{
					adjustWidthRange.X = adjustWidthRange.Y;
				}
			}
		}
		else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UUIText, adjustHeightRange))
		{
			if (PropertyName == "X")
			{
				if (adjustHeightRange.Y < adjustHeightRange.X)
				{
					adjustHeightRange.Y = adjustHeightRange.X;
				}
			}
			else if (PropertyName == "Y")
			{
				if (adjustHeightRange.X > adjustHeightRange.Y)
				{
					adjustHeightRange.X = adjustHeightRange.Y;
				}
			}
		}
		
		else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UUIText, listRichTextImageObjectInOutliner))
		{
			for (auto& imageObj : createdRichTextImageObjectArray)
			{
				if (IsValid(imageObj))
				{
					auto bListedInSceneOutliner_Property = FindFProperty<FBoolProperty>(AActor::StaticClass(), TEXT("bListedInSceneOutliner"));
					bListedInSceneOutliner_Property->SetPropertyValue_InContainer(imageObj->GetOwner(), listRichTextImageObjectInOutliner);
				}
			}
#if WITH_EDITOR
			ULGUIPrefabManagerObject::MarkBroadcastLevelActorListChanged();
#endif
		}
		else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UUIText, richText))
		{
			if (!richText)
			{
				ClearCreatedRichTextImageObject();
			}
		}
		else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UUIText, richTextImageData))
		{
			if (!IsValid(richTextImageData))//clear richTextImageData, then need to delete created object
			{
				ClearCreatedRichTextImageObject();
			}
		}
		else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UUIText, richTextTagFilterFlags))
		{
			if (!(richTextTagFilterFlags & (1 << (int)EUIText_RichTextTagFilterFlags::Image)))
			{
				ClearCreatedRichTextImageObject();
			}
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
void UUIText::OnPreChangeRichTextImageDataProperty()
{
	if (IsValid(richTextImageData))//unregister event from prev
	{
		UnregisterOnRichTextImageDataChange();
	}
}
void UUIText::OnPostChangeRichTextImageDataProperty()
{
	if (IsValid(richTextImageData))
	{
		RegisterOnRichTextImageDataChange();
	}
}
void UUIText::OnPreChangeRichTextCustomStyleDataProperty()
{
	if (IsValid(richTextCustomStyleData))
	{
		UnregisterOnRichTextCustomStyleDataChange();
	}
}
void UUIText::OnPostChangeRichTextCustomStyleDataProperty()
{
	if (IsValid(richTextCustomStyleData))
	{
		RegisterOnRichTextCustomStyleDataChange();
	}
}
#endif
void UUIText::RegisterOnRichTextImageDataChange()
{
	onRichTextImageDataChangedDelegateHandle = richTextImageData->OnDataChange.AddWeakLambda(this, [this] {
		this->MarkVerticesDirty(false, true, true, false);
		});
}
void UUIText::UnregisterOnRichTextImageDataChange()
{
	richTextImageData->OnDataChange.Remove(onRichTextImageDataChangedDelegateHandle);
	onRichTextImageDataChangedDelegateHandle.Reset();
}

void UUIText::RegisterOnRichTextCustomStyleDataChange()
{
	onRichTextCustomStyleDataChangedDelegateHandle = richTextCustomStyleData->OnDataChange.AddWeakLambda(this, [this] {
		this->MarkVerticesDirty(false, true, true, false);
		});
}
void UUIText::UnregisterOnRichTextCustomStyleDataChange()
{
	richTextCustomStyleData->OnDataChange.Remove(onRichTextCustomStyleDataChangedDelegateHandle);
	onRichTextCustomStyleDataChangedDelegateHandle.Reset();
}

FVector2D UUIText::GetTextRealSize()const
{
	UpdateCacheTextGeometry();
	return FVector2D(CacheTextGeometryData.textRealSize);
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
void UUIText::SetParagraphHorizontalAlignment(EUITextParagraphHorizontalAlign newHAlign) {
	if (hAlign != newHAlign)
	{
		MarkVertexPositionDirty();
		hAlign = newHAlign;
	}
}
void UUIText::SetParagraphVerticalAlignment(EUITextParagraphVerticalAlign newVAlign) {
	if (vAlign != newVAlign)
	{
		MarkVertexPositionDirty();
		vAlign = newVAlign;
	}
}
void UUIText::SetOverflowType(EUITextOverflowType newOverflowType) {
	if (overflowType != newOverflowType)
	{
		if (overflowType == EUITextOverflowType::ClampContent
			|| newOverflowType == EUITextOverflowType::ClampContent
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
void UUIText::SetMaxHorizontalWidth(float value)
{
	if (maxHorizontalWidth != value)
	{
		maxHorizontalWidth = value;
		MarkVertexPositionDirty();
	}
}
void UUIText::SetFontStyle(EUITextFontStyle newFontStyle) {
	if (fontStyle != newFontStyle)
	{
		if ((fontStyle == EUITextFontStyle::None || fontStyle == EUITextFontStyle::Italic)
			&& (newFontStyle == EUITextFontStyle::None || newFontStyle == EUITextFontStyle::Italic))//these only affect vertex position
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
		if (!richText)
		{
			ClearCreatedRichTextImageObject();
		}
	}
}
void UUIText::SetRichTextTagFilterFlags(int32 value)
{
	if (richTextTagFilterFlags != value)
	{
		MarkVerticesDirty(true, true, true, true);
		richTextTagFilterFlags = value;
		if (!(richTextTagFilterFlags & (1 << (int)EUIText_RichTextTagFilterFlags::Image)))
		{
			ClearCreatedRichTextImageObject();
		}
	}
}
void UUIText::SetRichTextImageData(ULGUIRichTextImageData_BaseObject* value)
{
	if (richTextImageData != value)
	{
		MarkVerticesDirty(true, true, true, true);
		richTextImageData = value;
		if (!IsValid(richTextImageData))//clear richTextImageData, then need to delete created object
		{
			ClearCreatedRichTextImageObject();
		}
	}
}
void UUIText::SetRichTextCustomStyleData(ULGUIRichTextCustomStyleData* value)
{
	if (richTextCustomStyleData != value)
	{
		MarkVerticesDirty(true, true, true, true);
		richTextCustomStyleData = value;
	}
}


void UUIText::ClearCreatedRichTextImageObject()
{
	for (auto& imageObj : createdRichTextImageObjectArray)
	{
		if (IsValid(imageObj))
		{
			LGUIUtils::DestroyActorWithHierarchy(imageObj->GetOwner());
		}
	}
	createdRichTextImageObjectArray.Empty();
}

void UUIText::MarkTextLayoutDirty()
{
	bTextLayoutDirty = true;
	ULGUIManagerWorldSubsystem::MarkUpdateLayout(this->GetWorld());
}
void UUIText::ConditionalMarkTextLayoutDirty()
{
	if (overflowType == EUITextOverflowType::HorizontalOverflow)
	{
		if (adjustWidth)
		{
			MarkTextLayoutDirty();
		}
	}
	else if (overflowType == EUITextOverflowType::VerticalOverflow)
	{
		if (adjustHeight)
		{
			MarkTextLayoutDirty();
		}
	}
	else if (overflowType == EUITextOverflowType::HorizontalAndVerticalOverflow)
	{
		if (adjustWidth)
		{
			MarkTextLayoutDirty();
		}
		if (adjustHeight)
		{
			MarkTextLayoutDirty();
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
			auto tempAdjustWidth = false, tempAdjustHeight = false;
			if (overflowType == EUITextOverflowType::HorizontalOverflow)
			{
				if (adjustWidth)
				{
					tempAdjustWidth = true;
				}
			}
			else if (overflowType == EUITextOverflowType::VerticalOverflow)
			{
				if (adjustHeight)
				{
					tempAdjustHeight = true;
				}
			}
			else if (overflowType == EUITextOverflowType::HorizontalAndVerticalOverflow)
			{
				if (adjustWidth)
				{
					tempAdjustWidth = true;
				}
				if (adjustHeight)
				{
					tempAdjustHeight = true;
				}
			}
			if (tempAdjustWidth)
			{
				if (adjustWidthRange == FVector2D::ZeroVector
					|| (CacheTextGeometryData.textRealSize.X >= adjustWidthRange.X && CacheTextGeometryData.textRealSize.X <= adjustWidthRange.Y)
					)
				{
					SetWidth(CacheTextGeometryData.textRealSize.X);
				}
			}
			if (tempAdjustHeight)
			{
				if (adjustHeightRange == FVector2D::ZeroVector
					|| (CacheTextGeometryData.textRealSize.Y >= adjustHeightRange.X && CacheTextGeometryData.textRealSize.Y <= adjustHeightRange.Y)
					)
				{
					SetHeight(CacheTextGeometryData.textRealSize.Y);
				}
			}
		}
	}
}
bool UUIText::GetCanLayoutControlAnchor_Implementation(class UUIItem* InUIItem, FLGUICanLayoutControlAnchor& OutResult)const
{
	if (this == InUIItem)
	{
		if (overflowType == EUITextOverflowType::HorizontalOverflow)
		{
			if (adjustWidth)
			{
				if (adjustWidthRange == FVector2D::ZeroVector
					|| (this->GetWidth() >= adjustWidthRange.X && this->GetWidth() <= adjustWidthRange.Y)
					)
				{
					OutResult.bCanControlHorizontalSizeDelta = true;
				}
				return true;
			}
		}
		else if (overflowType == EUITextOverflowType::VerticalOverflow)
		{
			if (adjustHeight)
			{
				if (adjustHeightRange == FVector2D::ZeroVector
					|| (this->GetHeight() >= adjustHeightRange.X && this->GetHeight() <= adjustHeightRange.Y)
					)
				{
					OutResult.bCanControlVerticalSizeDelta = true;
				}
				return true;
			}
		}
		else if (overflowType == EUITextOverflowType::HorizontalAndVerticalOverflow)
		{
			if (adjustWidth)
			{
				if (adjustWidthRange == FVector2D::ZeroVector
					|| (this->GetWidth() >= adjustWidthRange.X && this->GetWidth() <= adjustWidthRange.Y)
					)
				{
					OutResult.bCanControlHorizontalSizeDelta = true;
				}
			}
			if (adjustHeight)
			{
				if (adjustHeightRange == FVector2D::ZeroVector
					|| (this->GetHeight() >= adjustHeightRange.X && this->GetHeight() <= adjustHeightRange.Y)
					)
				{
					OutResult.bCanControlVerticalSizeDelta = true;
				}
			}
			if (adjustWidth || adjustHeight)
			{
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
		, FVector2f(this->GetPivot())
		, this->GetFinalColor()
		, CanvasGroupAlpha
		, FVector2f(this->GetFontSpace())
		, this->GetFontSize()
		, this->GetParagraphHorizontalAlignment()
		, this->GetParagraphVerticalAlignment()
		, this->GetOverflowType()
		, this->GetMaxHorizontalWidth()
		, this->GetUseKerning()
		, this->GetFontStyle()
		, this->GetRichText()
		, this->GetRichTextTagFilterFlags()
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
	if (InVertexPositionDirty)//position change, could cause layout change
	{
		ConditionalMarkTextLayoutDirty();
	}
	CacheTextGeometryData.MarkDirty();
	Super::MarkVerticesDirty(InTriangleDirty, InVertexPositionDirty, InVertexUVDirty, InVertexColorDirty);
}
void UUIText::MarkTextureDirty()
{
	//texture dirty, could because font change, then text's geometry will be recreate, so layout will change
	ConditionalMarkTextLayoutDirty();
	CacheTextGeometryData.MarkDirty();
	Super::MarkTextureDirty();
}

void UUIText::MarkAllDirty()
{
	ConditionalMarkTextLayoutDirty();
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
const TArray<FUIText_RichTextImageTag>& UUIText::GetRichTextImageTagArray()const
{
	UpdateCacheTextGeometry();
	return CacheTextGeometryData.cacheRichTextImageTagArray;
}

void UUIText::GenerateRichTextImageObject()
{
	if (!IsValid(richTextImageData))return;
	richTextImageData->CreateOrUpdateObject(this, CacheTextGeometryData.cacheRichTextImageTagArray, createdRichTextImageObjectArray, 
#if WITH_EDITOR
		listRichTextImageObjectInOutliner
#else
		false
#endif
	);
}






bool UUIText::MoveCaret(int32 moveType, int32& inOutCaretPositionIndex, int32& inOutCaretPositionLineIndex, FVector2f& inOutCaretPosition)
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
			for (int caretIndex = 0; caretIndex < lineProperty.caretPropertyList.Num(); caretIndex++)
			{
				if (totalCaretIndex == inOutCaretPositionIndex)//find caret
				{
					inOutCaretPositionLineIndex = lineIndex;
					inOutCaretPosition = lineProperty.caretPropertyList[caretIndex].caretPosition;
					//stop loop
					foundCaret = true;
					caretIndex = lineProperty.caretPropertyList.Num();
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
			inOutCaretPosition = lastLineProperty.caretPropertyList[lastLineProperty.caretPropertyList.Num() - 1].caretPosition;
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
			accumulatedCaretIndex += cacheLinePropertyArray[lineIndex].caretPropertyList.Num();
		}
		auto originCaretPosition = inOutCaretPosition;
		for (int caretIndex = 0; caretIndex < lineProperty.caretPropertyList.Num(); caretIndex++)
		{
			auto& caretProperty = lineProperty.caretPropertyList[caretIndex];
			auto distance = FMath::Abs(originCaretPosition.X - caretProperty.caretPosition.X);
			if (distance < minDistance)
			{
				minDistance = distance;
				inOutCaretPositionIndex = accumulatedCaretIndex;
				inOutCaretPosition = caretProperty.caretPosition;
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
		inOutCaretPosition = cacheLinePropertyArray[0].caretPropertyList[0].caretPosition;
	}
	break;
	case 5:
	{
		int32 accumulatedCaretIndex = 0;
		for (int lineIndex = 0; lineIndex < cacheLinePropertyArray.Num(); lineIndex++)
		{
			accumulatedCaretIndex += cacheLinePropertyArray[lineIndex].caretPropertyList.Num();
		}
		inOutCaretPositionIndex = accumulatedCaretIndex - 1;
		inOutCaretPositionLineIndex = cacheLinePropertyArray.Num() - 1;
		auto& lastLineProperty = cacheLinePropertyArray[cacheLinePropertyArray.Num() - 1];
		inOutCaretPosition = lastLineProperty.caretPropertyList[lastLineProperty.caretPropertyList.Num() - 1].caretPosition;
	}
	break;
	}
	if (originCaretPositionIndex != inOutCaretPositionIndex || originCaretPositionLineIndex != inOutCaretPositionLineIndex)
	{
		return true;
	}
	return false;
}

int UUIText::GetCharIndexByCaretIndex(int32 inCaretPositionIndex)
{
	UpdateCacheTextGeometry();
	auto& cacheLinePropertyArray = CacheTextGeometryData.cacheLinePropertyArray;
	int accumulatedCaretIndex = 0;
	for (int lineIndex = 0; lineIndex < cacheLinePropertyArray.Num(); lineIndex++)
	{
		auto& lineProperty = cacheLinePropertyArray[lineIndex];
		for (int caretIndex = 0; caretIndex < lineProperty.caretPropertyList.Num(); caretIndex++)
		{
			if (accumulatedCaretIndex == inCaretPositionIndex)//find caret
			{
				return lineProperty.caretPropertyList[caretIndex].charIndex;
			}
			accumulatedCaretIndex++;
		}
	}
	//not found caret, use last one
	auto& lastLineProperty = cacheLinePropertyArray[cacheLinePropertyArray.Num() - 1];
	return lastLineProperty.caretPropertyList[lastLineProperty.caretPropertyList.Num() - 1].charIndex;
}
int UUIText::GetLastCaret()
{
	UpdateCacheTextGeometry();
	auto& cacheLinePropertyArray = CacheTextGeometryData.cacheLinePropertyArray;
	int totalCaretIndex = 0;
	for (int lineIndex = 0; lineIndex < cacheLinePropertyArray.Num(); lineIndex++)
	{
		auto& lineProperty = cacheLinePropertyArray[lineIndex];
		totalCaretIndex += lineProperty.caretPropertyList.Num();
	}
	return totalCaretIndex - 1;
}
//caret is at left side of char
void UUIText::FindCaretByIndex(int32& inOutCaretPositionIndex, FVector2f& outCaretPosition, int32& outCaretPositionLineIndex, int32& outVisibleCaretStartIndex)
{
	UpdateCacheTextGeometry();
	auto& cacheLinePropertyArray = CacheTextGeometryData.cacheLinePropertyArray;

	if (inOutCaretPositionIndex < 0)inOutCaretPositionIndex = 0;
	outCaretPosition.X = outCaretPosition.Y = 0;
	outCaretPositionLineIndex = 0;
	outVisibleCaretStartIndex = 0;
	if (cacheLinePropertyArray.Num() == 0)
	{
		float pivotOffsetX = this->GetWidth() * (0.5f - this->GetPivot().X);
		float pivotOffsetY = this->GetHeight() * (0.5f - this->GetPivot().Y);
		switch (hAlign)
		{
		case EUITextParagraphHorizontalAlign::Left:
		{
			outCaretPosition.X = pivotOffsetX - this->GetWidth() * 0.5f;
		}
			break;
		case EUITextParagraphHorizontalAlign::Center:
		{
			outCaretPosition.X = pivotOffsetX;
		}
			break;
		case EUITextParagraphHorizontalAlign::Right:
		{
			outCaretPosition.X = pivotOffsetX + this->GetWidth() * 0.5f;
		}
			break;
		}
		switch (vAlign)
		{
		case EUITextParagraphVerticalAlign::Top:
		{
			outCaretPosition.Y = pivotOffsetY + this->GetHeight() * 0.5f - size * 0.5f;//fixed offset
		}
			break;
		case EUITextParagraphVerticalAlign::Middle:
		{
			outCaretPosition.Y = pivotOffsetY;
		}
			break;
		case EUITextParagraphVerticalAlign::Bottom:
		{
			outCaretPosition.Y = pivotOffsetY - this->GetHeight() * 0.5f + size * 0.5f;//fixed offset
		}
			break;
		}
	}
	else
	{
		if (inOutCaretPositionIndex == 0)//first char
		{
			outCaretPosition = cacheLinePropertyArray[0].caretPropertyList[0].caretPosition;
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
				for (int caretIndex = 0; caretIndex < lineProperty.caretPropertyList.Num(); caretIndex++)
				{
					if (accumulatedCaretIndex == inOutCaretPositionIndex)//find caret
					{
						outCaretPositionLineIndex = lineIndex;
						outCaretPosition = lineProperty.caretPropertyList[caretIndex].caretPosition;
						outVisibleCaretStartIndex = accumulatedCaretIndex;
						//stop loop
						foundCaret = true;
						caretIndex = lineProperty.caretPropertyList.Num();
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
				outCaretPosition = lastLineProperty.caretPropertyList[lastLineProperty.caretPropertyList.Num() - 1].caretPosition;
				outCaretPositionLineIndex = cacheLinePropertyArray.Num() - 1;
				outVisibleCaretStartIndex = 0;
			}
		}
	}
}
void UUIText::FindCaret(FVector2f& inOutCaretPosition, int32 inCaretPositionLineIndex, int32& outCaretPositionIndex)
{
	if (text.ToString().Len() == 0)//no text
		return;
	UpdateCacheTextGeometry();
	auto& cacheTextPropertyArray = CacheTextGeometryData.cacheLinePropertyArray;
	auto lineCount = cacheTextPropertyArray.Num();//line count
	outCaretPositionIndex = 0;

	//find nearest char to caret from this line
	auto& lineItem = cacheTextPropertyArray[inCaretPositionLineIndex];
	int charPropertyCount = lineItem.caretPropertyList.Num();//char count of this line
	float nearestDistance = MAX_FLT;
	int32 nearestIndex = -1;
	for (int charPropertyIndex = 0; charPropertyIndex < charPropertyCount; charPropertyIndex++)
	{
		auto& charItem = lineItem.caretPropertyList[charPropertyIndex];
		float distance = FMath::Abs(charItem.caretPosition.X - inOutCaretPosition.X);
		if (distance <= nearestDistance)
		{
			nearestDistance = distance;
			nearestIndex = charPropertyIndex;
			outCaretPositionIndex = charItem.charIndex;
		}
	}
	inOutCaretPosition = lineItem.caretPropertyList[nearestIndex].caretPosition;
}
//find caret by position, caret is on left side of char
void UUIText::FindCaretByWorldPosition(FVector inWorldPosition, FVector2f& outCaretPosition, int32& outCaretPositionLineIndex, int32& outCaretPositionIndex)
{
	if (text.ToString().Len() == 0)//no text
	{
		outCaretPositionIndex = 0;
		int tempVisibleCharStartIndex = 0;
		FindCaretByIndex(outCaretPositionIndex, outCaretPosition, outCaretPositionLineIndex, tempVisibleCharStartIndex);
	}
	else
	{
		UpdateCacheTextGeometry();
		auto& cacheLinePropertyArray = CacheTextGeometryData.cacheLinePropertyArray;

		auto localPosition = this->GetComponentTransform().InverseTransformPosition(inWorldPosition);
		auto localPosition2D = FVector2f(localPosition.Y, localPosition.Z);

		float nearestDistance = MAX_FLT;
		int accumulatedCaretIndex = 0;
		//find the nearest line, only need to compare Y
		int foundLineIndex = -1;
		for (int lineIndex = 0; lineIndex < cacheLinePropertyArray.Num(); lineIndex++)
		{
			auto& lineItem = cacheLinePropertyArray[lineIndex];
			float distance = FMath::Abs(lineItem.caretPropertyList[0].caretPosition.Y - localPosition2D.Y);
			if (distance <= nearestDistance)
			{
				nearestDistance = distance;
				outCaretPositionLineIndex = lineIndex;
				accumulatedCaretIndex += lineItem.caretPropertyList.Num();
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
		accumulatedCaretIndex -= cacheLinePropertyArray[foundLineIndex].caretPropertyList.Num();//remove prev line's caret count, because we need to add it when compare X pos
		//then find nearest char, only need to compare X
		nearestDistance = MAX_FLT;
		auto& nearestLine = cacheLinePropertyArray[outCaretPositionLineIndex];
		for (int caretIndex = 0; caretIndex < nearestLine.caretPropertyList.Num(); caretIndex++)
		{
			auto& caretItem = nearestLine.caretPropertyList[caretIndex];
			float distance = FMath::Abs(caretItem.caretPosition.X - localPosition2D.X);
			if (distance <= nearestDistance)
			{
				nearestDistance = distance;
				outCaretPositionIndex = accumulatedCaretIndex + caretIndex;
				outCaretPosition = caretItem.caretPosition;
			}
			else
			{
				break;
			}
		}
	}
}

int UUIText::GetCaretIndexByCharIndex(int32 inCharIndex)
{
	UpdateCacheTextGeometry();
	int accumulatedCaretIndex = 0;
	auto& cacheLinePropertyArray = CacheTextGeometryData.cacheLinePropertyArray;
	for (int lineIndex = 0; lineIndex < cacheLinePropertyArray.Num(); lineIndex++)
	{
		auto& lineProperty = cacheLinePropertyArray[lineIndex];
		for (int caretIndex = 0; caretIndex < lineProperty.caretPropertyList.Num(); caretIndex++)
		{
			if (lineProperty.caretPropertyList[caretIndex].charIndex == inCharIndex)//find char
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

bool UUIText::GetVisibleCharRangeForMultiLine(int32& inOutCaretPositionIndex, int32& inOutCaretPositionLineIndex, int32& inOutVisibleCaretStartLineIndex, int32& inOutVisibleCaretStartIndex, int inMaxLineCount, int32& outVisibleCharStartIndex, int32& outVisibleCharCount)
{
	UpdateCacheTextGeometry();
	auto& cacheLinePropertyArray = CacheTextGeometryData.cacheLinePropertyArray;
	int accumulatedCaretIndex = 0;
	bool foundCaret = false;
	for (int lineIndex = 0; lineIndex < cacheLinePropertyArray.Num(); lineIndex++)
	{
		auto& lineProperty = cacheLinePropertyArray[lineIndex];
		for (int caretIndex = 0; caretIndex < lineProperty.caretPropertyList.Num(); caretIndex++)
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
		inOutVisibleCaretStartIndex += lineProperty.caretPropertyList.Num();
	}
	auto& startLineProperty = cacheLinePropertyArray[inOutVisibleCaretStartLineIndex];
	auto& endLineProperty = cacheLinePropertyArray[VisibleCaretEndLineIndex];
	outVisibleCharStartIndex = startLineProperty.caretPropertyList[0].charIndex;
	auto lastIndex = endLineProperty.caretPropertyList.Num() - 1;
	auto lastCharIndex = endLineProperty.caretPropertyList[lastIndex].charIndex;
	if (lastCharIndex == -1)//-1 means newline break, so use next caret's char index
	{
		lastCharIndex = endLineProperty.caretPropertyList[lastIndex - 1].charIndex + 1;
	}
	outVisibleCharCount = lastCharIndex - outVisibleCharStartIndex;
	return outOfRange;
}

bool UUIText::GetVisibleCharRangeForSingleLine(int32& inOutCaretPositionIndex, int32& inOutVisibleCaretStartIndex, float inMaxWidth, int32& outVisibleCharStartIndex, int32& outVisibleCharCount)
{
	UpdateCacheTextGeometry();
	auto& cacheTextPropertyArray = CacheTextGeometryData.cacheLinePropertyArray;
	auto& lineProperty = cacheTextPropertyArray[0];//just single line
	inOutCaretPositionIndex = FMath::Clamp(inOutCaretPositionIndex, 0, lineProperty.caretPropertyList.Num() - 1);

	if (inOutVisibleCaretStartIndex > inOutCaretPositionIndex)
	{
		inOutVisibleCaretStartIndex = inOutCaretPositionIndex;
	}

	float calculatedSize = 0;
	bool outOfRange = false;
	int VisibleCaretEndIndex = inOutCaretPositionIndex;
	auto caretPosition = lineProperty.caretPropertyList[inOutCaretPositionIndex].caretPosition;
	//check from caret to VisibleCaretStartIndex
	for (int caretIndex = inOutCaretPositionIndex; caretIndex >= 0 && caretIndex >= inOutVisibleCaretStartIndex; caretIndex--)
	{
		auto& caretProperty = lineProperty.caretPropertyList[caretIndex];
		calculatedSize = caretPosition.X - caretProperty.caretPosition.X;
		if (calculatedSize >= inMaxWidth)
		{
			outOfRange = true;
			inOutVisibleCaretStartIndex = caretIndex + 1;
			break;
		}
	}
	if (!outOfRange)
	{
		//check from caret to right end
		float tempWidth = calculatedSize;
		for (int caretIndex = inOutCaretPositionIndex; caretIndex < lineProperty.caretPropertyList.Num(); caretIndex++)
		{
			auto& caretProperty = lineProperty.caretPropertyList[caretIndex];
			auto dist = caretProperty.caretPosition.X - caretPosition.X;
			tempWidth = calculatedSize + dist;
			if (tempWidth > inMaxWidth)
			{
				outOfRange = true;
				break;
			}
			VisibleCaretEndIndex++;
		}
		VisibleCaretEndIndex--;
		calculatedSize = tempWidth;

		if (!outOfRange)
		{
			//check from VisibleCaretStartIndex to left
			for (int caretIndex = inOutVisibleCaretStartIndex - 1; caretIndex >= 0; caretIndex--)
			{
				auto& caretProperty = lineProperty.caretPropertyList[caretIndex];
				auto dist = caretPosition.X - caretProperty.caretPosition.X;
				tempWidth = calculatedSize + dist;
				if (tempWidth >= inMaxWidth)
				{
					outOfRange = true;
					break;
				}
				inOutVisibleCaretStartIndex--;
			}
			calculatedSize = tempWidth;
		}
	}
	outVisibleCharStartIndex = lineProperty.caretPropertyList[inOutVisibleCaretStartIndex].charIndex;
	outVisibleCharCount = lineProperty.caretPropertyList[VisibleCaretEndIndex].charIndex - outVisibleCharStartIndex;
	return outOfRange;
}

void UUIText::GetSelectionProperty(int32 InSelectionStartCaretIndex, int32 InSelectionEndCaretIndex, TArray<FUITextSelectionProperty>& OutSelectionProeprtyArray)
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
		auto& firstLineCharPropertyList = cacheTextPropertyArray[startCaretPositionLineIndex].caretPropertyList;
		auto& firstLineLastCharProperty = firstLineCharPropertyList[firstLineCharPropertyList.Num() - 1];
		selectionProperty.Size = FMath::RoundToInt(firstLineLastCharProperty.caretPosition.X - startCaretPosition.X);
		//selectionProperty.Size = (1.0f - this->GetPivot().X) * this->GetWidth() - startCaretPosition.X;
		OutSelectionProeprtyArray.Add(selectionProperty);
		//middle line, use this->GetWidth() as size
		int middleLineCount = endCaretPositionLineIndex - startCaretPositionLineIndex - 1;
		for (int i = 0; i < middleLineCount; i++)
		{
			auto& charPropertyList = cacheTextPropertyArray[startCaretPositionLineIndex + i + 1].caretPropertyList;
			auto& firstPosition = charPropertyList[0].caretPosition;
			auto& lasPosition = charPropertyList[charPropertyList.Num() - 1].caretPosition;
			selectionProperty.Pos = firstPosition;
			selectionProperty.Size = FMath::RoundToInt(lasPosition.X - firstPosition.X);
			OutSelectionProeprtyArray.Add(selectionProperty);
		}
		//end line
		auto& firstPosition = cacheTextPropertyArray[endCaretPositionLineIndex].caretPropertyList[0].caretPosition;
		selectionProperty.Pos = firstPosition;
		selectionProperty.Size = FMath::RoundToInt(endCaretPosition.X - firstPosition.X);
		OutSelectionProeprtyArray.Add(selectionProperty);
	}
}

#undef LOCTEXT_NAMESPACE

#if LGUI_CAN_DISABLE_OPTIMIZATION
UE_ENABLE_OPTIMIZATION
#endif
