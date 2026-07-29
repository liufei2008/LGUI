// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LexUIComponentReference.h"
#include "LexUIDelegateHandleWrapper.h"
#include "Event/LexUIEventDelegate.h"
#include "Event/LexUIEventDelegate_PresetParameter.h"
#include "Core/LexUISpriteData_BaseObject.h"
#include "PrefabSystem/LexUIPrefab.h"
#include LEXUIPREFAB_SERIALIZER_NEWEST_INCLUDE
#include "LexUIBPLibrary.generated.h"

namespace LEXUIPREFAB_SERIALIZER_NEWEST_NAMESPACE
{
	struct FDuplicateWidgetDataContainer;
}

USTRUCT(BlueprintType)
struct FLexUIDuplicateDataContainer
{
	GENERATED_BODY()
public:
	bool bIsValid = false;
	LEXUIPREFAB_SERIALIZER_NEWEST_NAMESPACE::FDuplicateWidgetDataContainer DuplicateData;
};

UCLASS()
class LGUI_API ULexUIBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Duplicate actor and all it's children actors
	 * If duplicate same actor for multiple times, then use PrepareDuplicateData node to get data, and pass the data to DuplicateActorWithPreparedData.
	 */
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true", ToolTip = "Duplicate actor with hierarchy"), Category = LGUI)
		static ULexWidget* DuplicateWidget(UObject* WorldContextObject, ULexWidget* Target, ULexWidget* Parent);
	/**
	 * Optimized version of DuplicateActor node when you need to duplicate same actor for multiple times. Use the result data in DuplicateActorWithPreparedData node.
	 */
	UFUNCTION(BlueprintCallable, meta = (UnsafeDuringActorConstruction = "true"), Category = LGUI)
		static void PrepareDuplicateData(ULexWidget* Target, FLexUIDuplicateDataContainer& Data);
	/**
	 * Use this with PrepareDuplicateData node.
	 */
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"), Category = LGUI)
		static ULexWidget* DuplicateWidgetWithPreparedData(UObject* WorldContextObject, UPARAM(Ref) FLexUIDuplicateDataContainer& Data, ULexWidget* Parent);
	template<class T>
	static T* DuplicateWidgetT(T* Target, USceneComponent* Parent)
	{
		static_assert(TPointerIsConvertibleFromTo<T, const AActor>::Value, "'T' template parameter to DuplicateActor must be derived from AActor");
		return (T*)ULexUIBPLibrary::DuplicateWidget(Target, Parent);
	}

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (WorldContext = "WorldContextObject", DeterminesOutputType = "VisualClass"))
	static ULexVisual* CreateWidgetWithVisual(UObject* WorldContextObject, ULexWidget* Parent, TSubclassOf<ULexVisual> VisualClass);
	
