// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "LGUIBPLibrary.h"
#include "Utils/LGUIUtils.h"
#include "PrefabSystem/ActorSerializer.h"
#include "PrefabSystem/ActorCopier.h"
#include "LTweenActor.h"
#include "LTweenBPLibrary.h"
#include "Core/ActorComponent/UIItem.h"
#include "Core/ActorComponent/UIText.h"
#include "Engine/Engine.h"
#include "Framework/Application/SlateApplication.h"
#include "LGUI.h"

#pragma region QuickEntry
void ULGUIBPLibrary::SetUIAlpha(AActor* Target, float InAlpha)
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
void ULGUIBPLibrary::SetUIActive(AActor* Target, bool Acitve)
{
	if (!IsValid(Target))
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIBPLibrary::SetUIActive]Target is not valid!"));
		return;
	}
	if (auto uiItem = Cast<UUIItem>(Target->GetRootComponent()))
	{
		uiItem->SetIsUIActive(Acitve);
	}
}
void ULGUIBPLibrary::SetUIHierarchyIndex(AActor* Target, int32 index)
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

void ULGUIBPLibrary::DeleteActor(AActor* Target, bool WithHierarchy)
{
	LGUIUtils::DestroyActorWithHierarchy(Target, WithHierarchy);
}
void ULGUIBPLibrary::DestroyActorWithHierarchy(AActor* Target, bool WithHierarchy)
{
	LGUIUtils::DestroyActorWithHierarchy(Target, WithHierarchy);
}
AActor* ULGUIBPLibrary::LoadPrefab(UObject* WorldContextObject, ULGUIPrefab* InPrefab, USceneComponent* InParent, bool SetRelativeTransformToIdentity)
{
	auto world = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	return LGUIPrefabSystem::ActorSerializer::LoadPrefab(world, InPrefab, InParent, SetRelativeTransformToIdentity);
}
AActor* ULGUIBPLibrary::LoadPrefabWithTransform(UObject* WorldContextObject, ULGUIPrefab* InPrefab, USceneComponent* InParent, FVector Location, FRotator Rotation, FVector Scale)
{
	auto world = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	return LGUIPrefabSystem::ActorSerializer::LoadPrefab(world, InPrefab, InParent, Location, Rotation.Quaternion(), Scale);
}
AActor* ULGUIBPLibrary::LoadPrefabWithTransform(UObject* WorldContextObject, ULGUIPrefab* InPrefab, USceneComponent* InParent, FVector Location, FQuat Rotation, FVector Scale)
{
	auto world = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	return LGUIPrefabSystem::ActorSerializer::LoadPrefab(world, InPrefab, InParent, Location, Rotation, Scale);
}
AActor* ULGUIBPLibrary::DuplicateActor(AActor* Target, USceneComponent* Parent)
{
	return LGUIPrefabSystem::ActorCopier::DuplicateActor(Target, Parent);
}
UActorComponent* ULGUIBPLibrary::GetComponentInParent(AActor* InActor, TSubclassOf<UActorComponent> ComponentClass, bool IncludeSelf)
{
	if (!IsValid(InActor))
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIBPLibrary::GetComponentInParent]InActor is not valid!"));
		return nullptr;
	}
	AActor* parentActor = IncludeSelf ? InActor : InActor->GetAttachParentActor();
	while (parentActor != nullptr)
	{
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
TArray<UActorComponent*> ULGUIBPLibrary::GetComponentsInChildren(AActor* InActor, TSubclassOf<UActorComponent> ComponentClass, bool IncludeSelf)
{
	TArray<UActorComponent*> result;
	if (!IsValid(InActor))
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIBPLibrary::GetComponentInParent]InActor is not valid!"));
		return result;
	}
	if (IncludeSelf)
	{
		CollectComponentsInChildrenRecursive(InActor, ComponentClass, result);
	}
	else
	{
		TArray<AActor*> childrenActors;
		InActor->GetAttachedActors(childrenActors);
		if (childrenActors.Num() > 0)
		{
			for (AActor* actor : childrenActors)
			{
				CollectComponentsInChildrenRecursive(actor, ComponentClass, result);
			}
		}
	}
	return result;
}

