// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "../UIEffectTextAnimation.h"
#include "LTweener.h"
#include "UIEffectTextAnimation_PropertyWithEase.generated.h"

UCLASS(ClassGroup = (LGUI), Abstract, BlueprintType)
class LGUI_API UUIEffectTextAnimation_PropertyWithEase : public UUIEffectTextAnimation_Property
{
	GENERATED_BODY()
private:
	friend class FUIEffectTextAnimationPropertyCustomization;
	/** Animation type, same as LTween ease */
	UPROPERTY(EditAnywhere, Category = "Property")
		ELTweenEase easeType = ELTweenEase::InOutSine;
	/** Only valid if easeType = CurveFloat. Use CurveFloat to control the animation. */
	UPROPERTY(EditAnywhere, Category = "Property", meta = (EditCondition = "easeType == ELTweenEase::CurveFloat"))
		UCurveFloat* easeCurve;
	FLTweenFunction easeFunc;
	float EaseCurveFunction(float c, float b, float t, float d);
protected:
	const FLTweenFunction& GetEaseFunction();
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		ELTweenEase GetEaseType()const { return easeType; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		UCurveFloat* GetCurveFloat()const { return easeCurve; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetEaseType(ELTweenEase value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetEaseCurve(UCurveFloat* value);
};

UCLASS(ClassGroup = (LGUI), BlueprintType, meta = (DisplayName = "Position Property (UI Effect TextAnimation)"))
class LGUI_API UUIEffectTextAnimation_PositionProperty : public UUIEffectTextAnimation_PropertyWithEase
{
	GENERATED_BODY()
private:
	UPROPERTY(EditAnywhere, Category = "Property")
		FVector position;
public:
	virtual void ApplyProperty(class UUIText* InUIText, const FUIEffectTextAnimation_SelectResult& InSelection, UIGeometry* InGeometry) override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FVector GetPosition()const { return position; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetPosition(FVector value);
};

UCLASS(ClassGroup = (LGUI), BlueprintType, meta = (DisplayName = "PositionRandom Property (UI Effect TextAnimation)"))
class LGUI_API UUIEffectTextAnimation_PositionRandomProperty : public UUIEffectTextAnimation_PropertyWithEase
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
	virtual void ApplyProperty(class UUIText* InUIText, const FUIEffectTextAnimation_SelectResult& InSelection, UIGeometry* InGeometry) override;

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

UCLASS(ClassGroup = (LGUI), BlueprintType, meta = (DisplayName = "Rotation Property (UI Effect TextAnimation)"))
class LGUI_API UUIEffectTextAnimation_RotationProperty : public UUIEffectTextAnimation_PropertyWithEase
{
	GENERATED_BODY()
private:
	UPROPERTY(EditAnywhere, Category = "Property")
		FRotator rotator;
public:
	virtual void ApplyProperty(class UUIText* InUIText, const FUIEffectTextAnimation_SelectResult& InSelection, UIGeometry* InGeometry) override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FRotator GetRotator()const { return rotator; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetRotator(FRotator value);
};

UCLASS(ClassGroup = (LGUI), BlueprintType, meta = (DisplayName = "RotationRandom Property (UI Effect TextAnimation)"))
class LGUI_API UUIEffectTextAnimation_RotationRandomProperty : public UUIEffectTextAnimation_PropertyWithEase
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
	virtual void ApplyProperty(class UUIText* InUIText, const FUIEffectTextAnimation_SelectResult& InSelection, UIGeometry* InGeometry) override;

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

UCLASS(ClassGroup = (LGUI), BlueprintType, meta = (DisplayName = "Scale Property (UI Effect TextAnimation)"))
class LGUI_API UUIEffectTextAnimation_ScaleProperty : public UUIEffectTextAnimation_PropertyWithEase
{
	GENERATED_BODY()
private:
	UPROPERTY(EditAnywhere, Category = "Property")
		FVector scale = FVector::OneVector;
public:
	virtual void ApplyProperty(class UUIText* InUIText, const FUIEffectTextAnimation_SelectResult& InSelection, UIGeometry* InGeometry) override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FVector GetScale()const { return scale; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetScale(FVector value);
};

UCLASS(ClassGroup = (LGUI), BlueprintType, meta = (DisplayName = "ScaleRandom Property (UI Effect TextAnimation)"))
class LGUI_API UUIEffectTextAnimation_ScaleRandomProperty : public UUIEffectTextAnimation_PropertyWithEase
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
	virtual void ApplyProperty(class UUIText* InUIText, const FUIEffectTextAnimation_SelectResult& InSelection, UIGeometry* InGeometry) override;

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

UCLASS(ClassGroup = (LGUI), BlueprintType, meta = (DisplayName = "Alpha Property (UI Effect TextAnimation)"))
class LGUI_API UUIEffectTextAnimation_AlphaProperty : public UUIEffectTextAnimation_PropertyWithEase
{
	GENERATED_BODY()
private:
	/** Target alpha value, 0-1 range. */
	UPROPERTY(EditAnywhere, Category = "Property", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float alpha;
public:
	virtual void ApplyProperty(class UUIText* InUIText, const FUIEffectTextAnimation_SelectResult& InSelection, UIGeometry* InGeometry) override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetAlpha()const { return alpha; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetAlpha(float value);
};

UCLASS(ClassGroup = (LGUI), BlueprintType, meta = (DisplayName = "Color Property (UI Effect TextAnimation)"))
class LGUI_API UUIEffectTextAnimation_ColorProperty : public UUIEffectTextAnimation_PropertyWithEase
{
	GENERATED_BODY()
private:
	UPROPERTY(EditAnywhere, Category = "Property")
		FColor color = FColor::Green;
	/** Conver color to HSV(Hue, Saturate, Value) and interpolate, then convert the result back. Interpolate two colors in HSV may look better. */
	UPROPERTY(EditAnywhere, Category = "Property")
		bool useHSV = true;
public:
	virtual void ApplyProperty(class UUIText* InUIText, const FUIEffectTextAnimation_SelectResult& InSelection, UIGeometry* InGeometry) override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FColor GetColor()const { return color; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetUseHSV()const { return useHSV; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetColor(FColor value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetUseHSV(bool value);
};

UCLASS(ClassGroup = (LGUI), BlueprintType, meta = (DisplayName = "ColorRandom Property (UI Effect TextAnimation)"))
class LGUI_API UUIEffectTextAnimation_ColorRandomProperty : public UUIEffectTextAnimation_PropertyWithEase
{
	GENERATED_BODY()
private:
	/** Random seed. */
	UPROPERTY(EditAnywhere, Category = "Property")
		int seed = 0;
	/** Random min. */
	UPROPERTY(EditAnywhere, Category = "Property")
		FColor min = FColor::Green;
	/** Random max. */
	UPROPERTY(EditAnywhere, Category = "Property")
		FColor max = FColor::Red;
	/** convert color to linear hsv, interlpate, and convert back to color */
	UPROPERTY(EditAnywhere, Category = "Property")
		bool useHSV = true;
public:
	virtual void ApplyProperty(class UUIText* InUIText, const FUIEffectTextAnimation_SelectResult& InSelection, UIGeometry* InGeometry) override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		int GetSeed()const { return seed; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FColor GetMin()const { return min; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FColor GetMax()const { return max; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetUseHSV()const { return useHSV; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetSeed(int value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetMin(FColor value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetMax(FColor value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetUseHSV(bool value);
};
