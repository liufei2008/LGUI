// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Event/LGUIDrawableEvent.h"
#include "LGUIDelegateHandleWrapper.h"
#include "LGUIDrawableEvent_PresetParameter.generated.h"

#define MAKE_DRAWABLEEVENT_PRESETPARAM_DELEGATE(DrawableEventParamType, ParamType)\
DECLARE_DELEGATE_OneParam(FLGUIDrawableEvent_##DrawableEventParamType##_Delegate, ParamType);\
DECLARE_MULTICAST_DELEGATE_OneParam(FLGUIDrawableEvent_##DrawableEventParamType##_MulticastDelegate, ParamType);


#define MAKE_DRAWABLEEVENT_PRESETPARAM(DrawableEventParamType, ParamType)\
public:\
	FLGUIDrawableEvent_##DrawableEventParamType() :FLGUIDrawableEvent(LGUIDrawableEventParameterType::DrawableEventParamType) {}\
private:\
	mutable FLGUIDrawableEvent_##DrawableEventParamType##_MulticastDelegate eventDelegate;\
public:\
	FDelegateHandle Register(const TFunction<void(ParamType)>& function)const\
	{\
		return eventDelegate.AddLambda(function);\
	}\
	FDelegateHandle Register(const FLGUIDrawableEvent_##DrawableEventParamType##_Delegate& function)const\
	{\
		return eventDelegate.Add(function);\
	}\
	void Unregister(const FDelegateHandle& delegateHandle)const\
	{\
		eventDelegate.Remove(delegateHandle);\
	}\
	void operator() (ParamType InParam)const\
	{\
		FLGUIDrawableEvent::FireEvent(InParam);\
		if (eventDelegate.IsBound())eventDelegate.Broadcast(InParam);\
	}



DECLARE_DELEGATE(FLGUIDrawableEvent_Empty_Delegate); 
DECLARE_MULTICAST_DELEGATE(FLGUIDrawableEvent_Empty_MulticastDelegate);
DECLARE_DYNAMIC_DELEGATE(FLGUIDrawableEvent_Empty_DynamicDelegate);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIDrawableEvent_Empty : public FLGUIDrawableEvent
{
	GENERATED_BODY()
public:
	FLGUIDrawableEvent_Empty() :FLGUIDrawableEvent(LGUIDrawableEventParameterType::Empty) {}
private:
	mutable FLGUIDrawableEvent_Empty_MulticastDelegate eventDelegate;
public:
	FDelegateHandle Register(const TFunction<void()>& function)const
	{
		return eventDelegate.AddLambda(function);
	}
	FDelegateHandle Register(const FLGUIDrawableEvent_Empty_Delegate& function)const
	{
		return eventDelegate.Add(function);
	}
	void Unregister(const FDelegateHandle& delegateHandle)const
	{
		eventDelegate.Remove(delegateHandle);
	}
	void operator() ()const
	{
		FLGUIDrawableEvent::FireEvent();
		if (eventDelegate.IsBound())eventDelegate.Broadcast();
	}
};

MAKE_DRAWABLEEVENT_PRESETPARAM_DELEGATE(Bool, bool);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIDrawableEvent_Bool_DynamicDelegate, bool, value);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIDrawableEvent_Bool : public FLGUIDrawableEvent
{
	GENERATED_BODY()
MAKE_DRAWABLEEVENT_PRESETPARAM(Bool, bool);
};

MAKE_DRAWABLEEVENT_PRESETPARAM_DELEGATE(Float, float);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIDrawableEvent_Float_DynamicDelegate, float, value);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIDrawableEvent_Float : public FLGUIDrawableEvent
{
	GENERATED_BODY()
		MAKE_DRAWABLEEVENT_PRESETPARAM(Float, float);
};

MAKE_DRAWABLEEVENT_PRESETPARAM_DELEGATE(Double, double);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIDrawableEvent_Double_DynamicDelegate, double, value);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIDrawableEvent_Double : public FLGUIDrawableEvent
{
	GENERATED_BODY()
		MAKE_DRAWABLEEVENT_PRESETPARAM(Double, double);
};

