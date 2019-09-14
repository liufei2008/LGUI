// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "Core/LGUIFontData.h"
#include "UIRenderable.h"
#include "UIText.generated.h"


UENUM(BlueprintType)
enum class UITextParagraphHorizontalAlign : uint8
{
	Left			UMETA(DisplayName = "Left"),
	Center			UMETA(DisplayName = "Center"),
	Right			UMETA(DisplayName = "Right"),
};
UENUM(BlueprintType)
enum class UITextParagraphVerticalAlign : uint8
{
	Top				UMETA(DisplayName = "Top"),
	Middle			UMETA(DisplayName = "Middle"),
	Bottom			UMETA(DisplayName = "Bottom"),
};
UENUM(BlueprintType)
enum class UITextFontStyle :uint8
{
	None			UMETA(DisplayName = "None"),
	Bold			UMETA(DisplayName = "Bold"),
	Italic			UMETA(DisplayName = "Italic"),
	BoldAndItalic	UMETA(DisplayName = "BoldAndItalic"),
};

UENUM(BlueprintType)
enum class UITextOverflowType :uint8
{
	HorizontalOverflow,
	VerticalOverflow,
	ClampContent,
};

//single char property
struct FUITextCharProperty
{
	//caret position. caret is on left side of char
	FVector2D caretPosition;
	//char index in text
	int32 charIndex;
};
//a line of text property
struct FUITextLineProperty
{
	TArray<FUITextCharProperty> charPropertyList;
};
//for range selection in TextInputComponent
struct FUITextSelectionProperty
{
	FVector2D Pos;
	int32 Size;
};

//single char geometry
struct FUITextCharGeometry
{
	uint16 geoWidth = 0;
	uint16 geoHeight = 0;
	uint16 xadvance = 0;
	int16 xoffset = 0;
	int16 yoffset = 0;

	FVector2D uv0 = FVector2D(0, 0);
	FVector2D uv1 = FVector2D(0, 0);
	FVector2D uv2 = FVector2D(0, 0);
	FVector2D uv3 = FVector2D(0, 0);
};


UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIText : public UUIRenderable
{
	GENERATED_BODY()

public:	
	UUIText();
	~UUIText();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason)override;
	virtual void OnRegister()override;
	virtual void OnUnregister()override;
public:
	virtual void UpdateGeometry(const bool& parentTransformChanged) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)override;
	virtual void EditorForceUpdateImmediately() override;
protected:
	virtual void OnPreChangeFontProperty();
	virtual void OnPostChangeFontProperty();
#endif
#if WITH_EDITORONLY_DATA
	//current using font. the default font when creating new UIText
	static ULGUIFontData* CurrentUsingFontData;
#endif

protected:
	friend class FUITextCustomization;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		ULGUIFontData* font;
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
	//if overflowType = HorizontalOverflow then adjust widget width to true width
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool adjustWidth = false;
	//if overflowType = VerticalOverflow then adjust widget height to true height
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool adjustHeight = false;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		UITextFontStyle fontStyle = UITextFontStyle::None;
private:
	//visible/renderable char count of current text. -1 means not set yet
	int visibleCharCount = -1;
	//real size of this UIText, not the widget's width and height
	FVector2D textRealSize;
	void CreateGeometry();
	//cached texture property
	TArray<FUITextLineProperty> cachedTextPropertyList;
	
	void CheckCachedTextPropertyList();

	//cached text geometry list
	TArray<FUITextCharGeometry> cachedTextGeometryList;
	//calculate text geometry
	void CacheTextGeometry();

public:
	void ApplyFontTextureScaleUp();
	void ApplyFontTextureChange();
	void ApplyRecreateText();

	FVector2D GetCharSize(TCHAR character);
	FORCEINLINE static bool IsVisibleChar(TCHAR character)
	{
		return (character != 10 && character != 13 && character != 32);// 10 - /n, 13 - /r, 32 - space blank
	}
	//count visible char count of the string
	static int VisibleCharCountInString(const FString& srcStr);
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI") ULGUIFontData* GetFont()const { return font; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")	const FString& GetText()const { return text; }
	UFUNCTION(BlueprintCallable, Category = "LGUI") float GetSize()const { return size; }
	UFUNCTION(BlueprintCallable, Category = "LGUI") FVector2D GetFontSpace()const { return space; }
	UFUNCTION(BlueprintCallable, Category = "LGUI") UITextOverflowType GetOverflowType()const { return overflowType; }
	UFUNCTION(BlueprintCallable, Category = "LGUI") bool GetAdjustWidth()const { return adjustWidth; }
	UFUNCTION(BlueprintCallable, Category = "LGUI") bool GetAdjustHeight()const { return adjustHeight; }
	UFUNCTION(BlueprintCallable, Category = "LGUI") UITextFontStyle GetFontStyle()const { return fontStyle; }

	UFUNCTION(BlueprintCallable, Category = "LGUI") FVector2D GetRealSize();

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetFont(ULGUIFontData* newFont);
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

	virtual void UpdateBasePrevData()override;
	virtual void UpdateCachedData()override;
	virtual void WidthChanged()override;
	virtual void HeightChanged()override;
#pragma region UITextInputComponent
	//get caret position and line index
	void FindCaretByIndex(int32 caretPositionIndex, FVector2D& outCaretPosition, int32& outCaretPositionLineIndex);
	//find up of current caret position
	void FindCaretUp(FVector2D& inOutCaretPosition, int32 inCaretPositionLineIndex, int32& outCaretPositionIndex);
	//find down of current caret position
	void FindCaretDown(FVector2D& inOutCaretPosition, int32 inCaretPositionLineIndex, int32& outCaretPositionIndex);
	//find caret index by position
	void FindCaretByPosition(FVector inWorldPosition, FVector2D& outCaretPosition, int32& outCaretPositionLineIndex, int32& outCaretPositionIndex);

	//range selection
	void GetSelectionProperty(int32 InSelectionStartCaretIndex, int32 InSelectionEndCaretIndex, TArray<FUITextSelectionProperty>& OutSelectionProeprtyArray);
	//line count, return 0 means no char
	int GetLineCount();
#pragma endregion UITextInputComponent
};
