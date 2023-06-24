// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "LGUIPointerEventData.h"
#include "LGUIDelegateHandleWrapper.h"
#include "LGUIDelegateDeclaration.h"
#include "LGUIEventDelegate.generated.h"


UENUM()
enum class LGUIEventDelegateParameterType :uint8
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
	Object,
	/** for actor reference in level */
	Actor,
	/** for LGUIPointerEventData */
	PointerEvent	UMETA(DisplayName = "LGUIPointerEventData"),
	/** Class for UClass reference */
	Class,
	
	Rotator,

	Name,
	Text,
};
/** helper class for finding function */
class LGUI_API ULGUIEventDelegateParameterHelper
{
public:
	static bool IsSupportedFunction(UFunction* Target, LGUIEventDelegateParameterType& OutParamType);
	static bool IsStillSupported(UFunction* Target, LGUIEventDelegateParameterType InParamType);
	static FString ParameterTypeToName(LGUIEventDelegateParameterType paramType, const UFunction* InFunction = nullptr);
	/** if first parameter is an object type, then return it's objectclass */
	static UClass* GetObjectParameterClass(const UFunction* InFunction);
	static UEnum* GetEnumParameter(const UFunction* InFunction);
	static UClass* GetClassParameterClass(const UFunction* InFunction);
private:
	static bool IsFunctionCompatible(const UFunction* InFunction, LGUIEventDelegateParameterType& OutParameterType);
	static bool IsPropertyCompatible(const FProperty* InFunctionProperty, LGUIEventDelegateParameterType& OutParameterType);
};

/**
 * Editable event type in editor
 */
USTRUCT()
struct LGUI_API FLGUIEventDelegateData
{
	GENERATED_BODY()
#if WITH_EDITORONLY_DATA
public:
	FLGUIEventDelegateData();
	~FLGUIEventDelegateData();
	/**
	 * If TargetObject is a BlueprintCreatedComponent, when hit compile on blueprint editor, the TargetObject will lose reference.
	 * So we need to find the referenced TargetObject after blueprint compile.
	 */
	static void RefreshAllOnBlueprintRecompile();
private:
	static TArray<FLGUIEventDelegateData*> AllEventDelegateDataArray;
	void RefreshOnBlueprintRecompile();
#endif
private:
	friend struct FLGUIEventDelegate;
	friend class FLGUIEventDelegateCustomization;
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")bool BoolValue = false;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")float FloatValue = 0;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")double DoubleValue = 0;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")int8 Int8Value = 0;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")uint8 UInt8Value = 0;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")int16 Int16Value = 0;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")uint16 UInt16Value = 0;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")int32 Int32Value = 0;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")uint32 UInt32Value = 0;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")int64 Int64Value = 0;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")uint64 UInt64Value = 0;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")FVector2D Vector2Value = FVector2D::ZeroVector;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")FVector Vector3Value = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")FVector4 Vector4Value = FVector4(0, 0, 0, 0);
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")FQuat QuatValue = FQuat::Identity;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")FColor ColorValue = FColor::White;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")FLinearColor LinearColorValue = FLinearColor::White;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")FRotator RotatorValue = FRotator::ZeroRotator;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")FString StringValue;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")FName NameValue;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")FText TextValue;
#endif
#if WITH_EDITORONLY_DATA
	/** Editor helper actor, for direct reference actor */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		TObjectPtr<AActor> HelperActor = nullptr;
	/** Editor helper, target object class. If class is actor then TargetObject is HelperActor, if class is ActorComponent then TargetObject is the component. */
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TObjectPtr<UClass> HelperClass = nullptr;
	/** Editor helper, if TargetObject is actor component and HelperActor have multiple components, then select by component name. */
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		FName HelperComponentName;
#endif
	UPROPERTY(EditAnywhere, Category = "LGUI")
		TObjectPtr<UObject> TargetObject = nullptr;
	/** target function name */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FName functionName;
	/** target function supported parameter type */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		LGUIEventDelegateParameterType ParamType = LGUIEventDelegateParameterType::None;

	/** data buffer stores function's parameter */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		TArray<uint8> ParamBuffer;
	/** Object reference, can reference actor/class/asset */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		TObjectPtr<UObject> ReferenceObject = nullptr;

	/** use the function's native parameter? */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool UseNativeParameter = false;
private:
	UPROPERTY(Transient) TObjectPtr<UFunction> CacheFunction = nullptr;
	UPROPERTY(Transient) TObjectPtr<UObject> CacheTarget = nullptr;
public:
	void Execute();
	void Execute(void* InParam, LGUIEventDelegateParameterType InParameterType);
#if WITH_EDITOR
	/**
	 * Check if function parameter compatible with target function
	 * @return	true- is compatible, false- not
	 */
	bool CheckFunctionParameter()const;
#endif
private:
	void FindAndExecute(UObject* Target, void* ParamData = nullptr);
	void ExecuteTargetFunction(UObject* Target, UFunction* Func);
	void ExecuteTargetFunction(UObject* Target, UFunction* Func, void* ParamData);
};

/**
 * event or callback that can edit inside ue4 editor
 */
USTRUCT(BlueprintType)
struct LGUI_API FLGUIEventDelegate
{
	GENERATED_BODY()

public:
	FLGUIEventDelegate();
	FLGUIEventDelegate(LGUIEventDelegateParameterType InParameterType);
private:
	friend class FLGUIEventDelegateCustomization;
	/** event list */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		mutable TArray<FLGUIEventDelegateData> eventList;
	/** supported parameter type of this event */
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI", meta = (DisplayName = "NativeParameterType"))
		LGUIEventDelegateParameterType supportParameterType = LGUIEventDelegateParameterType::Empty;
	/** Parameter type must be the same as your declaration of FLGUIEventDelegate(LGUIEventDelegateParameterType InParameterType) */
	void FireEvent(void* InParam)const;
	void LogParameterError()const;
public:
	void SetParameterType(LGUIEventDelegateParameterType InParamType)
	{
		if (supportParameterType != InParamType)
		{
			supportParameterType = InParamType;
		}
	}
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
	void FireEvent(AActor* InParam)const;
	void FireEvent(ULGUIPointerEventData* InParam)const;
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
