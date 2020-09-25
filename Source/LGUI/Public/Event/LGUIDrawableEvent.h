// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "LGUIPointerEventData.h"
#include "LGUIDelegateHandleWrapper.h"
#include "LGUIDelegateDeclaration.h"
#include "LGUIDrawableEvent.generated.h"


UENUM()
enum class LGUIDrawableEventParameterType :uint8
{
	//not initialized
	None		UMETA(Hidden),
	Empty,
	Bool		UMETA(DisplayName = "Boolean"),
	Float,
	Double		UMETA(Hidden),
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
	//Name,
	//for asset reference
	Object,
	//for actor reference in level
	Actor,
	//for LGUIPointerEventData
	PointerEvent	UMETA(DisplayName = "LGUIPointerEvent"),
	//Class for UClass reference
	Class,
	
	Rotator,
};
//helper class for finding function
class LGUI_API ULGUIDrawableEventParameterHelper
{
public:
	static bool IsSupportedFunction(UFunction* Target, TArray<LGUIDrawableEventParameterType>& OutParamTypeArray);
	static bool IsStillSupported(UFunction* Target, const TArray<LGUIDrawableEventParameterType>& InParamTypeArray);
	static FString ParameterTypeToName(LGUIDrawableEventParameterType paramType, const UFunction* InFunction = nullptr);
	//if first parameter is an object type, then return it's objectclass
	static UClass* GetObjectParameterClass(const UFunction* InFunction);
	static UEnum* GetEnumParameter(const UFunction* InFunction);
	static UClass* GetClassParameterClass(const UFunction* InFunction);
private:
	static bool IsFunctionCompatible(const UFunction* InFunction, TArray<LGUIDrawableEventParameterType>& OutParameterTypeArray);
	static bool IsPropertyCompatible(const UProperty* InFunctionProperty, LGUIDrawableEventParameterType& OutParameterType);
};

#define LGUIEventActorSelfName "(ActorSelf)"
/**
 * drawable and editable event type in editor
 */
USTRUCT()
struct LGUI_API FLGUIDrawableEventData
{
	GENERATED_BODY()
public:
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")bool BoolValue;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")float FloatValue;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")double DoubleValue;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")int8 Int8Value;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")uint8 UInt8Value;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")int16 Int16Value;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")uint16 UInt16Value;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")int32 Int32Value;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")uint32 UInt32Value;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")int64 Int64Value;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")uint64 UInt64Value;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")FVector2D Vector2Value;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")FVector Vector3Value;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")FVector4 Vector4Value;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")FQuat QuatValue;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")FColor ColorValue;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")FLinearColor LinearColorValue;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")FRotator RotatorValue;
#endif
	//use actor and component class to find target object
	UPROPERTY(EditAnywhere, Category = "LGUI")
		AActor* targetActor;
	//component class.
	UPROPERTY(EditAnywhere, Category = "LGUI")
		UClass* componentClass;
	//if the actor only have one component of the componentClass, then just use that one.
	//if the actor have multiple component of same class, then check the componentName.
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FName componentName;
	//target function name
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FName functionName;
	//target function supported parameter type
	UPROPERTY(EditAnywhere, Category = "LGUI")
		LGUIDrawableEventParameterType ParamType = LGUIDrawableEventParameterType::None;

	//data buffer stores function's parameter
	UPROPERTY(EditAnywhere, Category = "LGUI")
		TArray<uint8> ParamBuffer;
	//asset reference
	UPROPERTY(EditAnywhere, Category = "LGUI")
		UObject* ReferenceObject;
	//actor reference
	UPROPERTY(EditAnywhere, Category = "LGUI")
		AActor* ReferenceActor;
	//UClass reference
	UPROPERTY(EditAnywhere, Category = "LGUI")
		UClass* ReferenceClass;
	//string reference
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FString ReferenceString;

	//use the function's native parameter?
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool UseNativeParameter = false;
private:
	UPROPERTY(Transient) UFunction* CacheFunction = nullptr;
	UPROPERTY(Transient) UObject* CacheTarget = nullptr;
public:
	void Execute();
	void Execute(void* InParam, LGUIDrawableEventParameterType InParameterType);
private:
	void FindAndExecute(UObject* Target, FName FunctionName, void* ParamData = nullptr);
	void ExecuteTargetFunction(UObject* Target, UFunction* Func);
	void ExecuteTargetFunction(UObject* Target, UFunction* Func, void* ParamData);
	void FindAndExecuteFromActor(void* InParam = nullptr);
};

