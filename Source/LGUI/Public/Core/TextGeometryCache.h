﻿// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TextGeometryCache.generated.h"

class UIGeometry;
class UUIText;
class ULGUIFontData_BaseObject;


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
	FVector2D caretPosition = FVector2D::ZeroVector;
	/** char index in text */
	int32 charIndex = 0;
};
/** a line of text property */
struct FUITextLineProperty
{
	TArray<FUITextCaretProperty> charPropertyList;
};
/** for range selection in TextInputComponent */
struct FUITextSelectionProperty
{
	FVector2D Pos = FVector2D::ZeroVector;
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
		FVector2D InPivot,
		FColor InColor,
		float InCanvasGroupAlpha,
		FVector2D InFontSpace,
		float InFontSize,
		UITextParagraphHorizontalAlign InParagraphHAlign,
		UITextParagraphVerticalAlign InParagraphVAlign,
		UITextOverflowType InOverflowType,
		bool InAdjustWidth,
		bool InAdjustHeight,
		bool InUseKerning,
		UITextFontStyle InFontStyle,
		bool InRichText,
		TWeakObjectPtr<ULGUIFontData_BaseObject> InFont
	);
private:
#pragma region InputParameters
	FString content = TEXT("");
	int32 visibleCharCount = -1;
	float width = 0;
	float height = 0;
	FVector2D pivot = FVector2D::ZeroVector;
	FColor color = FColor::White;
	float canvasGroupAlpha = 1.0f;
	FVector2D fontSpace = FVector2D::ZeroVector;
	float fontSize = 0;
	UITextParagraphHorizontalAlign paragraphHAlign = UITextParagraphHorizontalAlign::Left;
	UITextParagraphVerticalAlign paragraphVAlign = UITextParagraphVerticalAlign::Bottom;
	UITextOverflowType overflowType = UITextOverflowType::HorizontalOverflow;
	bool adjustWidth = false;
	bool adjustHeight = false;
	bool useKerning = false;
	UITextFontStyle fontStyle = UITextFontStyle::None;
	bool richText = false;
	TWeakObjectPtr<ULGUIFontData_BaseObject> font = nullptr;
#pragma endregion InputParameters

	bool bIsDirty = true;//vertex or triangle data is dirty
	bool bIsColorDirty = true;//only color data is dirty (no include rich text's color)
	TWeakObjectPtr<UUIText> UIText = nullptr;

public:
#pragma region OutputResults
	FVector2D textRealSize = FVector2D::ZeroVector;
	/** cached texture property */
	TArray<FUITextLineProperty> cacheTextPropertyArray;
	/** char properties, from first char to last one in array */
	TArray<FUITextCharProperty> cacheCharPropertyArray;
	TArray<FUIText_RichTextCustomTag> cacheRichTextCustomTagArray;
#pragma endregion OutputResults
public:
	void MarkDirty();
	/** check if dirty before calculate geometry */
	void ConditaionalCalculateGeometry();
};
