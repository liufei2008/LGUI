// Copyright 2019 LexLiu. All Rights Reserved.

#include "LGUIBPLibrary.h"
#include "Utils/LGUIUtils.h"
#include "PrefabSystem/ActorSerializer.h"
#include "PrefabSystem/ActorCopier.h"

DECLARE_CYCLE_STAT(TEXT("CopyActorHierarchy"), STAT_CopyActor, STATGROUP_LGUI);

void ULGUIBPLibrary::DeleteActor(AActor* Target, bool WithHierarchy)
{
	LGUIUtils::DeleteActor(Target, WithHierarchy);
}
AActor* ULGUIBPLibrary::LoadPrefab(ULGUIPrefab* InPrefab, USceneComponent* InParent, bool SetRelativeTransformToIdentity)
{
	return ActorSerializer::LoadPrefab(InPrefab, InParent, SetRelativeTransformToIdentity);
}
AActor* ULGUIBPLibrary::LoadPrefabWithTransform(ULGUIPrefab* InPrefab, USceneComponent* InParent, FVector Location, FRotator Rotation, FVector Scale)
{
	return ActorSerializer::LoadPrefab(InPrefab, InParent, Location, Rotation.Quaternion(), Scale);
}
AActor* ULGUIBPLibrary::LoadPrefabWithTransform(ULGUIPrefab* InPrefab, USceneComponent* InParent, FVector Location, FQuat Rotation, FVector Scale)
{
	return ActorSerializer::LoadPrefab(InPrefab, InParent, Location, Rotation, Scale);
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
