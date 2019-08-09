// Copyright 2019 LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UIText.h"
#include "LGUI.h"
#include "Core/UIGeometry.h"
#include "Core/ActorComponent/UIPanel.h"


#if WITH_EDITORONLY_DATA
ULGUIFontData* UUIText::CurrentUsingFontData = nullptr;
#endif
UUIText::UUIText()
{
	PrimaryComponentTick.bCanEverTick = false;
#if WITH_EDITORONLY_DATA
	if (UUIText::CurrentUsingFontData != nullptr)
	{
		font = CurrentUsingFontData;
	}
#endif
}
UUIText::~UUIText()
{
	
}
void UUIText::ApplyFontTextureScaleUp()
{
	auto& uvs = geometry->uvs;
	if (uvs.Num() != 0)
	{
		for (int i = 0; i < uvs.Num(); i++)
		{
			auto& uv = uvs[i];
			uv *= 0.5f;
		}
		MarkUVDirty();
	}
	for (int i = 0, count = cachedTextGeometryList.Num(); i < count; i++)
	{
		auto& textGeo = cachedTextGeometryList[i];
		textGeo.uv0 *= 0.5f;
		textGeo.uv1 *= 0.5f;
		textGeo.uv2 *= 0.5f;
		textGeo.uv3 *= 0.5f;
	}
	geometry->texture = font->texture;
	if (CheckRenderUIPanel())
	{
		RenderUIPanel->SetDrawcallTexture(geometry->drawcallIndex, font->texture, true);
	}
}

void UUIText::ApplyFontTextureChange()
{
	if (font != nullptr)
	{
		cachedTextPropertyList.Empty();
		cachedTextGeometryList.Empty();
		MarkTriangleDirty();
		MarkTextureDirty();
		geometry->texture = font->texture;
		if (CheckRenderUIPanel())
		{
			RenderUIPanel->SetDrawcallTexture(geometry->drawcallIndex, font->texture, true);
		}
	}
}

void UUIText::ApplyRecreateText()
{
	if (font != nullptr)
	{
		cachedTextPropertyList.Empty();
		cachedTextGeometryList.Empty();
		MarkVertexPositionDirty();
	}
}

