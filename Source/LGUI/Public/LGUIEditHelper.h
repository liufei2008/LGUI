// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "LGUIEditHelper.generated.h"

//for create a button in editor
USTRUCT(BlueprintType)
struct FLGUIEditHelperButton
{
	GENERATED_BODY()
private:
	friend class FLGUIEditHelperButtonCustomization;
	UPROPERTY(Transient, EditAnywhere, Category = "LGUI")
		int32 clickCount = 0;
	UPROPERTY(Transient, EditAnywhere, Category = "LGUI")
		int32 prevClickCount = 0;
public:
	bool IsClicked()
	{
		bool result = prevClickCount != clickCount;
		prevClickCount = clickCount;
		return result;
	}
};

//for reference a ActorComponent
USTRUCT(BlueprintType)
struct FLGUIComponentReference
{
	GENERATED_BODY()
	FLGUIComponentReference(){}
	FLGUIComponentReference(TSubclassOf<UActorComponent> InCompClass)
	{
		targetComponentClass = InCompClass;
	}
protected:
	friend class FLGUIComponentRefereceCustomization;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		AActor* targetActor;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		TSubclassOf<UActorComponent> targetComponentClass = UActorComponent::StaticClass();

	UPROPERTY(Transient) mutable UActorComponent* componentInstance;

public:
	AActor* GetActor()const
	{
		return targetActor;
	}
	UActorComponent* GetComponent()const
	{
		if (componentInstance == nullptr)
		{
			if (targetActor == nullptr)
				return nullptr;
			componentInstance = targetActor->FindComponentByClass(targetComponentClass);
		}
		return componentInstance;
	}
	template<class T>
	T* GetComponent()const
	{
		return (T*)(GetComponent());
	}
	TSubclassOf<UActorComponent> GetComponentClass()const
	{
		return targetComponentClass;
	}
};