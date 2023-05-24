// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once
#include "LGUIPlayTween.h"
#include "LGUIPlayTween_Params.generated.h"


UCLASS(BlueprintType, meta = (DisplayName = "LGUIPLayTween Float"))
class LGUI_API ULGUIPlayTween_Float : public ULGUIPlayTween
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Property")
		float from = 0.0f;
	UPROPERTY(EditAnywhere, Category = "Property")
		float to = 1.0f;
	/** parameter float is interpolated value from->to */
	UPROPERTY(EditAnywhere, Category = "Event")
		FLGUIEventDelegate onUpdateValue = FLGUIEventDelegate(LGUIEventDelegateParameterType::Float);

	virtual void OnUpdate(float progress)override
	{
		onUpdateValue.FireEvent(FMath::Lerp(from, to, progress));
	}
};

UCLASS(BlueprintType, meta = (DisplayName = "LGUIPLayTween Double"))
class LGUI_API ULGUIPlayTween_Double : public ULGUIPlayTween
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Property")
		double from = 0.0f;
	UPROPERTY(EditAnywhere, Category = "Property")
		double to = 1.0f;
	/** parameter float is interpolated value from->to */
	UPROPERTY(EditAnywhere, Category = "Event")
		FLGUIEventDelegate onUpdateValue = FLGUIEventDelegate(LGUIEventDelegateParameterType::Double);

	virtual void OnUpdate(float progress)override
	{
		onUpdateValue.FireEvent(FMath::Lerp(from, to, progress));
	}
};

UCLASS(BlueprintType, meta = (DisplayName = "LGUIPLayTween Color"))
class LGUI_API ULGUIPlayTween_Color : public ULGUIPlayTween
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Property")
		FColor from = FColor::White;
	UPROPERTY(EditAnywhere, Category = "Property")
		FColor to = FColor::Green;
	/** parameter float is interpolated value from->to */
	UPROPERTY(EditAnywhere, Category = "Event")
		FLGUIEventDelegate onUpdateValue = FLGUIEventDelegate(LGUIEventDelegateParameterType::Color);

	virtual void OnUpdate(float progress)override
	{
		FColor color;
		color.R = FMath::Lerp(from.R, to.R, progress);
		color.G = FMath::Lerp(from.G, to.G, progress);
		color.B = FMath::Lerp(from.B, to.B, progress);
		color.A = FMath::Lerp(from.A, to.A, progress);
		onUpdateValue.FireEvent(color);
	}
};

UCLASS(BlueprintType, meta = (DisplayName = "LGUIPLayTween Int"))
class LGUI_API ULGUIPlayTween_Int : public ULGUIPlayTween
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Property")
		int from = 0;
	UPROPERTY(EditAnywhere, Category = "Property")
		int to = 100;
	/** parameter float is interpolated value from->to */
	UPROPERTY(EditAnywhere, Category = "Event")
		FLGUIEventDelegate onUpdateValue = FLGUIEventDelegate(LGUIEventDelegateParameterType::Int32);

	virtual void OnUpdate(float progress)override
	{
		onUpdateValue.FireEvent(FMath::Lerp(from, to, progress));
	}
};

UCLASS(BlueprintType, meta = (DisplayName = "LGUIPLayTween LinearColor"))
class LGUI_API ULGUIPlayTween_LinearColor : public ULGUIPlayTween
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Property")
		FLinearColor from = FLinearColor::White;
	UPROPERTY(EditAnywhere, Category = "Property")
		FLinearColor to = FLinearColor::Green;
	/** parameter float is interpolated value from->to */
	UPROPERTY(EditAnywhere, Category = "Event")
		FLGUIEventDelegate onUpdateValue = FLGUIEventDelegate(LGUIEventDelegateParameterType::LinearColor);

	virtual void OnUpdate(float progress)override
	{
		onUpdateValue.FireEvent(FMath::Lerp(from, to, progress));
	}
};

UCLASS(BlueprintType, meta = (DisplayName = "LGUIPLayTween Quaternion"))
class LGUI_API ULGUIPlayTween_Quaternion : public ULGUIPlayTween
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Property")
		FQuat from = FQuat::Identity;
	UPROPERTY(EditAnywhere, Category = "Property")
		FQuat to = FQuat(FVector(0.0f, 0.0f, 1.0f), HALF_PI);
	/** parameter float is interpolated value from->to */
	UPROPERTY(EditAnywhere, Category = "Event")
		FLGUIEventDelegate onUpdateValue = FLGUIEventDelegate(LGUIEventDelegateParameterType::Quaternion);

	virtual void OnUpdate(float progress)override
	{
		onUpdateValue.FireEvent(FMath::Lerp(from, to, progress));
	}
};

UCLASS(BlueprintType, meta = (DisplayName = "LGUIPLayTween Rotator"))
class LGUI_API ULGUIPlayTween_Rotator : public ULGUIPlayTween
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Property")
		FRotator from = FRotator::ZeroRotator;
	UPROPERTY(EditAnywhere, Category = "Property")
		FRotator to = FRotator(0.0f, 0.0f, 90.0f);
	/** parameter float is interpolated value from->to */
	UPROPERTY(EditAnywhere, Category = "Event")
		FLGUIEventDelegate onUpdateValue = FLGUIEventDelegate(LGUIEventDelegateParameterType::Rotator);

	virtual void OnUpdate(float progress)override
	{
		onUpdateValue.FireEvent(FMath::Lerp(from, to, progress));
	}
};

UCLASS(BlueprintType, meta = (DisplayName = "LGUIPLayTween Vector2"))
class LGUI_API ULGUIPlayTween_Vector2: public ULGUIPlayTween
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Property")
		FVector2D from = FVector2D::ZeroVector;
	UPROPERTY(EditAnywhere, Category = "Property")
		FVector2D to = FVector2D(1.0f, 1.0f);
	/** parameter float is interpolated value from->to */
	UPROPERTY(EditAnywhere, Category = "Event")
		FLGUIEventDelegate onUpdateValue = FLGUIEventDelegate(LGUIEventDelegateParameterType::Vector2);

	virtual void OnUpdate(float progress)override
	{
		onUpdateValue.FireEvent(FMath::Lerp(from, to, progress));
	}
};

UCLASS(BlueprintType, meta = (DisplayName = "LGUIPLayTween Vector3"))
class LGUI_API ULGUIPlayTween_Vector3: public ULGUIPlayTween
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Property")
		FVector from = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, Category = "Property")
		FVector to = FVector::OneVector;
	/** parameter float is interpolated value from->to */
	UPROPERTY(EditAnywhere, Category = "Event")
		FLGUIEventDelegate onUpdateValue = FLGUIEventDelegate(LGUIEventDelegateParameterType::Vector3);

	virtual void OnUpdate(float progress)override
	{
		onUpdateValue.FireEvent(FMath::Lerp(from, to, progress));
	}
};

UCLASS(BlueprintType, meta = (DisplayName = "LGUIPLayTween Vector4"))
class LGUI_API ULGUIPlayTween_Vector4: public ULGUIPlayTween
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Property")
		FVector4 from = FVector4(0.0f, 0.0f, 0.0f, 0.0f);
	UPROPERTY(EditAnywhere, Category = "Property")
		FVector4 to = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
	/** parameter float is interpolated value from->to */
	UPROPERTY(EditAnywhere, Category = "Event")
		FLGUIEventDelegate onUpdateValue = FLGUIEventDelegate(LGUIEventDelegateParameterType::Vector4);

	virtual void OnUpdate(float progress)override
	{
		onUpdateValue.FireEvent(FMath::Lerp(from, to, progress));
	}
};
