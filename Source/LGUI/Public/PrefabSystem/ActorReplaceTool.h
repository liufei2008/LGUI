﻿// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SceneComponent.h"


class LGUI_API ActorReplaceTool
{
public:
	//todo: not handled condition: other actor(not child of this actor) reference this actor
	static AActor* ReplaceActorClass(AActor* TargetActor, TSubclassOf<AActor> NewActorClass);
private:
	AActor* ReplaceActorClassInternal(AActor* TargetActor, TSubclassOf<AActor> NewActorClass);
	AActor* CopySingleActorAndReplaceClass(AActor* Actor, TSubclassOf<AActor> NewActorClass);

	AActor* OriginActor = nullptr;
	AActor* CopiedActor = nullptr;

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
	//for create instanced object, use this as outer
	TArray<UObject*> OutterArray;
	UObject* Outter = nullptr;

	//copy property
	void CopyProperty(UObject* Origin, UObject* Target, TArray<FName> ExcludeProperties);
	void CopyPropertyForActorChecked(UObject* Origin, UObject* Target, TArray<FName> ExcludeProperties);
	//safe version of CopyProperty
	void CopyPropertyChecked(UObject* Origin, UObject* Target, TArray<FName> ExcludeProperties);

	//@param	return	true-for cpp array, if need to loop for next, false otherwise
	bool CopyCommonProperty(UProperty* Property, uint8* Src, uint8* Dest, int cppArrayIndex = 0, bool isInsideCppArray = false);

	bool CheckCommonProperty(UProperty* Property, uint8* Src, int cppArrayIndex = 0, bool isInsideCppArray = false);
	void CheckProperty(UObject* Origin);
	void CheckPropertyForActor(UObject* Origin);

	TArray<FName> GetActorExcludeProperties();
	TArray<FName> GetComponentExcludeProperties();
};