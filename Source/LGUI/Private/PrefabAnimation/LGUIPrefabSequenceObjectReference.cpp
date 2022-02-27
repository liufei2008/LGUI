// Copyright Epic Games, Inc. All Rights Reserved.

#include "PrefabAnimation/LGUIPrefabSequenceObjectReference.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "Engine/Blueprint.h"
#include "UObject/Package.h"


FLGUIPrefabSequenceObjectReference FLGUIPrefabSequenceObjectReference::CreateForObject(UObject* InObject)
{
	FLGUIPrefabSequenceObjectReference NewReference;
	NewReference.Object = InObject;
	return NewReference;
}

UObject* FLGUIPrefabSequenceObjectReference::Resolve() const
{
	return Object;
}

bool FLGUIPrefabSequenceObjectReferenceMap::HasBinding(const FGuid& ObjectId) const
{
	return BindingIds.Contains(ObjectId);
}

void FLGUIPrefabSequenceObjectReferenceMap::RemoveBinding(const FGuid& ObjectId)
{
	int32 Index = BindingIds.IndexOfByKey(ObjectId);
	if (Index != INDEX_NONE)
	{
		BindingIds.RemoveAtSwap(Index, 1, false);
		References.RemoveAtSwap(Index, 1, false);
	}
}

void FLGUIPrefabSequenceObjectReferenceMap::CreateBinding(const FGuid& ObjectId, const FLGUIPrefabSequenceObjectReference& ObjectReference)
{
	int32 ExistingIndex = BindingIds.IndexOfByKey(ObjectId);
	if (ExistingIndex == INDEX_NONE)
	{
		ExistingIndex = BindingIds.Num();

		BindingIds.Add(ObjectId);
		References.AddDefaulted();
	}

	References[ExistingIndex].Array.AddUnique(ObjectReference);
}

void FLGUIPrefabSequenceObjectReferenceMap::ResolveBinding(const FGuid& ObjectId, TArray<UObject*, TInlineAllocator<1>>& OutObjects) const
{
	int32 Index = BindingIds.IndexOfByKey(ObjectId);
	if (Index == INDEX_NONE)
	{
		return;
	}

	for (const FLGUIPrefabSequenceObjectReference& Reference : References[Index].Array)
	{
		if (UObject* Object = Reference.Resolve())
		{
			OutObjects.Add(Object);
		}
	}
}