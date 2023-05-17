// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "UIBatchGeometryRenderable.h"
#include "Core/ILGUICultureChangedInterface.h"
#include "Layout/ILGUILayoutInterface.h"
#include "Core/LGUITextData.h"
#include "UIText.generated.h"


class ULGUIFontData_BaseObject;
class ULGUIRichTextImageData_BaseObject;

UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIText : public UUIBatchGeometryRenderable, public ILGUICultureChangedInterface, public ILGUILayoutInterface
{
	GENERATED_BODY()

public:	
	UUIText(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason)override;
	virtual void OnRegister()override;
	virtual void OnUnregister()override;
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy)override;
public:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)override;
	virtual void EditorForceUpdate() override;
protected:
	virtual void OnPreChangeFontProperty();
	virtual void OnPostChangeFontProperty();
	virtual void OnPreChangeRichTextImageDataProperty();
	virtual void OnPostChangeRichTextImageDataProperty();
#endif
	void RegisterOnRichTextImageDataChange();
	void UnregisterOnRichTextImageDataChange();
	FDelegateHandle onRichTextImageDataChangedDelegateHandle;
#if WITH_EDITORONLY_DATA
	/** current using font. the default font when creating new UIText */
	static TWeakObjectPtr<ULGUIFontData_BaseObject> CurrentUsingFontData;
#endif
public:
	static const FName GetTextPropertyName()
	{
		return GET_MEMBER_NAME_CHECKED(UUIText, text);
	}

protected:
	friend class FUITextCustomization;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayThumbnail = "false"))
		ULGUIFontData_BaseObject* font;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (MultiLine="true"))
		FText text = FText::FromString(TEXT("New Text"));
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (ClampMin = "2", ClampMax = "200"))
		float size = 16;
	/** use font kerning for better text layout. */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool useKerning = true;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FVector2D space = FVector2D(0, 0);
	UPROPERTY(EditAnywhere, Category = "LGUI")
		UITextParagraphHorizontalAlign hAlign = UITextParagraphHorizontalAlign::Center;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		UITextParagraphVerticalAlign vAlign = UITextParagraphVerticalAlign::Middle;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		UITextOverflowType overflowType = UITextOverflowType::VerticalOverflow;
	/** adjust AnchorData width to true text content width */
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (EditCondition = "overflowType==UITextOverflowType::HorizontalOverflow||overflowType==UITextOverflowType::HorizontalAndVerticalOverflow"))
		bool adjustWidth = false;
	/** adjust when width in this range: adjustWidthRange.X < width < adjustWidthRange.Y, Zero means no range limit */
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (EditCondition = "overflowType==UITextOverflowType::HorizontalOverflow||overflowType==UITextOverflowType::HorizontalAndVerticalOverflow"))
		FVector2D adjustWidthRange = FVector2D::ZeroVector;
	/** adjust AnchorData height to true text content height */
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (EditCondition = "overflowType==UITextOverflowType::VerticalOverflow||overflowType==UITextOverflowType::HorizontalAndVerticalOverflow"))
		bool adjustHeight = false;
	/** adjust when height in this range: adjustHeightRange.X < height < adjustHeightRange.Y, Zero means no range limit */
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (EditCondition = "overflowType==UITextOverflowType::VerticalOverflow||overflowType==UITextOverflowType::HorizontalAndVerticalOverflow"))
		FVector2D adjustHeightRange = FVector2D::ZeroVector;
	/** if overflowType is HorizontalAndVerticalOverflow, this parameter will limit width for horizontal overflow */
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (EditCondition = "overflowType==UITextOverflowType::HorizontalAndVerticalOverflow"))
		float maxHorizontalWidth = 100;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		UITextFontStyle fontStyle = UITextFontStyle::None;
	/**
	 * rich text support, eg:
	 * <b>Bold</b>
	 * <i>Italic</i>
	 * <u>Underline</u>
	 * <s>Strikethrough</s>
	 * <size=48>Point size 48</size>
	 * <size=+18>Point size increased by 18</size>
	 * <size=-18>Point size decreased by 18</size>
	 * <color=yellow>Yellow text</color> support color name: black, blue, green, orange, purple, red, white, and yellow
	 * <color=#00ff00>Green text</color>
	 * <sup>Superscript</sup>
	 * <sub>Subscript</sub>
	 * <MyTag>Custom tag</MyTag> use any string as custom tag
	 * <img=smile/> display a image with key "smile" which defined in RichTextImageData property, can be used for emoji. @todo: image size option
	 */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool richText = false;
	/** Flags to enable/disable rich text tag. */
	UPROPERTY(EditAnywhere, Category = LGUI, meta = (Bitmask, BitmaskEnum = "/Script/LGUI.EUIText_RichTextTagFilterFlags", EditCondition = "richText"))
		int32 richTextTagFilterFlags = 0xffffffff;
	/** rich text image data for rendering image inside UIText */
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (EditCondition = "richText"))
		ULGUIRichTextImageData_BaseObject* richTextImageData = nullptr;
	/** created object for rich text image */
	UPROPERTY(VisibleAnywhere, Category = "LGUI", Transient, AdvancedDisplay)
		TArray<class UUIItem*> createdRichTextImageObjectArray;
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "LGUI", AdvancedDisplay)
		bool listRichTextImageObjectInOutliner = false;
