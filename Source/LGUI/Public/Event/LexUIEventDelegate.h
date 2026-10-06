// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "LexPointerEventData.h"
#include "LexUIEventDelegate.generated.h"


UENUM()
enum class ELexUIEventDelegateParameterType :uint8
{
	/** not initialized */
	None		UMETA(Hidden),
	Empty,
	Bool		UMETA(DisplayName = "Boolean"),
	Float,
	Double,
	Int8		UMETA(Hidden),
	UInt8		UMETA(DisplayName = "UInt8\Enum\Byte"),
	Int16		UMETA(Hidden),
	UInt16		UMETA(Hidden),
	Int32		UMETA(DisplayName = "Integer"),
	UInt32		UMETA(Hidden),
	Int64		UMETA(Hidden),
	UInt64		UMETA(Hidden),
	Vector2		UMETA(DisplayName = "Vector2"),
	Vector3		UMETA(DisplayName = "Vector3"),
	Vector4		UMETA(DisplayName = "Vector4"),
	Color,
	LinearColor,
	Quaternion,
	String,
	/** for asset reference */
	Asset,
	/** for LexWidget reference */
	LexWidget,
	/** for LexPointerEventData */
	PointerEvent	UMETA(DisplayName = "LexPointerEventData"),
	/** Class for UClass reference */
	Class,
	
	Rotator,

	Name,
	Text,
};
/** helper class for finding function */
class LGUI_API ULexUIEventDelegateParameterHelper
{
public:
	static bool IsSupportedFunction(UFunction* Target, ELexUIEventDelegateParameterType& OutParamType);
	static bool IsStillSupported(UFunction* Target, ELexUIEventDelegateParameterType InParamType);
	static FString ParameterTypeToName(ELexUIEventDelegateParameterType paramType, const UFunction* InFunction = nullptr);
	/** if first parameter is an object type, then return it's objectclass */
	static UClass* GetObjectParameterClass(const UFunction* InFunction);
	static UEnum* GetEnumParameter(const UFunction* InFunction);
	static UClass* GetClassParameterClass(const UFunction* InFunction);
private:
	static bool IsFunctionCompatible(const UFunction* InFunction, ELexUIEventDelegateParameterType& OutParameterType);
	static bool IsPropertyCompatible(const FProperty* InFunctionProperty, ELexUIEventDelegateParameterType& OutParameterType);
};

/**
 * Editable event type in editor
 */
USTRUCT()
struct LGUI_API FLexUIEventDelegateData
{
	GENERATED_BODY()
private:
	friend struct FLexUIEventDelegate;
	friend class FLexUIEventDelegateCustomization;
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Transient, Category = "LexUI")bool BoolValue = false;
	UPROPERTY(EditAnywhere, Transient, Category = "LexUI")float FloatValue = 0;
	UPROPERTY(EditAnywhere, Transient, Category = "LexUI")double DoubleValue = 0;
	UPROPERTY(EditAnywhere, Transient, Category = "LexUI")int8 Int8Value = 0;
	UPROPERTY(EditAnywhere, Transient, Category = "LexUI")uint8 UInt8Value = 0;
	UPROPERTY(EditAnywhere, Transient, Category = "LexUI")int16 Int16Value = 0;
	UPROPERTY(EditAnywhere, Transient, Category = "LexUI")uint16 UInt16Value = 0;
	UPROPERTY(EditAnywhere, Transient, Category = "LexUI")int32 Int32Value = 0;
	UPROPERTY(EditAnywhere, Transient, Category = "LexUI")uint32 UInt32Value = 0;
	UPROPERTY(EditAnywhere, Transient, Category = "LexUI")int64 Int64Value = 0;
	UPROPERTY(EditAnywhere, Transient, Category = "LexUI")uint64 UInt64Value = 0;
	UPROPERTY(EditAnywhere, Transient, Category = "LexUI")FVector2D Vector2Value = FVector2D::ZeroVector;
	UPROPERTY(EditAnywhere, Transient, Category = "LexUI")FVector Vector3Value = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, Transient, Category = "LexUI")FVector4 Vector4Value = FVector4(0, 0, 0, 0);
	UPROPERTY(EditAnywhere, Transient, Category = "LexUI")FQuat QuatValue = FQuat::Identity;
	UPROPERTY(EditAnywhere, Transient, Category = "LexUI")FColor ColorValue = FColor::White;
	UPROPERTY(EditAnywhere, Transient, Category = "LexUI")FLinearColor LinearColorValue = FLinearColor::White;
	UPROPERTY(EditAnywhere, Transient, Category = "LexUI")FRotator RotatorValue = FRotator::ZeroRotator;
	UPROPERTY(EditAnywhere, Transient, Category = "LexUI")FString StringValue;
	UPROPERTY(EditAnywhere, Transient, Category = "LexUI")FName NameValue;
	UPROPERTY(EditAnywhere, Transient, Category = "LexUI")FText TextValue;
