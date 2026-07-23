// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "../LexMeshModifierTextAnimation.h"
#include "LTweener.h"
#include "LexMeshModifierTextAnimation_PropertyWithEase.generated.h"

UCLASS(ClassGroup = (LGUI), Abstract, BlueprintType)
class LGUI_API ULexMeshModifierTextAnimation_PropertyWithEase : public ULexMeshModifierTextAnimation_Property
{
	GENERATED_BODY()
private:
	friend class FUIEffectTextAnimationPropertyCustomization;
	/** Animation type, same as LTween ease */
	UPROPERTY(EditAnywhere, Category = "Property")
		ELTweenEase EaseType = ELTweenEase::InOutSine;
	/** Only valid if easeType = CurveFloat. Use CurveFloat to control the animation. */
	UPROPERTY(EditAnywhere, Category = "Property", meta = (EditCondition = "EaseType == ELTweenEase::CurveFloat"))
		TObjectPtr<UCurveFloat> EaseCurve;
	FLTweenFunction EaseFunc;
	float EaseCurveFunction(float c, float b, float t, float d);
protected:
	const FLTweenFunction& GetEaseFunction();
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		ELTweenEase GetEaseType()const { return EaseType; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		UCurveFloat* GetCurveFloat()const { return EaseCurve; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetEaseType(ELTweenEase Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetEaseCurve(UCurveFloat* Value);
};

UCLASS(ClassGroup = (LGUI), BlueprintType, meta = (DisplayName = "Position Property (UI Effect TextAnimation)"))
class LGUI_API ULexMeshModifierTextAnimation_PositionProperty : public ULexMeshModifierTextAnimation_PropertyWithEase
{
	GENERATED_BODY()
private:
	UPROPERTY(EditAnywhere, Category = "Property")
		FVector Position;
public:
	virtual void ApplyProperty(class ULexText* InUIText, const FLexMeshModifierTextAnimation_SelectResult& InSelection, FLexUIGeometry* InGeometry) override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FVector GetPosition()const { return Position; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetPosition(FVector Value);
};

UCLASS(ClassGroup = (LGUI), BlueprintType, meta = (DisplayName = "PositionRandom Property (UI Effect TextAnimation)"))
class LGUI_API ULexMeshModifierTextAnimation_PositionRandomProperty : public ULexMeshModifierTextAnimation_PropertyWithEase
{
	GENERATED_BODY()
private:
	//random seed
	UPROPERTY(EditAnywhere, Category = "Property")
		int Seed = 0;
	//random min
	UPROPERTY(EditAnywhere, Category = "Property")
		FVector Min = FVector(0, 0, 0);
	//random max
	UPROPERTY(EditAnywhere, Category = "Property")
		FVector Max = FVector(0, 10, 0);
public:
	virtual void ApplyProperty(class ULexText* InUIText, const FLexMeshModifierTextAnimation_SelectResult& InSelection, FLexUIGeometry* InGeometry) override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		int GetSeed()const { return Seed; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FVector GetMin()const { return Min; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FVector GetMax()const { return Max; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetSeed(int Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetMin(FVector Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetMax(FVector Value);
};

UCLASS(ClassGroup = (LGUI), BlueprintType, meta = (DisplayName = "Rotation Property (UI Effect TextAnimation)"))
class LGUI_API ULexMeshModifierTextAnimation_RotationProperty : public ULexMeshModifierTextAnimation_PropertyWithEase
{
	GENERATED_BODY()
private:
	UPROPERTY(EditAnywhere, Category = "Property")
		FRotator rotator;
public:
	virtual void ApplyProperty(class ULexText* InUIText, const FLexMeshModifierTextAnimation_SelectResult& InSelection, FLexUIGeometry* InGeometry) override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FRotator GetRotator()const { return rotator; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetRotator(FRotator value);
};

UCLASS(ClassGroup = (LGUI), BlueprintType, meta = (DisplayName = "RotationRandom Property (UI Effect TextAnimation)"))
class LGUI_API ULexMeshModifierTextAnimation_RotationRandomProperty : public ULexMeshModifierTextAnimation_PropertyWithEase
{
	GENERATED_BODY()
private:
	//random seed
	UPROPERTY(EditAnywhere, Category = "Property")
		int Seed = 0;
	//random min
	UPROPERTY(EditAnywhere, Category = "Property")
		FRotator Min = FRotator(0, 0, 0);
	//random max
	UPROPERTY(EditAnywhere, Category = "Property")
		FRotator Max = FRotator(0, 90, 0);
public:
	virtual void ApplyProperty(class ULexText* InUIText, const FLexMeshModifierTextAnimation_SelectResult& InSelection, FLexUIGeometry* InGeometry) override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		int GetSeed()const { return Seed; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FRotator GetMin()const { return Min; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FRotator GetMax()const { return Max; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetSeed(int Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetMin(FRotator Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetMax(FRotator Value);
};

UCLASS(ClassGroup = (LGUI), BlueprintType, meta = (DisplayName = "Scale Property (UI Effect TextAnimation)"))
class LGUI_API ULexMeshModifierTextAnimation_ScaleProperty : public ULexMeshModifierTextAnimation_PropertyWithEase
{
	GENERATED_BODY()
private:
	UPROPERTY(EditAnywhere, Category = "Property")
		FVector Scale = FVector::OneVector;
public:
	virtual void ApplyProperty(class ULexText* InUIText, const FLexMeshModifierTextAnimation_SelectResult& InSelection, FLexUIGeometry* InGeometry) override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FVector GetScale()const { return Scale; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetScale(FVector Value);
};

UCLASS(ClassGroup = (LGUI), BlueprintType, meta = (DisplayName = "ScaleRandom Property (UI Effect TextAnimation)"))
class LGUI_API ULexMeshModifierTextAnimation_ScaleRandomProperty : public ULexMeshModifierTextAnimation_PropertyWithEase
{
	GENERATED_BODY()
private:
	//random seed
	UPROPERTY(EditAnywhere, Category = "Property")
		int Seed = 0;
	//random min
	UPROPERTY(EditAnywhere, Category = "Property")
		FVector Min = FVector(1, 1, 1);
	//random max
	UPROPERTY(EditAnywhere, Category = "Property")
		FVector Max = FVector(2, 2, 2);
public:
	virtual void ApplyProperty(class ULexText* InUIText, const FLexMeshModifierTextAnimation_SelectResult& InSelection, FLexUIGeometry* InGeometry) override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		int GetSeed()const { return Seed; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FVector GetMin()const { return Min; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FVector GetMax()const { return Max; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetSeed(int Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetMin(FVector Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetMax(FVector Value);
};

UCLASS(ClassGroup = (LGUI), BlueprintType, meta = (DisplayName = "Alpha Property (UI Effect TextAnimation)"))
class LGUI_API ULexMeshModifierTextAnimation_AlphaProperty : public ULexMeshModifierTextAnimation_PropertyWithEase
{
	GENERATED_BODY()
private:
	/** Target alpha value, 0-1 range. */
	UPROPERTY(EditAnywhere, Category = "Property", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float Alpha;
public:
	virtual void ApplyProperty(class ULexText* InUIText, const FLexMeshModifierTextAnimation_SelectResult& InSelection, FLexUIGeometry* InGeometry) override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetAlpha()const { return Alpha; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetAlpha(float Value);
};

UCLASS(ClassGroup = (LGUI), BlueprintType, meta = (DisplayName = "Color Property (UI Effect TextAnimation)"))
class LGUI_API ULexMeshModifierTextAnimation_ColorProperty : public ULexMeshModifierTextAnimation_PropertyWithEase
{
	GENERATED_BODY()
private:
	UPROPERTY(EditAnywhere, Category = "Property")
		FColor Color = FColor::Green;
	/** Convert color to HSV(Hue, Saturate, Value) and interpolate, then convert the result back. Interpolate two colors in HSV may look better. */
	UPROPERTY(EditAnywhere, Category = "Property")
		bool bUseHSV = true;
public:
	virtual void ApplyProperty(class ULexText* InUIText, const FLexMeshModifierTextAnimation_SelectResult& InSelection, FLexUIGeometry* InGeometry) override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FColor GetColor()const { return Color; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetUseHSV()const { return bUseHSV; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetColor(FColor Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetUseHSV(bool Value);
};

UCLASS(ClassGroup = (LGUI), BlueprintType, meta = (DisplayName = "ColorRandom Property (UI Effect TextAnimation)"))
class LGUI_API ULexMeshModifierTextAnimation_ColorRandomProperty : public ULexMeshModifierTextAnimation_PropertyWithEase
{
	GENERATED_BODY()
private:
	/** Random seed. */
	UPROPERTY(EditAnywhere, Category = "Property")
		int Seed = 0;
	/** Random min. */
	UPROPERTY(EditAnywhere, Category = "Property")
		FColor Min = FColor::Green;
	/** Random max. */
	UPROPERTY(EditAnywhere, Category = "Property")
		FColor Max = FColor::Red;
	/** convert color to linear hsv, interpolate, and convert back to color */
	UPROPERTY(EditAnywhere, Category = "Property")
		bool bUseHSV = true;
public:
	virtual void ApplyProperty(class ULexText* InUIText, const FLexMeshModifierTextAnimation_SelectResult& InSelection, FLexUIGeometry* InGeometry) override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		int GetSeed()const { return Seed; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FColor GetMin()const { return Min; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FColor GetMax()const { return Max; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetUseHSV()const { return bUseHSV; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetSeed(int Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetMin(FColor Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetMax(FColor Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetUseHSV(bool Value);
};
