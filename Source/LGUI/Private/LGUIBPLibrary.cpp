// Copyright 2019 LexLiu. All Rights Reserved.

#include "LGUIBPLibrary.h"
#include "Utils/LGUIUtils.h"
#include "PrefabSystem/ActorSerializer.h"
#include "PrefabSystem/ActorCopier.h"
#include "LTweenActor.h"
#include "Core/ActorComponent/UIItem.h"
#include "Core/ActorComponent/UIText.h"
#include "Extensions/UISector.h"

DECLARE_CYCLE_STAT(TEXT("CopyActorHierarchy"), STAT_CopyActor, STATGROUP_LGUI);

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
		uiItem->SetUIActive(Acitve);
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
	LGUIUtils::DeleteActor(Target, WithHierarchy);
}
AActor* ULGUIBPLibrary::LoadPrefab(UObject* WorldContextObject, ULGUIPrefab* InPrefab, USceneComponent* InParent, bool SetRelativeTransformToIdentity)
{
	auto world = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	return ActorSerializer::LoadPrefab(world, InPrefab, InParent, SetRelativeTransformToIdentity);
}
AActor* ULGUIBPLibrary::LoadPrefabWithTransform(UObject* WorldContextObject, ULGUIPrefab* InPrefab, USceneComponent* InParent, FVector Location, FRotator Rotation, FVector Scale)
{
	auto world = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	return ActorSerializer::LoadPrefab(world, InPrefab, InParent, Location, Rotation.Quaternion(), Scale);
}
AActor* ULGUIBPLibrary::LoadPrefabWithTransform(UObject* WorldContextObject, ULGUIPrefab* InPrefab, USceneComponent* InParent, FVector Location, FQuat Rotation, FVector Scale)
{
	auto world = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	return ActorSerializer::LoadPrefab(world, InPrefab, InParent, Location, Rotation, Scale);
}
AActor* ULGUIBPLibrary::DuplicateActor(AActor* Target, USceneComponent* Parent)
{
	return ActorCopier::DuplicateActor(Target, Parent);
}
UActorComponent* ULGUIBPLibrary::GetComponentInParent(AActor* InActor, TSubclassOf<UActorComponent> ComponentClass)
{
	if (!IsValid(InActor))
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIBPLibrary::GetComponentInParent]InActor is not valid!"));
		return nullptr;
	}
	UActorComponent* resultComp = InActor->FindComponentByClass(ComponentClass);
	if (resultComp != nullptr)
	{
		return resultComp;
	}
	AActor* parentActor = InActor->GetAttachParentActor();
	while (parentActor != nullptr)
	{
		resultComp = parentActor->FindComponentByClass(ComponentClass);
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
		if (IsValid(comp))
		{
			if (comp->StaticClass()->IsChildOf(ComponentClass->StaticClass()))
			{
				InOutArray.Add(comp);
			}
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

FString ULGUIBPLibrary::Conv_LGUIPointerEventDataToString(const FLGUIPointerEventData& InData)
{
	return InData.ToString();
}
FVector ULGUIBPLibrary::GetWorldPointInPlane(const FLGUIPointerEventData& InData)
{
	return InData.GetWorldPointInPlane();
}
FVector ULGUIBPLibrary::GetLocalPointInPlane(const FLGUIPointerEventData& InData)
{
	return InData.GetLocalPointInPlane();
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


#pragma region LTween

#pragma region UIItem
ULTweener* ULGUIBPLibrary::WidthTo(UUIItem* target, float endValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::WidthTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FloatGetterFunction::CreateUObject(target, &UUIItem::GetWidth), FloatSetterFunction::CreateUObject(target, &UUIItem::SetWidth), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULGUIBPLibrary::HeightTo(UUIItem* target, float endValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::HeightTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FloatGetterFunction::CreateUObject(target, &UUIItem::GetHeight), FloatSetterFunction::CreateUObject(target, &UUIItem::SetHeight), endValue, duration)->SetEase(ease)->SetDelay(delay);
}

ULTweener* ULGUIBPLibrary::ColorTo(UUIItem* target, FColor endValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::ColorTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(ColorGetterFunction::CreateUObject(target, &UUIItem::GetColor), ColorSetterFunction::CreateUObject(target, &UUIItem::SetColor), endValue, duration)->SetEase(ease)->SetDelay(delay);
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
	return ALTweenActor::To(ColorGetterFunction::CreateUObject(target, &UUIItem::GetColor), ColorSetterFunction::CreateUObject(target, &UUIItem::SetColor), endValue, duration)->SetEase(ease)->SetDelay(delay);
}

ULTweener* ULGUIBPLibrary::AlphaTo(UUIItem* target, float endValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::AlphaTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FloatGetterFunction::CreateUObject(target, &UUIItem::GetAlpha), FloatSetterFunction::CreateUObject(target, &UUIItem::SetAlpha), endValue, duration)->SetEase(ease)->SetDelay(delay);
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
	return ALTweenActor::To(FloatGetterFunction::CreateUObject(target, &UUIItem::GetAlpha), FloatSetterFunction::CreateUObject(target, &UUIItem::SetAlpha), endValue, duration)->SetEase(ease)->SetDelay(delay);
}

ULTweener* ULGUIBPLibrary::AnchorOffsetXTo(UUIItem* target, float endValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::AnchorOffsetXTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FloatGetterFunction::CreateUObject(target, &UUIItem::GetAnchorOffsetX), FloatSetterFunction::CreateUObject(target, &UUIItem::SetAnchorOffsetX), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULGUIBPLibrary::AnchorOffsetYTo(UUIItem* target, float endValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::AnchorOffsetYTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FloatGetterFunction::CreateUObject(target, &UUIItem::GetAnchorOffsetY), FloatSetterFunction::CreateUObject(target, &UUIItem::SetAnchorOffsetY), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULGUIBPLibrary::PivotTo(UUIItem* target, FVector2D endValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::PivotTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(Vector2DGetterFunction::CreateUObject(target, &UUIItem::GetPivot), Vector2DSetterFunction::CreateUObject(target, &UUIItem::SetPivot), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULGUIBPLibrary::StretchLeftTo(UUIItem* target, float endValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::StretchLeftTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FloatGetterFunction::CreateUObject(target, &UUIItem::GetStretchLeft), FloatSetterFunction::CreateUObject(target, &UUIItem::SetStretchLeft), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULGUIBPLibrary::StretchRightTo(UUIItem* target, float endValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::StretchRightTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FloatGetterFunction::CreateUObject(target, &UUIItem::GetStretchRight), FloatSetterFunction::CreateUObject(target, &UUIItem::SetStretchRight), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULGUIBPLibrary::StretchTopTo(UUIItem* target, float endValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::StretchTopTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FloatGetterFunction::CreateUObject(target, &UUIItem::GetStretchTop), FloatSetterFunction::CreateUObject(target, &UUIItem::SetStretchTop), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULGUIBPLibrary::StretchBottomTo(UUIItem* target, float endValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::StretchBottomTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FloatGetterFunction::CreateUObject(target, &UUIItem::GetStretchBottom), FloatSetterFunction::CreateUObject(target, &UUIItem::SetStretchBottom), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
#pragma endregion


#pragma region UISector
ULTweener* ULGUIBPLibrary::StartAngleTo(UUISector* target, float endValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::StartAngleTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FloatGetterFunction::CreateUObject(target, &UUISector::GetStartAngle), FloatSetterFunction::CreateUObject(target, &UUISector::SetStartAngle), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* ULGUIBPLibrary::EndAngleTo(UUISector* target, float endValue, float duration, float delay, LTweenEase ease)
{
	if (!IsValid(target))
	{
		UE_LOG(LGUI, Error, TEXT("ULGUIBPLibrary::EndAngleTo target is not valid:%s"), *(target->GetPathName()));
		return nullptr;
	}
	return ALTweenActor::To(FloatGetterFunction::CreateUObject(target, &UUISector::GetEndAngle), FloatSetterFunction::CreateUObject(target, &UUISector::SetEndAngle), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
#pragma endregion
#pragma endregion
