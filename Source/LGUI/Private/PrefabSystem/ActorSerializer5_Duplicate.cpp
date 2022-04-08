// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "PrefabSystem/ActorSerializer5.h"
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

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_DISABLE_OPTIMIZATION
#endif
namespace LGUIPrefabSystem5
{
	AActor* ActorSerializer::DuplicateActor(AActor* OriginRootActor, USceneComponent* Parent)
	{
		if (!OriginRootActor)
		{
			UE_LOG(LGUI, Error, TEXT("[ActorSerializer::DuplicateActor]OriginRootActor is null!"));
			return nullptr;
		}
		if (!OriginRootActor->GetWorld())
		{
			UE_LOG(LGUI, Error, TEXT("[ActorSerializer::DuplicateActor]Cannot get World from OriginRootActor!"));
			return nullptr;
		}
		ActorSerializer serializer;
		serializer.TargetWorld = OriginRootActor->GetWorld();
#if !WITH_EDITOR
		serializer.bIsEditorOrRuntime = false;
#endif
		serializer.bOverrideVersions = false;
		return serializer.SerializeActor_ForDuplicate(OriginRootActor, Parent);
	}

	AActor* ActorSerializer::DuplicateActorForEditor(AActor* OriginRootActor, USceneComponent* Parent
		, const TMap<AActor*, FLGUISubPrefabData>& InSubPrefabMap
		, const TMap<UObject*, FGuid>& InMapObjectToGuid
		, TMap<AActor*, FLGUISubPrefabData>& OutDuplicatedSubPrefabMap
		, TMap<FGuid, UObject*>& OutMapGuidToObject
	)
	{
		if (!OriginRootActor)
		{
			UE_LOG(LGUI, Error, TEXT("[ActorSerializer::DuplicateActor]OriginRootActor is null!"));
			return nullptr;
		}
		if (!OriginRootActor->GetWorld())
		{
			UE_LOG(LGUI, Error, TEXT("[ActorSerializer::DuplicateActor]Cannot get World from OriginRootActor!"));
			return nullptr;
		}

		auto StartTime = FDateTime::Now();

		ActorSerializer serializer;
		serializer.TargetWorld = OriginRootActor->GetWorld();
		serializer.MapObjectToGuid = InMapObjectToGuid;
#if !WITH_EDITOR
		serializer.bIsEditorOrRuntime = false;
#endif
		serializer.bOverrideVersions = false;
		//serialize
		serializer.SubPrefabMap = InSubPrefabMap;
		serializer.WriterOrReaderFunction = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? serializer.GetSceneComponentExcludeProperties() : TSet<FName>();
			LGUIPrefabSystem::FLGUIDuplicateObjectWriter Writer(InOutBuffer, serializer, ExcludeProperties);
			Writer.DoSerialize(InObject);
		};
		serializer.WriterOrReaderFunctionForSubPrefab = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, const TSet<FName>& InOverridePropertyNameSet) {
			LGUIPrefabSystem::FLGUIDuplicateOverrideParameterObjectWriter Writer(InOutBuffer, serializer, InOverridePropertyNameSet);
			Writer.DoSerialize(InObject);
		};
		FLGUIPrefabSaveData SaveData;
		serializer.SerializeActorToData(OriginRootActor, SaveData);

		//deserialize
		serializer.SubPrefabMap = {};//clear it for deserializer to fill
		serializer.WriterOrReaderFunction = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? serializer.GetSceneComponentExcludeProperties() : TSet<FName>();
			LGUIPrefabSystem::FLGUIDuplicateObjectReader Reader(InOutBuffer, serializer, ExcludeProperties);
			Reader.DoSerialize(InObject);
		};
		serializer.WriterOrReaderFunctionForSubPrefab = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, const TSet<FName>& InOverridePropertyNameSet) {
			LGUIPrefabSystem::FLGUIDuplicateOverrideParameterObjectReader Reader(InOutBuffer, serializer, InOverridePropertyNameSet);
			Reader.DoSerialize(InObject);
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

#if WITH_EDITOR
		GEditor->BroadcastLevelActorListChanged();//UE5 will not auto refresh scene outliner and display actor label, so manually refresh it.
#endif

		return CreatedRootActor;
	}

	AActor* ActorSerializer::SerializeActor_ForDuplicate(AActor* OriginRootActor, USceneComponent* Parent)
	{
		auto StartTime = FDateTime::Now();

		//serialize
		WriterOrReaderFunction = [this](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? GetSceneComponentExcludeProperties() : TSet<FName>();
			LGUIPrefabSystem::FLGUIDuplicateObjectWriter Writer(InOutBuffer, *this, ExcludeProperties);
			Writer.DoSerialize(InObject);
		};
		FLGUIPrefabSaveData SaveData;
		SerializeActorToData(OriginRootActor, SaveData);

		//deserialize
		WriterOrReaderFunction = [this](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? GetSceneComponentExcludeProperties() : TSet<FName>();
			LGUIPrefabSystem::FLGUIDuplicateObjectReader Reader(InOutBuffer, *this, ExcludeProperties);
			Reader.DoSerialize(InObject);
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
#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_ENABLE_OPTIMIZATION
#endif
