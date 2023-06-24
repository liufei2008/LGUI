// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "UObject/LazyObjectPtr.h"
#include "LGUIPrefabSequenceObjectReference.generated.h"

class UActorComponent;

/**
 * An external reference to an level sequence object, resolvable through an arbitrary context.
 */
USTRUCT()
struct LGUI_API FLGUIPrefabSequenceObjectReference
{
	GENERATED_BODY()

#if WITH_EDITOR
private:
	static TArray<FLGUIPrefabSequenceObjectReference*> AllObjectReferenceArray;
	void RefreshReference();
public:
	static void RefreshAllOnBlueprintRecompile();
#endif
public:
	FLGUIPrefabSequenceObjectReference();
	~FLGUIPrefabSequenceObjectReference();

#if WITH_EDITOR
	static FString GetActorPathRelativeToContextActor(AActor* InContextActor, AActor* InActor);
	static AActor* GetActorFromContextActorByRelativePath(AActor* InContextActor, const FString& InPath);
	bool FixObjectReferenceFromEditorHelpers(AActor* InContextActor);
	bool CanFixObjectReferenceFromEditorHelpers()const;
	bool IsObjectReferenceGood(AActor* InContextActor)const;
	bool IsEditorHelpersGood(AActor* InContextActor)const;
	bool FixEditorHelpers(AActor* InContextActor);
#endif
	static bool CreateForObject(AActor* InContextActor, UObject* InObject, FLGUIPrefabSequenceObjectReference& OutResult);

	/**
	 * Check whether this object reference is valid or not
	 */
	bool IsValidReference() const
	{
		return IsValid(Object);
	}

	/**
	 * Resolve this reference from the specified source actor
	 *
	 * @return The object
	 */
	UObject* Resolve() const;

	/**
	 * Equality comparator
	 */
	friend bool operator==(const FLGUIPrefabSequenceObjectReference& A, const FLGUIPrefabSequenceObjectReference& B)
	{
		return A.Object == B.Object;
	}

private:

	UPROPERTY()
	TObjectPtr<UObject> Object = nullptr;

#if WITH_EDITORONLY_DATA
	/** HelperActor's actor label/ */
	UPROPERTY()
		FString HelperActorLabel;
	/** HelperActor's path relative to context actor, split by '/'. If only '/' means it is the context actor. could use this to replace reference object in editor/ */
	UPROPERTY()
		FString HelperActorPath;
	/** Editor helper actor, for direct reference actor. */
	UPROPERTY()
		TObjectPtr<AActor> HelperActor = nullptr;
	/** Editor helper, target object class. If class is actor then Object is HelperActor, if class is ActorComponent then Object is the component. */
	UPROPERTY()
		TObjectPtr<UClass> HelperClass = nullptr;
	/** Editor helper, if Object is actor component and HelperActor have multiple components, then select by component name. */
	UPROPERTY()
		FName HelperComponentName;
#endif
};

USTRUCT()
struct FLGUIPrefabSequenceObjectReferences
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FLGUIPrefabSequenceObjectReference> Array;
};

USTRUCT()
struct FLGUIPrefabSequenceObjectReferenceMap
{
	GENERATED_BODY()

	/**
	 * Check whether this map has a binding for the specified object id
	 * @return true if this map contains a binding for the id, false otherwise
	 */
	bool HasBinding(const FGuid& ObjectId) const;

	/**
	 * Remove a binding for the specified ID
	 *
	 * @param ObjectId	The ID to remove
	 */
	void RemoveBinding(const FGuid& ObjectId);

	/**
	 * Create a binding for the specified ID
	 *
	 * @param ObjectId				The ID to associate the component with
	 * @param ObjectReference	The component reference to bind
	 */
	void CreateBinding(const FGuid& ObjectId, const FLGUIPrefabSequenceObjectReference& ObjectReference);

	/**
	 * Resolve a binding for the specified ID using a given context
	 *
	 * @param ObjectId		The ID to associate the object with
	 * @param OutObjects	Container to populate with bound components
	 */
	void ResolveBinding(const FGuid& ObjectId, TArray<UObject*, TInlineAllocator<1>>& OutObjects) const;

#if WITH_EDITOR
	bool IsObjectReferencesGood(AActor* InContextActor)const;
	bool IsEditorHelpersGood(AActor* InContextActor)const;
	//return true if anything changed
	bool FixObjectReferences(AActor* InContextActor);
	//return true if anything changed
	bool FixEditorHelpers(AActor* InContextActor);
#endif
private:
	
	UPROPERTY()
	TArray<FGuid> BindingIds;

	UPROPERTY()
	TArray<FLGUIPrefabSequenceObjectReferences> References;
};
