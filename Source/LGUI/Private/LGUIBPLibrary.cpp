// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LGUIBPLibrary.h"
#include "Utils/LGUIUtils.h"
#include "LTweenManager.h"
#include "LTweenBPLibrary.h"
#include "Core/ActorComponent/UIItem.h"
#include "Core/ActorComponent/UIBaseRenderable.h"
#include "Framework/Application/SlateApplication.h"
#include "LGUI.h"
#include "PrefabSystem/LGUIPrefab.h"
#include LGUIPREFAB_SERIALIZER_NEWEST_INCLUDE

void ULGUIBPLibrary::DestroyActorWithHierarchy(AActor* Target, bool WithHierarchy)
{
	LGUIUtils::DestroyActorWithHierarchy(Target, WithHierarchy);
}
AActor* ULGUIBPLibrary::LoadPrefab(UObject* WorldContextObject, ULGUIPrefab* InPrefab, USceneComponent* InParent, const FLGUIPrefab_LoadPrefabCallback& InCallbackBeforeAwake, bool SetRelativeTransformToIdentity)
{
	if (!IsValid(InPrefab))
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d InPrefab not valid"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		return nullptr;
	}
	return InPrefab->LoadPrefab(WorldContextObject, InParent, InCallbackBeforeAwake, SetRelativeTransformToIdentity);
}
AActor* ULGUIBPLibrary::LoadPrefabWithTransform(UObject* WorldContextObject, ULGUIPrefab* InPrefab, USceneComponent* InParent, FVector Location, FRotator Rotation, FVector Scale, const FLGUIPrefab_LoadPrefabCallback& InCallbackBeforeAwake)
{
	if (!IsValid(InPrefab))
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d InPrefab not valid"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		return nullptr;
	}
	return InPrefab->LoadPrefabWithTransform(WorldContextObject, InParent, Location, Rotation, Scale, InCallbackBeforeAwake);
}
AActor* ULGUIBPLibrary::LoadPrefabWithTransform(UObject* WorldContextObject, ULGUIPrefab* InPrefab, USceneComponent* InParent, FVector Location, FQuat Rotation, FVector Scale, const TFunction<void(AActor*)>& InCallbackBeforeAwake)
{
	if (!IsValid(InPrefab))
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d InPrefab not valid"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		return nullptr;
	}
	return InPrefab->LoadPrefabWithTransform(WorldContextObject, InParent, Location, Rotation, Scale, InCallbackBeforeAwake);
}
AActor* ULGUIBPLibrary::LoadPrefabWithReplacement(UObject* WorldContextObject, ULGUIPrefab* InPrefab, USceneComponent* InParent, const TMap<UObject*, UObject*>& InReplaceAssetMap, const TMap<UClass*, UClass*>& InReplaceClassMap, const FLGUIPrefab_LoadPrefabCallback& InCallbackBeforeAwake)
{
	if (!IsValid(InPrefab))
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d InPrefab not valid"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		return nullptr;
	}
	return InPrefab->LoadPrefabWithReplacement(WorldContextObject, InParent, InReplaceAssetMap, InReplaceClassMap, InCallbackBeforeAwake);
}

AActor* ULGUIBPLibrary::DuplicateActor(AActor* Target, USceneComponent* Parent)
{
	return LGUIPREFAB_SERIALIZER_NEWEST_NAMESPACE::ActorSerializer::DuplicateActor(Target, Parent);
}
void ULGUIBPLibrary::PrepareDuplicateData(AActor* Target, FLGUIDuplicateDataContainer& DataContainer)
{
	DataContainer.bIsValid = LGUIPREFAB_SERIALIZER_NEWEST_NAMESPACE::ActorSerializer::PrepareDataForDuplicate(Target, DataContainer.DuplicateData);
}
AActor* ULGUIBPLibrary::DuplicateActorWithPreparedData(FLGUIDuplicateDataContainer& Data, USceneComponent* Parent)
{
	if (Data.bIsValid)
	{
		return LGUIPREFAB_SERIALIZER_NEWEST_NAMESPACE::ActorSerializer::DuplicateActorWithPreparedData(Data.DuplicateData, Parent);
	}
	else
	{
		return nullptr;
	}
}

