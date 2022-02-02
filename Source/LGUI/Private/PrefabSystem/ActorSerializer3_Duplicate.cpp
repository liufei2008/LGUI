// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "PrefabSystem/ActorSerializer3.h"
#include "PrefabSystem/LGUIObjectReaderAndWriter.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/BufferArchive.h"
#include "Components/PrimitiveComponent.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "LGUI.h"
#if WITH_EDITOR
#include "Tools/UEdMode.h"
#include "Utils/LGUIUtils.h"
#endif

PRAGMA_DISABLE_OPTIMIZATION
namespace LGUIPrefabSystem3
{
	AActor* ActorSerializer3::DuplicateActor(AActor* OriginRootActor, USceneComponent* Parent)
	{
		if (!OriginRootActor)
		{
			UE_LOG(LGUI, Error, TEXT("[ActorSerializer3::DuplicateActor]OriginRootActor is null!"));
			return nullptr;
		}
		if (!OriginRootActor->GetWorld())
		{
			UE_LOG(LGUI, Error, TEXT("[ActorSerializer3::DuplicateActor]Cannot get World from OriginRootActor!"));
			return nullptr;
		}
		ActorSerializer3 serializer;
		serializer.TargetWorld = OriginRootActor->GetWorld();
#if !WITH_EDITOR
		serializer.bIsEditorOrRuntime = false;
#endif
		return serializer.SerializeActor_ForDuplicate(OriginRootActor, Parent);
	}

	AActor* ActorSerializer3::DuplicateActorForEditor(AActor* OriginRootActor, USceneComponent* Parent
		, const TMap<AActor*, FLGUISubPrefabData>& InSubPrefabMap
		, const TMap<UObject*, FGuid>& InMapObjectToGuid
		, TMap<AActor*, FLGUISubPrefabData>& OutDuplicatedSubPrefabMap
		, TMap<FGuid, UObject*>& OutMapGuidToObject
	)
	{
		if (!OriginRootActor)
		{
			UE_LOG(LGUI, Error, TEXT("[ActorSerializer3::DuplicateActor]OriginRootActor is null!"));
			return nullptr;
		}
		if (!OriginRootActor->GetWorld())
		{
			UE_LOG(LGUI, Error, TEXT("[ActorSerializer3::DuplicateActor]Cannot get World from OriginRootActor!"));
			return nullptr;
		}

		auto StartTime = FDateTime::Now();

		ActorSerializer3 serializer;
		serializer.TargetWorld = OriginRootActor->GetWorld();
		serializer.MapObjectToGuid = InMapObjectToGuid;
#if !WITH_EDITOR
		serializer.bIsEditorOrRuntime = false;
#endif
		//serialize
		serializer.SubPrefabMap = InSubPrefabMap;
		serializer.WriterOrReaderFunction = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? serializer.GetSceneComponentExcludeProperties() : TSet<FName>();
			FLGUIObjectWriter Writer(InObject, InOutBuffer, serializer, ExcludeProperties);
		};
		serializer.WriterOrReaderFunctionForSubPrefab = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, const TSet<FName>& InOverridePropertyNameSet) {
			FLGUIOverrideParameterObjectWriter Writer(InObject, InOutBuffer, serializer, InOverridePropertyNameSet);
		};
		FLGUIPrefabSaveData SaveData;
		serializer.SerializeActorToData(OriginRootActor, SaveData);

		serializer.SubPrefabMap = {};
		//deserialize
		serializer.WriterOrReaderFunction = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? serializer.GetSceneComponentExcludeProperties() : TSet<FName>();
			FLGUIObjectReader Reader(InObject, InOutBuffer, serializer, ExcludeProperties);
		};
		serializer.WriterOrReaderFunctionForSubPrefab = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, const TSet<FName>& InOverridePropertyNameSet) {
			FLGUIOverrideParameterObjectReader Reader(InObject, InOutBuffer, serializer, InOverridePropertyNameSet);
		};
		auto CreatedRootActor = serializer.DeserializeActorFromData(SaveData, Parent, false, FVector::ZeroVector, FQuat::Identity, FVector::OneVector);

		auto TimeSpan = FDateTime::Now() - StartTime;
		auto Name =
#if WITH_EDITOR
			OriginRootActor->GetActorLabel();
#else
			OriginRootActor->GetPathName();
#endif
		OutDuplicatedSubPrefabMap = serializer.SubPrefabMap;
		OutMapGuidToObject = serializer.MapGuidToObject;
		UE_LOG(LGUI, Log, TEXT("Take %fs duplicating actor: %s"), TimeSpan.GetTotalSeconds(), *Name);
		return CreatedRootActor;
	}

	AActor* ActorSerializer3::SerializeActor_ForDuplicate(AActor* OriginRootActor, USceneComponent* Parent)
	{
		auto StartTime = FDateTime::Now();

		//serialize
		WriterOrReaderFunction = [this](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? GetSceneComponentExcludeProperties() : TSet<FName>();
			FLGUIDuplicateObjectWriter Writer(InObject, InOutBuffer, *this, ExcludeProperties);
		};
		FLGUIPrefabSaveData SaveData;
		SerializeActorToData(OriginRootActor, SaveData);

		//deserialize
		WriterOrReaderFunction = [this](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? GetSceneComponentExcludeProperties() : TSet<FName>();
			FLGUIDuplicateObjectReader Reader(InObject, InOutBuffer, *this, ExcludeProperties);
		};
		auto CreatedRootActor = DeserializeActorFromData(SaveData, Parent, false, FVector::ZeroVector, FQuat::Identity, FVector::OneVector);

		auto TimeSpan = FDateTime::Now() - StartTime;
		auto Name =
#if WITH_EDITOR
			OriginRootActor->GetActorLabel();
#else
			OriginRootActor->GetPathName();
#endif
		UE_LOG(LGUI, Log, TEXT("Take %fs duplicating actor: %s"), TimeSpan.GetTotalSeconds(), *Name);
		return CreatedRootActor;
	}
}
PRAGMA_ENABLE_OPTIMIZATION
