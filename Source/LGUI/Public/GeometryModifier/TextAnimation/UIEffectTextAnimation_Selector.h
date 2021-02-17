// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "../UIEffectTextAnimation.h"
#include "UIEffectTextAnimation_Selector.generated.h"

class UCurveFloat;
UCLASS(ClassGroup = (LGUI), BlueprintType, meta = (DisplayName = "Range"))
class LGUI_API UUIEffectTextAnimation_RangeSelector : public UUIEffectTextAnimation_Selector
{
	GENERATED_BODY()
private:
	UPROPERTY(EditAnywhere, Category = "Property", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float range = 0.1f;
	UPROPERTY(EditAnywhere, Category = "Property")
		bool flipDirection = false;
	UPROPERTY(EditAnywhere, Category = "Property", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float start = 0.0f;
	UPROPERTY(EditAnywhere, Category = "Property", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float end = 1.0f;
public:
	virtual bool Select(class UUIText* InUIText, FUIEffectTextAnimation_SelectResult& OutSelection)override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetRange()const { return range; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetFlipDirection()const { return flipDirection; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetStart()const { return start; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetEnd()const { return end; }
	
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetRange(float value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetFlipDirection(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetStart(float value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetEnd(float value);
};

UCLASS(ClassGroup = (LGUI), BlueprintType, meta = (DisplayName = "Random"))
class LGUI_API UUIEffectTextAnimation_RandomSelector : public UUIEffectTextAnimation_Selector
{
	GENERATED_BODY()
private:
	//random seed
	UPROPERTY(EditAnywhere, Category = "Property")
		int seed = 0;
	UPROPERTY(EditAnywhere, Category = "Property", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float start = 0.0f;
	UPROPERTY(EditAnywhere, Category = "Property", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float end = 1.0f;
public:
	virtual bool Select(class UUIText* InUIText, FUIEffectTextAnimation_SelectResult& OutSelection)override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		int GetSeed()const { return seed; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetStart()const { return start; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetEnd()const { return end; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetSeed(int value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetStart(float value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetEnd(float value);
};

UCLASS(ClassGroup = (LGUI), BlueprintType, meta = (DisplayName = "RichTextTag"))
class LGUI_API UUIEffectTextAnimation_RichTextTagSelector : public UUIEffectTextAnimation_Selector
{
	GENERATED_BODY()
private:
	UPROPERTY(EditAnywhere, Category = "Property", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float range = 1.0f;
	UPROPERTY(EditAnywhere, Category = "Property")
		FName tagName;
	UPROPERTY(EditAnywhere, Category = "Property")
		bool flipDirection = false;
public:
	virtual bool Select(class UUIText* InUIText, FUIEffectTextAnimation_SelectResult& OutSelection)override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetRange()const { return range; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetFlipDirection()const { return flipDirection; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		const FName& GetTagName()const { return tagName; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetTagName(const FName& value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetRange(float value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetFlipDirection(bool value);
};