void UUIText::BeginPlay()
{
	Super::BeginPlay();
	if (font != nullptr)
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
	if (font != nullptr)
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
		if (font != nullptr)
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
		if (font != nullptr)
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

DECLARE_CYCLE_STAT(TEXT("UIText UpdateGeometry"), STAT_UITextUpdateGeometry, STATGROUP_LGUI);

void UUIText::UpdateGeometry(const bool& parentTransformChanged)
{
	SCOPE_CYCLE_COUNTER(STAT_UITextUpdateGeometry);
	if (IsUIActiveInHierarchy() == false)return;
	if (font == nullptr)
	{
		font = ULGUIFontData::GetDefaultFont();
	}
	if (!CheckRenderUIPanel())return;
	if (visibleCharCount == -1)
	{
		visibleCharCount = VisibleCharCountInString(text);
#if WITH_EDITOR
		if(this->GetWorld()->IsGameWorld())
#endif
		font->AddUIText(this);
	}
	CacheTextGeometry();
	if (geometry->vertices.Num() == 0)//if geometry not created yet
	{
		if (visibleCharCount > 0)//only valid if have visible char
		{
			CreateGeometry();
			RenderUIPanel->MarkRebuildAllDrawcall();
		}
		else//if not have visible char, just set geometry's properties
		{
			geometry->Clear();
			font->InitFreeType();
			geometry->texture = font->texture;
			geometry->material = CustomUIMaterial;
			geometry->isFontTexture = true;
			geometry->depth = widget.depth;
		}
	}
	else//if geometry is created, update data
	{
		if (cacheForThisUpdate_TextureChanged || cacheForThisUpdate_MaterialChanged)//texture change or material change, need to recreate drawcall
		{
			if (font == nullptr)//font is cleared
			{
				geometry->Clear();
				RenderUIPanel->MarkRebuildSpecificDrawcall(geometry->drawcallIndex);
				goto COMPLETE;
			}
			CreateGeometry();
			RenderUIPanel->MarkRebuildAllDrawcall();
			goto COMPLETE;
		}
		if (cacheForThisUpdate_DepthChanged)
		{
			if (CustomUIMaterial != nullptr)
			{
				CreateGeometry();
				RenderUIPanel->MarkRebuildAllDrawcall();
			}
			else
			{
				geometry->depth = widget.depth;
				RenderUIPanel->DepthChangeForDrawcall(this);
			}
		}
		if (cacheForThisUpdate_TriangleChanged)//triangle change, need to clear geometry then recreate the specific drawcall
		{
			CreateGeometry();
			RenderUIPanel->MarkRebuildSpecificDrawcall(geometry->drawcallIndex);
			goto COMPLETE;
		}
		else//update geometry
		{
			bool vertexChanged = false;//vertex data change
			if (cacheForThisUpdate_ColorChanged)
			{
				UIGeometry::UpdateUIColor(geometry, GetFinalColor());
				vertexChanged = true;
			}
			if (cacheForThisUpdate_VertexPositionChanged || cacheForThisUpdate_UVChanged)
			{
				float width = widget.width;
				float height = widget.height;
				UIGeometry::UpdateUITextVertexOrUV(text, width, height, widget.pivot, space, geometry, size, (uint8)hAlign, (uint8)vAlign, (uint8)overflowType, adjustWidth, adjustHeight, (uint8)fontStyle, textRealSize, RenderUIPanel->GetDynamicPixelsPerUnit(), true, cacheForThisUpdate_UVChanged, cachedTextPropertyList, cachedTextGeometryList);
				SetWidth(width);
				SetHeight(height);
				vertexChanged = true;
			}
			else if (parentTransformChanged)
			{
				vertexChanged = true;
				cacheForThisUpdate_VertexPositionChanged = true;
			}
			
			if (vertexChanged)
			{
				if (ApplyGeometryModifier())
				{
					UIGeometry::CheckAndApplyAdditionalChannel(geometry);
					RenderUIPanel->MarkRebuildSpecificDrawcall(geometry->drawcallIndex);
				}
				else
				{
					RenderUIPanel->MarkUpdateSpecificDrawcallVertex(geometry->drawcallIndex, cacheForThisUpdate_VertexPositionChanged);
				}
				if (cacheForThisUpdate_VertexPositionChanged)
				{
					UIGeometry::TransformVertices(RenderUIPanel, this, geometry, RenderUIPanel->GetRequireNormal(), RenderUIPanel->GetRequireTangent());
				}
			}
		}
	}
COMPLETE:
	;
	if (!geometry->CheckDataValid())
	{
		UE_LOG(LGUI, Error, TEXT("[UUIText::UpdateGeometry]UIText:%s geometry is not valid!"), *(this->GetFullName()));
	}
}

void UUIText::CreateGeometry()
{
	geometry->Clear();
	font->InitFreeType();
	geometry->texture = font->texture;
	geometry->material = CustomUIMaterial;
	geometry->depth = widget.depth;
	geometry->isFontTexture = true;
	float width = widget.width;
	float height = widget.height;
	UIGeometry::FromUIText(text, visibleCharCount, width, height, widget.pivot, GetFinalColor(), space, geometry, size, (int8)hAlign, (int8)vAlign, (uint8)overflowType, adjustWidth, adjustHeight, (uint8)fontStyle, textRealSize, RenderUIPanel->GetDynamicPixelsPerUnit(), cachedTextPropertyList, cachedTextGeometryList, RenderUIPanel->GetRequireNormal(), RenderUIPanel->GetRequireTangent(), RenderUIPanel->GetRequireUV1());
	SetWidth(width);
	SetHeight(height);
	ApplyGeometryModifier();
	UIGeometry::CheckAndApplyAdditionalChannel(geometry);
	UIGeometry::TransformVertices(RenderUIPanel, this, geometry, RenderUIPanel->GetRequireNormal(), RenderUIPanel->GetRequireTangent());
}


#if WITH_EDITOR
void UUIText::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	visibleCharCount = VisibleCharCountInString(text);
	if (auto Property = PropertyChangedEvent.Property)
	{
		auto PropertyName = Property->GetName();
		if (PropertyName == TEXT("font"))
		{
			if (font != nullptr)
			{
				font->AddUIText(this);
			}
		}
	}
}
void UUIText::EditorForceUpdateImmediately()
{
	Super::EditorForceUpdateImmediately();
#if WITH_EDITORONLY_DATA
	UUIText::CurrentUsingFontData = font;
#endif
	visibleCharCount = VisibleCharCountInString(text);
	if (font == nullptr)
	{
		font = ULGUIFontData::GetDefaultFont();
		if (font != nullptr)
		{
			font->AddUIText(this);
		}
	}
	cachedTextGeometryList.Empty();
}
void UUIText::OnPreChangeFontProperty()
{
	if (font != nullptr)
	{
		font->RemoveUIText(this);
	}
}
void UUIText::OnPostChangeFontProperty()
{
	if (font != nullptr)
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
			UIGeometry::UpdateUITextVertexOrUV(text, width, height, widget.pivot, space, geometry, size, (uint8)hAlign, (uint8)vAlign, (uint8)overflowType, adjustWidth, adjustHeight, (uint8)fontStyle, textRealSize, RenderUIPanel->GetDynamicPixelsPerUnit(), false, false, cachedTextPropertyList, cachedTextGeometryList);
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
		if (font != nullptr)
		{
			font->RemoveUIText(this);
		}
		font = newFont;
		cachedTextPropertyList.Empty();
		cachedTextGeometryList.Empty();
		MarkTextureDirty();
		//add to new
		if (font != nullptr)
		{
			font->AddUIText(this);
		}
	}
}
void UUIText::SetText(const FString& newText) {
	if (text != newText)
	{
		if (CheckRenderUIPanel()) RenderUIPanel->MarkNeedUpdate();
		text = newText;
		cachedTextPropertyList.Empty();
		cachedTextGeometryList.Empty();

		int newVisibleCharCount = VisibleCharCountInString(text);
		if (newVisibleCharCount != visibleCharCount)//visible char count change
		{
			MarkTriangleDirty();
			visibleCharCount = newVisibleCharCount;
		}
		else//visible char count not change, just makr update vertex and uv
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
		cachedTextPropertyList.Empty();
		cachedTextGeometryList.Empty();
	}
}
void UUIText::SetFontSpace(FVector2D newSpace) {
	if (space != newSpace)
	{
		MarkVertexPositionDirty();
		space = newSpace;
		cachedTextPropertyList.Empty();
		cachedTextGeometryList.Empty();
	}
}
void UUIText::SetParagraphHorizontalAlignment(UITextParagraphHorizontalAlign newHAlign) {
	if (hAlign != newHAlign)
	{
		MarkVertexPositionDirty();
		hAlign = newHAlign;
		cachedTextPropertyList.Empty();
		cachedTextGeometryList.Empty();
	}
}
void UUIText::SetParagraphVerticalAlignment(UITextParagraphVerticalAlign newVAlign) {
	if (vAlign != newVAlign)
	{
		MarkVertexPositionDirty();
		vAlign = newVAlign;
		cachedTextPropertyList.Empty();
		cachedTextGeometryList.Empty();
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
		cachedTextPropertyList.Empty();
		cachedTextGeometryList.Empty();
	}
}
void UUIText::SetAdjustWidth(bool newAdjustWidth) {
	if (adjustWidth != newAdjustWidth)
	{
		adjustWidth = newAdjustWidth;
		MarkVertexPositionDirty();
		cachedTextPropertyList.Empty();
		cachedTextGeometryList.Empty();
	}
}
void UUIText::SetAdjustHeight(bool newAdjustHeight) {
	if (adjustHeight != newAdjustHeight)
	{
		adjustHeight = newAdjustHeight;
		MarkVertexPositionDirty();
		cachedTextPropertyList.Empty();
		cachedTextGeometryList.Empty();
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
		cachedTextPropertyList.Empty();
		cachedTextGeometryList.Empty();
	}
}
void UUIText::CacheTextGeometry()
{
	if (cachedTextGeometryList.Num() != 0)return;//have cache data
	float charFixOffset = size * (font->fixedVerticalOffset - 0.25f);//some font may not render at vertical center, use this to mofidy it. 0.25 * size is tested value for most fonts
	bool dynamicPixelsPerUnitIsNot1 = RenderUIPanel->GetDynamicPixelsPerUnit() != 1;//use dynamicPixelsPerUnit or not
	bool bold = fontStyle == UITextFontStyle::Bold || fontStyle == UITextFontStyle::BoldAndItalic;
	bool italic = fontStyle == UITextFontStyle::Italic || fontStyle == UITextFontStyle::BoldAndItalic;
	if (visibleCharCount == -1)visibleCharCount = VisibleCharCountInString(text);
	cachedTextGeometryList.Reset(visibleCharCount);
	for (int i = 0, count = text.Len(); i < count; i++)
	{
		int charCode = text[i];
		if (!IsVisibleChar(charCode))continue;//skip invisible chars
		auto charData = font->GetCharData((uint16)charCode, size, bold, italic);
		FUITextCharGeometry charGeometry;
		charGeometry.geoWidth = charData->width;
		charGeometry.geoHeight = charData->height;
		charGeometry.xadvance = charData->xadvance;
		charGeometry.xoffset = charData->xoffset;
		charGeometry.yoffset = charData->yoffset + charFixOffset;
		charGeometry.vert0 = FVector2D(0, charData->height);
		charGeometry.vert1 = FVector2D(charData->width, charData->height);
		charGeometry.vert2 = FVector2D(0, 0);
		charGeometry.vert3 = FVector2D(charData->width, 0);

		auto overrideCharData = charData;
		if (dynamicPixelsPerUnitIsNot1)
		{
			auto calcFontSize = size * RenderUIPanel->GetDynamicPixelsPerUnit();
			calcFontSize = calcFontSize > 200 ? 200 : calcFontSize;//limit font size to 200. too large font size will result in large texture
			overrideCharData = font->GetCharData((uint16)charCode, (uint16)calcFontSize, bold, italic);
		}

		charGeometry.uv0 = overrideCharData->GetUV0();
		charGeometry.uv1 = overrideCharData->GetUV1();
		charGeometry.uv2 = overrideCharData->GetUV2();
		charGeometry.uv3 = overrideCharData->GetUV3();
		cachedTextGeometryList.Add(charGeometry);
	}
}
FVector2D UUIText::GetCharSize(TCHAR character)
{
	if (character == 10 || character == 13)return FVector2D::ZeroVector;
	if (character == 32)
	{
		return FVector2D(size * 0.5f, size);
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
		uint16 charIndexItem = srcStr[i];
		if (IsVisibleChar(charIndexItem) == false)// 10 - /n, 13 - /r, 32 - space
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
		UIGeometry::UpdateUITextVertexOrUV(text, width, height, widget.pivot, space, geometry, size, (uint8)hAlign, (uint8)vAlign, (uint8)overflowType, adjustWidth, adjustHeight, (uint8)fontStyle, textRealSize, RenderUIPanel->GetDynamicPixelsPerUnit(), false, false, cachedTextPropertyList, cachedTextGeometryList);
		SetWidth(width);
		SetHeight(height);
	}
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
		if (distance < nearestDistance)
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
		if (distance < nearestDistance)
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
			if (distance < nearestDistance)
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
			if (distance < nearestDistance)
			{
				nearestDistance = distance;
				outCaretPositionIndex = charItem.charIndex;
				outCaretPosition = charItem.caretPosition;
			}
			else
			{
				break;
			}
		}
	}
}

void UUIText::GetSelectionProperty(int32 InSelectionStartCaretIndex, int32 InSelectionEndCaretIndex, TArray<FUITextSelectionProperty>& OutSelectionProeprtyArray)
{
	OutSelectionProeprtyArray.Empty();
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