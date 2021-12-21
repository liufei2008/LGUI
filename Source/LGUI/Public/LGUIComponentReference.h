// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "LGUIComponentReference.generated.h"

/**
 * For direct reference a ActorComponent
 * Update in LGUI3: directly reference the ActorComponent object instead of component's name, so the reference will not lose after component name changes.
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
		TargetComp = InComp;
	}
protected:
	friend class FLGUIComponentReferenceCustomization;
#if WITH_EDITORONLY_DATA
	/** Editor helper actor */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		TWeakObjectPtr<AActor> HelperActor;
#endif
	UPROPERTY(EditAnywhere, Category = "LGUI")
		TWeakObjectPtr<UActorComponent> TargetComp;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta=(AllowAbstract="true"))
		TSubclassOf<UActorComponent> targetComponentClass;
public:
	AActor* GetActor()const;
	UActorComponent* GetComponent()const { return TargetComp.Get(); }
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

#if WITH_EDITORONLY_DATA
	/** old data */
	UPROPERTY(VisibleAnywhere, Category = "LGUI-old")
		TWeakObjectPtr<AActor> targetActor;
	UPROPERTY(VisibleAnywhere, Category = "LGUI-old")
		FName targetComonentName;
#endif
};