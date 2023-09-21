// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Event/LGUIEventDelegate.h"
#include "LGUIDelegateHandleWrapper.h"
#include "LGUIEventDelegate_PresetParameter.generated.h"

#define MAKE_EVENTDELEGATE_PRESETPARAM_DELEGATE(EventDelegateParamType, ParamType)\
DECLARE_DELEGATE_OneParam(FLGUIEventDelegate_##EventDelegateParamType##_Delegate, ParamType);\
DECLARE_MULTICAST_DELEGATE_OneParam(FLGUIEventDelegate_##EventDelegateParamType##_MulticastDelegate, ParamType);


#define MAKE_EVENTDELEGATE_PRESETPARAM(EventDelegateParamType, ParamType)\
public:\
	FLGUIEventDelegate_##EventDelegateParamType() :FLGUIEventDelegate(ELGUIEventDelegateParameterType::EventDelegateParamType) {}\
private:\
	mutable FLGUIEventDelegate_##EventDelegateParamType##_MulticastDelegate eventDelegate;\
public:\
	FDelegateHandle Register(const TFunction<void(ParamType)>& function)const\
	{\
		return eventDelegate.AddLambda(function);\
	}\
	FDelegateHandle Register(const FLGUIEventDelegate_##EventDelegateParamType##_Delegate& function)const\
	{\
		return eventDelegate.Add(function);\
	}\
	void Unregister(const FDelegateHandle& delegateHandle)const\
	{\
		eventDelegate.Remove(delegateHandle);\
	}\
	void operator() (ParamType InParam)const\
	{\
		FLGUIEventDelegate::FireEvent(InParam);\
		if (eventDelegate.IsBound())eventDelegate.Broadcast(InParam);\
	}



DECLARE_DELEGATE(FLGUIEventDelegate_Empty_Delegate); 
DECLARE_MULTICAST_DELEGATE(FLGUIEventDelegate_Empty_MulticastDelegate);
DECLARE_DYNAMIC_DELEGATE(FLGUIEventDelegate_Empty_DynamicDelegate);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIEventDelegate_Empty : public FLGUIEventDelegate
{
	GENERATED_BODY()
public:
	FLGUIEventDelegate_Empty() :FLGUIEventDelegate(ELGUIEventDelegateParameterType::Empty) {}
private:
	mutable FLGUIEventDelegate_Empty_MulticastDelegate eventDelegate;
public:
	FDelegateHandle Register(const TFunction<void()>& function)const
	{
		return eventDelegate.AddLambda(function);
	}
	FDelegateHandle Register(const FLGUIEventDelegate_Empty_Delegate& function)const
	{
		return eventDelegate.Add(function);
	}
	void Unregister(const FDelegateHandle& delegateHandle)const
	{
		eventDelegate.Remove(delegateHandle);
	}
	void operator() ()const
	{
		FLGUIEventDelegate::FireEvent();
		if (eventDelegate.IsBound())eventDelegate.Broadcast();
	}
};

MAKE_EVENTDELEGATE_PRESETPARAM_DELEGATE(Bool, bool);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIEventDelegate_Bool_DynamicDelegate, bool, value);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIEventDelegate_Bool : public FLGUIEventDelegate
{
	GENERATED_BODY()
MAKE_EVENTDELEGATE_PRESETPARAM(Bool, bool);
};

MAKE_EVENTDELEGATE_PRESETPARAM_DELEGATE(Float, float);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIEventDelegate_Float_DynamicDelegate, float, value);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIEventDelegate_Float : public FLGUIEventDelegate
{
	GENERATED_BODY()
		MAKE_EVENTDELEGATE_PRESETPARAM(Float, float);
};

MAKE_EVENTDELEGATE_PRESETPARAM_DELEGATE(Double, double);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIEventDelegate_Double_DynamicDelegate, double, value);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIEventDelegate_Double : public FLGUIEventDelegate
{
	GENERATED_BODY()
		MAKE_EVENTDELEGATE_PRESETPARAM(Double, double);
};

