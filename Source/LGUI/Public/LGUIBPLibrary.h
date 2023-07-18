// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LGUIComponentReference.h"
#include "Event/LGUIEventDelegate.h"
#include "Event/LGUIEventDelegate_PresetParameter.h"
#include "LTweener.h"
#include "Core/LGUISpriteData_BaseObject.h"
#include "PrefabSystem/ActorSerializer5.h"
#include "LGUIBPLibrary.generated.h"

class UUIItem;
class UUIBaseRenderable;
class UUISector;
class ULGUIPrefab;

namespace LGUIPrefabSystem5
{
	class ActorSerializer;
	struct FLGUIPrefabSaveData;
}

USTRUCT(BlueprintType)
struct FLGUIDuplicateDataContainer
{
	GENERATED_BODY()
public:
	bool bIsValid = false;
	LGUIPrefabSystem5::ActorSerializer ActorSerializer;
	LGUIPrefabSystem5::FLGUIPrefabSaveData SerializedData;
};

UCLASS()
class LGUI_API ULGUIBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Delete actor and all it's children actors */
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "WithHierarchy", UnsafeDuringActorConstruction = "true"), Category = LGUI)
		static void DestroyActorWithHierarchy(AActor* Target, bool WithHierarchy = true);

	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "SetRelativeTransformToIdentity", UnsafeDuringActorConstruction = "true", WorldContext = "WorldContextObject"), Category = LGUI)
		static AActor* LoadPrefab(UObject* WorldContextObject, ULGUIPrefab* InPrefab, USceneComponent* InParent, bool SetRelativeTransformToIdentity = false);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "", UnsafeDuringActorConstruction = "true", WorldContext = "WorldContextObject"), Category = LGUI)
		static AActor* LoadPrefabWithTransform(UObject* WorldContextObject, ULGUIPrefab* InPrefab, USceneComponent* InParent, FVector Location, FRotator Rotation, FVector Scale);
	static AActor* LoadPrefabWithTransform(UObject* WorldContextObject, ULGUIPrefab* InPrefab, USceneComponent* InParent, FVector Location, FQuat Rotation, FVector Scale);

	/**
	 * Duplicate actor and all it's children actors
	 * If duplicate same actor for multiple times, then use PrepareDuplicateData node to get data, and pass the data to DuplicateActorWithPreparedData.
	 */
	UFUNCTION(BlueprintCallable, meta = (DeterminesOutputType = "Target", UnsafeDuringActorConstruction = "true", ToolTip = "Duplicate actor with hierarchy"), Category = LGUI)
		static AActor* DuplicateActor(AActor* Target, USceneComponent* Parent);
	/**
	 * Optimized version of DuplicateActor node when you need to duplicate same actor for multiple times. Use the result data in DuplicateActorWithPreparedData node.
	 */
	UFUNCTION(BlueprintCallable, meta = (DeterminesOutputType = "Target", UnsafeDuringActorConstruction = "true"), Category = LGUI)
		static void PrepareDuplicateData(AActor* Target, FLGUIDuplicateDataContainer& Data);
	/**
	 * Use this with PrepareDuplicateData node.
	 */
	UFUNCTION(BlueprintCallable, meta = (DeterminesOutputType = "Target", UnsafeDuringActorConstruction = "true"), Category = LGUI)
		static AActor* DuplicateActorWithPreparedData(UPARAM(Ref) FLGUIDuplicateDataContainer& Data, USceneComponent* Parent);
	template<class T>
	static T* DuplicateActorT(T* Target, USceneComponent* Parent)
	{
		static_assert(TPointerIsConvertibleFromTo<T, const AActor>::Value, "'T' template parameter to DuplicateActor must be derived from AActor");
		return (T*)ULGUIBPLibrary::DuplicateActor(Target, Parent);
	}

	/**
	 * Find the first component in parent and up parent hierarchy with type.
	 * @param IncludeSelf	Include actor self.
	 * @param InStopNode	If parent is InStopNode then break the search chain. Can be null to ignore it.
	 */
	UFUNCTION(BlueprintPure, Category = LGUI, meta = (ComponentClass = "ActorComponent"), meta = (DeterminesOutputType = "ComponentClass"))
		static UActorComponent* GetComponentInParent(AActor* InActor, TSubclassOf<UActorComponent> ComponentClass, bool IncludeSelf = true, AActor* InStopNode = nullptr);
	/**
	 * Find all compoents in children with type.
	 * @param InActor Root actor to start from.
	 * @param ComponentClass The component type that need to search.
	 * @param IncludeSelf true- also search component at InActor.
	 * @param InExcludeNode If any child actor is included in this InExcludeNode, will skip that child actor and all it's children.
	 */
	UFUNCTION(BlueprintPure, Category = LGUI, meta = (ComponentClass = "ActorComponent"), meta = (DeterminesOutputType = "ComponentClass"))
		static TArray<UActorComponent*> GetComponentsInChildren(AActor* InActor, TSubclassOf<UActorComponent> ComponentClass, bool IncludeSelf, const TSet<AActor*>& InExcludeNode);
	/**
	 * Find the first component in children with type.
	 * @param InActor Root actor to start from.
	 * @param ComponentClass The component type that need to search.
	 * @param IncludeSelf true- also search component at InActor.
	 * @param InExcludeNode If any child actor is included in this InExcludeNode, will skip that child actor and all it's children.
	 */
	UFUNCTION(BlueprintPure, Category = LGUI, meta = (ComponentClass = "ActorComponent"), meta = (DeterminesOutputType = "ComponentClass"))
		static UActorComponent* GetComponentInChildren(AActor* InActor, TSubclassOf<UActorComponent> ComponentClass, bool IncludeSelf, const TSet<AActor*>& InExcludeNode);
