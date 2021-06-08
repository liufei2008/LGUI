// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "UIRenderable.h"
#include "UIText.generated.h"


UENUM(BlueprintType, Category = LGUI)
enum class UITextParagraphHorizontalAlign : uint8
{
	Left,
	Center,
	Right,
};
UENUM(BlueprintType, Category = LGUI)
enum class UITextParagraphVerticalAlign : uint8
{
	Top,
	Middle,
	Bottom,
};
UENUM(BlueprintType, Category = LGUI)
enum class UITextFontStyle :uint8
{
	None,
	Bold,
	Italic,
	BoldAndItalic,
};

UENUM(BlueprintType, Category = LGUI)
enum class UITextOverflowType :uint8
{
	/** chars will go out of rect range horizontally */
	HorizontalOverflow,
	/** chars will go out of rect range vertically */
	VerticalOverflow,
	/** remove chars on right if out of range */
	ClampContent,
};

/** single char property */
struct FUITextCaretProperty
{
	/** caret position. caret is on left side of char */
	FVector2D caretPosition;
	/** char index in text */
	int32 charIndex;
};
/** a line of text property */
struct FUITextLineProperty
{
	TArray<FUITextCaretProperty> charPropertyList;
};
/** for range selection in TextInputComponent */
struct FUITextSelectionProperty
{
	FVector2D Pos;
	int32 Size;
};
/** char property */
struct FUITextCharProperty
{
	/** char index in string */
	int32 CharIndex;
	/** vertex index in UIGeometry::vertices */
	int32 StartVertIndex;
	/** vertex count */
	int32 VertCount;
	/** triangle index in UIGeometry::triangles */
	int32 StartTriangleIndex;
	/** triangle indices count */
	int32 IndicesCount;

	/** center position of the char, in UIText's local space */
	//FVector2D CenterPosition;
};

struct FUIText_RichTextCustomTag
{
	/** Tag name */
	FName TagName;
	/** start char index in cacheCharPropertyArray */
	int32 CharIndexStart;
	/** end char index in cacheCharPropertyArray */
	int32 CharIndexEnd;
};

class ULGUIFontData_BaseObject;

UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIText : public UUIRenderable
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
public:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)override;
	virtual void EditorForceUpdateImmediately() override;
protected:
	virtual void OnPreChangeFontProperty();
	virtual void OnPostChangeFontProperty();
#endif
#if WITH_EDITORONLY_DATA
	/** current using font. the default font when creating new UIText */
	static TWeakObjectPtr<ULGUIFontData_BaseObject> CurrentUsingFontData;
#endif

protected:
	friend class FUITextCustomization;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayThumbnail = "false"))
		ULGUIFontData_BaseObject* font;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FString text = TEXT("New Text");
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (ClampMin = "2", ClampMax = "200"))
		float size = 16;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FVector2D space = FVector2D(0, 0);
	UPROPERTY(EditAnywhere, Category = "LGUI")
		UITextParagraphHorizontalAlign hAlign = UITextParagraphHorizontalAlign::Center;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		UITextParagraphVerticalAlign vAlign = UITextParagraphVerticalAlign::Middle;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		UITextOverflowType overflowType = UITextOverflowType::VerticalOverflow;
	/** if overflowType = HorizontalOverflow then adjust widget width to true width */
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (EditCondition = "overflowType==UITextOverflowType::HorizontalOverflow"))
		bool adjustWidth = false;
	/** if overflowType = VerticalOverflow then adjust widget height to true height */
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (EditCondition = "overflowType==UITextOverflowType::VerticalOverflow"))
		bool adjustHeight = false;
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
	 */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool richText = false;
private:
	/** visible/renderable char count of current text. -1 means not set yet */
	int visibleCharCount = -1;
	/** real size of this UIText, not the widget's width and height */
	FVector2D textRealSize;
	/** cached texture property */
	TArray<FUITextLineProperty> cachedTextPropertyArray;
	
	void CheckCachedTextPropertyList();

	/** calculate text geometry */
	void CacheTextGeometry();

	/** char properties, from first char to last one in array */
	TArray<FUITextCharProperty> cacheCharPropertyArray;
	TArray<FUIText_RichTextCustomTag> cacheRichTextCustomTagArray;