MAKE_EVENTDELEGATE_PRESETPARAM_DELEGATE(Int8, int8);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIEventDelegate_Int8_DynamicDelegate, int8, value);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIEventDelegate_Int8 : public FLGUIEventDelegate
{
	GENERATED_BODY()
		MAKE_EVENTDELEGATE_PRESETPARAM(Int8, int8);
};

MAKE_EVENTDELEGATE_PRESETPARAM_DELEGATE(UInt8, uint8);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIEventDelegate_UInt8_DynamicDelegate, uint8, value);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIEventDelegate_UInt8 : public FLGUIEventDelegate
{
	GENERATED_BODY()
		MAKE_EVENTDELEGATE_PRESETPARAM(UInt8, uint8);
};

MAKE_EVENTDELEGATE_PRESETPARAM_DELEGATE(Int16, int16);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIEventDelegate_Int16_DynamicDelegate, int16, value);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIEventDelegate_Int16 : public FLGUIEventDelegate
{
	GENERATED_BODY()
		MAKE_EVENTDELEGATE_PRESETPARAM(Int16, int16);
};

MAKE_EVENTDELEGATE_PRESETPARAM_DELEGATE(UInt16, uint16);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIEventDelegate_UInt16_DynamicDelegate, uint16, value);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIEventDelegate_UInt16 : public FLGUIEventDelegate
{
	GENERATED_BODY()
		MAKE_EVENTDELEGATE_PRESETPARAM(UInt16, uint16);
};

MAKE_EVENTDELEGATE_PRESETPARAM_DELEGATE(Int32, int32);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIEventDelegate_Int32_DynamicDelegate, int32, value);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIEventDelegate_Int32 : public FLGUIEventDelegate
{
	GENERATED_BODY()
		MAKE_EVENTDELEGATE_PRESETPARAM(Int32, int32);
};

MAKE_EVENTDELEGATE_PRESETPARAM_DELEGATE(UInt32, uint32);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIEventDelegate_UInt32_DynamicDelegate, uint32, value);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIEventDelegate_UInt32 : public FLGUIEventDelegate
{
	GENERATED_BODY()
		MAKE_EVENTDELEGATE_PRESETPARAM(UInt32, uint32);
};

MAKE_EVENTDELEGATE_PRESETPARAM_DELEGATE(Int64, int64);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIEventDelegate_Int64_DynamicDelegate, int64, value);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIEventDelegate_Int64 : public FLGUIEventDelegate
{
	GENERATED_BODY()
		MAKE_EVENTDELEGATE_PRESETPARAM(Int64, int64);
};

MAKE_EVENTDELEGATE_PRESETPARAM_DELEGATE(UInt64, uint64);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIEventDelegate_UInt64_DynamicDelegate, uint64, value);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIEventDelegate_UInt64 : public FLGUIEventDelegate
{
	GENERATED_BODY()
		MAKE_EVENTDELEGATE_PRESETPARAM(UInt64, uint64);
};

MAKE_EVENTDELEGATE_PRESETPARAM_DELEGATE(Vector2, FVector2D);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIEventDelegate_Vector2_DynamicDelegate, FVector2D, value);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIEventDelegate_Vector2 : public FLGUIEventDelegate
{
	GENERATED_BODY()
		MAKE_EVENTDELEGATE_PRESETPARAM(Vector2, FVector2D);
};

MAKE_EVENTDELEGATE_PRESETPARAM_DELEGATE(Vector3, FVector);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIEventDelegate_Vector3_DynamicDelegate, FVector, value);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIEventDelegate_Vector3 : public FLGUIEventDelegate
{
	GENERATED_BODY()
		MAKE_EVENTDELEGATE_PRESETPARAM(Vector3, FVector);
};

MAKE_EVENTDELEGATE_PRESETPARAM_DELEGATE(Vector4, FVector4);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIEventDelegate_Vector4_DynamicDelegate, FVector4, value);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIEventDelegate_Vector4 : public FLGUIEventDelegate
{
	GENERATED_BODY()
		MAKE_EVENTDELEGATE_PRESETPARAM(Vector4, FVector4);
};