#endif

	/** target widget */
	UPROPERTY(EditAnywhere, Category = "LexUI")
		TObjectPtr<ULexWidget> HelperWidget = nullptr;
	/** target object class. If class is LexWidget then TargetObject is HelperWidget, if class is LexUIBehaviour then TargetObject is the component. */
	UPROPERTY(VisibleAnywhere, Category = "LexUI")
		TObjectPtr<UClass> HelperClass = nullptr;
	/** if TargetObject is widget component and HelperWidget have multiple components, then select by component name. */
	UPROPERTY(VisibleAnywhere, Category = "LexUI")
		FName HelperComponentName;

	UPROPERTY(EditAnywhere, Transient, Category = "LexUI")
		TObjectPtr<UObject> TargetObject = nullptr;
	/** target function name */
	UPROPERTY(EditAnywhere, Category = "LexUI")
		FName FunctionName;
	/** target function supported parameter type */
	UPROPERTY(EditAnywhere, Category = "LexUI")
		ELexUIEventDelegateParameterType ParamType = ELexUIEventDelegateParameterType::None;

	/** data buffer stores function's parameter */
	UPROPERTY(EditAnywhere, Category = "LexUI")
		TArray<uint8> ParamBuffer;
	/** Object reference, can reference widget/class/asset */
	UPROPERTY(EditAnywhere, Category = "LexUI")
		TObjectPtr<UObject> ReferenceObject = nullptr;

	/** use the function's native parameter? */
	UPROPERTY(EditAnywhere, Category = "LexUI")
		bool bUseNativeParameter = false;
private:
	UPROPERTY(Transient) TObjectPtr<UFunction> CacheFunction = nullptr;
public:
	void Execute();
	void Execute(void* InParam, ELexUIEventDelegateParameterType InParameterType);
#if WITH_EDITOR
	/**
	 * Check if function parameter compatible with target function
	 * @return	true- is compatible, false- not
	 */
	bool CheckFunctionParameter()const;
#endif
private:
	bool CheckTargetObject();
	void FindAndExecute(UObject* Target, void* ParamData = nullptr);
	void ExecuteTargetFunction(UObject* Target, UFunction* Func);
	void ExecuteTargetFunction(UObject* Target, UFunction* Func, void* ParamData);
};

/**
 * event or callback that can edit inside editor
 */
USTRUCT(BlueprintType)
struct LGUI_API FLexUIEventDelegate
{
	GENERATED_BODY()

public:
	FLexUIEventDelegate();
	FLexUIEventDelegate(ELexUIEventDelegateParameterType InParameterType);
private:
	friend class FLexUIEventDelegateCustomization;
	/** event list */
	UPROPERTY(EditAnywhere, Category = "LexUI")
		mutable TArray<FLexUIEventDelegateData> EventList;
	/** supported parameter type of this event */
	UPROPERTY(EditAnywhere, Transient, Category = "LexUI", meta = (DisplayName = "NativeParameterType"))
		ELexUIEventDelegateParameterType SupportParameterType = ELexUIEventDelegateParameterType::Empty;
	/** Parameter type must be the same as your declaration of FLexUIEventDelegate(LexUIEventDelegateParameterType InParameterType) */
	void FireEvent(void* InParam)const;
	void LogParameterError(ELexUIEventDelegateParameterType WrongParamType)const;
public:
	bool IsBound()const;
public:
	void FireEvent()const;
	void FireEvent(bool InParam)const;
	void FireEvent(float InParam)const;
	void FireEvent(double InParam)const;
	void FireEvent(int8 InParam)const;
	void FireEvent(uint8 InParam)const;
	void FireEvent(int16 InParam)const;
	void FireEvent(uint16 InParam)const;
	void FireEvent(int32 InParam)const;
	void FireEvent(uint32 InParam)const;
	void FireEvent(int64 InParam)const;
	void FireEvent(uint64 InParam)const;
	void FireEvent(FVector2D InParam)const;
	void FireEvent(FVector InParam)const;
	void FireEvent(FVector4 InParam)const;
	void FireEvent(FColor InParam)const;
	void FireEvent(FLinearColor InParam)const;
	void FireEvent(FQuat InParam)const; 
	void FireEvent(const FString& InParam)const;
	void FireEvent(UObject* InParam)const;
	void FireEvent(ULexWidget* InParam)const;
	void FireEvent(ULexPointerEventData* InParam)const;
	void FireEvent(UClass* InParam)const;
	void FireEvent(FRotator InParam)const;
	void FireEvent(const FName& InParam)const;
	void FireEvent(const FText& InParam)const;

#if WITH_EDITOR
	/**
	 * Check if function parameter compatible with target function
	 * @return	true- is compatible, false- not
	 */
	bool CheckFunctionParameter()const;
#endif
};