UActorComponent* ULGUIBPLibrary::GetComponentInParent(AActor* InActor, TSubclassOf<UActorComponent> ComponentClass, bool IncludeSelf, AActor* InStopNode)
{
	if (!IsValid(InActor))
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIBPLibrary::GetComponentInParent]InActor is not valid!"));
		return nullptr;
	}
	AActor* parentActor = IncludeSelf ? InActor : InActor->GetAttachParentActor();
	while (parentActor != nullptr)
	{
		if (InStopNode != nullptr)
		{
			if (parentActor == InStopNode)return nullptr;
		}
		auto resultComp = parentActor->FindComponentByClass(ComponentClass);
		if (resultComp != nullptr)
		{
			return resultComp;
		}
		else
		{
			parentActor = parentActor->GetAttachParentActor();
		}
	}
	return nullptr;
}
TArray<UActorComponent*> ULGUIBPLibrary::GetComponentsInChildren(AActor* InActor, TSubclassOf<UActorComponent> ComponentClass, bool IncludeSelf, const TSet<AActor*>& InExcludeNode)
{
	TArray<UActorComponent*> result;
	if (!IsValid(InActor))
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIBPLibrary::GetComponentInParent]InActor is not valid!"));
		return result;
	}

	struct LOCAL
	{
		static void CollectComponentsInChildrenRecursive(AActor* InActor, TSubclassOf<UActorComponent> ComponentClass, TArray<UActorComponent*>& InOutArray, const TSet<AActor*>& InExcludeNode)
		{
			if (InExcludeNode.Contains(InActor))return;
			auto& components = InActor->GetComponents();
			for (UActorComponent* comp : components)
			{
				if (IsValid(comp) && comp->IsA(ComponentClass))
				{
					InOutArray.Add(comp);
				}
			}

			TArray<AActor*> childrenActors;
			InActor->GetAttachedActors(childrenActors);
			if (childrenActors.Num() > 0)
			{
				for (AActor* actor : childrenActors)
				{
					CollectComponentsInChildrenRecursive(actor, ComponentClass, InOutArray, InExcludeNode);
				}
			}
		}
	};
	if (IncludeSelf)
	{
		LOCAL::CollectComponentsInChildrenRecursive(InActor, ComponentClass, result, InExcludeNode);
	}
	else
	{
		TArray<AActor*> childrenActors;
		InActor->GetAttachedActors(childrenActors);
		if (childrenActors.Num() > 0)
		{
			for (AActor* actor : childrenActors)
			{
				LOCAL::CollectComponentsInChildrenRecursive(actor, ComponentClass, result, InExcludeNode);
			}
		}
	}
	return result;
}

UActorComponent* ULGUIBPLibrary::GetComponentInChildren(AActor* InActor, TSubclassOf<UActorComponent> ComponentClass, bool IncludeSelf, const TSet<AActor*>& InExcludeNode)
{
	if (!IsValid(InActor))
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIBPLibrary::GetComponentInChildren]InActor is not valid!"));
		return nullptr;
	}

	struct LOCAL
	{
		static UActorComponent* FindComponentInChildrenRecursive(AActor* InActor, TSubclassOf<UActorComponent> ComponentClass, const TSet<AActor*>& InExcludeNode)
		{
			if (InExcludeNode.Contains(InActor))return nullptr;
			if (auto comp = InActor->GetComponentByClass(ComponentClass))
			{
				if (IsValid(comp))
				{
					return comp;
				}
			}
			TArray<AActor*> childrenActors;
			InActor->GetAttachedActors(childrenActors);
			for (auto childActor : childrenActors)
			{
				auto comp = FindComponentInChildrenRecursive(childActor, ComponentClass, InExcludeNode);
				if (IsValid(comp))
				{
					return comp;
				}
			}
			return nullptr;
		}
	};

	UActorComponent* result = nullptr;
	if (IncludeSelf)
	{
		result = LOCAL::FindComponentInChildrenRecursive(InActor, ComponentClass, InExcludeNode);
	}
	else
	{
		TArray<AActor*> childrenActors;
		InActor->GetAttachedActors(childrenActors);
		if (childrenActors.Num() > 0)
		{
			for (AActor* actor : childrenActors)
			{
				result = LOCAL::FindComponentInChildrenRecursive(actor, ComponentClass, InExcludeNode);
				if (IsValid(result))
				{
					return result;
				}
			}
		}
	}
	return result;
}

UActorComponent* ULGUIBPLibrary::LGUICompRef_GetComponent(const FLGUIComponentReference& InLGUIComponentReference, TSubclassOf<UActorComponent> InComponentType)
{
	auto comp = InLGUIComponentReference.GetComponent();
	if (comp == nullptr)
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIBPLibrary::GetComponent]Target actor:%s dont have this kind of component:%s"), *(InLGUIComponentReference.GetActor()->GetPathName()), *(InComponentType->GetPathName()));
		return nullptr;
	}
	if (comp->GetClass() != InComponentType)
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIBPLibrary::GetComponent]InComponentType must be the same as InLGUIComponentReference's component type!"));
		return nullptr;
	}
	return comp;
}