public:
	const TArray<FUITextCharProperty>& GetCharPropertyArray(bool createIfNotExist = false);
	const TArray<FUIText_RichTextCustomTag>& GetRichTextCustomTagArray(bool createIfNotExist = false);
public:
	virtual void MarkAllDirtyRecursive()override;
protected:
	virtual bool HaveDataToCreateGeometry()override;
	virtual UTexture* GetTextureToCreateGeometry()override;
	virtual bool NeedTextureToCreateGeometry()override { return true; }

	virtual void OnBeforeCreateOrUpdateGeometry()override;
	virtual void OnCreateGeometry()override;
	virtual void OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)override;
public:
	void ApplyFontTextureScaleUp();
	void ApplyFontTextureChange();
	void ApplyRecreateText();

	FORCEINLINE static bool IsVisibleChar(TCHAR character)
	{
		return (character != '\n' && character != '\r' && character != ' ' && character != '\t');
	}
	/** count visible char count of the string */
	static int VisibleCharCountInString(const FString& srcStr);
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI") ULGUIFontData_BaseObject* GetFont()const { return font; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")	const FString& GetText()const { return text; }
	UFUNCTION(BlueprintCallable, Category = "LGUI") float GetSize()const { return size; }
	UFUNCTION(BlueprintCallable, Category = "LGUI") FVector2D GetFontSpace()const { return space; }
	UFUNCTION(BlueprintCallable, Category = "LGUI") UITextOverflowType GetOverflowType()const { return overflowType; }
	UFUNCTION(BlueprintCallable, Category = "LGUI") bool GetAdjustWidth()const { return adjustWidth; }
	UFUNCTION(BlueprintCallable, Category = "LGUI") bool GetAdjustHeight()const { return adjustHeight; }
	UFUNCTION(BlueprintCallable, Category = "LGUI") UITextFontStyle GetFontStyle()const { return fontStyle; }
	UFUNCTION(BlueprintCallable, Category = "LGUI") bool GetRichText()const { return richText; }

	UFUNCTION(BlueprintCallable, Category = "LGUI") FVector2D GetRealSize();

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetFont(ULGUIFontData_BaseObject* newFont);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetText(const FString& newText);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetFontSize(float newSize);
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
		void SetFontStyle(UITextFontStyle newFontStyle);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetRichText(bool newRichText);

	virtual void UpdateBasePrevData()override;
	virtual void UpdateCachedData()override;
	virtual void WidthChanged()override;
	virtual void HeightChanged()override;
#pragma region UITextInputComponent
	/** get caret position and line index */
	void FindCaretByIndex(int32 caretPositionIndex, FVector2D& outCaretPosition, int32& outCaretPositionLineIndex);
	/** find up of current caret position */
	void FindCaretUp(FVector2D& inOutCaretPosition, int32 inCaretPositionLineIndex, int32& outCaretPositionIndex);
	/** find down of current caret position */
	void FindCaretDown(FVector2D& inOutCaretPosition, int32 inCaretPositionLineIndex, int32& outCaretPositionIndex);
	/** find caret index by position */
	void FindCaretByPosition(FVector inWorldPosition, FVector2D& outCaretPosition, int32& outCaretPositionLineIndex, int32& outCaretPositionIndex);
	FString GetSubStringByLine(const FString& inString, int32& inOutLineStartIndex, int32& inOutLineEndIndex, int32& inOutCharStartIndex, int32& inOutCharEndIndex);

	/** range selection */
	void GetSelectionProperty(int32 InSelectionStartCaretIndex, int32 InSelectionEndCaretIndex, TArray<FUITextSelectionProperty>& OutSelectionProeprtyArray);
#pragma endregion UITextInputComponent
};
