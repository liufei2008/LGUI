﻿// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "PrefabSystem/ActorSerializer6.h"
#include "PrefabSystem/LGUIObjectReaderAndWriter.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/BufferArchive.h"
#include "Components/PrimitiveComponent.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "LGUI.h"
#include "Core/LGUISettings.h"
#if WITH_EDITOR
#include "Tools/UEdMode.h"
#include "Utils/LGUIUtils.h"
#endif

#if LGUI_CAN_DISABLE_OPTIMIZATION
UE_DISABLE_OPTIMIZATION
#endif
namespace LGUIPREFAB_SERIALIZER_NEWEST_NAMESPACE
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
		serializer.LGUIManagerActor = ALGUIManagerActor::GetInstance(serializer.TargetWorld, true);

		auto Name =
#if WITH_EDITOR
			OriginRootActor->GetActorLabel();
#else
			OriginRootActor->GetPathName();
#endif
		auto StartTime = FDateTime::Now();

		//serialize
		serializer.WriterOrReaderFunction = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? serializer.GetSceneComponentExcludeProperties() : TSet<FName>();
			LGUIPrefabSystem::FLGUIDuplicateObjectWriter Writer(InOutBuffer, serializer, ExcludeProperties);
			Writer.DoSerialize(InObject);
		};
		serializer.WriterOrReaderFunctionForObjectReference = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? serializer.GetSceneComponentExcludeProperties() : TSet<FName>();
			LGUIPrefabSystem::FLGUIDuplicateObjectWriter Writer(InOutBuffer, serializer, ExcludeProperties);
			Writer.DoSerialize(InObject);
		};
		FLGUIPrefabSaveData SaveData;
		serializer.SerializeActorToData(OriginRootActor, SaveData);

		//deserialize
		serializer.WriterOrReaderFunction = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? serializer.GetSceneComponentExcludeProperties() : TSet<FName>();
			LGUIPrefabSystem::FLGUIDuplicateObjectReader Reader(InOutBuffer, serializer, ExcludeProperties);
			Reader.DoSerialize(InObject);
		};
		serializer.WriterOrReaderFunctionForObjectReference = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? serializer.GetSceneComponentExcludeProperties() : TSet<FName>();
			LGUIPrefabSystem::FLGUIDuplicateObjectReader Reader(InOutBuffer, serializer, ExcludeProperties);
			Reader.DoSerialize(InObject);
		};
		auto CreatedRootActor = serializer.DeserializeActorFromData(SaveData, Parent, false, FVector::ZeroVector, FQuat::Identity, FVector::OneVector);

		if (ULGUISettings::GetLogPrefabLoadTime())
		{
			auto TimeSpan = FDateTime::Now() - StartTime;
			UE_LOG(LGUI, Log, TEXT("Duplicate actor: '%s', total time: %fms"), *Name, TimeSpan.GetTotalMilliseconds());
		}

#if WITH_EDITOR
		ULGUIEditorManagerObject::MarkBroadcastLevelActorListChanged();//UE5 will not auto refresh scene outliner and display actor label, so manually refresh it.
