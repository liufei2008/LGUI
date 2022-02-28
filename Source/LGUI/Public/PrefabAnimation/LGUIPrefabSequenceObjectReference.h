// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "UObject/LazyObjectPtr.h"
#include "LGUIPrefabSequenceObjectReference.generated.h"

class UActorComponent;

/**
 * An external reference to an level sequence object, resolvable through an arbitrary context.
 */
USTRUCT()
struct FLGUIPrefabSequenceObjectReference
{
	GENERATED_BODY()

	/**
	 * Default construction to a null reference
	 */
	FLGUIPrefabSequenceObjectReference()
	{}


	LGUI_API static FLGUIPrefabSequenceObjectReference CreateForObject(UObject* InObject);

	/**
	 * Check whether this object reference is valid or not
	 */
	bool IsValid() const
	{
		return Object != nullptr && !Object->IsPendingKill();
	}

	/**
	 * Resolve this reference from the specified source actor
	 *
	 * @return The object
	 */
	LGUI_API UObject* Resolve() const;

	/**
	 * Equality comparator
	 */
	friend bool operator==(const FLGUIPrefabSequenceObjectReference& A, const FLGUIPrefabSequenceObjectReference& B)
	{
		return A.Object == B.Object;
	}

private:

	UPROPERTY()
	UObject* Object = nullptr;
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

private:
	
	UPROPERTY()
	TArray<FGuid> BindingIds;

	UPROPERTY()
	TArray<FLGUIPrefabSequenceObjectReferences> References;
};