private:
	static void CollectComponentsInChildrenRecursive(AActor* InActor, TSubclassOf<UActorComponent> ComponentClass, TArray<UActorComponent*>& InOutArray, const TSet<AActor*>& InExcludeNode);
	static UActorComponent* FindComponentInChildrenRecursive(AActor* InActor, TSubclassOf<UActorComponent> ComponentClass, const TSet<AActor*>& InExcludeNode);
public:
#pragma region EventDelegate
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIEventDelegateExecuteEmpty(const FLGUIEventDelegate& InEvent) { InEvent.FireEvent(); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIEventDelegateExecuteBool(const FLGUIEventDelegate& InEvent, const bool& InParameter) { InEvent.FireEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIEventDelegateExecuteFloat(const FLGUIEventDelegate& InEvent, const float& InParameter) { InEvent.FireEvent(InParameter); }
	//UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIEventDelegateExecuteDouble(const FLGUIEventDelegate& InEvent, const double& InParameter) { InEvent.FireEvent(InParameter); }
	//UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIEventDelegateExecuteInt8(const FLGUIEventDelegate& InEvent, const int8& InParameter) { InEvent.FireEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIEventDelegateExecuteUInt8(const FLGUIEventDelegate& InEvent, const uint8& InParameter) { InEvent.FireEvent(InParameter); }
	//UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIEventDelegateExecuteInt16(const FLGUIEventDelegate& InEvent, const int16& InParameter) { InEvent.FireEvent(InParameter); }
	//UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIEventDelegateExecuteUInt16(const FLGUIEventDelegate& InEvent, const uint16& InParameter) { InEvent.FireEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIEventDelegateExecuteInt32(const FLGUIEventDelegate& InEvent, const int32& InParameter) { InEvent.FireEvent(InParameter); }
	//UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIEventDelegateExecuteUInt32(const FLGUIEventDelegate& InEvent, const uint32& InParameter) { InEvent.FireEvent(InParameter); }
	//UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIEventDelegateExecuteInt64(const FLGUIEventDelegate& InEvent, const int64& InParameter) { InEvent.FireEvent(InParameter); }
	//UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIEventDelegateExecuteUInt64(const FLGUIEventDelegate& InEvent, const uint64& InParameter) { InEvent.FireEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIEventDelegateExecuteVector2(const FLGUIEventDelegate& InEvent, const FVector2D& InParameter) { InEvent.FireEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIEventDelegateExecuteVector3(const FLGUIEventDelegate& InEvent, const FVector& InParameter) { InEvent.FireEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIEventDelegateExecuteVector4(const FLGUIEventDelegate& InEvent, const FVector4& InParameter) { InEvent.FireEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIEventDelegateExecuteColor(const FLGUIEventDelegate& InEvent, const FColor& InParameter) { InEvent.FireEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIEventDelegateExecuteLinearColor(const FLGUIEventDelegate& InEvent, const FLinearColor& InParameter) { InEvent.FireEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIEventDelegateExecuteQuaternion(const FLGUIEventDelegate& InEvent, const FQuat& InParameter) { InEvent.FireEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIEventDelegateExecuteString(const FLGUIEventDelegate& InEvent, const FString& InParameter) { InEvent.FireEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIEventDelegateExecuteObject(const FLGUIEventDelegate& InEvent, UObject* InParameter) { InEvent.FireEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIEventDelegateExecuteActor(const FLGUIEventDelegate& InEvent, AActor* InParameter) { InEvent.FireEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIEventDelegateExecuteClass(const FLGUIEventDelegate& InEvent, UClass* InParameter) { InEvent.FireEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIEventDelegateExecutePointerEvent(const FLGUIEventDelegate& InEvent, ULGUIPointerEventData* InParameter) { InEvent.FireEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIEventDelegateExecuteRotator(const FLGUIEventDelegate& InEvent, const FRotator& InParameter) { InEvent.FireEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIEventDelegateExecuteText(const FLGUIEventDelegate& InEvent, const FText& InParameter) { InEvent.FireEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIEventDelegateExecuteName(const FLGUIEventDelegate& InEvent, const FName& InParameter) { InEvent.FireEvent(InParameter); }


	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LGUIEventDelegate_Empty_Execute(const FLGUIEventDelegate_Empty& InEvent) { InEvent(); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLGUIDelegateHandleWrapper LGUIEventDelegate_Empty_Register(const FLGUIEventDelegate_Empty& InEvent, FLGUIEventDelegate_Empty_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LGUIEventDelegate_Empty_Unregister(const FLGUIEventDelegate_Empty& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LGUIEventDelegate_Bool_Execute(const FLGUIEventDelegate_Bool& InEvent, bool InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLGUIDelegateHandleWrapper LGUIEventDelegate_Bool_Register(const FLGUIEventDelegate_Bool& InEvent, FLGUIEventDelegate_Bool_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LGUIEventDelegate_Bool_Unregister(const FLGUIEventDelegate_Bool& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LGUIEventDelegate_Float_Execute(const FLGUIEventDelegate_Float& InEvent, float InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLGUIDelegateHandleWrapper LGUIEventDelegate_Float_Register(const FLGUIEventDelegate_Float& InEvent, FLGUIEventDelegate_Float_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LGUIEventDelegate_Float_Unregister(const FLGUIEventDelegate_Float& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LGUIEventDelegate_Double_Execute(const FLGUIEventDelegate_Double& InEvent, double InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLGUIDelegateHandleWrapper LGUIEventDelegate_Double_Register(const FLGUIEventDelegate_Double& InEvent, FLGUIEventDelegate_Double_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LGUIEventDelegate_Double_Unregister(const FLGUIEventDelegate_Double& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	//UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
	//	static void LGUIEventDelegate_Int8_Execute(const FLGUIEventDelegate_Int8& InEvent, int8 InParameter) { InEvent(InParameter); }
	//UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
	//	static FLGUIDelegateHandleWrapper LGUIEventDelegate_Int8_Register(const FLGUIEventDelegate_Int8& InEvent, FLGUIEventDelegate_Int8_DynamicDelegate InDelegate);
	//UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
	//	static void LGUIEventDelegate_Int8_Unregister(const FLGUIEventDelegate_Int8& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LGUIEventDelegate_UInt8_Execute(const FLGUIEventDelegate_UInt8& InEvent, uint8 InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLGUIDelegateHandleWrapper LGUIEventDelegate_UInt8_Register(const FLGUIEventDelegate_UInt8& InEvent, FLGUIEventDelegate_UInt8_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LGUIEventDelegate_UInt8_Unregister(const FLGUIEventDelegate_UInt8& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	//UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
	//	static void LGUIEventDelegate_Int16_Execute(const FLGUIEventDelegate_Int16& InEvent, int16 InParameter) { InEvent(InParameter); }
	//UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
	//	static FLGUIDelegateHandleWrapper LGUIEventDelegate_Int16_Register(const FLGUIEventDelegate_Int16& InEvent, FLGUIEventDelegate_Int16_DynamicDelegate InDelegate);
	//UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
	//	static void LGUIEventDelegate_Int16_Unregister(const FLGUIEventDelegate_Int16& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	//UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
	//	static void LGUIEventDelegate_UInt16_Execute(const FLGUIEventDelegate_UInt16& InEvent, uint16 InParameter) { InEvent(InParameter); }
	//UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
	//	static FLGUIDelegateHandleWrapper LGUIEventDelegate_UInt16_Register(const FLGUIEventDelegate_UInt16& InEvent, FLGUIEventDelegate_UInt16_DynamicDelegate InDelegate);
	//UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
	//	static void LGUIEventDelegate_UInt16_Unregister(const FLGUIEventDelegate_UInt16& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LGUIEventDelegate_Int32_Execute(const FLGUIEventDelegate_Int32& InEvent, int32 InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLGUIDelegateHandleWrapper LGUIEventDelegate_Int32_Register(const FLGUIEventDelegate_Int32& InEvent, FLGUIEventDelegate_Int32_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LGUIEventDelegate_Int32_Unregister(const FLGUIEventDelegate_Int32& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	//UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
	//	static void LGUIEventDelegate_UInt32_Execute(const FLGUIEventDelegate_UInt32& InEvent, uint32 InParameter) { InEvent(InParameter); }
	//UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
	//	static FLGUIDelegateHandleWrapper LGUIEventDelegate_UInt32_Register(const FLGUIEventDelegate_UInt32& InEvent, FLGUIEventDelegate_UInt32_DynamicDelegate InDelegate);
	//UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
	//	static void LGUIEventDelegate_UInt32_Unregister(const FLGUIEventDelegate_UInt32& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LGUIEventDelegate_Int64_Execute(const FLGUIEventDelegate_Int64& InEvent, int64 InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLGUIDelegateHandleWrapper LGUIEventDelegate_Int64_Register(const FLGUIEventDelegate_Int64& InEvent, FLGUIEventDelegate_Int64_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LGUIEventDelegate_Int64_Unregister(const FLGUIEventDelegate_Int64& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	//UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
	//	static void LGUIEventDelegate_UInt64_Execute(const FLGUIEventDelegate_UInt64& InEvent, uint64 InParameter) { InEvent(InParameter); }
	//UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
	//	static FLGUIDelegateHandleWrapper LGUIEventDelegate_UInt64_Register(const FLGUIEventDelegate_UInt64& InEvent, FLGUIEventDelegate_UInt64_DynamicDelegate InDelegate);
	//UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
	//	static void LGUIEventDelegate_UInt64_Unregister(const FLGUIEventDelegate_UInt64& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LGUIEventDelegate_Vector2_Execute(const FLGUIEventDelegate_Vector2& InEvent, FVector2D InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLGUIDelegateHandleWrapper LGUIEventDelegate_Vector2_Register(const FLGUIEventDelegate_Vector2& InEvent, FLGUIEventDelegate_Vector2_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LGUIEventDelegate_Vector2_Unregister(const FLGUIEventDelegate_Vector2& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LGUIEventDelegate_Vector3_Execute(const FLGUIEventDelegate_Vector3& InEvent, FVector InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLGUIDelegateHandleWrapper LGUIEventDelegate_Vector3_Register(const FLGUIEventDelegate_Vector3& InEvent, FLGUIEventDelegate_Vector3_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LGUIEventDelegate_Vector3_Unregister(const FLGUIEventDelegate_Vector3& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LGUIEventDelegate_Vector4_Execute(const FLGUIEventDelegate_Vector4& InEvent, FVector4 InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLGUIDelegateHandleWrapper LGUIEventDelegate_Vector4_Register(const FLGUIEventDelegate_Vector4& InEvent, FLGUIEventDelegate_Vector4_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LGUIEventDelegate_Vector4_Unregister(const FLGUIEventDelegate_Vector4& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LGUIEventDelegate_Color_Execute(const FLGUIEventDelegate_Color& InEvent, FColor InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLGUIDelegateHandleWrapper LGUIEventDelegate_Color_Register(const FLGUIEventDelegate_Color& InEvent, FLGUIEventDelegate_Color_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LGUIEventDelegate_Color_Unregister(const FLGUIEventDelegate_Color& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LGUIEventDelegate_LinearColor_Execute(const FLGUIEventDelegate_LinearColor& InEvent, FLinearColor InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLGUIDelegateHandleWrapper LGUIEventDelegate_LinearColor_Register(const FLGUIEventDelegate_LinearColor& InEvent, FLGUIEventDelegate_LinearColor_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LGUIEventDelegate_LinearColor_Unregister(const FLGUIEventDelegate_LinearColor& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LGUIEventDelegate_Quaternion_Execute(const FLGUIEventDelegate_Quaternion& InEvent, FQuat InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLGUIDelegateHandleWrapper LGUIEventDelegate_Quaternion_Register(const FLGUIEventDelegate_Quaternion& InEvent, FLGUIEventDelegate_Quaternion_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LGUIEventDelegate_Quaternion_Unregister(const FLGUIEventDelegate_Quaternion& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LGUIEventDelegate_String_Execute(const FLGUIEventDelegate_String& InEvent, FString InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLGUIDelegateHandleWrapper LGUIEventDelegate_String_Register(const FLGUIEventDelegate_String& InEvent, FLGUIEventDelegate_String_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LGUIEventDelegate_String_Unregister(const FLGUIEventDelegate_String& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LGUIEventDelegate_Object_Execute(const FLGUIEventDelegate_Object& InEvent, UObject* InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLGUIDelegateHandleWrapper LGUIEventDelegate_Object_Register(const FLGUIEventDelegate_Object& InEvent, FLGUIEventDelegate_Object_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LGUIEventDelegate_Object_Unregister(const FLGUIEventDelegate_Object& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LGUIEventDelegate_Actor_Execute(const FLGUIEventDelegate_Actor& InEvent, AActor* InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLGUIDelegateHandleWrapper LGUIEventDelegate_Actor_Register(const FLGUIEventDelegate_Actor& InEvent, FLGUIEventDelegate_Actor_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LGUIEventDelegate_Actor_Unregister(const FLGUIEventDelegate_Actor& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LGUIEventDelegate_PointerEvent_Execute(const FLGUIEventDelegate_PointerEvent& InEvent, ULGUIPointerEventData* InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLGUIDelegateHandleWrapper LGUIEventDelegate_PointerEvent_Register(const FLGUIEventDelegate_PointerEvent& InEvent, FLGUIEventDelegate_PointerEvent_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LGUIEventDelegate_PointerEvent_Unregister(const FLGUIEventDelegate_PointerEvent& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LGUIEventDelegate_Class_Execute(const FLGUIEventDelegate_Class& InEvent, UClass* InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLGUIDelegateHandleWrapper LGUIEventDelegate_Class_Register(const FLGUIEventDelegate_Class& InEvent, FLGUIEventDelegate_Class_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LGUIEventDelegate_Class_Unregister(const FLGUIEventDelegate_Class& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LGUIEventDelegate_Rotator_Execute(const FLGUIEventDelegate_Rotator& InEvent, FRotator InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLGUIDelegateHandleWrapper LGUIEventDelegate_Rotator_Register(const FLGUIEventDelegate_Rotator& InEvent, FLGUIEventDelegate_Rotator_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LGUIEventDelegate_Rotator_Unregister(const FLGUIEventDelegate_Rotator& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LGUIEventDelegate_Text_Execute(const FLGUIEventDelegate_Text& InEvent, FText InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLGUIDelegateHandleWrapper LGUIEventDelegate_Text_Register(const FLGUIEventDelegate_Text& InEvent, FLGUIEventDelegate_Text_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LGUIEventDelegate_Text_Unregister(const FLGUIEventDelegate_Text& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Execute"))
		static void LGUIEventDelegate_Name_Execute(const FLGUIEventDelegate_Name& InEvent, FName InParameter) { InEvent(InParameter); }
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Register"))
		static FLGUIDelegateHandleWrapper LGUIEventDelegate_Name_Register(const FLGUIEventDelegate_Name& InEvent, FLGUIEventDelegate_Name_DynamicDelegate InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Unregister"))
		static void LGUIEventDelegate_Name_Unregister(const FLGUIEventDelegate_Name& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle);
#pragma endregion EventDelegate

	/** InComponentType must be the same as InLGUIComponentReference's component type */
	UFUNCTION(BlueprintPure, Category = LGUI, meta = (DisplayName = "Get Component", CompactNodeTitle = "Component", BlueprintAutocast, DeterminesOutputType = "InComponentType"))
		static UActorComponent* LGUICompRef_GetComponent(const FLGUIComponentReference& InLGUIComponentReference, TSubclassOf<UActorComponent> InComponentType);
	UFUNCTION(BlueprintPure, Category = LGUI, meta = (DisplayName = "Get Actor", CompactNodeTitle = "Actor", BlueprintAutocast))
		static AActor* LGUICompRef_GetActor(const FLGUIComponentReference& InLGUIComponentReference);

	UFUNCTION(BlueprintPure, Category = LGUI, meta = (DisplayName = "Get", CompactNodeTitle = ".", BlueprintInternalUseOnly = "true"))
		static void K2_LGUICompRef_GetComponent(const FLGUIComponentReference& InLGUICompRef, UActorComponent*& OutResult);

#pragma region LTween

#pragma region UIItem
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		static ULTweener* WidthTo(UUIItem* target, float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		static ULTweener* HeightTo(UUIItem* target, float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);

	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		static ULTweener* ColorTo(UUIBaseRenderable* target, FColor endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		static ULTweener* ColorFrom(UUIBaseRenderable* target, FColor startValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);

	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		static ULTweener* AlphaTo(UUIBaseRenderable* target, float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		static ULTweener* AlphaFrom(UUIBaseRenderable* target, float startValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);

	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		static ULTweener* HorizontalAnchoredPositionTo(UUIItem* target, float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		static ULTweener* VerticalAnchoredPositionTo(UUIItem* target, float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		static ULTweener* AnchoredPositionTo(UUIItem* target, FVector2D endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		static ULTweener* PivotTo(UUIItem* target, FVector2D endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		static ULTweener* AnchorLeftTo(UUIItem* target, float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		static ULTweener* AnchorRightTo(UUIItem* target, float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		static ULTweener* AnchorTopTo(UUIItem* target, float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		static ULTweener* AnchorBottomTo(UUIItem* target, float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
#pragma endregion

	UFUNCTION(BlueprintCallable, Category = LGUI)
		static void LGUIExecuteControllerInputAxis(FKey inputKey, float value);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		static void LGUIExecuteControllerInputAction(FKey inputKey, bool pressOrRelease);

#pragma endregion
	UFUNCTION(BlueprintPure, Category = LGUI)
		static void GetSpriteSize(const FLGUISpriteInfo& SpriteInfo, int32& width, int32& height);
	UFUNCTION(BlueprintPure, Category = LGUI)
		static void GetSpriteBorderSize(const FLGUISpriteInfo& SpriteInfo, int32& borderLeft, int32& borderRight, int32& borderTop, int32& borderBottom);
	UFUNCTION(BlueprintPure, Category = LGUI)
		static void GetSpriteUV(const FLGUISpriteInfo& SpriteInfo, float& UV0X, float& UV0Y, float& UV3X, float& UV3Y);
	UFUNCTION(BlueprintPure, Category = LGUI)
		static void GetSpriteBorderUV(const FLGUISpriteInfo& SpriteInfo, float& borderUV0X, float& borderUV0Y, float& borderUV3X, float& borderUV3Y);



	UE_DEPRECATED(4.23, "Use LTween's LocalPositionTo instead.")
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "UI Local Position To", DeprecatedFunction, DeprecationMessage = "Use LTween's LocalPositionTo instead."), Category = "LTweenLGUI")
		static ULTweener* UILocalPositionTo(UUIItem* target, FVector endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UE_DEPRECATED(4.24, "This node is not valid any more. Use LGUIExecuteControllerInputAxis instead.")
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DeprecatedFunction, DeprecationMessage = "This node is not valid any more. Use LGUIExecuteControllerInputAxis instead"))
		static void LGUIExecuteInputAxis(FKey inputKey, float value) 
	{ 
		LGUIExecuteControllerInputAxis(inputKey, value);
	}
	UE_DEPRECATED(4.24, "This node is not valid any more. Use LGUIExecuteControllerInputAction instead.")
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DeprecatedFunction, DeprecationMessage = "This node is not valid any more. Use LGUIExecuteControllerInputAction instead"))
		static void LGUIExecuteInputAction(FKey inputKey, bool pressOrRelease)
	{
		LGUIExecuteControllerInputAction(inputKey, pressOrRelease);
	}
	UE_DEPRECATED(4.24, "Use HorizontalAnchoredPositionTo instead.")
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "Anchor Offset X To", DeprecatedFunction, DeprecationMessage = "Use HorizontalAnchoredPositionTo instead."), Category = "LTweenLGUI")
		static ULTweener* AnchorOffsetXTo(UUIItem* target, float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UE_DEPRECATED(4.24, "Use VerticalAnchoredPositionTo instead.")
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "Anchor Offset Y To", DeprecatedFunction, DeprecationMessage = "Use VerticalAnchoredPositionTo instead."), Category = "LTweenLGUI")
		static ULTweener* AnchorOffsetYTo(UUIItem* target, float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UE_DEPRECATED(4.24, "Use AnchoredPositionTo instead.")
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "Anchor Offset To", DeprecatedFunction, DeprecationMessage = "Use AnchoredPositionTo instead."), Category = "LTweenLGUI")
		static ULTweener* AnchorOffsetTo(UUIItem* target, FVector2D endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UE_DEPRECATED(4.24, "Use AnchorLeftTo instead.")
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DeprecatedFunction, DeprecationMessage = "Use AnchorLeftTo instead."), Category = "LTweenLGUI")
		static ULTweener* StretchLeftTo(UUIItem* target, float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UE_DEPRECATED(4.24, "Use AnchorRightTo instead.")
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DeprecatedFunction, DeprecationMessage = "Use AnchorRightTo instead."), Category = "LTweenLGUI")
		static ULTweener* StretchRightTo(UUIItem* target, float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UE_DEPRECATED(4.24, "Use AnchorTopTo instead.")
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DeprecatedFunction, DeprecationMessage = "Use AnchorTopTo instead."), Category = "LTweenLGUI")
		static ULTweener* StretchTopTo(UUIItem* target, float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UE_DEPRECATED(4.24, "Use AnchorBottomTo instead.")
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DeprecatedFunction, DeprecationMessage = "Use AnchorBottomTo instead."), Category = "LTweenLGUI")
		static ULTweener* StretchBottomTo(UUIItem* target, float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);

	UE_DEPRECATED(4.24, "LGUIComponentReference now can direct reference ActorComponent, so no need to store component class.")
	UFUNCTION(BlueprintPure, Category = LGUI, meta = (DisplayName = "Get Component Class", CompactNodeTitle = "Class", BlueprintAutocast, DeprecatedFunction, DeprecationMessage = "LGUIComponentReference now can direct reference ActorComponent, so no need to store component class."))
		static TSubclassOf<UActorComponent> LGUICompRef_GetComponentClass(const FLGUIComponentReference& InLGUIComponentReference);
};