#endif
		return CreatedRootActor;
	}
	bool ActorSerializer::PrepareDataForDuplicate(AActor* OriginRootActor, FLGUIPrefabSaveData& OutData, ActorSerializer& OutSerializer)
	{
		if (!OriginRootActor)
		{
			UE_LOG(LGUI, Error, TEXT("[ActorSerializer::PrepareDataForDuplicate]OriginRootActor is null!"));
			return false;
		}
		if (!OriginRootActor->GetWorld())
		{
			UE_LOG(LGUI, Error, TEXT("[ActorSerializer::PrepareDataForDuplicate]Cannot get World from OriginRootActor!"));
			return false;
		}

		auto Name =
#if WITH_EDITOR
			OriginRootActor->GetActorLabel();
#else
			OriginRootActor->GetPathName();
#endif
		auto StartTime = FDateTime::Now();

		ActorSerializer serializer;
		serializer.TargetWorld = OriginRootActor->GetWorld();
#if !WITH_EDITOR
		serializer.bIsEditorOrRuntime = false;
#endif
		serializer.bOverrideVersions = false;
		serializer.LGUIManagerActor = ALGUIManagerActor::GetInstance(serializer.TargetWorld, true);

		//serialize
		serializer.WriterOrReaderFunction = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? serializer.GetSceneComponentExcludeProperties() : TSet<FName>();
			LGUIPrefabSystem::FLGUIDuplicateObjectWriter Writer(InOutBuffer, serializer, ExcludeProperties);
			Writer.DoSerialize(InObject);
		};
		serializer.WriterOrReaderFunctionForObjectReference = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? serializer.GetSceneComponentExcludeProperties() : TSet<FName>();
			LGUIPrefabSystem::FLGUIDuplicateObjectWriter Writer(InOutBuffer, serializer, ExcludeProperties);
			Writer.DoSerialize(InObject);
		};
		serializer.SerializeActorToData(OriginRootActor, OutData);

		OutSerializer = serializer;

		//for deserialize
		OutSerializer.WriterOrReaderFunction = [&OutSerializer](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? OutSerializer.GetSceneComponentExcludeProperties() : TSet<FName>();
			LGUIPrefabSystem::FLGUIDuplicateObjectReader Reader(InOutBuffer, OutSerializer, ExcludeProperties);
			Reader.DoSerialize(InObject);
		};

		auto TimeSpan = FDateTime::Now() - StartTime;
		UE_LOG(LGUI, Log, TEXT("PrepareData_ForDuplicate, actor: '%s' total time: %fms"), *Name, TimeSpan.GetTotalMilliseconds());
		return true;
	}
	AActor* ActorSerializer::DuplicateActorWithPreparedData(FLGUIPrefabSaveData& InData, ActorSerializer& InSerializer, USceneComponent* InParent)
	{
		auto StartTime = FDateTime::Now();
		ActorSerializer copiedSerializer;//use copied, incase undesired data
		copiedSerializer.TargetWorld = InSerializer.TargetWorld;
		copiedSerializer.bIsEditorOrRuntime = InSerializer.bIsEditorOrRuntime;
		copiedSerializer.bOverrideVersions = InSerializer.bOverrideVersions;
		copiedSerializer.LGUIManagerActor = InSerializer.LGUIManagerActor;
		copiedSerializer.ReferenceAssetList = InSerializer.ReferenceAssetList;
		copiedSerializer.ReferenceClassList = InSerializer.ReferenceClassList;
		copiedSerializer.ReferenceNameList = InSerializer.ReferenceNameList;
		copiedSerializer.WriterOrReaderFunction = [&copiedSerializer](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? copiedSerializer.GetSceneComponentExcludeProperties() : TSet<FName>();
			LGUIPrefabSystem::FLGUIDuplicateObjectReader Reader(InOutBuffer, copiedSerializer, ExcludeProperties);
			Reader.DoSerialize(InObject);
		};
		copiedSerializer.WriterOrReaderFunctionForObjectReference = [&copiedSerializer](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? copiedSerializer.GetSceneComponentExcludeProperties() : TSet<FName>();
			LGUIPrefabSystem::FLGUIDuplicateObjectReader Reader(InOutBuffer, copiedSerializer, ExcludeProperties);
			Reader.DoSerialize(InObject);
		};
		auto CreatedRootActor = copiedSerializer.DeserializeActorFromData(InData, InParent, false, FVector::ZeroVector, FQuat::Identity, FVector::OneVector);
		auto TimeSpan = FDateTime::Now() - StartTime;
		UE_LOG(LGUI, Log, TEXT("DuplicateActorWithPreparedData total time: %fms"), TimeSpan.GetTotalMilliseconds());
#if WITH_EDITOR
		ULGUIEditorManagerObject::MarkBroadcastLevelActorListChanged();//UE5 will not auto refresh scene outliner and display actor label, so manually refresh it.
#endif
		return CreatedRootActor;
	}

	AActor* ActorSerializer::DuplicateActorForEditor(AActor* OriginRootActor, USceneComponent* Parent
		, const TMap<TObjectPtr<AActor>, FLGUISubPrefabData>& InSubPrefabMap
		, const TMap<UObject*, FGuid>& InMapObjectToGuid
		, TMap<TObjectPtr<AActor>, FLGUISubPrefabData>& OutDuplicatedSubPrefabMap
		, TMap<FGuid, TObjectPtr<UObject>>& OutMapGuidToObject
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

		auto Name =
#if WITH_EDITOR
			OriginRootActor->GetActorLabel();
#else
			OriginRootActor->GetPathName();
#endif
		UE_LOG(LGUI, Log, TEXT("Begin duplicate actor: '%s'"), *Name);
		auto StartTime = FDateTime::Now();

		ActorSerializer serializer;
		serializer.TargetWorld = OriginRootActor->GetWorld();
		serializer.MapObjectToGuid = InMapObjectToGuid;
		serializer.LGUIManagerActor = ALGUIManagerActor::GetInstance(serializer.TargetWorld, true);
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
		serializer.WriterOrReaderFunctionForObjectReference = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? serializer.GetSceneComponentExcludeProperties() : TSet<FName>();
			LGUIPrefabSystem::FLGUIDuplicateObjectWriter Writer(InOutBuffer, serializer, ExcludeProperties);
			Writer.DoSerialize(InObject);
			};
		serializer.WriterOrReaderFunctionForSubPrefabOverride = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, const TArray<FName>& InOverridePropertyNames) {
			LGUIPrefabSystem::FLGUIDuplicateOverrideParameterObjectWriter Writer(InOutBuffer, serializer, InOverridePropertyNames);
			Writer.DoSerialize(InObject);
		};
		serializer.WriterOrReaderFunctionForSubPrefabOverrideForObjectReference = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, const TArray<FName>& InOverridePropertyNames) {
			LGUIPrefabSystem::FLGUIDuplicateOverrideParameterObjectWriter Writer(InOutBuffer, serializer, InOverridePropertyNames);
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
		serializer.WriterOrReaderFunctionForObjectReference = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? serializer.GetSceneComponentExcludeProperties() : TSet<FName>();
			LGUIPrefabSystem::FLGUIDuplicateObjectReader Reader(InOutBuffer, serializer, ExcludeProperties);
			Reader.DoSerialize(InObject);
			};
		serializer.WriterOrReaderFunctionForSubPrefabOverride = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, const TArray<FName>& InOverridePropertyNameSet) {
			LGUIPrefabSystem::FLGUIDuplicateOverrideParameterObjectReader Reader(InOutBuffer, serializer, InOverridePropertyNameSet);
			Reader.DoSerialize(InObject);
		};
		serializer.WriterOrReaderFunctionForSubPrefabOverrideForObjectReference = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, const TArray<FName>& InOverridePropertyNameSet) {
			LGUIPrefabSystem::FLGUIDuplicateOverrideParameterObjectReader Reader(InOutBuffer, serializer, InOverridePropertyNameSet);
			Reader.DoSerialize(InObject);
			};
		auto CreatedRootActor = serializer.DeserializeActorFromData(SaveData, Parent, false, FVector::ZeroVector, FQuat::Identity, FVector::OneVector);

		OutDuplicatedSubPrefabMap = serializer.SubPrefabMap;
		OutMapGuidToObject = serializer.MapGuidToObject;
		auto TimeSpan = FDateTime::Now() - StartTime;
		UE_LOG(LGUI, Log, TEXT("End duplicate actor: '%s', total time: %fms"), *Name, TimeSpan.GetTotalMilliseconds());

#if WITH_EDITOR
		ULGUIEditorManagerObject::MarkBroadcastLevelActorListChanged();//UE5 will not auto refresh scene outliner and display actor label, so manually refresh it.
#endif

		return CreatedRootActor;
	}
}
#if LGUI_CAN_DISABLE_OPTIMIZATION
UE_ENABLE_OPTIMIZATION
#endif