AActor* ULGUIBPLibrary::LGUICompRef_GetActor(const FLGUIComponentReference& InLGUIComponentReference)
{
	return InLGUIComponentReference.GetActor();
}

void ULGUIBPLibrary::K2_LGUICompRef_GetComponent(const FLGUIComponentReference& InLGUICompRef, UActorComponent*& OutResult)
{
	OutResult = InLGUICompRef.GetComponent();
}


#pragma region LTween

#pragma region UIItem
ULTweener* ULGUIBPLibrary::WidthTo(UUIItem* target, float endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::WidthTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return target->WidthTo(endValue, duration, delay, ease);
}
ULTweener* ULGUIBPLibrary::HeightTo(UUIItem* target, float endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::HeightTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return target->HeightTo(endValue, duration, delay, ease);
}

ULTweener* ULGUIBPLibrary::ColorTo(UUIBaseRenderable* target, FColor endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::ColorTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return target->ColorTo(endValue, duration, delay, ease);
}
ULTweener* ULGUIBPLibrary::ColorFrom(UUIBaseRenderable* target, FColor startValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::ColorFrom target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto endValue = target->GetColor();
	target->SetColor(startValue);
	return target->ColorTo(endValue, duration, delay, ease);
}

ULTweener* ULGUIBPLibrary::AlphaTo(UUIBaseRenderable* target, float endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::AlphaTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return target->AlphaTo(endValue, duration, delay, ease);
}
ULTweener* ULGUIBPLibrary::AlphaFrom(UUIBaseRenderable* target, float startValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::AlphaFrom target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto endValue = target->GetAlpha();
	target->SetAlpha(startValue);
	return target->AlphaTo(endValue, duration, delay, ease);
}

ULTweener* ULGUIBPLibrary::HorizontalAnchoredPositionTo(UUIItem* target, float endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::HorizontalAnchoredPositionTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return target->HorizontalAnchoredPositionTo(endValue, duration, delay, ease);
}
ULTweener* ULGUIBPLibrary::VerticalAnchoredPositionTo(UUIItem* target, float endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::VerticalAnchoredPositionTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return target->VerticalAnchoredPositionTo(endValue, duration, delay, ease);
}
ULTweener* ULGUIBPLibrary::AnchoredPositionTo(UUIItem* target, FVector2D endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::AnchoredPositionTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return target->AnchoredPositionTo(endValue, duration, delay, ease);
}
ULTweener* ULGUIBPLibrary::PivotTo(UUIItem* target, FVector2D endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::PivotTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return target->PivotTo(endValue, duration, delay, ease);
}

ULTweener* ULGUIBPLibrary::AnchorLeftTo(UUIItem* target, float endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::AnchorLeftTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return target->AnchorLeftTo(endValue, duration, delay, ease);
}
ULTweener* ULGUIBPLibrary::AnchorRightTo(UUIItem* target, float endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::AnchorRightTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return target->AnchorRightTo(endValue, duration, delay, ease);
}
ULTweener* ULGUIBPLibrary::AnchorTopTo(UUIItem* target, float endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::AnchorTopTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return target->AnchorTopTo(endValue, duration, delay, ease);
}
ULTweener* ULGUIBPLibrary::AnchorBottomTo(UUIItem* target, float endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::AnchorBottomTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return target->AnchorBottomTo(endValue, duration, delay, ease);
}
#pragma endregion


void ULGUIBPLibrary::LGUIExecuteControllerInputAxis(FKey inputKey, float value)
{
	if (inputKey.IsValid())
	{
		FPlatformUserId UserId = IPlatformInputDeviceMapper::Get().GetPrimaryPlatformUser();
		FInputDeviceId DeviceId = IPlatformInputDeviceMapper::Get().GetDefaultInputDevice();
		const FGamepadKeyNames::Type keyName = inputKey.GetFName();
		FSlateApplication::Get().OnControllerAnalog(keyName, UserId, DeviceId, value);
	}
}
void ULGUIBPLibrary::LGUIExecuteControllerInputAction(FKey inputKey, bool pressOrRelease)
{
	if (inputKey.IsValid())
	{
		FPlatformUserId UserId = IPlatformInputDeviceMapper::Get().GetPrimaryPlatformUser();
		FInputDeviceId DeviceId = IPlatformInputDeviceMapper::Get().GetDefaultInputDevice();
		const FGamepadKeyNames::Type keyName = inputKey.GetFName();
		if (pressOrRelease)
		{
			FSlateApplication::Get().OnControllerButtonPressed(keyName, UserId, DeviceId, false);
		}
		else
		{
			FSlateApplication::Get().OnControllerButtonReleased(keyName, UserId, DeviceId, false);
		}
	}
}
#pragma endregion