public:
#pragma region EventDelegate
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LexUIEventDelegateExecuteEmpty(const FLexUIEventDelegate& InEvent) { InEvent.FireEvent(); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LexUIEventDelegateExecuteBool(const FLexUIEventDelegate& InEvent, const bool& InParameter) { InEvent.FireEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LexUIEventDelegateExecuteFloat(const FLexUIEventDelegate& InEvent, const float& InParameter) { InEvent.FireEvent(InParameter); }
	//UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIEventDelegateExecuteDouble(const FLGUIEventDelegate& InEvent, const double& InParameter) { InEvent.FireEvent(InParameter); }
	//UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIEventDelegateExecuteInt8(const FLGUIEventDelegate& InEvent, const int8& InParameter) { InEvent.FireEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LexUIEventDelegateExecuteUInt8(const FLexUIEventDelegate& InEvent, const uint8& InParameter) { InEvent.FireEvent(InParameter); }
	//UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIEventDelegateExecuteInt16(const FLGUIEventDelegate& InEvent, const int16& InParameter) { InEvent.FireEvent(InParameter); }
	//UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIEventDelegateExecuteUInt16(const FLGUIEventDelegate& InEvent, const uint16& InParameter) { InEvent.FireEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LexUIEventDelegateExecuteInt32(const FLexUIEventDelegate& InEvent, const int32& InParameter) { InEvent.FireEvent(InParameter); }
	//UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIEventDelegateExecuteUInt32(const FLGUIEventDelegate& InEvent, const uint32& InParameter) { InEvent.FireEvent(InParameter); }
	//UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIEventDelegateExecuteInt64(const FLGUIEventDelegate& InEvent, const int64& InParameter) { InEvent.FireEvent(InParameter); }
	//UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIEventDelegateExecuteUInt64(const FLGUIEventDelegate& InEvent, const uint64& InParameter) { InEvent.FireEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LexUIEventDelegateExecuteVector2(const FLexUIEventDelegate& InEvent, const FVector2D& InParameter) { InEvent.FireEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LexUIEventDelegateExecuteVector3(const FLexUIEventDelegate& InEvent, const FVector& InParameter) { InEvent.FireEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LexUIEventDelegateExecuteVector4(const FLexUIEventDelegate& InEvent, const FVector4& InParameter) { InEvent.FireEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LexUIEventDelegateExecuteColor(const FLexUIEventDelegate& InEvent, const FColor& InParameter) { InEvent.FireEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LexUIEventDelegateExecuteLinearColor(const FLexUIEventDelegate& InEvent, const FLinearColor& InParameter) { InEvent.FireEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LexUIEventDelegateExecuteQuaternion(const FLexUIEventDelegate& InEvent, const FQuat& InParameter) { InEvent.FireEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LexUIEventDelegateExecuteString(const FLexUIEventDelegate& InEvent, const FString& InParameter) { InEvent.FireEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LexUIEventDelegateExecuteObject(const FLexUIEventDelegate& InEvent, UObject* InParameter) { InEvent.FireEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LexUIEventDelegateExecuteActor(const FLexUIEventDelegate& InEvent, AActor* InParameter) { InEvent.FireEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LexUIEventDelegateExecuteClass(const FLexUIEventDelegate& InEvent, UClass* InParameter) { InEvent.FireEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LexUIEventDelegateExecutePointerEvent(const FLexUIEventDelegate& InEvent, ULexPointerEventData* InParameter) { InEvent.FireEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LexUIEventDelegateExecuteRotator(const FLexUIEventDelegate& InEvent, const FRotator& InParameter) { InEvent.FireEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LexUIEventDelegateExecuteText(const FLexUIEventDelegate& InEvent, const FText& InParameter) { InEvent.FireEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LexUIEventDelegateExecuteName(const FLexUIEventDelegate& InEvent, const FName& InParameter) { InEvent.FireEvent(InParameter); }


	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LexUIEventDelegate_Empty_Execute(const FLexUIEventDelegate_Empty& InEvent) { InEvent(); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLexUIDelegateHandleWrapper LexUIEventDelegate_Empty_Register(const FLexUIEventDelegate_Empty& InEvent, FLexUIEventDelegate_Empty_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LexUIEventDelegate_Empty_Unregister(const FLexUIEventDelegate_Empty& InEvent, const FLexUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LexUIEventDelegate_Bool_Execute(const FLexUIEventDelegate_Bool& InEvent, bool InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLexUIDelegateHandleWrapper LexUIEventDelegate_Bool_Register(const FLexUIEventDelegate_Bool& InEvent, FLexUIEventDelegate_Bool_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LexUIEventDelegate_Bool_Unregister(const FLexUIEventDelegate_Bool& InEvent, const FLexUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LexUIEventDelegate_Float_Execute(const FLexUIEventDelegate_Float& InEvent, float InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLexUIDelegateHandleWrapper LexUIEventDelegate_Float_Register(const FLexUIEventDelegate_Float& InEvent, FLexUIEventDelegate_Float_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LexUIEventDelegate_Float_Unregister(const FLexUIEventDelegate_Float& InEvent, const FLexUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LexUIEventDelegate_Double_Execute(const FLexUIEventDelegate_Double& InEvent, double InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLexUIDelegateHandleWrapper LexUIEventDelegate_Double_Register(const FLexUIEventDelegate_Double& InEvent, FLexUIEventDelegate_Double_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LexUIEventDelegate_Double_Unregister(const FLexUIEventDelegate_Double& InEvent, const FLexUIDelegateHandleWrapper& InDelegateHandle);

	//UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
	//	static void LexUIEventDelegate_Int8_Execute(const FLexUIEventDelegate_Int8& InEvent, int8 InParameter) { InEvent(InParameter); }
	//UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
	//	static FLGUIDelegateHandleWrapper LexUIEventDelegate_Int8_Register(const FLexUIEventDelegate_Int8& InEvent, FLexUIEventDelegate_Int8_DynamicDelegate InDelegate);
	//UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
	//	static void LexUIEventDelegate_Int8_Unregister(const FLexUIEventDelegate_Int8& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LexUIEventDelegate_UInt8_Execute(const FLexUIEventDelegate_UInt8& InEvent, uint8 InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLexUIDelegateHandleWrapper LexUIEventDelegate_UInt8_Register(const FLexUIEventDelegate_UInt8& InEvent, FLexUIEventDelegate_UInt8_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LexUIEventDelegate_UInt8_Unregister(const FLexUIEventDelegate_UInt8& InEvent, const FLexUIDelegateHandleWrapper& InDelegateHandle);

	//UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
	//	static void LexUIEventDelegate_Int16_Execute(const FLexUIEventDelegate_Int16& InEvent, int16 InParameter) { InEvent(InParameter); }
	//UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
	//	static FLGUIDelegateHandleWrapper LexUIEventDelegate_Int16_Register(const FLexUIEventDelegate_Int16& InEvent, FLexUIEventDelegate_Int16_DynamicDelegate InDelegate);
	//UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
	//	static void LexUIEventDelegate_Int16_Unregister(const FLexUIEventDelegate_Int16& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	//UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
	//	static void LexUIEventDelegate_UInt16_Execute(const FLexUIEventDelegate_UInt16& InEvent, uint16 InParameter) { InEvent(InParameter); }
	//UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
	//	static FLGUIDelegateHandleWrapper LexUIEventDelegate_UInt16_Register(const FLexUIEventDelegate_UInt16& InEvent, FLexUIEventDelegate_UInt16_DynamicDelegate InDelegate);
	//UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
	//	static void LexUIEventDelegate_UInt16_Unregister(const FLexUIEventDelegate_UInt16& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LexUIEventDelegate_Int32_Execute(const FLexUIEventDelegate_Int32& InEvent, int32 InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLexUIDelegateHandleWrapper LexUIEventDelegate_Int32_Register(const FLexUIEventDelegate_Int32& InEvent, FLexUIEventDelegate_Int32_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LexUIEventDelegate_Int32_Unregister(const FLexUIEventDelegate_Int32& InEvent, const FLexUIDelegateHandleWrapper& InDelegateHandle);

	//UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
	//	static void LexUIEventDelegate_UInt32_Execute(const FLexUIEventDelegate_UInt32& InEvent, uint32 InParameter) { InEvent(InParameter); }
	//UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
	//	static FLGUIDelegateHandleWrapper LexUIEventDelegate_UInt32_Register(const FLexUIEventDelegate_UInt32& InEvent, FLexUIEventDelegate_UInt32_DynamicDelegate InDelegate);
	//UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
	//	static void LexUIEventDelegate_UInt32_Unregister(const FLexUIEventDelegate_UInt32& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LexUIEventDelegate_Int64_Execute(const FLexUIEventDelegate_Int64& InEvent, int64 InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLexUIDelegateHandleWrapper LexUIEventDelegate_Int64_Register(const FLexUIEventDelegate_Int64& InEvent, FLexUIEventDelegate_Int64_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LexUIEventDelegate_Int64_Unregister(const FLexUIEventDelegate_Int64& InEvent, const FLexUIDelegateHandleWrapper& InDelegateHandle);

	//UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
	//	static void LexUIEventDelegate_UInt64_Execute(const FLexUIEventDelegate_UInt64& InEvent, uint64 InParameter) { InEvent(InParameter); }
	//UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
	//	static FLGUIDelegateHandleWrapper LexUIEventDelegate_UInt64_Register(const FLexUIEventDelegate_UInt64& InEvent, FLexUIEventDelegate_UInt64_DynamicDelegate InDelegate);
	//UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
	//	static void LexUIEventDelegate_UInt64_Unregister(const FLexUIEventDelegate_UInt64& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LexUIEventDelegate_Vector2_Execute(const FLexUIEventDelegate_Vector2& InEvent, FVector2D InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLexUIDelegateHandleWrapper LexUIEventDelegate_Vector2_Register(const FLexUIEventDelegate_Vector2& InEvent, FLexUIEventDelegate_Vector2_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LexUIEventDelegate_Vector2_Unregister(const FLexUIEventDelegate_Vector2& InEvent, const FLexUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LexUIEventDelegate_Vector3_Execute(const FLexUIEventDelegate_Vector3& InEvent, FVector InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLexUIDelegateHandleWrapper LexUIEventDelegate_Vector3_Register(const FLexUIEventDelegate_Vector3& InEvent, FLexUIEventDelegate_Vector3_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LexUIEventDelegate_Vector3_Unregister(const FLexUIEventDelegate_Vector3& InEvent, const FLexUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LexUIEventDelegate_Vector4_Execute(const FLexUIEventDelegate_Vector4& InEvent, FVector4 InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLexUIDelegateHandleWrapper LexUIEventDelegate_Vector4_Register(const FLexUIEventDelegate_Vector4& InEvent, FLexUIEventDelegate_Vector4_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LexUIEventDelegate_Vector4_Unregister(const FLexUIEventDelegate_Vector4& InEvent, const FLexUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LexUIEventDelegate_Color_Execute(const FLexUIEventDelegate_Color& InEvent, FColor InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLexUIDelegateHandleWrapper LexUIEventDelegate_Color_Register(const FLexUIEventDelegate_Color& InEvent, FLexUIEventDelegate_Color_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LexUIEventDelegate_Color_Unregister(const FLexUIEventDelegate_Color& InEvent, const FLexUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LexUIEventDelegate_LinearColor_Execute(const FLexUIEventDelegate_LinearColor& InEvent, FLinearColor InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLexUIDelegateHandleWrapper LexUIEventDelegate_LinearColor_Register(const FLexUIEventDelegate_LinearColor& InEvent, FLexUIEventDelegate_LinearColor_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LexUIEventDelegate_LinearColor_Unregister(const FLexUIEventDelegate_LinearColor& InEvent, const FLexUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LexUIEventDelegate_Quaternion_Execute(const FLexUIEventDelegate_Quaternion& InEvent, FQuat InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLexUIDelegateHandleWrapper LexUIEventDelegate_Quaternion_Register(const FLexUIEventDelegate_Quaternion& InEvent, FLexUIEventDelegate_Quaternion_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LexUIEventDelegate_Quaternion_Unregister(const FLexUIEventDelegate_Quaternion& InEvent, const FLexUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LexUIEventDelegate_String_Execute(const FLexUIEventDelegate_String& InEvent, FString InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLexUIDelegateHandleWrapper LexUIEventDelegate_String_Register(const FLexUIEventDelegate_String& InEvent, FLexUIEventDelegate_String_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LexUIEventDelegate_String_Unregister(const FLexUIEventDelegate_String& InEvent, const FLexUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LexUIEventDelegate_Object_Execute(const FLexUIEventDelegate_Asset& InEvent, UObject* InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLexUIDelegateHandleWrapper LexUIEventDelegate_Asset_Register(const FLexUIEventDelegate_Asset& InEvent, FLexUIEventDelegate_Asset_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LexUIEventDelegate_Asset_Unregister(const FLexUIEventDelegate_Asset& InEvent, const FLexUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LexUIEventDelegate_LexWidget_Execute(const FLexUIEventDelegate_LexWidget& InEvent, ULexWidget* InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLexUIDelegateHandleWrapper LexUIEventDelegate_LexWidget_Register(const FLexUIEventDelegate_LexWidget& InEvent, FLexUIEventDelegate_LexWidget_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LexUIEventDelegate_LexWidget_Unregister(const FLexUIEventDelegate_LexWidget& InEvent, const FLexUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LexUIEventDelegate_PointerEvent_Execute(const FLexUIEventDelegate_PointerEvent& InEvent, ULexPointerEventData* InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLexUIDelegateHandleWrapper LexUIEventDelegate_PointerEvent_Register(const FLexUIEventDelegate_PointerEvent& InEvent, FLexUIEventDelegate_PointerEvent_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LexUIEventDelegate_PointerEvent_Unregister(const FLexUIEventDelegate_PointerEvent& InEvent, const FLexUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LexUIEventDelegate_Class_Execute(const FLexUIEventDelegate_Class& InEvent, UClass* InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLexUIDelegateHandleWrapper LexUIEventDelegate_Class_Register(const FLexUIEventDelegate_Class& InEvent, FLexUIEventDelegate_Class_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LexUIEventDelegate_Class_Unregister(const FLexUIEventDelegate_Class& InEvent, const FLexUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LexUIEventDelegate_Rotator_Execute(const FLexUIEventDelegate_Rotator& InEvent, FRotator InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLexUIDelegateHandleWrapper LexUIEventDelegate_Rotator_Register(const FLexUIEventDelegate_Rotator& InEvent, FLexUIEventDelegate_Rotator_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LexUIEventDelegate_Rotator_Unregister(const FLexUIEventDelegate_Rotator& InEvent, const FLexUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LexUIEventDelegate_Text_Execute(const FLexUIEventDelegate_Text& InEvent, FText InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLexUIDelegateHandleWrapper LexUIEventDelegate_Text_Register(const FLexUIEventDelegate_Text& InEvent, FLexUIEventDelegate_Text_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LexUIEventDelegate_Text_Unregister(const FLexUIEventDelegate_Text& InEvent, const FLexUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LexUIEventDelegate_Name_Execute(const FLexUIEventDelegate_Name& InEvent, FName InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLexUIDelegateHandleWrapper LexUIEventDelegate_Name_Register(const FLexUIEventDelegate_Name& InEvent, FLexUIEventDelegate_Name_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LexUIEventDelegate_Name_Unregister(const FLexUIEventDelegate_Name& InEvent, const FLexUIDelegateHandleWrapper& InDelegateHandle);
#pragma endregion EventDelegate

	/** InComponentType must be the same as InLexUIComponentReference's component type */
	UFUNCTION(BlueprintPure, Category = LGUI, meta = (DisplayName = "Get Component", CompactNodeTitle = "Component", BlueprintAutocast, DeterminesOutputType = "InComponentType"))
		static UActorComponent* LexUICompRef_GetComponent(const FLexUIComponentReference& InLexUIComponentReference, TSubclassOf<UActorComponent> InComponentType);
	UFUNCTION(BlueprintPure, Category = LGUI, meta = (DisplayName = "Get Actor", CompactNodeTitle = "Actor", BlueprintAutocast))
		static AActor* LexUICompRef_GetActor(const FLexUIComponentReference& InLexUIComponentReference);

	UFUNCTION(BlueprintPure, Category = LGUI, meta = (DisplayName = "Get", CompactNodeTitle = ".", BlueprintInternalUseOnly = "true"))
		static void K2_LexUICompRef_GetComponent(const FLexUIComponentReference& InLexUICompRef, UActorComponent*& OutResult);

#pragma region LTween

	UFUNCTION(BlueprintCallable, Category = LGUI)
		static void LexUIExecuteControllerInputAxis(FKey inputKey, float value);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		static void LexUIExecuteControllerInputAction(FKey inputKey, bool pressOrRelease);

#pragma endregion
	UFUNCTION(BlueprintPure, Category = LGUI)
		static void GetSpriteSize(const FLexUISpriteInfo& SpriteInfo, int32& width, int32& height);
	UFUNCTION(BlueprintPure, Category = LGUI)
		static void GetSpriteBorderSize(const FLexUISpriteInfo& SpriteInfo, int32& borderLeft, int32& borderRight, int32& borderTop, int32& borderBottom);
	UFUNCTION(BlueprintPure, Category = LGUI)
		static void GetSpriteUV(const FLexUISpriteInfo& SpriteInfo, float& UV0X, float& UV0Y, float& UV3X, float& UV3Y);
	UFUNCTION(BlueprintPure, Category = LGUI)
		static void GetSpriteBorderUV(const FLexUISpriteInfo& SpriteInfo, float& borderUV0X, float& borderUV0Y, float& borderUV3X, float& borderUV3Y);
};
