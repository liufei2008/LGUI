// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LGUITextData.generated.h"

class UIGeometry;
class UUIText;
class ULGUIFontData_BaseObject;
class ULGUIRichTextImageData;


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
	HorizontalOverflow = 0,
	/** chars will go out of rect range vertically */
	VerticalOverflow = 1,
	/** if with less than maxHorizontalWidth then use HorizontalOverlow, if grater than maxHorizontalWidth then use VerticalOverflow */
	HorizontalAndVerticalOverflow = 3,
	/** remove chars on right if out of range */
	ClampContent = 2,
};

/** single char property */
struct FUITextCaretProperty
{
	/** caret position. caret is on left side of char */
	FVector2f caretPosition = FVector2f::ZeroVector;
	/** char index in text, -1 means line end caret */
	int32 charIndex = 0;
};
/** a line of text property */
struct FUITextLineProperty
{
	TArray<FUITextCaretProperty> caretPropertyList;
};
/** for range selection in TextInputComponent */
struct FUITextSelectionProperty
{
	FVector2f Pos = FVector2f::ZeroVector;
	int32 Size = 0;
};
/** char property */
USTRUCT(BlueprintType, Category = LGUI)
struct FUITextCharProperty
{
	GENERATED_BODY()
	/** char index in string */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LGUI) int32 CharIndex = 0;
	/** vertex index in UIGeometry::vertices */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LGUI) int32 StartVertIndex = 0;
	/** vertex count */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LGUI) int32 VertCount = 0;
	/** triangle index in UIGeometry::triangles */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LGUI) int32 StartTriangleIndex = 0;
	/** triangle indices count */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LGUI) int32 IndicesCount = 0;

	/** center position of the char, in UIText's local space */
	//FVector2D CenterPosition;
};

USTRUCT(BlueprintType, Category = LGUI)
struct FUIText_RichTextCustomTag
{
	GENERATED_BODY()
	/** Tag name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LGUI) FName TagName;
	/** start char index in cacheCharPropertyArray */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LGUI) int32 CharIndexStart = 0;
	/** end char index in cacheCharPropertyArray */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LGUI) int32 CharIndexEnd = 0;
};

USTRUCT(BlueprintType, Category = LGUI)
struct FUIText_RichTextImageTag
{
	GENERATED_BODY()
	/** Tag name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LGUI) FName TagName;
	/** image object position */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LGUI) FVector2D Position = FVector2D::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LGUI) float Size = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LGUI) FColor TintColor = FColor::White;
};

UENUM(BlueprintType, meta = (Bitflags), Category = LGUI)
enum class EUIText_RichTextTagFilterFlags : uint8
{
	Bold, Italic, Underline, Strikethrough, Size, Color, Superscript, Subscript, CustomTag, Image
};
ENUM_CLASS_FLAGS(EUIText_RichTextTagFilterFlags);

struct LGUI_API FTextGeometryCache
{
public:
	FTextGeometryCache() {}
	FTextGeometryCache(UUIText* InUIText);
	/**
	 * @return true - anything change
	 */
	bool SetInputParameters(
		const FString& InContent,
		int32 InVisibleCharCount,
		float InWidth,
		float InHeight,
		FVector2f InPivot,
		FColor InColor,
		float InCanvasGroupAlpha,
		FVector2f InFontSpace,
		float InFontSize,
		UITextParagraphHorizontalAlign InParagraphHAlign,
		UITextParagraphVerticalAlign InParagraphVAlign,
		UITextOverflowType InOverflowType,
		float InMaxHorizontalWidth,
		bool InUseKerning,
		UITextFontStyle InFontStyle,
		bool InRichText,
		int32 InRichTextFilterFlags,
		ULGUIFontData_BaseObject* InFont
	);
private:
#pragma region InputParameters
	FString content = TEXT("");
	int32 visibleCharCount = -1;
	float width = 0;
	float height = 0;
	FVector2f pivot = FVector2f::ZeroVector;
	FColor color = FColor::White;
	float canvasGroupAlpha = 1.0f;
	FVector2f fontSpace = FVector2f::ZeroVector;
	float fontSize = 0;
	UITextParagraphHorizontalAlign paragraphHAlign = UITextParagraphHorizontalAlign::Left;
	UITextParagraphVerticalAlign paragraphVAlign = UITextParagraphVerticalAlign::Bottom;
	UITextOverflowType overflowType = UITextOverflowType::HorizontalOverflow;
	float maxHorizontalWidth = 100;
	bool useKerning = false;
	UITextFontStyle fontStyle = UITextFontStyle::None;
	bool richText = false;
	int32 richTextFilterFlags = 0xffffffff;
	TWeakObjectPtr<ULGUIFontData_BaseObject> font = nullptr;
#pragma endregion InputParameters

	bool bIsDirty = true;//vertex or triangle data is dirty
	bool bIsColorDirty = true;//only color data is dirty (no include rich text's color)
	TWeakObjectPtr<UUIText> UIText = nullptr;

public:
#pragma region OutputResults
	FVector2f textRealSize = FVector2f::ZeroVector;
	/** line properties, from first line to last one in array */
	TArray<FUITextLineProperty> cacheLinePropertyArray;
	/** char properties, from first char to last one in array */
	TArray<FUITextCharProperty> cacheCharPropertyArray;
	TArray<FUIText_RichTextCustomTag> cacheRichTextCustomTagArray;
	TArray<FUIText_RichTextImageTag> cacheRichTextImageTagArray;
#pragma endregion OutputResults
public:
	void MarkDirty();
	/** check if dirty before calculate geometry */
	void ConditaionalCalculateGeometry();
};