MAKE_DRAWABLEEVENT_PRESETPARAM_DELEGATE(Int8, int8);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIDrawableEvent_Int8_DynamicDelegate, int8, value);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIDrawableEvent_Int8 : public FLGUIDrawableEvent
{
	GENERATED_BODY()
		MAKE_DRAWABLEEVENT_PRESETPARAM(Int8, int8);
};

MAKE_DRAWABLEEVENT_PRESETPARAM_DELEGATE(UInt8, uint8);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIDrawableEvent_UInt8_DynamicDelegate, uint8, value);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIDrawableEvent_UInt8 : public FLGUIDrawableEvent
{
	GENERATED_BODY()
		MAKE_DRAWABLEEVENT_PRESETPARAM(UInt8, uint8);
};

MAKE_DRAWABLEEVENT_PRESETPARAM_DELEGATE(Int16, int16);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIDrawableEvent_Int16_DynamicDelegate, int16, value);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIDrawableEvent_Int16 : public FLGUIDrawableEvent
{
	GENERATED_BODY()
		MAKE_DRAWABLEEVENT_PRESETPARAM(Int16, int16);
};

MAKE_DRAWABLEEVENT_PRESETPARAM_DELEGATE(UInt16, uint16);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIDrawableEvent_UInt16_DynamicDelegate, uint16, value);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIDrawableEvent_UInt16 : public FLGUIDrawableEvent
{
	GENERATED_BODY()
		MAKE_DRAWABLEEVENT_PRESETPARAM(UInt16, uint16);
};

MAKE_DRAWABLEEVENT_PRESETPARAM_DELEGATE(Int32, int32);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIDrawableEvent_Int32_DynamicDelegate, int32, value);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIDrawableEvent_Int32 : public FLGUIDrawableEvent
{
	GENERATED_BODY()
		MAKE_DRAWABLEEVENT_PRESETPARAM(Int32, int32);
};

MAKE_DRAWABLEEVENT_PRESETPARAM_DELEGATE(UInt32, uint32);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIDrawableEvent_UInt32_DynamicDelegate, uint32, value);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIDrawableEvent_UInt32 : public FLGUIDrawableEvent
{
	GENERATED_BODY()
		MAKE_DRAWABLEEVENT_PRESETPARAM(UInt32, uint32);
};

MAKE_DRAWABLEEVENT_PRESETPARAM_DELEGATE(Int64, int64);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIDrawableEvent_Int64_DynamicDelegate, int64, value);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIDrawableEvent_Int64 : public FLGUIDrawableEvent
{
	GENERATED_BODY()
		MAKE_DRAWABLEEVENT_PRESETPARAM(Int64, int64);
};

MAKE_DRAWABLEEVENT_PRESETPARAM_DELEGATE(UInt64, uint64);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIDrawableEvent_UInt64_DynamicDelegate, uint64, value);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIDrawableEvent_UInt64 : public FLGUIDrawableEvent
{
	GENERATED_BODY()
		MAKE_DRAWABLEEVENT_PRESETPARAM(UInt64, uint64);
};

MAKE_DRAWABLEEVENT_PRESETPARAM_DELEGATE(Vector2, FVector2D);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIDrawableEvent_Vector2_DynamicDelegate, FVector2D, value);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIDrawableEvent_Vector2 : public FLGUIDrawableEvent
{
	GENERATED_BODY()
		MAKE_DRAWABLEEVENT_PRESETPARAM(Vector2, FVector2D);
};

MAKE_DRAWABLEEVENT_PRESETPARAM_DELEGATE(Vector3, FVector);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIDrawableEvent_Vector3_DynamicDelegate, FVector, value);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIDrawableEvent_Vector3 : public FLGUIDrawableEvent
{
	GENERATED_BODY()
		MAKE_DRAWABLEEVENT_PRESETPARAM(Vector3, FVector);
};

