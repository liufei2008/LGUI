// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once
#include "LexUIPlayTween.h"
#include "LexUIPlayTween_Params.generated.h"


UCLASS(BlueprintType, meta = (DisplayName = "LexUIPlayTween Float (Single)"))
class LGUI_API ULexUIPlayTween_Float : public ULexUIPlayTween
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Property")
		float From = 0.0f;
	UPROPERTY(EditAnywhere, Category = "Property")
		float To = 1.0f;
	/** parameter float is interpolated value From->To */
	UPROPERTY(EditAnywhere, Category = "Event")
		FLexUIEventDelegate OnUpdateValue = FLexUIEventDelegate(ELexUIEventDelegateParameterType::Float);

	virtual void OnUpdate(float progress)override
	{
		OnUpdateValue.FireEvent(FMath::Lerp(From, To, progress));
	}
};

UCLASS(BlueprintType, meta = (DisplayName = "LexUIPlayTween Float (Double)"))
class LGUI_API ULexUIPlayTween_Double : public ULexUIPlayTween
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Property")
		double From = 0.0f;
	UPROPERTY(EditAnywhere, Category = "Property")
		double To = 1.0f;
	/** parameter float is interpolated value From->To */
	UPROPERTY(EditAnywhere, Category = "Event")
		FLexUIEventDelegate OnUpdateValue = FLexUIEventDelegate(ELexUIEventDelegateParameterType::Double);

	virtual void OnUpdate(float progress)override
	{
		OnUpdateValue.FireEvent(FMath::Lerp(From, To, progress));
	}
};

UCLASS(BlueprintType, meta = (DisplayName = "LexUIPlayTween Color"))
class LGUI_API ULexUIPlayTween_Color : public ULexUIPlayTween
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Property")
		FColor From = FColor::White;
	UPROPERTY(EditAnywhere, Category = "Property")
		FColor To = FColor::Green;
	/** parameter float is interpolated value From->To */
	UPROPERTY(EditAnywhere, Category = "Event")
		FLexUIEventDelegate OnUpdateValue = FLexUIEventDelegate(ELexUIEventDelegateParameterType::Color);

	virtual void OnUpdate(float progress)override
	{
		FColor color;
		color.R = FMath::Lerp(From.R, To.R, progress);
		color.G = FMath::Lerp(From.G, To.G, progress);
		color.B = FMath::Lerp(From.B, To.B, progress);
		color.A = FMath::Lerp(From.A, To.A, progress);
		OnUpdateValue.FireEvent(color);
	}
};

UCLASS(BlueprintType, meta = (DisplayName = "LexUIPlayTween Int"))
class LGUI_API ULexUIPlayTween_Int : public ULexUIPlayTween
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Property")
		int From = 0;
	UPROPERTY(EditAnywhere, Category = "Property")
		int To = 100;
	/** parameter float is interpolated value From->To */
	UPROPERTY(EditAnywhere, Category = "Event")
		FLexUIEventDelegate OnUpdateValue = FLexUIEventDelegate(ELexUIEventDelegateParameterType::Int32);

	virtual void OnUpdate(float progress)override
	{
		OnUpdateValue.FireEvent(FMath::Lerp(From, To, progress));
	}
};

UCLASS(BlueprintType, meta = (DisplayName = "LexUIPlayTween LinearColor"))
class LGUI_API ULexUIPlayTween_LinearColor : public ULexUIPlayTween
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Property")
		FLinearColor From = FLinearColor::White;
	UPROPERTY(EditAnywhere, Category = "Property")
		FLinearColor To = FLinearColor::Green;
	/** parameter float is interpolated value From->To */
	UPROPERTY(EditAnywhere, Category = "Event")
		FLexUIEventDelegate OnUpdateValue = FLexUIEventDelegate(ELexUIEventDelegateParameterType::LinearColor);

	virtual void OnUpdate(float progress)override
	{
		OnUpdateValue.FireEvent(FMath::Lerp(From, To, progress));
	}
};

