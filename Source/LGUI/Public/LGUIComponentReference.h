// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "LGUIComponentReference.generated.h"

/**
 * for reference a ActorComponent
 */ 
USTRUCT(BlueprintType)
struct LGUI_API FLGUIComponentReference
{
	GENERATED_BODY()
	FLGUIComponentReference(){}
	FLGUIComponentReference(TSubclassOf<UActorComponent> InCompClass)
	{
		targetComponentClass = InCompClass;
	}
	FLGUIComponentReference(UActorComponent* InComp)
	{
		targetComponentClass = InComp->StaticClass();
		targetActor = InComp->GetOwner();
		targetComonentName = InComp->GetFName();
	}
protected:
	friend class FLGUIComponentRefereceCustomization;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		TWeakObjectPtr<AActor> targetActor;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		TSubclassOf<UActorComponent> targetComponentClass;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FName targetComonentName;

	UPROPERTY(Transient) mutable TWeakObjectPtr<UActorComponent> componentInstance;
public:
	AActor* GetActor()const
	{
		return targetActor.Get();
	}
	UActorComponent* GetComponent()const;
	template<class T>
	T* GetComponent()const
	{
		static_assert(TPointerIsConvertibleFromTo<T, const UActorComponent>::Value, "'T' template parameter to GetComponent must be derived from UActorComponent");
		if (T::StaticClass() != targetComponentClass.Get())
		{
			UE_LOG(LogTemp, Error, TEXT("[FLGUIComponentReference::GetComponent<T>]provided parameter T must equal to targetComponentClass!"));
			return nullptr;
		}
		return (T*)(GetComponent());
	}
	TSubclassOf<UActorComponent> GetComponentClass()const
	{
		return targetComponentClass;
	}
	bool IsValid()const;

	static UActorComponent* GetComponentFromTargetActor(AActor* InActor, FName InCompName, UClass* InClass);
};