#pragma region EventDelegate
#define IMPLEMENT_EVENTDELEGATE_BP(EventDelegateParamType, ParamType)\
FLGUIDelegateHandleWrapper ULGUIBPLibrary::LGUIEventDelegate_##EventDelegateParamType##_Register(const FLGUIEventDelegate_##EventDelegateParamType& InEvent, FLGUIEventDelegate_##EventDelegateParamType##_DynamicDelegate InDelegate)\
{\
	auto delegateHandle = InEvent.Register([InDelegate](ParamType value) {\
		if (InDelegate.IsBound())\
		{\
			InDelegate.Execute(value);\
		}\
		});\
	return FLGUIDelegateHandleWrapper(delegateHandle);\
}\
void ULGUIBPLibrary::LGUIEventDelegate_##EventDelegateParamType##_Unregister(const FLGUIEventDelegate_##EventDelegateParamType& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle)\
{\
	InEvent.Unregister(InDelegateHandle.DelegateHandle);\
}

FLGUIDelegateHandleWrapper ULGUIBPLibrary::LGUIEventDelegate_Empty_Register(const FLGUIEventDelegate_Empty& InEvent, FLGUIEventDelegate_Empty_DynamicDelegate InDelegate)
{
	auto delegateHandle = InEvent.Register([InDelegate]() {
		if (InDelegate.IsBound())
		{
			InDelegate.Execute();
		}
		});
	return FLGUIDelegateHandleWrapper(delegateHandle);
}
void ULGUIBPLibrary::LGUIEventDelegate_Empty_Unregister(const FLGUIEventDelegate_Empty& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle)
{
	InEvent.Unregister(InDelegateHandle.DelegateHandle);
}

IMPLEMENT_EVENTDELEGATE_BP(Bool, bool);
IMPLEMENT_EVENTDELEGATE_BP(Float, float);
IMPLEMENT_EVENTDELEGATE_BP(Double, double);
//IMPLEMENT_EVENTDELEGATE_BP(Int8, int8);
IMPLEMENT_EVENTDELEGATE_BP(UInt8, uint8);
//IMPLEMENT_EVENTDELEGATE_BP(Int16, int16);
//IMPLEMENT_EVENTDELEGATE_BP(UInt16, uint16);
IMPLEMENT_EVENTDELEGATE_BP(Int32, int32);
//IMPLEMENT_EVENTDELEGATE_BP(UInt32, uint32);
IMPLEMENT_EVENTDELEGATE_BP(Int64, int64);
//IMPLEMENT_EVENTDELEGATE_BP(UInt64, uint64);
IMPLEMENT_EVENTDELEGATE_BP(Vector2, FVector2D);
IMPLEMENT_EVENTDELEGATE_BP(Vector3, FVector);
IMPLEMENT_EVENTDELEGATE_BP(Vector4, FVector4);
IMPLEMENT_EVENTDELEGATE_BP(Color, FColor);
IMPLEMENT_EVENTDELEGATE_BP(LinearColor, FLinearColor);
IMPLEMENT_EVENTDELEGATE_BP(Quaternion, FQuat);
IMPLEMENT_EVENTDELEGATE_BP(String, FString);
IMPLEMENT_EVENTDELEGATE_BP(Object, UObject*);
IMPLEMENT_EVENTDELEGATE_BP(Actor, AActor*);
IMPLEMENT_EVENTDELEGATE_BP(PointerEvent, ULGUIPointerEventData*);
IMPLEMENT_EVENTDELEGATE_BP(Class, UClass*);
IMPLEMENT_EVENTDELEGATE_BP(Rotator, FRotator);
IMPLEMENT_EVENTDELEGATE_BP(Text, FText);
IMPLEMENT_EVENTDELEGATE_BP(Name, FName);

#pragma endregion