#endif
private:
	bool bHasAddToFont = false;
	/** visible/renderable char count of current text. -1 means not set yet */
	mutable int visibleCharCount = -1;
	bool bTextLayoutDirty = false;
	void MarkTextLayoutDirty();
	void ConditionalMarkTextLayoutDirty();

	virtual void OnUpdateLayout_Implementation()override;//@todo: should we implement ILayoutElement for AdjustWidth/AdjustHeight?
	virtual bool GetCanLayoutControlAnchor_Implementation(class UUIItem* InUIItem, FLGUICanLayoutControlAnchor& OutResult)const override;
	mutable FTextGeometryCache CacheTextGeometryData;
	bool UpdateCacheTextGeometry()const;
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		const TArray<FUITextCharProperty>& GetCharPropertyArray()const;
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		int32 GetVisibleCharCount()const;
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		const TArray<FUIText_RichTextCustomTag>& GetRichTextCustomTagArray()const;
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		const TArray<FUIText_RichTextImageTag>& GetRichTextImageTagArray()const;
public:
	virtual void MarkAllDirty()override;

	virtual UTexture* GetTextureToCreateGeometry()override;
	virtual UMaterialInterface* GetMaterialToCreateGeometry()override;

	virtual void OnBeforeCreateOrUpdateGeometry()override;
	virtual bool GetShouldAffectByPixelPerfect()const override;
	virtual void OnUpdateGeometry(UIGeometry& InGeo, bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)override;
	virtual void UpdateMaterialClipType()override;
	virtual void OnCultureChanged_Implementation()override;

	void CheckFontAdditionalShaderChannels();
