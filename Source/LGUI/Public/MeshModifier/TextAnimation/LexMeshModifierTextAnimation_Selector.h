// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "../LexMeshModifierTextAnimation.h"
#include "LexMeshModifierTextAnimation_Selector.generated.h"

class UCurveFloat;

/** Range selector defines start and end range of characters in UIText, and provide 0 to 1 value(for interpolation) from start to end. */
UCLASS(ClassGroup = (LGUI), BlueprintType, meta = (DisplayName = "Range Selector (UI Effect TextAnimation)"))
class LGUI_API ULexMeshModifierTextAnimation_RangeSelector : public ULexMeshModifierTextAnimation_Selector
{
	GENERATED_BODY()
private:
	/** *Selector* can provide 0 to 1 value from start to end, but sometime *Properties* effect may look too smooth, so lower this value can let *Properties* effect more sharp. */
	UPROPERTY(EditAnywhere, Category = "Property", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float Range = 0.1f;
	/** *Selector* can provide 0 to 1 value from start to end when this value is false, if it is true then 1 to 0 from start to end. */
	UPROPERTY(EditAnywhere, Category = "Property")
		bool bFlipDirection = false;
	/** Start character position from 0 to 1, 0 is first character of text, 1 is last one. */
	UPROPERTY(EditAnywhere, Category = "Property", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float Start = 0.0f;
	/** End character position from 0 to 1, 0 is first character of text, 1 is last one. */
	UPROPERTY(EditAnywhere, Category = "Property", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float End = 1.0f;
public:
	virtual bool Select(ULexText* InUIText, FLexMeshModifierTextAnimation_SelectResult& OutSelection)override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetRange()const { return Range; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetFlipDirection()const { return bFlipDirection; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetStart()const { return Start; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetEnd()const { return End; }
	
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetRange(float Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetFlipDirection(bool Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetStart(float Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetEnd(float Value);
};

/** Random selector will select characters randomly, and generate random value from 0 to 1 for interpolation. */
UCLASS(ClassGroup = (LGUI), BlueprintType, meta = (DisplayName = "Random Selector (UI Effect TextAnimation)"))
class LGUI_API ULexMeshModifierTextAnimation_RandomSelector : public ULexMeshModifierTextAnimation_Selector
{
	GENERATED_BODY()
private:
	/** Random seed. */
	UPROPERTY(EditAnywhere, Category = "Property")
		int Seed = 0;
	/** Start character position from 0 to 1, 0 is first character of text, 1 is last one. */
	UPROPERTY(EditAnywhere, Category = "Property", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float Start = 0.0f;
	/** End character position from 0 to 1, 0 is first character of text, 1 is last one. */
	UPROPERTY(EditAnywhere, Category = "Property", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float End = 1.0f;
public:
	virtual bool Select(ULexText* InUIText, FLexMeshModifierTextAnimation_SelectResult& OutSelection)override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		int GetSeed()const { return Seed; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetStart()const { return Start; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetEnd()const { return End; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetSeed(int Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetStart(float Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetEnd(float Value);
};

/** RichTextTag selector can select characters by rich-text custom-tag, and provide 0 to 1 value(for interpolation) from start to end. */
UCLASS(ClassGroup = (LGUI), BlueprintType, meta = (DisplayName = "RichTextTag Selector (UI Effect TextAnimation)"))
class LGUI_API ULexMeshModifierTextAnimation_RichTextTagSelector : public ULexMeshModifierTextAnimation_Selector
{
	GENERATED_BODY()
private:
	/** Like the property in RangeSelector. Lower this value can let *Properties* effect more sharp. */
	UPROPERTY(EditAnywhere, Category = "Property", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float Range = 1.0f;
	/** Custom tag name */
	UPROPERTY(EditAnywhere, Category = "Property")
		FName TagName;
	/** Like the property in RangeSelector, flip 0-1 to 1-0. */
	UPROPERTY(EditAnywhere, Category = "Property")
		bool bFlipDirection = false;
public:
	virtual bool Select(ULexText* InUIText, FLexMeshModifierTextAnimation_SelectResult& OutSelection)override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetRange()const { return Range; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetFlipDirection()const { return bFlipDirection; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		const FName& GetTagName()const { return TagName; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetTagName(const FName& Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetRange(float Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetFlipDirection(bool Value);
};