// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "../UIEffectTextAnimation.h"
#include "LTweener.h"
#include "UIEffectTextAnimation_Property.generated.h"

UCLASS(ClassGroup = (LGUI), BlueprintType, meta = (DisplayName = "Position"))
class LGUI_API UUIEffectTextAnimation_PositionProperty : public UUIEffectTextAnimation_Property
{
	GENERATED_BODY()
private:
	UPROPERTY(EditAnywhere, Category = "Property")
		FVector position;
public:
	virtual void ApplyProperty(class UUIText* InUIText, const TArray<FUIEffectTextAnimation_SelectResult>& InSelection, TSharedPtr<UIGeometry> OutGeometry) override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FVector GetPosition()const { return position; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetPosition(FVector value);
};

UCLASS(ClassGroup = (LGUI), BlueprintType, meta = (DisplayName = "PositionRandom"))
class LGUI_API UUIEffectTextAnimation_PositionRandomProperty : public UUIEffectTextAnimation_Property
{
	GENERATED_BODY()
private:
	//random seed
	UPROPERTY(EditAnywhere, Category = "Property")
		int seed = 0;
	//random min
	UPROPERTY(EditAnywhere, Category = "Property")
		FVector min = FVector(0, 0, 0);
	//random max
	UPROPERTY(EditAnywhere, Category = "Property")
		FVector max = FVector(0, 10, 0);
public:
	virtual void ApplyProperty(class UUIText* InUIText, const TArray<FUIEffectTextAnimation_SelectResult>& InSelection, TSharedPtr<UIGeometry> OutGeometry) override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		int GetSeed()const { return seed; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FVector GetMin()const { return min; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FVector GetMax()const { return max; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetSeed(int value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetMin(FVector value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetMax(FVector value);
};

UCLASS(ClassGroup = (LGUI), BlueprintType, meta = (DisplayName = "Rotation"))
class LGUI_API UUIEffectTextAnimation_RotationProperty : public UUIEffectTextAnimation_Property
{
	GENERATED_BODY()
private:
	UPROPERTY(EditAnywhere, Category = "Property")
		FRotator rotator;
public:
	virtual void ApplyProperty(class UUIText* InUIText, const TArray<FUIEffectTextAnimation_SelectResult>& InSelection, TSharedPtr<UIGeometry> OutGeometry) override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FRotator GetRotator()const { return rotator; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetRotator(FRotator value);
};

UCLASS(ClassGroup = (LGUI), BlueprintType, meta = (DisplayName = "RotationRandom"))
class LGUI_API UUIEffectTextAnimation_RotationRandomProperty : public UUIEffectTextAnimation_Property
{
	GENERATED_BODY()
private:
	//random seed
	UPROPERTY(EditAnywhere, Category = "Property")
		int seed = 0;
	//random min
	UPROPERTY(EditAnywhere, Category = "Property")
		FRotator min = FRotator(0, 0, 0);
	//random max
	UPROPERTY(EditAnywhere, Category = "Property")
		FRotator max = FRotator(0, 90, 0);
public:
	virtual void ApplyProperty(class UUIText* InUIText, const TArray<FUIEffectTextAnimation_SelectResult>& InSelection, TSharedPtr<UIGeometry> OutGeometry) override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		int GetSeed()const { return seed; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FRotator GetMin()const { return min; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FRotator GetMax()const { return max; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetSeed(int value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetMin(FRotator value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetMax(FRotator value);
};

UCLASS(ClassGroup = (LGUI), BlueprintType, meta = (DisplayName = "Scale"))
class LGUI_API UUIEffectTextAnimation_ScaleProperty : public UUIEffectTextAnimation_Property
{
	GENERATED_BODY()
private:
	UPROPERTY(EditAnywhere, Category = "Property")
		FVector scale = FVector::OneVector;
public:
	virtual void ApplyProperty(class UUIText* InUIText, const TArray<FUIEffectTextAnimation_SelectResult>& InSelection, TSharedPtr<UIGeometry> OutGeometry) override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FVector GetScale()const { return scale; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetScale(FVector value);
};

UCLASS(ClassGroup = (LGUI), BlueprintType, meta = (DisplayName = "ScaleRandom"))
class LGUI_API UUIEffectTextAnimation_ScaleRandomProperty : public UUIEffectTextAnimation_Property
{
	GENERATED_BODY()
private:
	//random seed
	UPROPERTY(EditAnywhere, Category = "Property")
		int seed = 0;
	//random min
	UPROPERTY(EditAnywhere, Category = "Property")
		FVector min = FVector(1, 1, 1);
	//random max
	UPROPERTY(EditAnywhere, Category = "Property")
		FVector max = FVector(2, 2, 2);
public:
	virtual void ApplyProperty(class UUIText* InUIText, const TArray<FUIEffectTextAnimation_SelectResult>& InSelection, TSharedPtr<UIGeometry> OutGeometry) override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		int GetSeed()const { return seed; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FVector GetMin()const { return min; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FVector GetMax()const { return max; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetSeed(int value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetMin(FVector value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetMax(FVector value);
};

UCLASS(ClassGroup = (LGUI), BlueprintType, meta = (DisplayName = "Alpha"))
class LGUI_API UUIEffectTextAnimation_AlphaProperty : public UUIEffectTextAnimation_Property
{
	GENERATED_BODY()
private:
	UPROPERTY(EditAnywhere, Category = "Property", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float alpha;
public:
	virtual void ApplyProperty(class UUIText* InUIText, const TArray<FUIEffectTextAnimation_SelectResult>& InSelection, TSharedPtr<UIGeometry> OutGeometry) override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetAlpha()const { return alpha; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetAlpha(float value);
};

UCLASS(ClassGroup = (LGUI), BlueprintType, meta = (DisplayName = "Color"))
class LGUI_API UUIEffectTextAnimation_ColorProperty : public UUIEffectTextAnimation_Property
{
	GENERATED_BODY()
private:
	UPROPERTY(EditAnywhere, Category = "Property")
		FColor color = FColor::Green;
public:
	virtual void ApplyProperty(class UUIText* InUIText, const TArray<FUIEffectTextAnimation_SelectResult>& InSelection, TSharedPtr<UIGeometry> OutGeometry) override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FColor GetColor()const { return color; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetColor(FColor value);
};

UCLASS(ClassGroup = (LGUI), BlueprintType, meta = (DisplayName = "ColorRandom"))
class LGUI_API UUIEffectTextAnimation_ColorRandomProperty : public UUIEffectTextAnimation_Property
{
	GENERATED_BODY()
private:
	//random seed
	UPROPERTY(EditAnywhere, Category = "Property")
		int seed = 0;
	//random min
	UPROPERTY(EditAnywhere, Category = "Property")
		FColor min = FColor::Green;
	//random max
	UPROPERTY(EditAnywhere, Category = "Property")
		FColor max = FColor::Red;
public:
	virtual void ApplyProperty(class UUIText* InUIText, const TArray<FUIEffectTextAnimation_SelectResult>& InSelection, TSharedPtr<UIGeometry> OutGeometry) override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		int GetSeed()const { return seed; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FColor GetMin()const { return min; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FColor GetMax()const { return max; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetSeed(int value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetMin(FColor value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetMax(FColor value);
};
