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
		FColor color = FColor::White;
public:
	virtual void ApplyProperty(class UUIText* InUIText, const TArray<FUIEffectTextAnimation_SelectResult>& InSelection, TSharedPtr<UIGeometry> OutGeometry) override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FColor GetColor()const { return color; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetColor(FColor value);
};
