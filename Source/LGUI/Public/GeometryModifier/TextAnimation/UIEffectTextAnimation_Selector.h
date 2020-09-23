// Copyright 2019-2020 LexLiu. All Rights Reserved.

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
	UPROPERTY(EditAnywhere, Category = "Property", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float offset = 0.5f;
	UPROPERTY(EditAnywhere, Category = "Property")
		bool flipDirection = false;
public:
	virtual bool Select(class UUIText* InUIText, TArray<FUIEffectTextAnimation_SelectResult>& OutSelection)override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetRange()const { return range; }
	UFUNCTION(BlueprintCallable, Category="LGUI")
		float GetOffset()const { return offset; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetFlipDirection()const { return flipDirection; }
	
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetRange(float value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetOffset(float value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetFlipDirection(bool value);
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
		float offset = 0.5f;
public:
	virtual bool Select(class UUIText* InUIText, TArray<FUIEffectTextAnimation_SelectResult>& OutSelection)override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		int GetSeed()const { return seed; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetOffset()const { return offset; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetSeed(int value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetOffset(float value);
};