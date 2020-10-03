// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LGUIComponentReference.h"
#include "Event/LGUIDrawableEvent.h"
#include "Event/LGUIDrawableEvent_PresetParameter.h"
#include "LTweener.h"
#include "LGUIBPLibrary.generated.h"

class UUIItem;
class UUISector;

UCLASS()
class LGUI_API ULGUIBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
#pragma region QuickEntry
	//Set alpha if root component is a UIItem component
	UFUNCTION(BlueprintCallable, Category = LGUI)
		static void SetUIAlpha(AActor* Target, float InAlpha);
	//Set UIActive if root component is a UIItem component
	UFUNCTION(BlueprintCallable, Category = LGUI)
		static void SetUIActive(AActor* Target, bool Acitve);
	//Set HierarchyIndex if root component is a UIItem component
	UFUNCTION(BlueprintCallable, Category = LGUI)
		static void SetUIHierarchyIndex(AActor* Target, int32 index);
#pragma endregion
	//Delete actor and all it's children actors
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "WithHierarchy", UnsafeDuringActorConstruction = "true"), Category = LGUI)
		static void DeleteActor(AActor* Target, bool WithHierarchy = true);

	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "SetRelativeTransformToIdentity", UnsafeDuringActorConstruction = "true", WorldContext = "WorldContextObject"), Category = LGUI)
		static AActor* LoadPrefab(UObject* WorldContextObject, ULGUIPrefab* InPrefab, USceneComponent* InParent, bool SetRelativeTransformToIdentity = true);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "", WorldContext = "WorldContextObject"), Category = LGUI)
		static AActor* LoadPrefabWithTransform(UObject* WorldContextObject, ULGUIPrefab* InPrefab, USceneComponent* InParent, FVector Location, FRotator Rotation, FVector Scale);
	static AActor* LoadPrefabWithTransform(UObject* WorldContextObject, ULGUIPrefab* InPrefab, USceneComponent* InParent, FVector Location, FQuat Rotation, FVector Scale);

	//Duplicate actor and all it's children actors
	UFUNCTION(BlueprintCallable, meta = (DeterminesOutputType = "Target", UnsafeDuringActorConstruction = "true", ToolTip = "Copy actor with hierarchy"), Category = LGUI)
		static AActor* DuplicateActor(AActor* Target, USceneComponent* Parent);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (ComponentClass = "ActorComponent"), meta = (DeterminesOutputType = "ComponentClass"))
		//find the first component in parent and up parent hierarchy with type
		static UActorComponent* GetComponentInParent(AActor* InActor, TSubclassOf<UActorComponent> ComponentClass);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (ComponentClass = "ActorComponent"), meta = (DeterminesOutputType = "ComponentClass"))
		//find all compoents in children with type
		static TArray<UActorComponent*> GetComponentsInChildren(AActor* InActor, TSubclassOf<UActorComponent> ComponentClass, bool IncludeSelf);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (ComponentClass = "ActorComponent"), meta = (DeterminesOutputType = "ComponentClass"))
		//find the first component in children with type
		static UActorComponent* GetComponentInChildren(AActor* InActor, TSubclassOf<UActorComponent> ComponentClass, bool IncludeSelf);
private:
	static void CollectComponentsInChildrenRecursive(AActor* InActor, TSubclassOf<UActorComponent> ComponentClass, TArray<UActorComponent*>& InOutArray);
	static UActorComponent* FindComponentInChildrenRecursive(AActor* InActor, TSubclassOf<UActorComponent> ComponentClass);
