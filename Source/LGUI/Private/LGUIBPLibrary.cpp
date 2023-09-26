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
AActor* ULGUIBPLibrary::LoadPrefab(UObject* WorldContextObject, ULGUIPrefab* InPrefab, USceneComponent* InParent, bool SetRelativeTransformToIdentity)
{
	return InPrefab->LoadPrefab(WorldContextObject, InParent, SetRelativeTransformToIdentity);
}
AActor* ULGUIBPLibrary::LoadPrefabWithTransform(UObject* WorldContextObject, ULGUIPrefab* InPrefab, USceneComponent* InParent, FVector Location, FRotator Rotation, FVector Scale)
{
	return InPrefab->LoadPrefabWithTransform(WorldContextObject, InParent, Location, Rotation, Scale);
}
AActor* ULGUIBPLibrary::LoadPrefabWithTransform(UObject* WorldContextObject, ULGUIPrefab* InPrefab, USceneComponent* InParent, FVector Location, FQuat Rotation, FVector Scale)
{
	return InPrefab->LoadPrefabWithTransform(WorldContextObject, InParent, Location, Rotation, Scale);
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
	if (IncludeSelf)
	{
		CollectComponentsInChildrenRecursive(InActor, ComponentClass, result, InExcludeNode);
	}
	else
	{
		TArray<AActor*> childrenActors;
		InActor->GetAttachedActors(childrenActors);
		if (childrenActors.Num() > 0)
		{
			for (AActor* actor : childrenActors)
			{
				CollectComponentsInChildrenRecursive(actor, ComponentClass, result, InExcludeNode);
			}
		}
	}
	return result;
}

void ULGUIBPLibrary::CollectComponentsInChildrenRecursive(AActor* InActor, TSubclassOf<UActorComponent> ComponentClass, TArray<UActorComponent*>& InOutArray, const TSet<AActor*>& InExcludeNode)
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