void ULGUIBPLibrary::CollectComponentsInChildrenRecursive(AActor* InActor, TSubclassOf<UActorComponent> ComponentClass, TArray<UActorComponent*>& InOutArray)
{
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
			CollectComponentsInChildrenRecursive(actor, ComponentClass, InOutArray);
		}
	}
}

UActorComponent* ULGUIBPLibrary::GetComponentInChildren(AActor* InActor, TSubclassOf<UActorComponent> ComponentClass, bool IncludeSelf)
{
	if (!IsValid(InActor))
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIBPLibrary::GetComponentInChildren]InActor is not valid!"));
		return nullptr;
	}
	UActorComponent* result = nullptr;
	if (IncludeSelf)
	{
		result = FindComponentInChildrenRecursive(InActor, ComponentClass);
	}
	else
	{
		TArray<AActor*> childrenActors;
		InActor->GetAttachedActors(childrenActors);
		if (childrenActors.Num() > 0)
		{
			for (AActor* actor : childrenActors)
			{
				result = FindComponentInChildrenRecursive(InActor, ComponentClass);
				if (IsValid(result))
				{
					return result;
				}
			}
		}
	}
	return result;
}
UActorComponent* ULGUIBPLibrary::FindComponentInChildrenRecursive(AActor* InActor, TSubclassOf<UActorComponent> ComponentClass)
{
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
		auto comp = FindComponentInChildrenRecursive(childActor, ComponentClass);
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

TSubclassOf<UActorComponent> ULGUIBPLibrary::LGUICompRef_GetComponentClass(const FLGUIComponentReference& InLGUIComponentReference)
{
	return InLGUIComponentReference.GetComponentClass();
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
ULTweener* ULGUIBPLibrary::UILocalPositionTo(UUIItem* target, FVector endValue, float duration, float delay, LTweenEase ease)
{
	return ULTweenBPLibrary::LocalPositionTo(target, endValue, duration, delay, ease);
}
ULTweener* ULGUIBPLibrary::WidthTo(UUIItem* target, float endValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::WidthTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(target, FLTweenFloatGetterFunction::CreateUObject(target, &UUIItem::GetWidth), FLTweenFloatSetterFunction::CreateUObject(target, &UUIItem::SetWidth), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULGUIBPLibrary::HeightTo(UUIItem* target, float endValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::HeightTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(target, FLTweenFloatGetterFunction::CreateUObject(target, &UUIItem::GetHeight), FLTweenFloatSetterFunction::CreateUObject(target, &UUIItem::SetHeight), endValue, duration)->SetEase(ease)->SetDelay(delay);
}

ULTweener* ULGUIBPLibrary::ColorTo(UUIItem* target, FColor endValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::ColorTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(target, FLTweenColorGetterFunction::CreateUObject(target, &UUIItem::GetColor), FLTweenColorSetterFunction::CreateUObject(target, &UUIItem::SetColor), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULGUIBPLibrary::ColorFrom(UUIItem* target, FColor startValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::ColorFrom target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto endValue = target->GetColor();
	target->SetColor(startValue);
	return ALTweenActor::To(target, FLTweenColorGetterFunction::CreateUObject(target, &UUIItem::GetColor), FLTweenColorSetterFunction::CreateUObject(target, &UUIItem::SetColor), endValue, duration)->SetEase(ease)->SetDelay(delay);
}

ULTweener* ULGUIBPLibrary::AlphaTo(UUIItem* target, float endValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::AlphaTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(target, FLTweenFloatGetterFunction::CreateUObject(target, &UUIItem::GetAlpha), FLTweenFloatSetterFunction::CreateUObject(target, &UUIItem::SetAlpha), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULGUIBPLibrary::AlphaFrom(UUIItem* target, float startValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::AlphaFrom target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	auto endValue = target->GetAlpha();
	target->SetAlpha(startValue);
	return ALTweenActor::To(target, FLTweenFloatGetterFunction::CreateUObject(target, &UUIItem::GetAlpha), FLTweenFloatSetterFunction::CreateUObject(target, &UUIItem::SetAlpha), endValue, duration)->SetEase(ease)->SetDelay(delay);
}

ULTweener* ULGUIBPLibrary::AnchorOffsetXTo(UUIItem* target, float endValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::AnchorOffsetXTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(target, FLTweenFloatGetterFunction::CreateUObject(target, &UUIItem::GetAnchorOffsetX), FLTweenFloatSetterFunction::CreateUObject(target, &UUIItem::SetAnchorOffsetX), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULGUIBPLibrary::AnchorOffsetYTo(UUIItem* target, float endValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::AnchorOffsetYTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(target, FLTweenFloatGetterFunction::CreateUObject(target, &UUIItem::GetAnchorOffsetY), FLTweenFloatSetterFunction::CreateUObject(target, &UUIItem::SetAnchorOffsetY), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULGUIBPLibrary::AnchorOffsetTo(UUIItem* target, FVector2D endValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::AnchorOffsetTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(target, FLTweenVector2DGetterFunction::CreateUObject(target, &UUIItem::GetAnchorOffset), FLTweenVector2DSetterFunction::CreateUObject(target, &UUIItem::SetAnchorOffset), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULGUIBPLibrary::PivotTo(UUIItem* target, FVector2D endValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::PivotTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(target, FLTweenVector2DGetterFunction::CreateUObject(target, &UUIItem::GetPivot), FLTweenVector2DSetterFunction::CreateUObject(target, &UUIItem::SetPivot), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULGUIBPLibrary::StretchLeftTo(UUIItem* target, float endValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::StretchLeftTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(target, FLTweenFloatGetterFunction::CreateUObject(target, &UUIItem::GetStretchLeft), FLTweenFloatSetterFunction::CreateUObject(target, &UUIItem::SetStretchLeft), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULGUIBPLibrary::StretchRightTo(UUIItem* target, float endValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::StretchRightTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(target, FLTweenFloatGetterFunction::CreateUObject(target, &UUIItem::GetStretchRight), FLTweenFloatSetterFunction::CreateUObject(target, &UUIItem::SetStretchRight), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULGUIBPLibrary::StretchTopTo(UUIItem* target, float endValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::StretchTopTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(target, FLTweenFloatGetterFunction::CreateUObject(target, &UUIItem::GetStretchTop), FLTweenFloatSetterFunction::CreateUObject(target, &UUIItem::SetStretchTop), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULGUIBPLibrary::StretchBottomTo(UUIItem* target, float endValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::StretchBottomTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(target, FLTweenFloatGetterFunction::CreateUObject(target, &UUIItem::GetStretchBottom), FLTweenFloatSetterFunction::CreateUObject(target, &UUIItem::SetStretchBottom), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
#pragma endregion


void ULGUIBPLibrary::LGUIExecuteControllerInputAxis(FKey inputKey, float value)
{
	if (inputKey.IsValid())
	{
		const FGamepadKeyNames::Type keyName = inputKey.GetFName();
		FSlateApplication::Get().OnControllerAnalog(keyName, 0, value);
	}
}
void ULGUIBPLibrary::LGUIExecuteControllerInputAction(FKey inputKey, bool pressOrRelease)
{
	if (inputKey.IsValid())
	{
		const FGamepadKeyNames::Type keyName = inputKey.GetFName();
		if (pressOrRelease)
		{
			FSlateApplication::Get().OnControllerButtonPressed(keyName, 0, false);
		}
		else
		{
			FSlateApplication::Get().OnControllerButtonReleased(keyName, 0, false);
		}
	}
}
#pragma endregion

#pragma region DrawableEvent
#define IMPLEMENT_DRAWABLEEVENT_BP(DrawableEventParamType, ParamType)\
FLGUIDelegateHandleWrapper ULGUIBPLibrary::LGUIDrawableEvent_##DrawableEventParamType##_Register(const FLGUIDrawableEvent_##DrawableEventParamType& InEvent, FLGUIDrawableEvent_##DrawableEventParamType##_DynamicDelegate InDelegate)\
{\
	auto delegateHandle = InEvent.Register([InDelegate](ParamType value) {\
		if (InDelegate.IsBound())\
		{\
			InDelegate.Execute(value);\
		}\
		});\
	return FLGUIDelegateHandleWrapper(delegateHandle);\
}\
void ULGUIBPLibrary::LGUIDrawableEvent_##DrawableEventParamType##_Unregister(const FLGUIDrawableEvent_##DrawableEventParamType& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle)\
{\
	InEvent.Unregister(InDelegateHandle.DelegateHandle);\
}

FLGUIDelegateHandleWrapper ULGUIBPLibrary::LGUIDrawableEvent_Empty_Register(const FLGUIDrawableEvent_Empty& InEvent, FLGUIDrawableEvent_Empty_DynamicDelegate InDelegate)
{
	auto delegateHandle = InEvent.Register([InDelegate]() {
		if (InDelegate.IsBound())
		{
			InDelegate.Execute();
		}
		});
	return FLGUIDelegateHandleWrapper(delegateHandle);
}
void ULGUIBPLibrary::LGUIDrawableEvent_Empty_Unregister(const FLGUIDrawableEvent_Empty& InEvent, const FLGUIDelegateHandleWrapper& InDelegateHandle)
{
	InEvent.Unregister(InDelegateHandle.DelegateHandle);
}

IMPLEMENT_DRAWABLEEVENT_BP(Bool, bool);
IMPLEMENT_DRAWABLEEVENT_BP(Float, float);
//IMPLEMENT_DRAWABLEEVENT_BP(Double, double);
//IMPLEMENT_DRAWABLEEVENT_BP(Int8, int8);
IMPLEMENT_DRAWABLEEVENT_BP(UInt8, uint8);
//IMPLEMENT_DRAWABLEEVENT_BP(Int16, int16);
//IMPLEMENT_DRAWABLEEVENT_BP(UInt16, uint16);
IMPLEMENT_DRAWABLEEVENT_BP(Int32, int32);
//IMPLEMENT_DRAWABLEEVENT_BP(UInt32, uint32);
IMPLEMENT_DRAWABLEEVENT_BP(Int64, int64);
//IMPLEMENT_DRAWABLEEVENT_BP(UInt64, uint64);
IMPLEMENT_DRAWABLEEVENT_BP(Vector2, FVector2D);
IMPLEMENT_DRAWABLEEVENT_BP(Vector3, FVector);
IMPLEMENT_DRAWABLEEVENT_BP(Vector4, FVector4);
IMPLEMENT_DRAWABLEEVENT_BP(Color, FColor);
IMPLEMENT_DRAWABLEEVENT_BP(LinearColor, FLinearColor);
IMPLEMENT_DRAWABLEEVENT_BP(Quaternion, FQuat);
IMPLEMENT_DRAWABLEEVENT_BP(String, FString);
IMPLEMENT_DRAWABLEEVENT_BP(Object, UObject*);
IMPLEMENT_DRAWABLEEVENT_BP(Actor, AActor*);
IMPLEMENT_DRAWABLEEVENT_BP(PointerEvent, ULGUIPointerEventData*);
IMPLEMENT_DRAWABLEEVENT_BP(Class, UClass*);
IMPLEMENT_DRAWABLEEVENT_BP(Rotator, FRotator);
IMPLEMENT_DRAWABLEEVENT_BP(Text, FText);
IMPLEMENT_DRAWABLEEVENT_BP(Name, FName);

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
