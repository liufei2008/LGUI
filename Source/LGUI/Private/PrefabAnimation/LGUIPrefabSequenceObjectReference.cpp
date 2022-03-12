// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "PrefabAnimation/LGUIPrefabSequenceObjectReference.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "Engine/Blueprint.h"
#include "UObject/Package.h"


#if WITH_EDITOR
TArray<FLGUIPrefabSequenceObjectReference*> FLGUIPrefabSequenceObjectReference::AllObjectReferenceArray;
void FLGUIPrefabSequenceObjectReference::RefreshReference()
{
	if (!IsValid(Object))
	{
		if (IsValid(HelperActor) && IsValid(HelperClass))
		{
			if (HelperClass == AActor::StaticClass())
			{
				Object = HelperActor;
			}
			else
			{
				TArray<UActorComponent*> Components;
				HelperActor->GetComponents(HelperClass, Components);
				if (Components.Num() == 1)
				{
					Object = Components[0];
				}
				else if (Components.Num() > 1)
				{
					for (auto Comp : Components)
					{
						if (Comp->GetFName() == HelperComponentName)
						{
							Object = Comp;
						}
					}
				}
			}
		}
	}
}
void FLGUIPrefabSequenceObjectReference::RefreshAllOnBlueprintRecompile()
{
	for (auto& Item : AllObjectReferenceArray)
	{
		Item->RefreshReference();
	}
}
#endif

FLGUIPrefabSequenceObjectReference::FLGUIPrefabSequenceObjectReference()
{
#if WITH_EDITOR
	AllObjectReferenceArray.Add(this);
#endif
}
FLGUIPrefabSequenceObjectReference::~FLGUIPrefabSequenceObjectReference()
{
#if WITH_EDITOR
	AllObjectReferenceArray.Remove(this);
#endif
}

bool FLGUIPrefabSequenceObjectReference::CreateForObject(UObject* InObject, FLGUIPrefabSequenceObjectReference& OutResult)
{
	OutResult.Object = InObject;
#if WITH_EDITOR
	if (auto Actor = Cast<AActor>(InObject))
	{
		OutResult.HelperActor = Actor;
		OutResult.HelperClass = AActor::StaticClass();
		OutResult.HelperActorLabel = Actor->GetActorLabel();
		OutResult.HelperComponentName = NAME_None;
		return true;
	}
	else
	{
		if (auto Component = Cast<UActorComponent>(InObject))
		{
			Actor = Component->GetOwner();
			OutResult.HelperActor = Actor;
			OutResult.HelperClass = Component->GetClass();
			OutResult.HelperActorLabel = Actor->GetActorLabel();
			OutResult.HelperComponentName = Component->GetFName();
			return true;
		}
	}
#endif
	return false;
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