void ULGUIBPLibrary::GetSpriteSize(const FLGUISpriteInfo& SpriteInfo, int32& width, int32& height)
{
	width = SpriteInfo.width;
	height = SpriteInfo.height;
}
void ULGUIBPLibrary::GetSpriteBorderSize(const FLGUISpriteInfo& SpriteInfo, int32& borderLeft, int32& borderRight, int32& borderTop, int32& borderBottom)
{
	borderLeft = SpriteInfo.borderLeft;
	borderRight = SpriteInfo.borderRight;
	borderTop = SpriteInfo.borderTop;
	borderBottom = SpriteInfo.borderBottom;
}
void ULGUIBPLibrary::GetSpriteUV(const FLGUISpriteInfo& SpriteInfo, float& UV0X, float& UV0Y, float& UV3X, float& UV3Y)
{
	UV0X = SpriteInfo.uv0X;
	UV0Y = SpriteInfo.uv0Y;
	UV3X = SpriteInfo.uv3X;
	UV3Y = SpriteInfo.uv3Y;
}
void ULGUIBPLibrary::GetSpriteBorderUV(const FLGUISpriteInfo& SpriteInfo, float& borderUV0X, float& borderUV0Y, float& borderUV3X, float& borderUV3Y)
{
	borderUV0X = SpriteInfo.uv0X;
	borderUV0Y = SpriteInfo.uv0Y;
	borderUV3X = SpriteInfo.uv3X;
	borderUV3Y = SpriteInfo.uv3Y;
}


ULTweener* ULGUIBPLibrary::UILocalPositionTo(UUIItem* target, FVector endValue, float duration, float delay, ELTweenEase ease)
{
	return ULTweenBPLibrary::LocalPositionTo(target, endValue, duration, delay, ease);
}

ULTweener* ULGUIBPLibrary::AnchorOffsetXTo(UUIItem* target, float endValue, float duration, float delay, ELTweenEase ease)
{
	// Disable deprecation warnings so we can call the deprecated function to support this function (which is also deprecated)
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	auto tweener = HorizontalAnchoredPositionTo(target, endValue, duration, delay, ease);
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
	return tweener;
}
ULTweener* ULGUIBPLibrary::AnchorOffsetYTo(UUIItem* target, float endValue, float duration, float delay, ELTweenEase ease)
{
	// Disable deprecation warnings so we can call the deprecated function to support this function (which is also deprecated)
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	auto tweener =  VerticalAnchoredPositionTo(target, endValue, duration, delay, ease);
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
	return tweener;
}
ULTweener* ULGUIBPLibrary::AnchorOffsetTo(UUIItem* target, FVector2D endValue, float duration, float delay, ELTweenEase ease)
{
	// Disable deprecation warnings so we can call the deprecated function to support this function (which is also deprecated)
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	auto tweener =  AnchoredPositionTo(target, endValue, duration, delay, ease);
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
	return tweener;
}
ULTweener* ULGUIBPLibrary::StretchLeftTo(UUIItem* target, float endValue, float duration, float delay, ELTweenEase ease)
{
	// Disable deprecation warnings so we can call the deprecated function to support this function (which is also deprecated)
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	auto tweener =  AnchorLeftTo(target, endValue, duration, delay, ease);
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
	return tweener;
}
ULTweener* ULGUIBPLibrary::StretchRightTo(UUIItem* target, float endValue, float duration, float delay, ELTweenEase ease)
{
	// Disable deprecation warnings so we can call the deprecated function to support this function (which is also deprecated)
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	auto tweener =  AnchorRightTo(target, endValue, duration, delay, ease);
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
	return tweener;
}
ULTweener* ULGUIBPLibrary::StretchTopTo(UUIItem* target, float endValue, float duration, float delay, ELTweenEase ease)
{
	// Disable deprecation warnings so we can call the deprecated function to support this function (which is also deprecated)
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	auto tweener =  AnchorTopTo(target, endValue, duration, delay, ease);
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
	return tweener;
}
ULTweener* ULGUIBPLibrary::StretchBottomTo(UUIItem* target, float endValue, float duration, float delay, ELTweenEase ease)
{
	// Disable deprecation warnings so we can call the deprecated function to support this function (which is also deprecated)
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	auto tweener =  AnchorBottomTo(target, endValue, duration, delay, ease);
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
	return tweener;
}
TSubclassOf<UActorComponent> ULGUIBPLibrary::LGUICompRef_GetComponentClass(const FLGUIComponentReference& InLGUIComponentReference)
{
	auto comp = InLGUIComponentReference.GetComponent();
	if (comp == nullptr)
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIBPLibrary::GetComponentClass]Missing component on target actor:%s"), *(InLGUIComponentReference.GetActor()->GetPathName()));
		return nullptr;
	}
	return comp->GetClass();
}
