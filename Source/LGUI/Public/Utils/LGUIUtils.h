// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#if WITH_EDITOR
#include "Editor.h"
#include "EditorStyle.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#endif

class UUIItem;
class ULGUICanvas;

class LGUI_API LGUIUtils
{
public:	
	//Delete actor and all it's hierarchy children
	static void DeleteActor(AActor* Target, bool WithHierarchy = true);
	//Find first component of type T from InActor, if not found go up hierarchy until found
	template<class T>
	static T* GetComponentInParent(AActor* InActor, bool IncludeUnregisteredComponent = true)
	{
		static_assert(TPointerIsConvertibleFromTo<T, const UActorComponent>::Value, "'T' template parameter to FindComponentUpHierarchy must be derived from UActorComponent");
		T* resultComp = nullptr;
		AActor* parentActor = InActor;
		while (IsValid(parentActor))
		{
			resultComp = parentActor->FindComponentByClass<T>();
			if (IsValid(resultComp))
			{
				if (resultComp->IsRegistered())
				{
					return resultComp;
				}
				else
				{
					if (IncludeUnregisteredComponent)
					{
						return resultComp;
					}
				}
			}
			parentActor = parentActor->GetAttachParentActor();
		}
		return nullptr;
	}
	//Collect all component of type T from children
	template<class T>
	static TArray<T*> GetComponentsInChildren(AActor* InActor, bool IncludeSelf = false)
	{
		static_assert(TPointerIsConvertibleFromTo<T, const UActorComponent>::Value, "'T' template parameter to GetComponentsInChildren must be derived from UActorComponent");
		TArray<T*> result;
		if (!IsValid(InActor))
		{
			UE_LOG(LogTemp, Error, TEXT("[LGUIUtils::GetComponentsInChildren]InActor is not valid!"));
			return result;
		}
		if (IncludeSelf)
		{
			CollectComponentsInChildrenRecursive(InActor, result);
		}
		else
		{
			TArray<AActor*> childrenActors;
			InActor->GetAttachedActors(childrenActors);
			if (childrenActors.Num() > 0)
			{
				for (AActor* actor : childrenActors)
				{
					CollectComponentsInChildrenRecursive(actor, result);
				}
			}
		}
		return result;
	}
	template<class T>
	static T* GetComponentInChildren(AActor* InActor, bool IncludeSelf = false)
	{
		static_assert(TPointerIsConvertibleFromTo<T, const UActorComponent>::Value, "'T' template parameter to GetComponentsInChildren must be derived from UActorComponent");
		if (!IsValid(InActor))
		{
			UE_LOG(LogTemp, Error, TEXT("[LGUIUtils::GetComponentsInChildren]InActor is not valid!"));
			return nullptr;
		}
		if (IncludeSelf)
		{
			return GetComponentInChildrenRecursive<T>(InActor);
		}
		else
		{
			TArray<AActor*> childrenActors;
			InActor->GetAttachedActors(childrenActors);
			if (childrenActors.Num() > 0)
			{
				for (AActor* actor : childrenActors)
				{
					if (auto result = GetComponentInChildrenRecursive<T>(actor))
					{
						return result;
					}
				}
			}
		}
		return nullptr;
	}
	static void SortUIItemDepth(TArray<class UUIRenderable*>& shapeList);
	static void SortUIItemDepth(TArray<TSharedPtr<class UIGeometry>>& shapeList);
	//create drawcall
	static void CreateDrawcallFast(TArray<class UUIRenderable*>& sortedList, TArray<TSharedPtr<class UUIDrawcall>>& drawcallList);
	//find root LGUICanvas
	static void FindTopMostCanvas(AActor* actor, ULGUICanvas*& resultCanvas);
	//find LGUICanvas component in parent, not include self
	static void FindParentCanvas(AActor* actor, ULGUICanvas*& resultCanvas);
	static float INV_255;
	static FColor MultiplyColor(FColor A, FColor B);
#if WITH_EDITOR
	//nodify some informations in editor
	static void EditorNotification(FText NofityText);
#endif
	static void CollectChildrenActors(AActor* Target, TArray<AActor*>& AllChildrenActors);

	static UTexture2D* CreateTransientBlackTransparentTexture(int32 InSize, FName InDefaultName = NAME_None);
private:
	static TSharedPtr<class UUIDrawcall> GetAvalibleDrawcall(TArray<TSharedPtr<class UUIDrawcall>>& drawcallList, int& prevDrawcallListCount, int& drawcallCount);
	template<class T>
	static void CollectComponentsInChildrenRecursive(AActor* InActor, TArray<T*>& InOutArray)
	{
		auto& components = InActor->GetComponents();
		for (UActorComponent* comp : components)
		{
			if (comp->IsA(T::StaticClass()))
			{
				InOutArray.Add((T*)comp);
			}
		}

		TArray<AActor*> childrenActors;
		InActor->GetAttachedActors(childrenActors);
		if (childrenActors.Num() > 0)
		{
			for (AActor* actor : childrenActors)
			{
				CollectComponentsInChildrenRecursive(actor, InOutArray);
			}
		}
	}
	template<class T>
	static T* GetComponentInChildrenRecursive(AActor* InActor)
	{
		auto& components = InActor->GetComponents();
		for (UActorComponent* comp : components)
		{
			if (comp->IsA(T::StaticClass()))
			{
				return (T*)comp;
			}
		}

		TArray<AActor*> childrenActors;
		InActor->GetAttachedActors(childrenActors);
		if (childrenActors.Num() > 0)
		{
			for (AActor* actor : childrenActors)
			{
				if (auto result = GetComponentInChildrenRecursive<T>(actor))
				{
					return result;
				}
			}
		}
		return nullptr;
	}
};