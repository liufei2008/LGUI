// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "../LexMeshModifierTextAnimation.h"
#include "LexMeshModifierTextAnimation_PropertyWithWave.generated.h"

UCLASS(ClassGroup = (LGUI), Abstract, BlueprintType)
class LGUI_API ULexMeshModifierTextAnimation_PropertyWithWave : public ULexMeshModifierTextAnimation_Property
{
	GENERATED_BODY()
protected:
	/** Higher frequency will generate smaller wavelength. */
	UPROPERTY(EditAnywhere, Category = "Property")
		float Frequency = 1.0f;
	/** Move speed of the wave. */
	UPROPERTY(EditAnywhere, Category = "Property")
		float Speed = 1.0f;
	/** Flip move speed direction of the wave. */
	UPROPERTY(EditAnywhere, Category = "Property")
		bool FlipDirection = false;
	FTSTicker::FDelegateHandle UpdateDelegateHandle;
	virtual bool OnUpdate(float deltaTime);
	UPROPERTY(Transient)TObjectPtr<class ULexText> TextObject;
public:
	virtual void Init()override;
	virtual void Deinit()override;
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetFrequency()const { return Speed; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetFrequency(float Value);
};

UCLASS(ClassGroup = (LGUI), BlueprintType, meta = (DisplayName = "PositionWave Property (UI Effect TextAnimation)"))
class LGUI_API ULexMeshModifierTextAnimation_PositionWaveProperty : public ULexMeshModifierTextAnimation_PropertyWithWave
{
	GENERATED_BODY()
private:
	/** Max position value for sin wave. Sin function generate values from -1 to 1, so the result will be from -position to position. */
	UPROPERTY(EditAnywhere, Category = "Property")
		FVector Position;
public:
	virtual void ApplyProperty(class ULexText* InUIText, const FLexMeshModifierTextAnimation_SelectResult& InSelection, FLexUIGeometry* InGeometry) override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FVector GetPosition()const { return Position; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetPosition(FVector Value);
};

UCLASS(ClassGroup = (LGUI), BlueprintType, meta = (DisplayName = "RotationWave Property (UI Effect TextAnimation)"))
class LGUI_API ULexMeshModifierTextAnimation_RotationWaveProperty : public ULexMeshModifierTextAnimation_PropertyWithWave
{
	GENERATED_BODY()
private:
	/** Max rotator value for sin wave. Sin function generate values from -1 to 1, so the result will be from -rotator to rotator. */
	UPROPERTY(EditAnywhere, Category = "Property")
		FRotator Rotator;
public:
	virtual void ApplyProperty(class ULexText* InUIText, const FLexMeshModifierTextAnimation_SelectResult& InSelection, FLexUIGeometry* InGeometry) override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FRotator GetRotator()const { return Rotator; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetRotator(FRotator Value);
};

UCLASS(ClassGroup = (LGUI), BlueprintType, meta = (DisplayName = "ScaleWave Property (UI Effect TextAnimation)"))
class LGUI_API ULexMeshModifierTextAnimation_ScaleWaveProperty : public ULexMeshModifierTextAnimation_PropertyWithWave
{
	GENERATED_BODY()
private:
	/** Max scale value for sin wave. Sin function generate values from -1 to 1, so the result will be from -scale to scale. */
	UPROPERTY(EditAnywhere, Category = "Property")
		FVector Scale = FVector::OneVector;
public:
	virtual void ApplyProperty(class ULexText* InUIText, const FLexMeshModifierTextAnimation_SelectResult& InSelection, FLexUIGeometry* InGeometry) override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FVector GetScale()const { return Scale; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetScale(FVector Value);
};