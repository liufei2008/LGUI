// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Text/TextLayout.h"
#include "LexUITextData.generated.h"

class FLexUIGeometry;
class ULexText;
class ULexUIFontData_BaseObject;
class ULexUIRichTextImageData;


UENUM(BlueprintType, Category = "LexUI")
enum class ELexUITextParagraphHorizontalAlign : uint8
{
	Left,
	Center,
	Right,
};

UENUM(BlueprintType, Category = "LexUI")
enum class ELexUITextParagraphVerticalAlign : uint8
{
	Top,
	Middle,
	Bottom,
};

UENUM(BlueprintType, Category = "LexUI")
enum class ELexUITextFontStyle :uint8
{
	None,
	Bold,
	Italic,
	BoldAndItalic,
};

UENUM(BlueprintType, Category = "LexUI")
enum class ELexUITextOverflowType :uint8
{
	/** chars will go out of rect range horizontally */
	HorizontalOverflow = 0,
	/** chars will go out of rect range vertically */
	VerticalOverflow = 1,
	/** remove chars on right if out of range */
	Truncate = 2,
	/** replace chars with ... if out of range */
	Ellipsis = 3,
};

/** single char property */
struct FLexUITextCaretProperty
{
	/** caret position. caret is on left side of char */
	FVector2f CaretPosition = FVector2f::ZeroVector;
	/** char index in text, -1 means line end caret */
	int32 CharIndex = 0;
};
/** a line of text property */
struct FLexUITextLineProperty
{
	TArray<FLexUITextCaretProperty> CaretPropertyList;
};
/** for range selection in TextInputComponent */
struct FLexUITextSelectionProperty
{
	FVector2f Pos = FVector2f::ZeroVector;
	int32 Size = 0;
};
/** char property */
USTRUCT(BlueprintType, Category = "LexUI")
struct FLexUITextCharProperty
{
	GENERATED_BODY()
	/** char index in string */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LexUI") int32 CharIndex = 0;
	/** vertex index in UIGeometry::vertices */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LexUI") int32 StartVertIndex = 0;
	/** vertex count */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LexUI") int32 VertCount = 0;
	/** triangle index in UIGeometry::triangles */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LexUI") int32 StartTriangleIndex = 0;
	/** triangle indices count */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LexUI") int32 IndicesCount = 0;

	/** center position of the char, in UIText's local space */
	//FVector2D CenterPosition;
};

USTRUCT(BlueprintType, Category = "LexUI")
struct FLexUIText_RichTextCustomTag
{
	GENERATED_BODY()
	/** Tag name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LexUI") FName TagName;
	/** start char index in cacheCharPropertyArray */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LexUI") int32 CharIndexStart = 0;
	/** end char index in cacheCharPropertyArray */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LexUI") int32 CharIndexEnd = 0;
};

USTRUCT(BlueprintType, Category = "LexUI")
struct FLexUIText_RichTextImageTag
{
	GENERATED_BODY()
	/** Tag name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LexUI") FName TagName;
	/** image object position */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LexUI") FVector2D Position = FVector2D::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LexUI") FVector2D Size = FVector2D::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LexUI") FColor TintColor = FColor::White;
};

USTRUCT(BlueprintType, Category = "LexUI")
struct FLexUIText_Emoji
{
	GENERATED_BODY()
	/** Emoji char */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LexUI") int32 EmojiCode = 0;
	/** image object position */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LexUI") FVector2D Position = FVector2D::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LexUI") FVector2D Size = FVector2D::ZeroVector;
};

UENUM(BlueprintType, meta = (Bitflags), Category = "LexUI")
enum class ELexUIText_RichTextTagFilterFlags : uint8
{
	Bold, Italic, Underline, Strikethrough, Size, Color, Superscript, Subscript, CustomTag, Image
};
ENUM_CLASS_FLAGS(ELexUIText_RichTextTagFilterFlags);

