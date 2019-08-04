﻿// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SceneComponent.h"


class LGUI_API ActorCopier
{
public:	
	template<class T>
	static T* DuplicateActorT(T* RootActor, USceneComponent* Parent)
	{
		static_assert(TPointerIsConvertibleFromTo<T, const AActor>::Value, "'T' template parameter to CopyActor must be derived from AActor");
		return (T*)(DuplicateActor(RootActor, Parent));
	}
	static AActor* DuplicateActor(AActor* RootActor, USceneComponent* Parent);
	static void CopyComponentValue(UActorComponent* SrcComp, UActorComponent* TargetComp);

private:
	AActor * CopyActorInternal(AActor* RootActor, USceneComponent* Parent);
	void CopyComponentValueInternal(UActorComponent* SrcComp, UActorComponent* TargetComp);

	TMap<AActor*, int32> MapOriginActorToID;//origin actor to id
	void GenerateActorIDRecursive(AActor* Actor, int32& id);
	TMap<int32, AActor*> MapIDToCopiedActor;//id to copied actor
	struct UPropertyMapStruct 
	{
		UProperty* ObjProperty;//UObjectProperty
		int32 id;//this property's UObject's id
		uint8* Dest;//UObjectProperty's container address
	};
	TArray<UPropertyMapStruct> ObjectPropertyMapStructList;//Actor reference need to wait all actor spawn, use this to collect
	TArray<AActor*> CreatedActors;

	//SceneComponent that belong to same actor, use this struct to store parent's name and then reparent it
	struct SceneComponentToParentNameStruct
	{
		USceneComponent* Comp = nullptr;
		FName ParentName;
	};
	//for blueprint actor to reattach parent after all actor finish spawn
	struct ActorAndParentStruct
	{
		AActor* Actor = nullptr;
		USceneComponent* Parent = nullptr;
	};
	TArray<ActorAndParentStruct> BlueprintActorAndParentArray;//for blueprint actor, we need to attach after ActorSpawn
	//for create instanced object, use this as outer
	TArray<UObject*> OutterArray;
	UObject* Outter = nullptr;

	AActor* CopySingleActor(AActor* Actor, USceneComponent* Parent);
	//copy Actor with all children
	AActor* CopyActorRecursive(AActor* Actor, USceneComponent* Parent, int32& copiedActorId);
	//copy property
	void CopyProperty(UObject* Origin, UObject* Target, TArray<FName> ExcludeProperties);
	void CopyPropertyForActor(UObject* Origin, UObject* Target, TArray<FName> ExcludeProperties);
	//safe version of CopyProperty
	void CopyPropertyChecked(UObject* Origin, UObject* Target, TArray<FName> ExcludeProperties);

	//@param	return	true-for cpp array, if need to loop for next, false otherwise
	bool CopyCommonProperty(UProperty* Property, uint8* Src, uint8* Dest, int cppArrayIndex = 0, bool isInsideCppArray = false);
	void CopyCommonPropertyForChecked(UProperty* Property, uint8* Src, uint8* Dest);

	TArray<FName> GetActorExcludeProperties();
	TArray<FName> GetComponentExcludeProperties();


public:
	//todo: copy actor referece
	static AActor* ReplaceActorClass(AActor* TargetActor, TSubclassOf<AActor> NewActorClass);
private:
	AActor* ReplaceActorClassInternal(AActor* TargetActor, TSubclassOf<AActor> NewActorClass);
	AActor* CopySingleActorAndReplaceClass(AActor* Actor, TSubclassOf<AActor> NewActorClass);
};