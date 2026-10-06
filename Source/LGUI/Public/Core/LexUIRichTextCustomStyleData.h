// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FRichTextParser.h"
#include "LexUIRichTextCustomStyleData.generated.h"

UENUM(BlueprintType)
enum class ELexUIRichTextCustomStyleData_SizeType :uint8
{
	KeepOrigin,
	SizeValue,
	SizeValueAsAdditional,
};
UENUM(BlueprintType)
enum class ELexUIRichTextCustomStyleData_ColorType : uint8
{
	KeepOrigin,
	Replace,
	Multiply,
};
UENUM(BlueprintType)
enum class ELexUIRichTextCustomStyleData_SupOrSubType : uint8
{
	KeepOrigin,
	None,
	Superscript,
	Subscript,
};

USTRUCT(BlueprintType)
struct FLexUIRichTextCustomStyleItemData
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "LexUI")
		bool bold = false;
	UPROPERTY(EditAnywhere, Category = "LexUI")
		bool italic = false;
	UPROPERTY(EditAnywhere, Category = "LexUI")
		bool underline = false;
	UPROPERTY(EditAnywhere, Category = "LexUI")
		bool strikethrough = false;
	UPROPERTY(EditAnywhere, Category = "LexUI")
		ELexUIRichTextCustomStyleData_SizeType sizeType = ELexUIRichTextCustomStyleData_SizeType::KeepOrigin;
	UPROPERTY(EditAnywhere, Category = "LexUI", meta=(EditCondition="sizeType!=ELexUIRichTextCustomStyleData_SizeType::KeepOrigin"))
		int size = 0;
	UPROPERTY(EditAnywhere, Category = "LexUI")
		ELexUIRichTextCustomStyleData_ColorType colorType = ELexUIRichTextCustomStyleData_ColorType::KeepOrigin;
	UPROPERTY(EditAnywhere, Category = "LexUI", meta = (EditCondition = "colorType!=ELexUIRichTextCustomStyleData_ColorType::KeepOrigin"))
		FColor color = FColor::White;
	UPROPERTY(EditAnywhere, Category = "LexUI")
		ELexUIRichTextCustomStyleData_SupOrSubType supOrSub = ELexUIRichTextCustomStyleData_SupOrSubType::KeepOrigin;

	void ApplyToRichTextParseResult(LexUIRichTextParser::FRichTextParseResult& value)const;
};

/**
 * For rich text on UIText.
 * Add your own string as tag and customize your own style.
 */
UCLASS(NotBlueprintable, BlueprintType)
class LGUI_API ULexUIRichTextCustomStyleData : public UObject
{
	GENERATED_BODY()
private:
	UPROPERTY(EditAnywhere, Category = "LexUI")
		TMap<FName, FLexUIRichTextCustomStyleItemData> DataMap;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
public:
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		const TMap<FName, FLexUIRichTextCustomStyleItemData>& GetDataMap()const { return DataMap; }

	DECLARE_EVENT(ULexUIRichTextCustomStyleData, FLGUIRichTextCustomStyleDataRefreshEvent);
	/** Called when any data change, and need UIText to refresh. */
	FLGUIRichTextCustomStyleDataRefreshEvent OnDataChange;
};
