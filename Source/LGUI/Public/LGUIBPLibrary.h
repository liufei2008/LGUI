// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIItem.h"
#include "Event/LGUIDrawableEvent.h"
#include "Event/LGUIDelegateHandleWrapper.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LGUIEditHelper.h"
#include "LGUIBPLibrary.generated.h"

UCLASS()
class LGUI_API ULGUIBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
#pragma region QuickEntry
	//Set alpha if root component is a UIItem component
	UFUNCTION(BlueprintCallable, Category = LGUI)
		static void SetUIAlpha(AActor* Target, float InAlpha)
	{
		if (!IsValid(Target))
		{
			UE_LOG(LGUI, Error, TEXT("[ULGUIBPLibrary::SetAlpha]Target is not valid!"));
			return;
		}
		if (auto uiItem = Cast<UUIItem>(Target->GetRootComponent()))
		{
			uiItem->SetAlpha(InAlpha);
		}
	}
	//Set UIActive if root component is a UIItem component
	UFUNCTION(BlueprintCallable, Category = LGUI)
		static void SetUIActive(AActor* Target, bool Acitve)
	{
		if (!IsValid(Target))
		{
			UE_LOG(LGUI, Error, TEXT("[ULGUIBPLibrary::SetUIActive]Target is not valid!"));
			return;
		}
		if (auto uiItem = Cast<UUIItem>(Target->GetRootComponent()))
		{
			uiItem->SetUIActive(Acitve);
		}
	}
	//Set HierarchyIndex if root component is a UIItem component
	UFUNCTION(BlueprintCallable, Category = LGUI)
		static void SetUIHierarchyIndex(AActor* Target, int32 index)
	{
		if (!IsValid(Target))
		{
			UE_LOG(LGUI, Error, TEXT("[ULGUIBPLibrary::SetHierarchyIndex]Target is not valid!"));
			return;
		}
		if (auto uiItem = Cast<UUIItem>(Target->GetRootComponent()))
		{
			uiItem->SetHierarchyIndex(index);
		}
	}
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
	UFUNCTION(BlueprintCallable, Category = LGUI)static void LGUIDrawableEventExecutePointerEvent(const FLGUIDrawableEvent& InEvent, const FLGUIPointerEventData& InParameter) { InEvent.FireEvent(InParameter); }
#pragma endregion DrawableEvent

	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToString (LGUIPointerEventData)", CompactNodeTitle = ".", BlueprintAutocast), Category = "LGUI")
		static FString Conv_LGUIPointerEventDataToString(const FLGUIPointerEventData& InData);
	//use a line-plane intersection to get world point
	UFUNCTION(BlueprintPure, Category = LGUI)static FVector GetWorldPointInPlane(const FLGUIPointerEventData& InData);
	//use a line-plane intersection to get world point, and convert to press component's local space
	UFUNCTION(BlueprintPure, Category = LGUI)static FVector GetLocalPointInPlane(const FLGUIPointerEventData& InData);

	//InComponentType must be the same as InLGUIComponentReference's component type
	UFUNCTION(BlueprintPure, Category = LGUI, meta = (DisplayName = "GetComponent", CompactNodeTitle = ".", BlueprintAutocast, DeterminesOutputType = "InComponentType"))
		static UActorComponent* LGUICompRef_GetComponent(const FLGUIComponentReference& InLGUIComponentReference, TSubclassOf<UActorComponent> InComponentType);
	UFUNCTION(BlueprintPure, Category = LGUI, meta = (DisplayName = "GetComponentClass", CompactNodeTitle = "Class", BlueprintAutocast))
		static TSubclassOf<UActorComponent> LGUICompRef_GetComponentClass(const FLGUIComponentReference& InLGUIComponentReference);
	UFUNCTION(BlueprintPure, Category = LGUI, meta = (DisplayName = "GetActor", CompactNodeTitle = "Actor", BlueprintAutocast))
		static AActor* LGUICompRef_GetActor(const FLGUIComponentReference& InLGUIComponentReference);
};