enum class ELexUIText_CodeType
{
	Text = 0,
	Emoji = 1,
};
struct FLexUIText_TextProcessingElement
{
	uint32 Unicode;
	int StringIndex;
	int Length;
	ELexUIText_CodeType Type;
};

/// <summary>
/// Commonly referenced Unicode characters in the text generation process.
/// </summary>
namespace FLexUIText_CodePoint
{
	// constexpr uint32 SPACE = 0x20;
	// constexpr uint32 DOUBLE_QUOTE = 0x22;
	// constexpr uint32 NUMBER_SIGN = 0x23;
	// constexpr uint32 PERCENTAGE = 0x25;
	// constexpr uint32 PLUS = 0x2B;
	// constexpr uint32 MINUS = 0x2D;
	// constexpr uint32 PERIOD = 0x2E;
	//
	// constexpr uint32 HYPHEN_MINUS = 0x2D;
	// constexpr uint32 SOFT_HYPHEN = 0xAD;
	// constexpr uint32 HYPHEN = 0x2010;
	// constexpr uint32 NON_BREAKING_HYPHEN = 0x2011;
	// constexpr uint32 ZERO_WIDTH_SPACE = 0x200B;
	// constexpr uint32 RIGHT_SINGLE_QUOTATION = 0x2019;
	// constexpr uint32 APOSTROPHE = 0x27;
	// constexpr uint32 WORD_JOINER = 0x2060;
	constexpr uint32 HIGH_SURROGATE_START = 0xD800;
	constexpr uint32 HIGH_SURROGATE_END = 0xDBFF;
	constexpr uint32 LOW_SURROGATE_START = 0xDC00;
	constexpr uint32 LOW_SURROGATE_END = 0xDFFF;
	constexpr uint32 UNICODE_PLANE01_START = 0x10000;
	constexpr uint32 UNICODE_VS_BLACK = 0xFE0E;
	constexpr uint32 UNICODE_VS_COLOR = 0xFE0F;

	inline uint32 ConvertToUTF32(uint32 highSurrogate, uint32 lowSurrogate)
	{
		return (((highSurrogate - FLexUIText_CodePoint::HIGH_SURROGATE_START) << 10) | (lowSurrogate - FLexUIText_CodePoint::LOW_SURROGATE_START)) + FLexUIText_CodePoint::UNICODE_PLANE01_START;
	}
	inline bool IsEmoji(uint32 Codepoint)
	{
		// Emoji Block 1
		if (Codepoint >= 0x1F300 && Codepoint <= 0x1F5FF) return true;

		// Emoticons
		if (Codepoint >= 0x1F600 && Codepoint <= 0x1F64F) return true;

		// Transport & Map Symbols
		if (Codepoint >= 0x1F680 && Codepoint <= 0x1F6FF) return true;

		// Supplemental Symbols and Pictographs
		if (Codepoint >= 0x1F900 && Codepoint <= 0x1F9FF) return true;

		// Symbols and Pictographs Extended-A
		if (Codepoint >= 0x1FA70 && Codepoint <= 0x1FAFF) return true;

		// Flags (Regional Indicator Symbols)
		if (Codepoint >= 0x1F1E6 && Codepoint <= 0x1F1FF) return true;

		return false;
	}
	inline FLexUIText_TextProcessingElement ReadCodePoint(const FString& InString, int InStringLen, int& InOutCharIndex)
	{
		auto charCode = InString[InOutCharIndex];
		if (charCode >= FLexUIText_CodePoint::HIGH_SURROGATE_START && charCode <= FLexUIText_CodePoint::HIGH_SURROGATE_END
				&& InOutCharIndex + 1 < InStringLen
				&& InString[InOutCharIndex + 1] >= FLexUIText_CodePoint::LOW_SURROGATE_START && InString[InOutCharIndex + 1] <= FLexUIText_CodePoint::LOW_SURROGATE_END)
		{
			FLexUIText_TextProcessingElement Element;
			Element.Unicode = FLexUIText_CodePoint::ConvertToUTF32(charCode, InString[InOutCharIndex + 1]);
			Element.StringIndex = InOutCharIndex;
			Element.Type = IsEmoji(Element.Unicode) ? ELexUIText_CodeType::Emoji : ELexUIText_CodeType::Text;
			if (InOutCharIndex + 2 < InStringLen
				&& (InString[InOutCharIndex + 2] == FLexUIText_CodePoint::UNICODE_VS_BLACK || InString[InOutCharIndex + 2] == FLexUIText_CodePoint::UNICODE_VS_COLOR))
			{
				Element.Length = 3;
				InOutCharIndex+=2;
			}
			else
			{
				Element.Length = 2;
				InOutCharIndex+=1;
			}
			return Element;
		}
		else
		{
			return FLexUIText_TextProcessingElement{charCode, InOutCharIndex, 1, ELexUIText_CodeType::Text};
		}
	}
};