UCLASS(BlueprintType, meta = (DisplayName = "LexUIPlayTween Quaternion"))
class LGUI_API ULexUIPlayTween_Quaternion : public ULexUIPlayTween
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Property")
		FQuat From = FQuat::Identity;
	UPROPERTY(EditAnywhere, Category = "Property")
		FQuat To = FQuat(FVector(0.0f, 0.0f, 1.0f), HALF_PI);
	/** parameter float is interpolated value From->To */
	UPROPERTY(EditAnywhere, Category = "Event")
		FLexUIEventDelegate OnUpdateValue = FLexUIEventDelegate(ELexUIEventDelegateParameterType::Quaternion);

	virtual void OnUpdate(float progress)override
	{
		OnUpdateValue.FireEvent(FMath::Lerp(From, To, progress));
	}
};

UCLASS(BlueprintType, meta = (DisplayName = "LexUIPlayTween Rotator"))
class LGUI_API ULexUIPlayTween_Rotator : public ULexUIPlayTween
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Property")
		FRotator From = FRotator::ZeroRotator;
	UPROPERTY(EditAnywhere, Category = "Property")
		FRotator To = FRotator(0.0f, 0.0f, 90.0f);
	/** parameter float is interpolated value From->To */
	UPROPERTY(EditAnywhere, Category = "Event")
		FLexUIEventDelegate OnUpdateValue = FLexUIEventDelegate(ELexUIEventDelegateParameterType::Rotator);

	virtual void OnUpdate(float progress)override
	{
		OnUpdateValue.FireEvent(FMath::Lerp(From, To, progress));
	}
};

UCLASS(BlueprintType, meta = (DisplayName = "LexUIPlayTween Vector2"))
class LGUI_API ULexUIPlayTween_Vector2: public ULexUIPlayTween
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Property")
		FVector2D From = FVector2D::ZeroVector;
	UPROPERTY(EditAnywhere, Category = "Property")
		FVector2D To = FVector2D(1.0f, 1.0f);
	/** parameter float is interpolated value From->To */
	UPROPERTY(EditAnywhere, Category = "Event")
		FLexUIEventDelegate OnUpdateValue = FLexUIEventDelegate(ELexUIEventDelegateParameterType::Vector2);

	virtual void OnUpdate(float progress)override
	{
		OnUpdateValue.FireEvent(FMath::Lerp(From, To, progress));
	}
};

UCLASS(BlueprintType, meta = (DisplayName = "LexUIPlayTween Vector3"))
class LGUI_API ULexUIPlayTween_Vector3: public ULexUIPlayTween
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Property")
		FVector From = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, Category = "Property")
		FVector To = FVector::OneVector;
	/** parameter float is interpolated value From->To */
	UPROPERTY(EditAnywhere, Category = "Event")
		FLexUIEventDelegate OnUpdateValue = FLexUIEventDelegate(ELexUIEventDelegateParameterType::Vector3);

	virtual void OnUpdate(float progress)override
	{
		OnUpdateValue.FireEvent(FMath::Lerp(From, To, progress));
	}
};

UCLASS(BlueprintType, meta = (DisplayName = "LexUIPlayTween Vector4"))
class LGUI_API ULexUIPlayTween_Vector4: public ULexUIPlayTween
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Property")
		FVector4 From = FVector4(0.0f, 0.0f, 0.0f, 0.0f);
	UPROPERTY(EditAnywhere, Category = "Property")
		FVector4 To = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
	/** parameter float is interpolated value From->To */
	UPROPERTY(EditAnywhere, Category = "Event")
		FLexUIEventDelegate OnUpdateValue = FLexUIEventDelegate(ELexUIEventDelegateParameterType::Vector4);

	virtual void OnUpdate(float progress)override
	{
		OnUpdateValue.FireEvent(FMath::Lerp(From, To, progress));
	}
};