UActorComponent* ULGUIBPLibrary::GetComponentInChildren(AActor* InActor, TSubclassOf<UActorComponent> ComponentClass, bool IncludeSelf, const TSet<AActor*>& InExcludeNode)
{
	if (!IsValid(InActor))
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIBPLibrary::GetComponentInChildren]InActor is not valid!"));
		return nullptr;
	}
	UActorComponent* result = nullptr;
	if (IncludeSelf)
	{
		result = FindComponentInChildrenRecursive(InActor, ComponentClass, InExcludeNode);
	}
	else
	{
		TArray<AActor*> childrenActors;
		InActor->GetAttachedActors(childrenActors);
		if (childrenActors.Num() > 0)
		{
			for (AActor* actor : childrenActors)
			{
				result = FindComponentInChildrenRecursive(InActor, ComponentClass, InExcludeNode);
				if (IsValid(result))
				{
					return result;
				}
			}
		}
	}
	return result;
}
UActorComponent* ULGUIBPLibrary::FindComponentInChildrenRecursive(AActor* InActor, TSubclassOf<UActorComponent> ComponentClass, const TSet<AActor*>& InExcludeNode)
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
	return ULTweenManager::To(target, FLTweenFloatGetterFunction::CreateUObject(target, &UUIItem::GetWidth), FLTweenFloatSetterFunction::CreateUObject(target, &UUIItem::SetWidth), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULGUIBPLibrary::HeightTo(UUIItem* target, float endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::HeightTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ULTweenManager::To(target, FLTweenFloatGetterFunction::CreateUObject(target, &UUIItem::GetHeight), FLTweenFloatSetterFunction::CreateUObject(target, &UUIItem::SetHeight), endValue, duration)->SetEase(ease)->SetDelay(delay);
}

ULTweener* ULGUIBPLibrary::ColorTo(UUIBaseRenderable* target, FColor endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::ColorTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ULTweenManager::To(target, FLTweenColorGetterFunction::CreateUObject(target, &UUIBaseRenderable::GetColor), FLTweenColorSetterFunction::CreateUObject(target, &UUIBaseRenderable::SetColor), endValue, duration)->SetEase(ease)->SetDelay(delay);
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
	return ULTweenManager::To(target, FLTweenColorGetterFunction::CreateUObject(target, &UUIBaseRenderable::GetColor), FLTweenColorSetterFunction::CreateUObject(target, &UUIBaseRenderable::SetColor), endValue, duration)->SetEase(ease)->SetDelay(delay);
}

ULTweener* ULGUIBPLibrary::AlphaTo(UUIBaseRenderable* target, float endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::AlphaTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ULTweenManager::To(target, FLTweenFloatGetterFunction::CreateUObject(target, &UUIBaseRenderable::GetAlpha), FLTweenFloatSetterFunction::CreateUObject(target, &UUIBaseRenderable::SetAlpha), endValue, duration)->SetEase(ease)->SetDelay(delay);
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
	return ULTweenManager::To(target, FLTweenFloatGetterFunction::CreateUObject(target, &UUIBaseRenderable::GetAlpha), FLTweenFloatSetterFunction::CreateUObject(target, &UUIBaseRenderable::SetAlpha), endValue, duration)->SetEase(ease)->SetDelay(delay);
}

ULTweener* ULGUIBPLibrary::HorizontalAnchoredPositionTo(UUIItem* target, float endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::HorizontalAnchoredPositionTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ULTweenManager::To(target, FLTweenFloatGetterFunction::CreateUObject(target, &UUIItem::GetHorizontalAnchoredPosition), FLTweenFloatSetterFunction::CreateUObject(target, &UUIItem::SetHorizontalAnchoredPosition), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULGUIBPLibrary::VerticalAnchoredPositionTo(UUIItem* target, float endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::VerticalAnchoredPositionTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ULTweenManager::To(target, FLTweenFloatGetterFunction::CreateUObject(target, &UUIItem::GetVerticalAnchoredPosition), FLTweenFloatSetterFunction::CreateUObject(target, &UUIItem::SetVerticalAnchoredPosition), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULGUIBPLibrary::AnchoredPositionTo(UUIItem* target, FVector2D endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::AnchoredPositionTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ULTweenManager::To(target, FLTweenVector2DGetterFunction::CreateUObject(target, &UUIItem::GetAnchoredPosition), FLTweenVector2DSetterFunction::CreateUObject(target, &UUIItem::SetAnchoredPosition), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULGUIBPLibrary::PivotTo(UUIItem* target, FVector2D endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::PivotTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ULTweenManager::To(target, FLTweenVector2DGetterFunction::CreateUObject(target, &UUIItem::GetPivot), FLTweenVector2DSetterFunction::CreateUObject(target, &UUIItem::SetPivot), endValue, duration)->SetEase(ease)->SetDelay(delay);
}

ULTweener* ULGUIBPLibrary::AnchorLeftTo(UUIItem* target, float endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::AnchorLeftTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ULTweenManager::To(target, FLTweenFloatGetterFunction::CreateUObject(target, &UUIItem::GetAnchorLeft), FLTweenFloatSetterFunction::CreateUObject(target, &UUIItem::SetAnchorLeft), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULGUIBPLibrary::AnchorRightTo(UUIItem* target, float endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::AnchorRightTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ULTweenManager::To(target, FLTweenFloatGetterFunction::CreateUObject(target, &UUIItem::GetAnchorRight), FLTweenFloatSetterFunction::CreateUObject(target, &UUIItem::SetAnchorRight), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULGUIBPLibrary::AnchorTopTo(UUIItem* target, float endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::AnchorTopTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ULTweenManager::To(target, FLTweenFloatGetterFunction::CreateUObject(target, &UUIItem::GetAnchorTop), FLTweenFloatSetterFunction::CreateUObject(target, &UUIItem::SetAnchorTop), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULGUIBPLibrary::AnchorBottomTo(UUIItem* target, float endValue, float duration, float delay, ELTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::AnchorBottomTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ULTweenManager::To(target, FLTweenFloatGetterFunction::CreateUObject(target, &UUIItem::GetAnchorBottom), FLTweenFloatSetterFunction::CreateUObject(target, &UUIItem::SetAnchorBottom), endValue, duration)->SetEase(ease)->SetDelay(delay);
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
	return HorizontalAnchoredPositionTo(target, endValue, duration, delay, ease);
}
ULTweener* ULGUIBPLibrary::AnchorOffsetYTo(UUIItem* target, float endValue, float duration, float delay, ELTweenEase ease)
{
	return VerticalAnchoredPositionTo(target, endValue, duration, delay, ease);
}
ULTweener* ULGUIBPLibrary::AnchorOffsetTo(UUIItem* target, FVector2D endValue, float duration, float delay, ELTweenEase ease)
{
	return AnchoredPositionTo(target, endValue, duration, delay, ease);
}
ULTweener* ULGUIBPLibrary::StretchLeftTo(UUIItem* target, float endValue, float duration, float delay, ELTweenEase ease)
{
	return AnchorLeftTo(target, endValue, duration, delay, ease);
}
ULTweener* ULGUIBPLibrary::StretchRightTo(UUIItem* target, float endValue, float duration, float delay, ELTweenEase ease)
{
	return AnchorRightTo(target, endValue, duration, delay, ease);
}
ULTweener* ULGUIBPLibrary::StretchTopTo(UUIItem* target, float endValue, float duration, float delay, ELTweenEase ease)
{
	return AnchorTopTo(target, endValue, duration, delay, ease);
}
ULTweener* ULGUIBPLibrary::StretchBottomTo(UUIItem* target, float endValue, float duration, float delay, ELTweenEase ease)
{
	return AnchorBottomTo(target, endValue, duration, delay, ease);
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