MAKE_DRAWABLEEVENT_PRESETPARAM_DELEGATE(Vector4, FVector4);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIDrawableEvent_Vector4_DynamicDelegate, FVector4, value);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIDrawableEvent_Vector4 : public FLGUIDrawableEvent
{
	GENERATED_BODY()
		MAKE_DRAWABLEEVENT_PRESETPARAM(Vector4, FVector4);
};

MAKE_DRAWABLEEVENT_PRESETPARAM_DELEGATE(Color, FColor);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIDrawableEvent_Color_DynamicDelegate, FColor, value);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIDrawableEvent_Color : public FLGUIDrawableEvent
{
	GENERATED_BODY()
		MAKE_DRAWABLEEVENT_PRESETPARAM(Color, FColor);
};

MAKE_DRAWABLEEVENT_PRESETPARAM_DELEGATE(LinearColor, FLinearColor);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIDrawableEvent_LinearColor_DynamicDelegate, FLinearColor, value);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIDrawableEvent_LinearColor : public FLGUIDrawableEvent
{
	GENERATED_BODY()
		MAKE_DRAWABLEEVENT_PRESETPARAM(LinearColor, FLinearColor);
};

MAKE_DRAWABLEEVENT_PRESETPARAM_DELEGATE(Quaternion, FQuat);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIDrawableEvent_Quaternion_DynamicDelegate, FQuat, value);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIDrawableEvent_Quaternion : public FLGUIDrawableEvent
{
	GENERATED_BODY()
		MAKE_DRAWABLEEVENT_PRESETPARAM(Quaternion, FQuat);
};

MAKE_DRAWABLEEVENT_PRESETPARAM_DELEGATE(String, FString);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIDrawableEvent_String_DynamicDelegate, FString, value);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIDrawableEvent_String : public FLGUIDrawableEvent
{
	GENERATED_BODY()
		MAKE_DRAWABLEEVENT_PRESETPARAM(String, FString);
};

MAKE_DRAWABLEEVENT_PRESETPARAM_DELEGATE(Object, UObject*);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIDrawableEvent_Object_DynamicDelegate, UObject*, value);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIDrawableEvent_Object : public FLGUIDrawableEvent
{
	GENERATED_BODY()
		MAKE_DRAWABLEEVENT_PRESETPARAM(Object, UObject*);
};

MAKE_DRAWABLEEVENT_PRESETPARAM_DELEGATE(Actor, AActor*);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIDrawableEvent_Actor_DynamicDelegate, AActor*, value);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIDrawableEvent_Actor : public FLGUIDrawableEvent
{
	GENERATED_BODY()
		MAKE_DRAWABLEEVENT_PRESETPARAM(Actor, AActor*);
};

MAKE_DRAWABLEEVENT_PRESETPARAM_DELEGATE(PointerEvent, ULGUIPointerEventData*);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIDrawableEvent_PointerEvent_DynamicDelegate, ULGUIPointerEventData*, value);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIDrawableEvent_PointerEvent : public FLGUIDrawableEvent
{
	GENERATED_BODY()
		MAKE_DRAWABLEEVENT_PRESETPARAM(PointerEvent, ULGUIPointerEventData*);
};

MAKE_DRAWABLEEVENT_PRESETPARAM_DELEGATE(Class, UClass*);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIDrawableEvent_Class_DynamicDelegate, UClass*, value);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIDrawableEvent_Class : public FLGUIDrawableEvent
{
	GENERATED_BODY()
		MAKE_DRAWABLEEVENT_PRESETPARAM(Class, UClass*);
};

MAKE_DRAWABLEEVENT_PRESETPARAM_DELEGATE(Rotator, FRotator);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIDrawableEvent_Rotator_DynamicDelegate, FRotator, value);
USTRUCT(BlueprintType)
struct LGUI_API FLGUIDrawableEvent_Rotator : public FLGUIDrawableEvent
{
	GENERATED_BODY()
		MAKE_DRAWABLEEVENT_PRESETPARAM(Rotator, FRotator);
};