/*
event or callback that can edit inside ue4 editor
*/
USTRUCT(BlueprintType)
struct LGUI_API FLGUIDrawableEvent
{
	GENERATED_BODY()

public:
	FLGUIDrawableEvent();
	FLGUIDrawableEvent(LGUIDrawableEventParameterType InParameterType);
public:
	//event list
	UPROPERTY(EditAnywhere, Category = "LGUI")
		TArray<FLGUIDrawableEventData> eventList;
	//supported parameter type of this event
	UPROPERTY(EditAnywhere, Category = "LGUI", meta=(DisplayName="NativeParameterType"))
		LGUIDrawableEventParameterType supportParameterType = LGUIDrawableEventParameterType::Empty;
private:
	//Parameter type must be the same as your declaration of FLGUIDrawableEvent(LGUIDrawableEventParameterType InParameterType)
	void FireEvent(void* InParam)const;
	void LogParameterError()const;
public:
	bool IsBound()const;

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
	void FireEvent(FString InParam)const;
	void FireEvent(UObject* InParam)const;
	void FireEvent(AActor* InParam)const;
	void FireEvent(ULGUIPointerEventData* InParam)const;
	void FireEvent(FRotator InParam)const;
};



#if 0
USTRUCT()
struct LGUI_API FLGUIDrawableEventData_DataContainer
{
	GENERATED_BODY()
public:
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")bool BoolValue;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")float FloatValue;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")double DoubleValue;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")int8 Int8Value;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")uint8 UInt8Value;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")int16 Int16Value;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")uint16 UInt16Value;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")int32 Int32Value;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")uint32 UInt32Value;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")int64 Int64Value;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")uint64 UInt64Value;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")FVector2D Vector2Value;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")FVector Vector3Value;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")FVector4 Vector4Value;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")FQuat QuatValue;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")FColor ColorValue;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")FLinearColor LinearColorValue;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")FRotator RotatorValue;
#endif
	//target function supported parameter type
	UPROPERTY(EditAnywhere, Category = "LGUI")
		LGUIDrawableEventParameterType ParamType = LGUIDrawableEventParameterType::None;
	//data buffer stores function's parameter
	UPROPERTY(EditAnywhere, Category = "LGUI")
		TArray<uint8> ParamBuffer;
	//asset reference
	UPROPERTY(EditAnywhere, Category = "LGUI")
		UObject* ReferenceObject;
	//actor reference
	UPROPERTY(EditAnywhere, Category = "LGUI")
		AActor* ReferenceActor;
	//UClass reference
	UPROPERTY(EditAnywhere, Category = "LGUI")
		UClass* ReferenceClass;
	//string reference
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FString ReferenceString;
};

USTRUCT()
struct LGUI_API FLGUIDrawableEventDataTwoParam
{
	GENERATED_BODY()
public:
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")bool BoolValue;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")float FloatValue;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")double DoubleValue;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")int8 Int8Value;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")uint8 UInt8Value;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")int16 Int16Value;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")uint16 UInt16Value;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")int32 Int32Value;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")uint32 UInt32Value;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")int64 Int64Value;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")uint64 UInt64Value;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")FVector2D Vector2Value;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")FVector Vector3Value;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")FVector4 Vector4Value;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")FQuat QuatValue;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")FColor ColorValue;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")FLinearColor LinearColorValue;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")FRotator RotatorValue;
#endif
	//use actor and component class to find target object
	UPROPERTY(EditAnywhere, Category = "LGUI")
		AActor* targetActor;
	//component class. if an actor have multiple component of same class, not supported
	UPROPERTY(EditAnywhere, Category = "LGUI")
		UClass* componentClass;
	//target function name
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FName functionName;
	//target function supported parameter type
	UPROPERTY(EditAnywhere, Category = "LGUI")
		LGUIDrawableEventParameterType ParamType = LGUIDrawableEventParameterType::Empty;

	//data buffer stores function's parameter
	UPROPERTY(EditAnywhere, Category = "LGUI")
		TArray<uint8> ParamBuffer;
	//asset reference
	UPROPERTY(EditAnywhere, Category = "LGUI")
		UObject* ReferenceObject;
	//actor reference
	UPROPERTY(EditAnywhere, Category = "LGUI")
		AActor* ReferenceActor;
	//UClass reference
	UPROPERTY(EditAnywhere, Category = "LGUI")
		UClass* ReferenceClass;
	//string reference
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FString ReferenceString;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FLGUIDrawableEventData_DataContainer param2DataContainer;

	//use the function's native parameter?
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool UseNativeParameter = false;
private:
	UFunction* CacheFunction;
	UObject* CacheTarget;
};
USTRUCT(BlueprintType)
struct LGUI_API FLGUIDrawableEventTwoParam
{
	GENERATED_BODY()

public:
	FLGUIDrawableEventTwoParam() {}
	FLGUIDrawableEventTwoParam(LGUIDrawableEventParameterType InParameterType1, LGUIDrawableEventParameterType InParameterType2)
	{
		supportParameterType1 = InParameterType1;
		supportParameterType2 = InParameterType2;
	}
public:
	//event list
	UPROPERTY(EditAnywhere, Category = "LGUI")
		TArray<FLGUIDrawableEventDataTwoParam> eventList;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "CanChangeNativeParameterType?"))
		bool canChangeSupportParameterType = true;
	//supported parameter type of this event
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "NativeParameterType1"))
		LGUIDrawableEventParameterType supportParameterType1 = LGUIDrawableEventParameterType::Bool;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "NativeParameterType2"))
		LGUIDrawableEventParameterType supportParameterType2 = LGUIDrawableEventParameterType::Bool;
};
#endif
