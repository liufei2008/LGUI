// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "LexUIComponentReference.generated.h"

/**
 * For direct reference a ActorComponent
 */ 
USTRUCT(BlueprintType)
struct LGUI_API FLexUIComponentReference
{
	GENERATED_BODY()
	FLexUIComponentReference(TSubclassOf<UActorComponent> InCompClass);
	FLexUIComponentReference(UActorComponent* InComp, TSubclassOf<UActorComponent> InCompClass);
	FLexUIComponentReference(UActorComponent* InComp);
	FLexUIComponentReference();
protected:
	friend class FLexUIComponentReferenceCustomization;
	/** Editor helper actor */
	UPROPERTY(EditAnywhere, Category = "LexUI")
		TObjectPtr<AActor> HelperActor = nullptr;
	UPROPERTY(EditAnywhere, Category = "LexUI", meta = (AllowAbstract = "true"))
		TSubclassOf<UActorComponent> HelperClass;
	UPROPERTY(VisibleAnywhere, Category = "LexUI")
		FName HelperComponentName;

	UPROPERTY(EditAnywhere, Transient, Category = "LexUI")
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
				UE_LOG(LogTemp, Error, TEXT("[%s].%d TargetComp is null!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
				return nullptr;
			}
			if (!TargetComp->IsA(T::StaticClass()))
			{
				UE_LOG(LogTemp, Error, TEXT("[%s].%d Provided parameter T: '%s' must be parent of or equal to HelperClass: '%s'!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(T::StaticClass()->GetName()), *(HelperClass->GetName()));
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
};