MAKE_EVENTDELEGATE_PRESETPARAM_DELEGATE(Color, FColor);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIEventDelegate_Color_DynamicDelegate, FColor, value);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIEventDelegate_Color : public FLGUIEventDelegate
{
	GENERATED_BODY()
		MAKE_EVENTDELEGATE_PRESETPARAM(Color, FColor);
};

MAKE_EVENTDELEGATE_PRESETPARAM_DELEGATE(LinearColor, FLinearColor);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIEventDelegate_LinearColor_DynamicDelegate, FLinearColor, value);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIEventDelegate_LinearColor : public FLGUIEventDelegate
{
	GENERATED_BODY()
		MAKE_EVENTDELEGATE_PRESETPARAM(LinearColor, FLinearColor);
};

MAKE_EVENTDELEGATE_PRESETPARAM_DELEGATE(Quaternion, FQuat);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIEventDelegate_Quaternion_DynamicDelegate, FQuat, value);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIEventDelegate_Quaternion : public FLGUIEventDelegate
{
	GENERATED_BODY()
		MAKE_EVENTDELEGATE_PRESETPARAM(Quaternion, FQuat);
};

MAKE_EVENTDELEGATE_PRESETPARAM_DELEGATE(String, FString);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIEventDelegate_String_DynamicDelegate, FString, value);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIEventDelegate_String : public FLGUIEventDelegate
{
	GENERATED_BODY()
		MAKE_EVENTDELEGATE_PRESETPARAM(String, FString);
};

MAKE_EVENTDELEGATE_PRESETPARAM_DELEGATE(Object, UObject*);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIEventDelegate_Object_DynamicDelegate, UObject*, value);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIEventDelegate_Object : public FLGUIEventDelegate
{
	GENERATED_BODY()
		MAKE_EVENTDELEGATE_PRESETPARAM(Object, UObject*);
};

MAKE_EVENTDELEGATE_PRESETPARAM_DELEGATE(Actor, AActor*);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIEventDelegate_Actor_DynamicDelegate, AActor*, value);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIEventDelegate_Actor : public FLGUIEventDelegate
{
	GENERATED_BODY()
		MAKE_EVENTDELEGATE_PRESETPARAM(Actor, AActor*);
};

MAKE_EVENTDELEGATE_PRESETPARAM_DELEGATE(PointerEvent, ULGUIPointerEventData*);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIEventDelegate_PointerEvent_DynamicDelegate, ULGUIPointerEventData*, value);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIEventDelegate_PointerEvent : public FLGUIEventDelegate
{
	GENERATED_BODY()
		MAKE_EVENTDELEGATE_PRESETPARAM(PointerEvent, ULGUIPointerEventData*);
};

MAKE_EVENTDELEGATE_PRESETPARAM_DELEGATE(Class, UClass*);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIEventDelegate_Class_DynamicDelegate, UClass*, value);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIEventDelegate_Class : public FLGUIEventDelegate
{
	GENERATED_BODY()
		MAKE_EVENTDELEGATE_PRESETPARAM(Class, UClass*);
};

MAKE_EVENTDELEGATE_PRESETPARAM_DELEGATE(Rotator, FRotator);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIEventDelegate_Rotator_DynamicDelegate, FRotator, value);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIEventDelegate_Rotator : public FLGUIEventDelegate
{
	GENERATED_BODY()
		MAKE_EVENTDELEGATE_PRESETPARAM(Rotator, FRotator);
};

MAKE_EVENTDELEGATE_PRESETPARAM_DELEGATE(Text, FText);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIEventDelegate_Text_DynamicDelegate, FText, value);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIEventDelegate_Text : public FLGUIEventDelegate
{
	GENERATED_BODY()
		MAKE_EVENTDELEGATE_PRESETPARAM(Text, FText);
};

MAKE_EVENTDELEGATE_PRESETPARAM_DELEGATE(Name, FName);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIEventDelegate_Name_DynamicDelegate, FName, value);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIEventDelegate_Name : public FLGUIEventDelegate
{
	GENERATED_BODY()
		MAKE_EVENTDELEGATE_PRESETPARAM(Name, FName);
};
