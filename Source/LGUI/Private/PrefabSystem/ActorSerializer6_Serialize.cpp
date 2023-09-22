﻿// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "PrefabSystem/ActorSerializer6.h"
#include "PrefabSystem/LGUIObjectReaderAndWriter.h"
#include "PrefabSystem/LGUIPrefabHelperActor.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIItem.h"
#include "Misc/NetworkVersion.h"
#if WITH_EDITOR
#include "Tools/UEdMode.h"
#include "Utils/LGUIUtils.h"
#endif

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_DISABLE_OPTIMIZATION
#endif
namespace LGUIPrefabSystem6
{
	void ActorSerializer::SavePrefab(AActor* OriginRootActor, ULGUIPrefab* InPrefab
		, TMap<UObject*, FGuid>& InOutMapObjectToGuid, TMap<TObjectPtr<AActor>, FLGUISubPrefabData>& InSubPrefabMap
		, bool InForEditorOrRuntimeUse
	)
	{
		if (!OriginRootActor || !InPrefab)
		{
			UE_LOG(LGUI, Error, TEXT("[ActorSerializer::SerializeActor]OriginRootActor Or InPrefab is null!"));
			return;
		}
		if (!OriginRootActor->GetWorld())
		{
			UE_LOG(LGUI, Error, TEXT("[ActorSerializer::SerializeActor]Cannot get World from OriginRootActor!"));
			return;
		}
		ActorSerializer serializer;
		serializer.TargetWorld = OriginRootActor->GetWorld();
		for (auto& KeyValue : InOutMapObjectToGuid)//Preprocess the map, ignore invalid object
		{
			if (IsValid(KeyValue.Key))
			{
				serializer.MapObjectToGuid.Add(KeyValue.Key, KeyValue.Value);
			}
		}
		serializer.SubPrefabMap = InSubPrefabMap;
		serializer.bIsEditorOrRuntime = InForEditorOrRuntimeUse;
		serializer.WriterOrReaderFunction = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? serializer.GetSceneComponentExcludeProperties() : TSet<FName>();
			LGUIPrefabSystem::FLGUIObjectWriter Writer(InOutBuffer, serializer, ExcludeProperties);
			Writer.DoSerialize(InObject);
		};
		serializer.WriterOrReaderFunctionForObjectReference = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? serializer.GetSceneComponentExcludeProperties() : TSet<FName>();
			LGUIPrefabSystem::FLGUIObjectWriter Writer(InOutBuffer, serializer, ExcludeProperties);
			Writer.DoSerialize(InObject);
			};
		serializer.WriterOrReaderFunctionForSubPrefabOverride = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, const TArray<FName>& InOverridePropertyNames) {
			LGUIPrefabSystem::FLGUIOverrideParameterObjectWriter Writer(InOutBuffer, serializer, InOverridePropertyNames);
			Writer.DoSerialize(InObject);
		};
		serializer.WriterOrReaderFunctionForSubPrefabOverrideForObjectReference = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, const TArray<FName>& InOverridePropertyNames) {
			LGUIPrefabSystem::FLGUIOverrideParameterObjectWriter Writer(InOutBuffer, serializer, InOverridePropertyNames);
			Writer.DoSerialize(InObject);
			};
		serializer.SerializeActor(OriginRootActor, InPrefab);
		InOutMapObjectToGuid = serializer.MapObjectToGuid;
	}

	void ActorSerializer::SerializeActorRecursive(AActor* Actor, FLGUIActorSaveData& OutActorSaveData, TMap<FGuid, TArray<uint8>>& SavedObjectReferences)
	{
		if (!IsValid(Actor))return;
		if (auto SubPrefabDataPtr = SubPrefabMap.Find(Actor))//sub prefab's actor is not collected in WillSerailizeActorArray
		{
			OutActorSaveData.bIsPrefab = true;
			OutActorSaveData.PrefabAssetIndex = FindOrAddAssetIdFromList(SubPrefabDataPtr->PrefabAsset);
			OutActorSaveData.ActorGuid = MapObjectToGuid[Actor];
			OutActorSaveData.MapObjectGuidFromParentPrefabToSubPrefab = SubPrefabDataPtr->MapObjectGuidFromParentPrefabToSubPrefab;

			//serialize override parameter data
			for (auto& DataItem : SubPrefabDataPtr->ObjectOverrideParameterArray)
			{
				TArray<uint8> SubPrefabOverrideData;
				auto SubPrefabObject = DataItem.Object.Get();
				if (MapObjectToGuid.Contains(SubPrefabObject))
				{
					WriterOrReaderFunctionForSubPrefabOverride(SubPrefabObject, SubPrefabOverrideData, DataItem.MemberPropertyNames);
					FLGUIPrefabOverrideParameterSaveData RecordDataItem;
					RecordDataItem.OverrideParameterData = SubPrefabOverrideData;
					RecordDataItem.OverrideParameterNames = DataItem.MemberPropertyNames;
					WriterOrReaderFunctionForSubPrefabOverrideForObjectReference(SubPrefabObject, RecordDataItem.OverrideObjectReferenceParameterData, DataItem.MemberPropertyNames);
					OutActorSaveData.MapObjectGuidToSubPrefabOverrideParameter.Add(MapObjectToGuid[SubPrefabObject], RecordDataItem);
				}
			}
		}
		else
		{
			if (!WillSerailizeActorArray.Contains(Actor))return;
			auto ActorGuid = MapObjectToGuid[Actor];

			OutActorSaveData.ObjectClass = FindOrAddClassFromList(Actor->GetClass());
			OutActorSaveData.ActorGuid = ActorGuid;
			OutActorSaveData.ObjectFlags = (uint32)Actor->GetFlags();
			WriterOrReaderFunction(Actor, OutActorSaveData.PropertyData, false);
			WriterOrReaderFunctionForObjectReference(Actor, SavedObjectReferences.Add(ActorGuid), false);
			if (auto RootComp = Actor->GetRootComponent())
			{
				OutActorSaveData.RootComponentGuid = MapObjectToGuid[RootComp];
			}
			TArray<UObject*> DefaultSubObjects;
			Actor->CollectDefaultSubobjects(DefaultSubObjects);
			for (auto DefaultSubObject : DefaultSubObjects)
			{
				if (DefaultSubObject->HasAnyFlags(EObjectFlags::RF_Transient))continue;
				if (!MapObjectToGuid.Contains(DefaultSubObject))
				{
					MapObjectToGuid.Add(DefaultSubObject, FGuid::NewGuid());
				}
				OutActorSaveData.DefaultSubObjectGuidArray.Add(MapObjectToGuid[DefaultSubObject]);
				OutActorSaveData.DefaultSubObjectNameArray.Add(DefaultSubObject->GetFName());
			}

			TArray<AActor*> ChildrenActors;
			Actor->GetAttachedActors(ChildrenActors);
#if WITH_EDITOR
			if (!TargetWorld->IsGameWorld())//only need in edit mode, because runtime serialize (or duplicate) don't use sub-prefab
			{
				//sort on hierarchy, so hierarchy order will be good when deserialize it. Actually normal UIItem's hierarchyIndex property can do the job, but sub prefab's root actor not, so sort it to make sure.
				Algo::Sort(ChildrenActors, [](const AActor* A, const AActor* B) {
					auto ARoot = A->GetRootComponent();
					auto BRoot = B->GetRootComponent();
					if (ARoot != nullptr && BRoot != nullptr)
					{
						auto AUIRoot = Cast<UUIItem>(ARoot);
						auto BUIRoot = Cast<UUIItem>(BRoot);
						if (AUIRoot != nullptr && BUIRoot != nullptr)
						{
							return AUIRoot->GetHierarchyIndex() < BUIRoot->GetHierarchyIndex();
						}
					}
					return false;
					});
			}
#endif
			TArray<FLGUIActorSaveData> ChildSaveDataList;
			for (auto ChildActor : ChildrenActors)
			{
				FLGUIActorSaveData ChildActorSaveData;
				SerializeActorRecursive(ChildActor, ChildActorSaveData, SavedObjectReferences);
				ChildSaveDataList.Add(ChildActorSaveData);
			}
			OutActorSaveData.ChildrenActorDataArray = ChildSaveDataList;
		}
	}
	void ActorSerializer::SerializeActorToData(AActor* OriginRootActor, FLGUIPrefabSaveData& OutData)
	{
		CollectActorRecursive(OriginRootActor);
		//serailize actor
		SerializeActorRecursive(OriginRootActor, OutData.SavedActor, OutData.SavedObjectReferences);
		//serialize objects and components
		SerializeObjectArray(OutData.SavedObjects, OutData.SavedObjectReferences, OutData.MapSceneComponentToParent);
	}
	void ActorSerializer::SerializeActor(AActor* OriginRootActor, ULGUIPrefab* InPrefab)
	{
		if (!InPrefab)
		{
			UE_LOG(LGUI, Error, TEXT("Save Prefab, InPrefab is null!"));
			return;
		}
		if (!IsValid(OriginRootActor))
		{
			UE_LOG(LGUI, Error, TEXT("Save Prefab, OriginRootActor is not valid!"));
			return;
		}
		if (OriginRootActor->HasAnyFlags(EObjectFlags::RF_Transient))
		{
			UE_LOG(LGUI, Error, TEXT("Save Prefab, OriginRootActor is transient!"));
			return;
		}

		auto StartTime = FDateTime::Now();

		FLGUIPrefabSaveData SaveData;
		SerializeActorToData(OriginRootActor, SaveData);

		FBufferArchive ToBinary;
#if WITH_EDITOR
		if (bIsEditorOrRuntime)
		{
			FStructuredArchiveFromArchive(ToBinary).GetSlot() << SaveData;
		}
		else
#endif
		{
			ToBinary << SaveData;
		}

		if (ToBinary.Num() <= 0)
		{
			UE_LOG(LGUI, Warning, TEXT("Save binary length is 0!"));
			return;
		}
#if WITH_EDITOR
		if (bIsEditorOrRuntime)
		{
			InPrefab->BinaryData = ToBinary;
			InPrefab->ThumbnailDirty = true;
			InPrefab->CreateTime = FDateTime::UtcNow();

			//clear old reference data
			InPrefab->ReferenceAssetList.Empty();
			InPrefab->ReferenceClassList.Empty();
			InPrefab->ReferenceNameList.Empty();
			InPrefab->ReferenceTextList.Empty();
			InPrefab->ReferenceStringList.Empty();
			//fill new reference data
			InPrefab->ReferenceAssetList = this->ReferenceAssetList;
			InPrefab->ReferenceClassList = this->ReferenceClassList;
			InPrefab->ReferenceNameList = this->ReferenceNameList;

			InPrefab->ArchiveVersion = GPackageFileUEVersion.FileVersionUE4;
			InPrefab->ArchiveVersionUE5 = GPackageFileUEVersion.FileVersionUE5;
			InPrefab->ArchiveLicenseeVer = GPackageFileLicenseeUEVersion;
			InPrefab->ArEngineNetVer = FNetworkVersion::GetNetworkProtocolVersion(FEngineNetworkCustomVersion::Guid);
			InPrefab->ArGameNetVer = FNetworkVersion::GetNetworkProtocolVersion(FGameNetworkCustomVersion::Guid);

			InPrefab->MarkPackageDirty();
		}
		else
#endif
		{
			InPrefab->BinaryDataForBuild = ToBinary;

			//fill new reference data
			InPrefab->ReferenceAssetListForBuild = this->ReferenceAssetList;
			InPrefab->ReferenceClassListForBuild = this->ReferenceClassList;
			InPrefab->ReferenceNameListForBuild = this->ReferenceNameList;

			InPrefab->ArchiveVersion_ForBuild = GPackageFileUEVersion.FileVersionUE4;
			InPrefab->ArchiveVersionUE5_ForBuild = GPackageFileUEVersion.FileVersionUE5;
			InPrefab->ArchiveLicenseeVer_ForBuild = GPackageFileLicenseeUEVersion;
			InPrefab->ArEngineNetVer_ForBuild = FNetworkVersion::GetNetworkProtocolVersion(FEngineNetworkCustomVersion::Guid);
			InPrefab->ArGameNetVer_ForBuild = FNetworkVersion::GetNetworkProtocolVersion(FGameNetworkCustomVersion::Guid);
		}

		InPrefab->EngineMajorVersion = ENGINE_MAJOR_VERSION;
		InPrefab->EngineMinorVersion = ENGINE_MINOR_VERSION;
		InPrefab->PrefabVersion = LGUI_CURRENT_PREFAB_VERSION;

		auto TimeSpan = FDateTime::Now() - StartTime;
		UE_LOG(LGUI, Log, TEXT("Take %fs saving prefab: %s"), TimeSpan.GetTotalSeconds(), *InPrefab->GetName());
	}

	void ActorSerializer::CollectActorRecursive(AActor* Actor)
	{
		if (!IsValid(Actor))return;
		if (Actor->HasAnyFlags(EObjectFlags::RF_Transient))return;
#if WITH_EDITOR
		if (bIsEditorOrRuntime)
		{
		}
		else
#endif
		{
			if (Actor->bIsEditorOnlyActor)return;
		}
		//collect actor
		if (!SubPrefabMap.Contains(Actor))//sub prefab's actor should not put to the list
		{
			WillSerailizeActorArray.Add(Actor);
		}
		if (!MapObjectToGuid.Contains(Actor))
		{
			MapObjectToGuid.Add(Actor, FGuid::NewGuid());
		}

		TArray<AActor*> ChildrenActors;
		Actor->GetAttachedActors(ChildrenActors);
		for (auto ChildActor : ChildrenActors)
		{
			CollectActorRecursive(ChildActor);//collect all actor, include subprefab's actor
		}
	}

	void ActorSerializer::SerializeObjectArray(TMap<FGuid, FLGUIObjectSaveData>& ObjectSaveDataArray, TMap<FGuid, TArray<uint8>>& SavedObjectReferences, TMap<FGuid, FGuid>& MapSceneComponentToParent)
	{
		for (int i = 0; i < WillSerailizeObjectArray.Num(); i++)
		{
			auto Object = WillSerailizeObjectArray[i];
			auto Class = Object->GetClass();
			FLGUIObjectSaveData ObjectSaveDataItem;
			ObjectSaveDataItem.ObjectClass = FindOrAddClassFromList(Class);
			ObjectSaveDataItem.ObjectName = Object->GetFName();
			ObjectSaveDataItem.ObjectFlags = (uint32)Object->GetFlags();
			ObjectSaveDataItem.OuterObjectGuid = MapObjectToGuid[Object->GetOuter()];
			auto SceneComp = Cast<USceneComponent>(Object);
			if (SceneComp)
			{
				if (auto ParentComp = SceneComp->GetAttachParent())
				{
					if (WillSerailizeActorArray.Contains(ParentComp->GetOwner()))//check if parent component belongs to this prefab
					{
						MapSceneComponentToParent.Add(MapObjectToGuid[Object], MapObjectToGuid[ParentComp]);
					}
				}
			}
			WriterOrReaderFunction(Object, ObjectSaveDataItem.PropertyData, SceneComp != nullptr);
			WriterOrReaderFunctionForObjectReference(Object, SavedObjectReferences.Add(MapObjectToGuid[Object]), SceneComp != nullptr);
			TArray<UObject*> DefaultSubObjects;
			Object->CollectDefaultSubobjects(DefaultSubObjects);
			for (auto DefaultSubObject : DefaultSubObjects)
			{
				if (DefaultSubObject->HasAnyFlags(EObjectFlags::RF_Transient))continue;
				if (!MapObjectToGuid.Contains(DefaultSubObject))
				{
					MapObjectToGuid.Add(DefaultSubObject, FGuid::NewGuid());
				}
				ObjectSaveDataItem.DefaultSubObjectGuidArray.Add(MapObjectToGuid[DefaultSubObject]);
				ObjectSaveDataItem.DefaultSubObjectNameArray.Add(DefaultSubObject->GetFName());
			}
			ObjectSaveDataArray.Add(MapObjectToGuid[Object], ObjectSaveDataItem);
		}
	}
}
#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_ENABLE_OPTIMIZATION
#endif