public:
#pragma region DrawableEvent
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIDrawableEventExecuteEmpty(const FLGUIDrawableEvent& InEvent) { InEvent.FireEvent(); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIDrawableEventExecuteBool(const FLGUIDrawableEvent& InEvent, const bool& InParameter) { InEvent.FireEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIDrawableEventExecuteFloat(const FLGUIDrawableEvent& InEvent, const float& InParameter) { InEvent.FireEvent(InParameter); }
	//UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIDrawableEventExecuteDouble(const FLGUIDrawableEvent& InEvent, const double& InParameter) { InEvent.FireEvent(InParameter); }
	//UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIDrawableEventExecuteInt8(const FLGUIDrawableEvent& InEvent, const int8& InParameter) { InEvent.FireEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIDrawableEventExecuteUInt8(const FLGUIDrawableEvent& InEvent, const uint8& InParameter) { InEvent.FireEvent(InParameter); }
	//UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIDrawableEventExecuteInt16(const FLGUIDrawableEvent& InEvent, const int16& InParameter) { InEvent.FireEvent(InParameter); }
	//UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIDrawableEventExecuteUInt16(const FLGUIDrawableEvent& InEvent, const uint16& InParameter) { InEvent.FireEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIDrawableEventExecuteInt32(const FLGUIDrawableEvent& InEvent, const int32& InParameter) { InEvent.FireEvent(InParameter); }
	//UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIDrawableEventExecuteUInt32(const FLGUIDrawableEvent& InEvent, const uint32& InParameter) { InEvent.FireEvent(InParameter); }
	//UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIDrawableEventExecuteInt64(const FLGUIDrawableEvent& InEvent, const int64& InParameter) { InEvent.FireEvent(InParameter); }
	//UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIDrawableEventExecuteUInt64(const FLGUIDrawableEvent& InEvent, const uint64& InParameter) { InEvent.FireEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIDrawableEventExecuteVector2(const FLGUIDrawableEvent& InEvent, const FVector2D& InParameter) { InEvent.FireEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIDrawableEventExecuteVector3(const FLGUIDrawableEvent& InEvent, const FVector& InParameter) { InEvent.FireEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIDrawableEventExecuteVector4(const FLGUIDrawableEvent& InEvent, const FVector4& InParameter) { InEvent.FireEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIDrawableEventExecuteColor(const FLGUIDrawableEvent& InEvent, const FColor& InParameter) { InEvent.FireEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIDrawableEventExecuteLinearColor(const FLGUIDrawableEvent& InEvent, const FLinearColor& InParameter) { InEvent.FireEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIDrawableEventExecuteQuaternion(const FLGUIDrawableEvent& InEvent, const FQuat& InParameter) { InEvent.FireEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIDrawableEventExecuteString(const FLGUIDrawableEvent& InEvent, const FString& InParameter) { InEvent.FireEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIDrawableEventExecuteObject(const FLGUIDrawableEvent& InEvent, UObject* InParameter) { InEvent.FireEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIDrawableEventExecuteActor(const FLGUIDrawableEvent& InEvent, AActor* InParameter) { InEvent.FireEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIDrawableEventExecuteClass(const FLGUIDrawableEvent& InEvent, UClass* InParameter) { InEvent.FireEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIDrawableEventExecutePointerEvent(const FLGUIDrawableEvent& InEvent, ULGUIPointerEventData* InParameter) { InEvent.FireEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIDrawableEventExecuteRotator(const FLGUIDrawableEvent& InEvent, const FRotator& InParameter) { InEvent.FireEvent(InParameter); }


	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LGUIDrawableEvent_Empty_Execute(const FLGUIDrawableEvent_Empty& InEvent) { InEvent(); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLGUIDelegateHandleWrapper LGUIDrawableEvent_Empty_Register(const FLGUIDrawableEvent_Empty& InEvent, FLGUIDrawableEvent_Empty_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LGUIDrawableEvent_Empty_Unregister(const FLGUIDrawableEvent_Empty& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LGUIDrawableEvent_Bool_Execute(const FLGUIDrawableEvent_Bool& InEvent, bool InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLGUIDelegateHandleWrapper LGUIDrawableEvent_Bool_Register(const FLGUIDrawableEvent_Bool& InEvent, FLGUIDrawableEvent_Bool_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LGUIDrawableEvent_Bool_Unregister(const FLGUIDrawableEvent_Bool& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LGUIDrawableEvent_Float_Execute(const FLGUIDrawableEvent_Float& InEvent, float InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLGUIDelegateHandleWrapper LGUIDrawableEvent_Float_Register(const FLGUIDrawableEvent_Float& InEvent, FLGUIDrawableEvent_Float_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LGUIDrawableEvent_Float_Unregister(const FLGUIDrawableEvent_Float& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	//UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
	//	static void LGUIDrawableEvent_Double_Execute(const FLGUIDrawableEvent_Double& InEvent, double InParameter) { InEvent(InParameter); }
	//UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
	//	static FLGUIDelegateHandleWrapper LGUIDrawableEvent_Double_Register(const FLGUIDrawableEvent_Double& InEvent, FLGUIDrawableEvent_Double_DynamicDelegate InDelegate);
	//UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
	//	static void LGUIDrawableEvent_Double_Unregister(const FLGUIDrawableEvent_Double& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	//UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
	//	static void LGUIDrawableEvent_Int8_Execute(const FLGUIDrawableEvent_Int8& InEvent, int8 InParameter) { InEvent(InParameter); }
	//UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
	//	static FLGUIDelegateHandleWrapper LGUIDrawableEvent_Int8_Register(const FLGUIDrawableEvent_Int8& InEvent, FLGUIDrawableEvent_Int8_DynamicDelegate InDelegate);
	//UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
	//	static void LGUIDrawableEvent_Int8_Unregister(const FLGUIDrawableEvent_Double& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LGUIDrawableEvent_UInt8_Execute(const FLGUIDrawableEvent_UInt8& InEvent, uint8 InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLGUIDelegateHandleWrapper LGUIDrawableEvent_UInt8_Register(const FLGUIDrawableEvent_UInt8& InEvent, FLGUIDrawableEvent_UInt8_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LGUIDrawableEvent_UInt8_Unregister(const FLGUIDrawableEvent_UInt8& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	//UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
	//	static void LGUIDrawableEvent_Int16_Execute(const FLGUIDrawableEvent_Int16& InEvent, int16 InParameter) { InEvent(InParameter); }
	//UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
	//	static FLGUIDelegateHandleWrapper LGUIDrawableEvent_Int16_Register(const FLGUIDrawableEvent_Int16& InEvent, FLGUIDrawableEvent_Int16_DynamicDelegate InDelegate);
	//UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
	//	static void LGUIDrawableEvent_Int16_Unregister(const FLGUIDrawableEvent_Int16& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	//UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
	//	static void LGUIDrawableEvent_UInt16_Execute(const FLGUIDrawableEvent_UInt16& InEvent, uint16 InParameter) { InEvent(InParameter); }
	//UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
	//	static FLGUIDelegateHandleWrapper LGUIDrawableEvent_UInt16_Register(const FLGUIDrawableEvent_UInt16& InEvent, FLGUIDrawableEvent_UInt16_DynamicDelegate InDelegate);
	//UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
	//	static void LGUIDrawableEvent_UInt16_Unregister(const FLGUIDrawableEvent_UInt16& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LGUIDrawableEvent_Int32_Execute(const FLGUIDrawableEvent_Int32& InEvent, int32 InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLGUIDelegateHandleWrapper LGUIDrawableEvent_Int32_Register(const FLGUIDrawableEvent_Int32& InEvent, FLGUIDrawableEvent_Int32_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LGUIDrawableEvent_Int32_Unregister(const FLGUIDrawableEvent_Int32& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	//UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
	//	static void LGUIDrawableEvent_UInt32_Execute(const FLGUIDrawableEvent_UInt32& InEvent, uint32 InParameter) { InEvent(InParameter); }
	//UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
	//	static FLGUIDelegateHandleWrapper LGUIDrawableEvent_UInt32_Register(const FLGUIDrawableEvent_UInt32& InEvent, FLGUIDrawableEvent_UInt32_DynamicDelegate InDelegate);
	//UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
	//	static void LGUIDrawableEvent_UInt32_Unregister(const FLGUIDrawableEvent_UInt32& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LGUIDrawableEvent_Int64_Execute(const FLGUIDrawableEvent_Int64& InEvent, int64 InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLGUIDelegateHandleWrapper LGUIDrawableEvent_Int64_Register(const FLGUIDrawableEvent_Int64& InEvent, FLGUIDrawableEvent_Int64_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LGUIDrawableEvent_Int64_Unregister(const FLGUIDrawableEvent_Int64& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	//UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
	//	static void LGUIDrawableEvent_UInt64_Execute(const FLGUIDrawableEvent_UInt64& InEvent, uint64 InParameter) { InEvent(InParameter); }
	//UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
	//	static FLGUIDelegateHandleWrapper LGUIDrawableEvent_UInt64_Register(const FLGUIDrawableEvent_UInt64& InEvent, FLGUIDrawableEvent_UInt64_DynamicDelegate InDelegate);
	//UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
	//	static void LGUIDrawableEvent_UInt64_Unregister(const FLGUIDrawableEvent_UInt64& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LGUIDrawableEvent_Vector2_Execute(const FLGUIDrawableEvent_Vector2& InEvent, FVector2D InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLGUIDelegateHandleWrapper LGUIDrawableEvent_Vector2_Register(const FLGUIDrawableEvent_Vector2& InEvent, FLGUIDrawableEvent_Vector2_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LGUIDrawableEvent_Vector2_Unregister(const FLGUIDrawableEvent_Vector2& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LGUIDrawableEvent_Vector3_Execute(const FLGUIDrawableEvent_Vector3& InEvent, FVector InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLGUIDelegateHandleWrapper LGUIDrawableEvent_Vector3_Register(const FLGUIDrawableEvent_Vector3& InEvent, FLGUIDrawableEvent_Vector3_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LGUIDrawableEvent_Vector3_Unregister(const FLGUIDrawableEvent_Vector3& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LGUIDrawableEvent_Vector4_Execute(const FLGUIDrawableEvent_Vector4& InEvent, FVector4 InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLGUIDelegateHandleWrapper LGUIDrawableEvent_Vector4_Register(const FLGUIDrawableEvent_Vector4& InEvent, FLGUIDrawableEvent_Vector4_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LGUIDrawableEvent_Vector4_Unregister(const FLGUIDrawableEvent_Vector4& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LGUIDrawableEvent_Color_Execute(const FLGUIDrawableEvent_Color& InEvent, FColor InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLGUIDelegateHandleWrapper LGUIDrawableEvent_Color_Register(const FLGUIDrawableEvent_Color& InEvent, FLGUIDrawableEvent_Color_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LGUIDrawableEvent_Color_Unregister(const FLGUIDrawableEvent_Color& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LGUIDrawableEvent_LinearColor_Execute(const FLGUIDrawableEvent_LinearColor& InEvent, FLinearColor InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLGUIDelegateHandleWrapper LGUIDrawableEvent_LinearColor_Register(const FLGUIDrawableEvent_LinearColor& InEvent, FLGUIDrawableEvent_LinearColor_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LGUIDrawableEvent_LinearColor_Unregister(const FLGUIDrawableEvent_LinearColor& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LGUIDrawableEvent_Quaternion_Execute(const FLGUIDrawableEvent_Quaternion& InEvent, FQuat InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLGUIDelegateHandleWrapper LGUIDrawableEvent_Quaternion_Register(const FLGUIDrawableEvent_Quaternion& InEvent, FLGUIDrawableEvent_Quaternion_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LGUIDrawableEvent_Quaternion_Unregister(const FLGUIDrawableEvent_Quaternion& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LGUIDrawableEvent_String_Execute(const FLGUIDrawableEvent_String& InEvent, FString InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLGUIDelegateHandleWrapper LGUIDrawableEvent_String_Register(const FLGUIDrawableEvent_String& InEvent, FLGUIDrawableEvent_String_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LGUIDrawableEvent_String_Unregister(const FLGUIDrawableEvent_String& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LGUIDrawableEvent_Object_Execute(const FLGUIDrawableEvent_Object& InEvent, UObject* InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLGUIDelegateHandleWrapper LGUIDrawableEvent_Object_Register(const FLGUIDrawableEvent_Object& InEvent, FLGUIDrawableEvent_Object_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LGUIDrawableEvent_Object_Unregister(const FLGUIDrawableEvent_Object& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LGUIDrawableEvent_Actor_Execute(const FLGUIDrawableEvent_Actor& InEvent, AActor* InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLGUIDelegateHandleWrapper LGUIDrawableEvent_Actor_Register(const FLGUIDrawableEvent_Actor& InEvent, FLGUIDrawableEvent_Actor_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LGUIDrawableEvent_Actor_Unregister(const FLGUIDrawableEvent_Actor& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LGUIDrawableEvent_PointerEvent_Execute(const FLGUIDrawableEvent_PointerEvent& InEvent, ULGUIPointerEventData* InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLGUIDelegateHandleWrapper LGUIDrawableEvent_PointerEvent_Register(const FLGUIDrawableEvent_PointerEvent& InEvent, FLGUIDrawableEvent_PointerEvent_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LGUIDrawableEvent_PointerEvent_Unregister(const FLGUIDrawableEvent_PointerEvent& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LGUIDrawableEvent_Class_Execute(const FLGUIDrawableEvent_Class& InEvent, UClass* InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLGUIDelegateHandleWrapper LGUIDrawableEvent_Class_Register(const FLGUIDrawableEvent_Class& InEvent, FLGUIDrawableEvent_Class_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LGUIDrawableEvent_Class_Unregister(const FLGUIDrawableEvent_Class& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LGUIDrawableEvent_Rotator_Execute(const FLGUIDrawableEvent_Rotator& InEvent, FRotator InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLGUIDelegateHandleWrapper LGUIDrawableEvent_Rotator_Register(const FLGUIDrawableEvent_Rotator& InEvent, FLGUIDrawableEvent_Rotator_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LGUIDrawableEvent_Rotator_Unregister(const FLGUIDrawableEvent_Rotator& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);
#pragma endregion DrawableEvent

	//InComponentType must be the same as InLGUIComponentReference's component type
	UFUNCTION(BlueprintPure, Category = LGUI, meta = (DisplayName = "GetComponent", CompactNodeTitle = ".", BlueprintAutocast, DeterminesOutputType = "InComponentType", DeprecatedFunction, DeprecationMessage = "This node is not valid any more. Use default \"Get\" node to get component."))
		static UActorComponent* LGUICompRef_GetComponent(const FLGUIComponentReference& InLGUIComponentReference, TSubclassOf<UActorComponent> InComponentType);
	UFUNCTION(BlueprintPure, Category = LGUI, meta = (DisplayName = "GetComponentClass", CompactNodeTitle = "Class", BlueprintAutocast))
		static TSubclassOf<UActorComponent> LGUICompRef_GetComponentClass(const FLGUIComponentReference& InLGUIComponentReference);
	UFUNCTION(BlueprintPure, Category = LGUI, meta = (DisplayName = "GetActor", CompactNodeTitle = "Actor", BlueprintAutocast))
		static AActor* LGUICompRef_GetActor(const FLGUIComponentReference& InLGUIComponentReference);

	UFUNCTION(BlueprintPure, Category = LGUI, meta = (DisplayName = "Get", CompactNodeTitle = ".", BlueprintInternalUseOnly = "true"))
		static void K2_LGUICompRef_GetComponent(const FLGUIComponentReference& InLGUICompRef, UActorComponent*& OutResult);

#pragma region LTween

#pragma region UIItem
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTween-LGUI")
		static ULTweener* WidthTo(UUIItem* target, float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTween-LGUI")
		static ULTweener* HeightTo(UUIItem* target, float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);

	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTween-LGUI")
		static ULTweener* ColorTo(UUIItem* target, FColor endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTween-LGUI")
		static ULTweener* ColorFrom(UUIItem* target, FColor startValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);

	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTween-LGUI")
		static ULTweener* AlphaTo(UUIItem* target, float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTween-LGUI")
		static ULTweener* AlphaFrom(UUIItem* target, float startValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);

	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTween-LGUI")
		static ULTweener* AnchorOffsetXTo(UUIItem* target, float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTween-LGUI")
		static ULTweener* AnchorOffsetYTo(UUIItem* target, float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTween-LGUI")
		static ULTweener* PivotTo(UUIItem* target, FVector2D endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTween-LGUI")
		static ULTweener* StretchLeftTo(UUIItem* target, float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTween-LGUI")
		static ULTweener* StretchRightTo(UUIItem* target, float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTween-LGUI")
		static ULTweener* StretchTopTo(UUIItem* target, float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTween-LGUI")
		static ULTweener* StretchBottomTo(UUIItem* target, float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
#pragma endregion

#pragma region UISector
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTween-LGUI")
		static ULTweener* StartAngleTo(UUISector* target, float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTween-LGUI")
		static ULTweener* EndAngleTo(UUISector* target, float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
#pragma endregion

	UFUNCTION(BlueprintCallable, Category = LGUI)
		static void LGUIExecuteInputAxis(FKey inputKey, float value);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		static void LGUIExecuteInputAction(FKey inputKey, bool pressOrRelease);

#pragma endregion

};
