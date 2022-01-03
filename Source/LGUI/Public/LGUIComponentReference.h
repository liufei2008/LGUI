// Copyright 2019-2021 LexLiu. All Rights Reserved.

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
	FLGUIComponentReference() {}
protected:
	friend class FLGUIComponentReferenceCustomization;
#if WITH_EDITORONLY_DATA
	/** Editor helper actor */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		AActor* HelperActor = nullptr;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (AllowAbstract = "true"))
		TSubclassOf<UActorComponent> HelperClass;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		FName HelperComponentName;
#endif
	UPROPERTY(EditAnywhere, Category = "LGUI")
		UActorComponent* TargetComp = nullptr;//If use TWeakObjectPtr here, then this pointer will become STALE when recompile the component from blueprint (not sure about c++ hot compile).
public:
	AActor* GetActor()const;
	UActorComponent* GetComponent()const { return TargetComp; }
	template<class T>
	T* GetComponent()const
	{
#if WITH_EDITOR
		static_assert(TPointerIsConvertibleFromTo<T, const UActorComponent>::Value, "'T' template parameter to GetComponent must be derived from UActorComponent");
		if (T::StaticClass() != HelperClass.Get())
		{
			UE_LOG(LogTemp, Error, TEXT("[FLGUIComponentReference::GetComponent<T>]provided parameter T must equal to HelperClass!"));
			return nullptr;
		}
#endif
		return (T*)(GetComponent());
	}
#if WITH_EDITOR
	TSubclassOf<UActorComponent> GetComponentClass()const
	{
		return HelperClass;
	}
#endif
	bool IsValid()const;

#if WITH_EDITORONLY_DATA
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