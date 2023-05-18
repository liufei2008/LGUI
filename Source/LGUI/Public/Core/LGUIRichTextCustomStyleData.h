// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RichTextParser.h"
#include "LGUIRichTextCustomStyleData.generated.h"

UENUM(BlueprintType)
enum class ELGUIRichTextCustomStyleData_SizeType :uint8
{
	KeepOrigin,
	SizeValue,
	SizeValueAsAdditional,
};
UENUM(BlueprintType)
enum class ELGUIRichTextCustomStyleData_ColorType : uint8
{
	KeepOrigin,
	Replace,
	Multiply,
};
UENUM(BlueprintType)
enum class ELGUIRichTextCustomStyleData_SupOrSubType : uint8
{
	KeepOrigin,
	None,
	Superscript,
	Subscript,
};

USTRUCT(BlueprintType)
struct FLGUIRichTextCustomStyleItemData
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool bold = false;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool italic = false;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool underline = false;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool strikethrough = false;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		ELGUIRichTextCustomStyleData_SizeType sizeType = ELGUIRichTextCustomStyleData_SizeType::KeepOrigin;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta=(EditCondition="sizeType!=ELGUIRichTextCustomStyleData_SizeType::KeepOrigin"))
		int size = 0;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		ELGUIRichTextCustomStyleData_ColorType colorType = ELGUIRichTextCustomStyleData_ColorType::KeepOrigin;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (EditCondition = "colorType!=ELGUIRichTextCustomStyleData_ColorType::KeepOrigin"))
		FColor color = FColor::White;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		ELGUIRichTextCustomStyleData_SupOrSubType supOrSub = ELGUIRichTextCustomStyleData_SupOrSubType::KeepOrigin;

	void ApplyToRichTextParseResult(LGUIRichTextParser::RichTextParseResult& value)const;
};

/**
 * For rich text on UIText.
 * Add your own string as tag and customize your own style.
 */
UCLASS(NotBlueprintable, BlueprintType)
class LGUI_API ULGUIRichTextCustomStyleData : public UObject
{
	GENERATED_BODY()
private:
	UPROPERTY(EditAnywhere, Category = "LGUI")
		TMap<FName, FLGUIRichTextCustomStyleItemData> DataMap;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		const TMap<FName, FLGUIRichTextCustomStyleItemData>& GetDataMap()const { return DataMap; }

	DECLARE_EVENT(ULGUIRichTextCustomStyleData, FLGUIRichTextCustomStyleDataRefreshEvent);
	/** Called when any data change, and need UIText to refresh. */
	FLGUIRichTextCustomStyleDataRefreshEvent OnDataChange;
};