struct LGUI_API FLexUITextGeometryCache
{
public:
	FLexUITextGeometryCache() {}
	FLexUITextGeometryCache(ULexText* InUIText);
	/**
	 * @return true - anything change
	 */
	bool SetInputParameters(
		const FString& InContent,
		float InWidth,
		float InHeight,
		FVector2f InPivot,
		FColor InColor,
		float InRenderOpacityForRichText,
		FVector2f InFontSpace,
		float InFontSize,
		ELexUITextParagraphHorizontalAlign InParagraphHAlign,
		ELexUITextParagraphVerticalAlign InParagraphVAlign,
		ELexUITextOverflowType InOverflowType,
		ETextWrappingPolicy InWrappingPolicy,
		bool InUseKerning,
		ELexUITextFontStyle InFontStyle,
		bool InRichText,
		int32 InRichTextFilterFlags,
		ULexUIFontData_BaseObject* InFont
	);
private:
#pragma region InputParameters
	TArray<FLexUIText_TextProcessingElement> TextProcessingArray;
	FString Content = TEXT("");
	float Width = 0;
	float Height = 0;
	FVector2f Pivot = FVector2f::ZeroVector;
	FColor Color = FColor::White;
	float RenderOpacityForRichText = 1.0f;
	FVector2f FontSpace = FVector2f::ZeroVector;
	float FontSize = 0;
	ELexUITextParagraphHorizontalAlign ParagraphHAlign = ELexUITextParagraphHorizontalAlign::Left;
	ELexUITextParagraphVerticalAlign ParagraphVAlign = ELexUITextParagraphVerticalAlign::Bottom;
	ELexUITextOverflowType OverflowType = ELexUITextOverflowType::HorizontalOverflow;
	ETextWrappingPolicy WrappingPolicy = ETextWrappingPolicy::AllowPerCharacterWrapping;
	bool bUseKerning = false;
	ELexUITextFontStyle FontStyle = ELexUITextFontStyle::None;
	bool bRichText = false;
	int32 RichTextFilterFlags = 0xffffffff;
	TWeakObjectPtr<ULexUIFontData_BaseObject> Font = nullptr;
#pragma endregion InputParameters

	bool bIsDirty = true;//vertex or triangle data is dirty
	bool bIsColorDirty = true;//only color data is dirty (no include rich text's color)
	TWeakObjectPtr<ULexText> TextComp = nullptr;

public:
#pragma region OutputResults
	/** indicating whether the text is Truncated or using Ellipsis */
	bool textTruncated = false;
	FVector2f textPreferredSize = FVector2f::ZeroVector;
	/** line properties, from first line to last one in array */
	TArray<FLexUITextLineProperty> cacheLinePropertyArray;
	/** char properties, from first char to last one in array */
	TArray<FLexUITextCharProperty> cacheCharPropertyArray;
	TArray<FLexUIText_RichTextCustomTag> cacheRichTextCustomTagArray;
	TArray<FLexUIText_RichTextImageTag> cacheRichTextImageTagArray;
	TArray<FLexUIText_Emoji> cacheEmojiArray;
#pragma endregion OutputResults
	void MarkDirty();
	/** check if dirty before calculate geometry */
	void ConditionalCalculateGeometry();
};
