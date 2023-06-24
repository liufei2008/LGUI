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
	~FLGUIComponentReference();
#if WITH_EDITORONLY_DATA
	/**
	 * If TargetObject is a BlueprintCreatedComponent, when hit compile on blueprint editor, the TargetObject will lose reference.
	 * So we need to find the referenced TargetObject after blueprint compile.
	 */
	static void RefreshAllOnBlueprintRecompile();
private:
	static TArray<FLGUIComponentReference*> AllLGUIComponentReferenceArray;
	void RefreshOnBlueprintRecompile();
#endif
protected:
	friend class FLGUIComponentReferenceCustomization;
#if WITH_EDITORONLY_DATA
	/** Editor helper actor */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		TObjectPtr<AActor> HelperActor = nullptr;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (AllowAbstract = "true"))
		TSubclassOf<UActorComponent> HelperClass;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		FName HelperComponentName;
#endif
	UPROPERTY(EditAnywhere, Category = "LGUI")
		TObjectPtr<UObject> TargetComp = nullptr;//If use TWeakObjectPtr here, then this pointer will become STALE when recompile the component from blueprint (not sure about c++ hot compile).
										//And, if use UActorComponent here, then reference just missing
public:
	AActor* GetActor()const;
	UActorComponent* GetComponent()const { return (UActorComponent*)TargetComp; }
	template<class T>
	T* GetComponent()const
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
#if WITH_EDITOR
	TSubclassOf<UActorComponent> GetComponentClass()const
	{
		return HelperClass;
	}
#endif
	bool IsValidComponentReference()const;

#if WITH_EDITORONLY_DATA
	//can't delete those old data, or blueprint will throw error.
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