public:
	void ApplyFontTextureScaleUp();
	void ApplyFontTextureChange();
	void ApplyFontMaterialChange();
	void ApplyRecreateText();

	virtual void MarkVerticesDirty(bool InTriangleDirty, bool InVertexPositionDirty, bool InVertexUVDirty, bool InVertexColorDirty)override;
	virtual void MarkTextureDirty()override;

	FORCEINLINE static bool IsVisibleChar(TCHAR character)
	{
		return (character != '\n' && character != '\r' && character != ' ' && character != '\t');
	}
	/** count visible char count of the string */
	static int VisibleCharCountInString(const FString& srcStr);

	void GenerateRichTextImageObject();
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI") ULGUIFontData_BaseObject* GetFont()const { return font; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")	const FText& GetText()const { return text; }
	UE_DEPRECATED(4.24, "Use GetFontSize instead")
	UFUNCTION(BlueprintCallable, Category = "LGUI", meta = (DeprecatedFunction, DeprecationMessage = "Use GetFontSize instead"))
		float GetSize()const { return size; }
	UFUNCTION(BlueprintCallable, Category = "LGUI") float GetFontSize()const { return size; }
	UFUNCTION(BlueprintCallable, Category = "LGUI") bool GetUseKerning()const { return useKerning; }
	UFUNCTION(BlueprintCallable, Category = "LGUI") FVector2D GetFontSpace()const { return space; }
	UFUNCTION(BlueprintCallable, Category = "LGUI") UITextOverflowType GetOverflowType()const { return overflowType; }
	UFUNCTION(BlueprintCallable, Category = "LGUI") bool GetAdjustWidth()const { return adjustWidth; }
	UFUNCTION(BlueprintCallable, Category = "LGUI") FVector2D GetAdjustWidthRange()const { return adjustWidthRange; }
	UFUNCTION(BlueprintCallable, Category = "LGUI") bool GetAdjustHeight()const { return adjustHeight; }
	UFUNCTION(BlueprintCallable, Category = "LGUI") FVector2D GetAdjustHeightRange()const { return adjustHeightRange; }
	UFUNCTION(BlueprintCallable, Category = "LGUI") float GetMaxHorizontalWidth()const { return maxHorizontalWidth; }
	UFUNCTION(BlueprintCallable, Category = "LGUI") UITextFontStyle GetFontStyle()const { return fontStyle; }
	UFUNCTION(BlueprintCallable, Category = "LGUI") bool GetRichText()const { return richText; }
	UFUNCTION(BlueprintCallable, Category = "LGUI") int32 GetRichTextTagFilterFlags()const { return richTextTagFilterFlags; }
	UFUNCTION(BlueprintCallable, Category = "LGUI") ULGUIRichTextImageData_BaseObject* GetRichTextImageData()const { return richTextImageData; }
	UFUNCTION(BlueprintCallable, Category = "LGUI") UITextParagraphHorizontalAlign GetParagraphHorizontalAlignment()const { return hAlign; }
	UFUNCTION(BlueprintCallable, Category = "LGUI") UITextParagraphVerticalAlign GetParagraphVerticalAlignment()const { return vAlign; }

	UFUNCTION(BlueprintCallable, Category = "LGUI") FVector2D GetTextRealSize()const;
	UE_DEPRECATED(4.24, "Use GetTextRealSize instead")
	UFUNCTION(BlueprintCallable, Category = "LGUI", meta = (DeprecatedFunction, DeprecationMessage = "Use GetTextRealSize instead"))
		FVector2D GetRealSize() { return GetTextRealSize(); }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetFont(ULGUIFontData_BaseObject* newFont);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetText(const FText& newText);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetFontSize(float newSize);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetUseKerning(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetFontSpace(FVector2D newSpace);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetParagraphHorizontalAlignment(UITextParagraphHorizontalAlign newHAlign);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetParagraphVerticalAlignment(UITextParagraphVerticalAlign newVAlign);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetOverflowType(UITextOverflowType newOverflowType);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetAdjustWidth(bool newAdjustWidth);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetAdjustHeight(bool newAdjustHeight);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetMaxHorizontalWidth(float value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetFontStyle(UITextFontStyle newFontStyle);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetRichText(bool newRichText);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetRichTextTagFilterFlags(int32 value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetRichTextImageData(ULGUIRichTextImageData_BaseObject* value);
private:
	void ClearCreatedRichTextImageObject();
protected:
	virtual void OnAnchorChange(bool InPivotChange, bool InWidthChange, bool InHeightChange, bool InDiscardCache = true)override;
public:
#pragma region UITextInputComponent
	/**
	 * .
	 * @param moveType 0-left, 1-right, 2-up, 3-down, 4-start, 5-end
	 * @return true- data changed
	 */
	bool MoveCaret(int32 moveType, int32& inOutCaretPositionIndex, int32& inOutCaretPositionLineIndex, FVector2D& inOutCaretPosition);
	int GetCharIndexByCaretIndex(int32 inCaretPositionIndex);
	int GetLastCaret();
	/** get caret position and line index */
	void FindCaretByIndex(int32& inOutCaretPositionIndex, FVector2D& outCaretPosition, int32& outCaretPositionLineIndex, int32& outVisibleCaretStartIndex);
	/** find current caret position */
	void FindCaret(FVector2D& inOutCaretPosition, int32 inCaretPositionLineIndex, int32& outCaretPositionIndex);
	/** find caret index by position */
	void FindCaretByWorldPosition(FVector inWorldPosition, FVector2D& outCaretPosition, int32& outCaretPositionLineIndex, int32& outCaretPositionIndex);
	int GetCaretIndexByCharIndex(int32 inCharIndex);
	bool GetVisibleCharRangeForMultiLine(int32& inOutCaretPositionIndex, int32& inOutCaretPositionLineIndex, int32& inOutVisibleCaretStartLineIndex, int32& inOutVisibleCaretStartIndex, int inMaxLineCount, int32& outVisibleCharStartIndex, int32& outVisibleCharCount);
	/**
	 * .
	 * @return true- value changed
	 */
	bool GetVisibleCharRangeForSingleLine(int32& inOutCaretPositionIndex, int32& inOutVisibleCaretStartIndex, float inMaxWidth, int32& outVisibleCharStartIndex, int32& outVisibleCharCount);

	/** range selection */
	void GetSelectionProperty(int32 InSelectionStartCaretIndex, int32 InSelectionEndCaretIndex, TArray<FUITextSelectionProperty>& OutSelectionProeprtyArray);
	const FTextGeometryCache& GetCacheTextGeometryData()const { UpdateCacheTextGeometry(); return CacheTextGeometryData; }
#pragma endregion UITextInputComponent
};
