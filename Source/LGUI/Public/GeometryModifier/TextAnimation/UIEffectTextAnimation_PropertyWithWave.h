// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "../UIEffectTextAnimation.h"
#include "UIEffectTextAnimation_PropertyWithWave.generated.h"

UCLASS(ClassGroup = (LGUI), Abstract, BlueprintType)
class LGUI_API UUIEffectTextAnimation_PropertyWithWave : public UUIEffectTextAnimation_Property
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditAnywhere, Category = "Property")
		float frequency = 1.0f;
	UPROPERTY(EditAnywhere, Category = "Property")
		float speed = 1.0f;
	UPROPERTY(EditAnywhere, Category = "Property")
		bool flipDirection = false;
	struct FLTweenDelegateHandleWrapper delegateHandle;
	virtual void OnUpdate(float deltaTime);
	UPROPERTY(Transient)class UUIText* uiText;
public:
	virtual void Init()override;
	virtual void Deinit()override;
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetFrequency()const { return speed; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetFrequency(float value);
};

UCLASS(ClassGroup = (LGUI), BlueprintType, meta = (DisplayName = "PositionWave"))
class LGUI_API UUIEffectTextAnimation_PositionWaveProperty : public UUIEffectTextAnimation_PropertyWithWave
{
	GENERATED_BODY()
private:
	UPROPERTY(EditAnywhere, Category = "Property")
		FVector position;
public:
	virtual void ApplyProperty(class UUIText* InUIText, const FUIEffectTextAnimation_SelectResult& InSelection, TSharedPtr<UIGeometry> OutGeometry) override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FVector GetPosition()const { return position; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetPosition(FVector value);
};

UCLASS(ClassGroup = (LGUI), BlueprintType, meta = (DisplayName = "RotationWave"))
class LGUI_API UUIEffectTextAnimation_RotationWaveProperty : public UUIEffectTextAnimation_PropertyWithWave
{
	GENERATED_BODY()
private:
	UPROPERTY(EditAnywhere, Category = "Property")
		FRotator rotator;
public:
	virtual void ApplyProperty(class UUIText* InUIText, const FUIEffectTextAnimation_SelectResult& InSelection, TSharedPtr<UIGeometry> OutGeometry) override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FRotator GetRotator()const { return rotator; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetRotator(FRotator value);
};

UCLASS(ClassGroup = (LGUI), BlueprintType, meta = (DisplayName = "ScaleWave"))
class LGUI_API UUIEffectTextAnimation_ScaleWaveProperty : public UUIEffectTextAnimation_PropertyWithWave
{
	GENERATED_BODY()
private:
	UPROPERTY(EditAnywhere, Category = "Property")
		FVector scale = FVector::OneVector;
public:
	virtual void ApplyProperty(class UUIText* InUIText, const FUIEffectTextAnimation_SelectResult& InSelection, TSharedPtr<UIGeometry> OutGeometry) override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FVector GetScale()const { return scale; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetScale(FVector value);
};