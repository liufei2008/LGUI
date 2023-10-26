// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "LGUIComponentReference.generated.h"

/**
 * For direct reference a ActorComponent
 * Update in LGUI3: Directly reference the ActorComponent object instead of component's name, so the reference will not lose after component name changes.
 *					But this result in an issue: BlueprintCreatedComponents will not be saved, so only use this for InstancedComponents.
 */ 
USTRUCT(BlueprintType)
struct LGUI_API FLGUIComponentReference
{
	GENERATED_BODY()
	FLGUIComponentReference(TSubclassOf<UActorComponent> InCompClass);
	FLGUIComponentReference(UActorComponent* InComp);
	FLGUIComponentReference();
protected:
	friend class FLGUIComponentReferenceCustomization;
	/** Editor helper actor */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		TObjectPtr<AActor> HelperActor = nullptr;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (AllowAbstract = "true"))
		TSubclassOf<UActorComponent> HelperClass;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		FName HelperComponentName;

	UPROPERTY(EditAnywhere, Transient, Category = "LGUI")
		mutable TObjectPtr<UActorComponent> TargetComp = nullptr;

	bool CheckTargetObject()const;
public:
	AActor* GetActor()const;
	UActorComponent* GetComponent()const
	{
		if (CheckTargetObject())
		{
			return TargetComp;
		}
		return nullptr;
	}
	template<class T>
	T* GetComponent()const
	{
		if (CheckTargetObject())
		{
#if WITH_EDITOR
			static_assert(TPointerIsConvertibleFromTo<T, const UActorComponent>::Value, "'T' template parameter to GetComponent must be derived from UActorComponent");
			if (!TargetComp)
			{
				UE_LOG(LogTemp, Error, TEXT("[FLGUIComponentReference::GetComponent<T>] TargetComp is null!"));
				return nullptr;
			}
			if (!TargetComp->IsA(T::StaticClass()))
			{
				UE_LOG(LogTemp, Error, TEXT("[FLGUIComponentReference::GetComponent<T>] Provided parameter T: '%s' must be parent of or equal to HelperClass: '%s'!"), *(T::StaticClass()->GetName()), *(HelperClass->GetName()));
				return nullptr;
			}
#endif
			return (T*)(TargetComp);
		}
		else
		{
			return nullptr;
		}
	}
	TSubclassOf<UActorComponent> GetComponentClass()const
	{
		return HelperClass;
	}
	bool IsValidComponentReference()const;

#if WITH_EDITORONLY_DATA
	//can't delete those old data, or new data will missing and K2Node will compile fail.
	/** old data */
	UPROPERTY(VisibleAnywhere, Category = "LGUI-old")
		TWeakObjectPtr<AActor> targetActor;
	/** old data */
	UPROPERTY(VisibleAnywhere, Category = "LGUI-old")
		FName targetComonentName;
	/** old data */
	UPROPERTY(EditAnywhere, Category = "LGUI-old", meta = (AllowAbstract = "true"))
		TSubclassOf<UActorComponent> targetComponentClass;
#endif
};