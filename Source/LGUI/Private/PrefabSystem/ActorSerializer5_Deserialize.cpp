﻿// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "PrefabSystem/ActorSerializer5.h"
#include "PrefabSystem/LGUIObjectReaderAndWriter.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIItem.h"
#include "Misc/NetworkVersion.h"
#if WITH_EDITOR
#include "PrefabSystem/ActorSerializer3.h"
#include "PrefabSystem/ActorSerializer4.h"
#include "Utils/LGUIUtils.h"
#endif

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_DISABLE_OPTIMIZATION
#endif
namespace LGUIPrefabSystem5
{
	AActor* ActorSerializer::LoadPrefabWithExistingObjects(UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent
		, TMap<FGuid, UObject*>& InOutMapGuidToObjects, TMap<AActor*, FLGUISubPrefabData>& OutSubPrefabMap
		, bool InSetHierarchyIndexForRootComponent
	)
	{
		ActorSerializer serializer;
		serializer.TargetWorld = InWorld;
		for (auto KeyValue : InOutMapGuidToObjects)//Preprocess the map, ignore invalid object
		{
			if (IsValid(KeyValue.Value))
			{
				serializer.MapGuidToObject.Add(KeyValue.Key, KeyValue.Value);
			}
		}
#if !WITH_EDITOR
		serializer.bIsEditorOrRuntime = false;
#endif
		serializer.bOverrideVersions = true;
		serializer.WriterOrReaderFunction = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? serializer.GetSceneComponentExcludeProperties() : TSet<FName>();
			LGUIPrefabSystem::FLGUIObjectReader Reader(InOutBuffer, serializer, ExcludeProperties);
			Reader.DoSerialize(InObject);
		};
		serializer.WriterOrReaderFunctionForSubPrefab = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, const TSet<FName>& InOverridePropertyNameSet) {
			LGUIPrefabSystem::FLGUIOverrideParameterObjectReader Reader(InOutBuffer, serializer, InOverridePropertyNameSet);
			Reader.DoSerialize(InObject);
		};
		serializer.bSetHierarchyIndexForRootComponent = InSetHierarchyIndexForRootComponent;
		auto rootActor = serializer.DeserializeActor(Parent, InPrefab, false, FVector::ZeroVector, FQuat::Identity, FVector::OneVector);
		InOutMapGuidToObjects = serializer.MapGuidToObject;
		OutSubPrefabMap = serializer.SubPrefabMap;
		return rootActor;
	}

	AActor* ActorSerializer::LoadPrefab(UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent, bool SetRelativeTransformToIdentity, TFunction<void(AActor*)> CallbackBeforeAwake)
	{
		ActorSerializer serializer;
		serializer.TargetWorld = InWorld;
		serializer.CallbackBeforeAwake = CallbackBeforeAwake;
#if !WITH_EDITOR
		serializer.bIsEditorOrRuntime = false;
#endif
		serializer.bOverrideVersions = true;
		serializer.WriterOrReaderFunction = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? serializer.GetSceneComponentExcludeProperties() : TSet<FName>();
			LGUIPrefabSystem::FLGUIObjectReader Reader(InOutBuffer, serializer, ExcludeProperties);
			Reader.DoSerialize(InObject);
		};
		serializer.WriterOrReaderFunctionForSubPrefab = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, const TSet<FName>& InOverridePropertyNameSet) {
			LGUIPrefabSystem::FLGUIOverrideParameterObjectReader Reader(InOutBuffer, serializer, InOverridePropertyNameSet);
			Reader.DoSerialize(InObject);
		};
		AActor* result = nullptr;
		if (SetRelativeTransformToIdentity)
		{
			result = serializer.DeserializeActor(Parent, InPrefab, true);
		}
		else
		{
			result = serializer.DeserializeActor(Parent, InPrefab);
		}
		return result;
	}
	AActor* ActorSerializer::LoadPrefab(UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent, FVector RelativeLocation, FQuat RelativeRotation, FVector RelativeScale, TFunction<void(AActor*)> CallbackBeforeAwake)
	{
		ActorSerializer serializer;
		serializer.TargetWorld = InWorld;
		serializer.CallbackBeforeAwake = CallbackBeforeAwake;
#if !WITH_EDITOR
		serializer.bIsEditorOrRuntime = false;
#endif
		serializer.bOverrideVersions = true;
		serializer.WriterOrReaderFunction = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? serializer.GetSceneComponentExcludeProperties() : TSet<FName>();
			LGUIPrefabSystem::FLGUIObjectReader Reader(InOutBuffer, serializer, ExcludeProperties);
			Reader.DoSerialize(InObject);
		};
		serializer.WriterOrReaderFunctionForSubPrefab = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, const TSet<FName>& InOverridePropertyNameSet) {
			LGUIPrefabSystem::FLGUIOverrideParameterObjectReader Reader(InOutBuffer, serializer, InOverridePropertyNameSet);
			Reader.DoSerialize(InObject);
		};
		return serializer.DeserializeActor(Parent, InPrefab, true, RelativeLocation, RelativeRotation, RelativeScale);
	}
	AActor* ActorSerializer::LoadSubPrefab(
		UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent
		, AActor* InParentLoadedRootActor
		, int32& InOutActorIndex
		, TMap<FGuid, UObject*>& InMapGuidToObject
		, TFunction<void(AActor*, const TMap<FGuid, UObject*>&)> InOnSubPrefabFinishDeserializeFunction
	)
	{
		ActorSerializer serializer;
		serializer.TargetWorld = InWorld;
#if !WITH_EDITOR
		serializer.bIsEditorOrRuntime = false;
#endif
		serializer.bOverrideVersions = true;
		serializer.bSetHierarchyIndexForRootComponent = false;
		serializer.MapGuidToObject = InMapGuidToObject;
		serializer.LoadedRootActor = InParentLoadedRootActor;
		serializer.bIsSubPrefab = true;
		serializer.ActorIndexInPrefab = InOutActorIndex;
		serializer.WriterOrReaderFunction = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? serializer.GetSceneComponentExcludeProperties() : TSet<FName>();
			LGUIPrefabSystem::FLGUIObjectReader Reader(InOutBuffer, serializer, ExcludeProperties);
			Reader.DoSerialize(InObject);
		};
		serializer.WriterOrReaderFunctionForSubPrefab = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, const TSet<FName>& InOverridePropertyNameSet) {
			LGUIPrefabSystem::FLGUIOverrideParameterObjectReader Reader(InOutBuffer, serializer, InOverridePropertyNameSet);
			Reader.DoSerialize(InObject);
		};
		serializer.CallbackBeforeAwakeForSubPrefab = InOnSubPrefabFinishDeserializeFunction;
		auto rootActor = serializer.DeserializeActor(Parent, InPrefab, false, FVector::ZeroVector, FQuat::Identity, FVector::OneVector);
		InOutActorIndex = serializer.ActorIndexInPrefab;
		return rootActor;
	}

#if !UE_BUILD_SHIPPING
#define LGUIPREFAB_LOG_DETAIL_TIME 0
#endif
	AActor* ActorSerializer::DeserializeActorFromData(FLGUIPrefabSaveData& SaveData, USceneComponent* Parent, bool ReplaceTransform, FVector InLocation, FQuat InRotation, FVector InScale)
	{
#if LGUIPREFAB_LOG_DETAIL_TIME
		auto Time = FDateTime::Now();
#endif
		PreGenerateActorRecursive(SaveData.SavedActor, nullptr, FGuid());
		PreGenerateObjectArray(SaveData.SavedObjects, SaveData.SavedComponents);
#if LGUIPREFAB_LOG_DETAIL_TIME
		UE_LOG(LGUI, Log, TEXT("--GenerateObject take time: %fms"), (FDateTime::Now() - Time).GetTotalMilliseconds());
		Time = FDateTime::Now();
#endif
		DeserializeObjectArray(SaveData.SavedObjects, SaveData.SavedComponents);
		auto CreatedRootActor = DeserializeActorRecursive(SaveData.SavedActor);
#if LGUIPREFAB_LOG_DETAIL_TIME
		UE_LOG(LGUI, Log, TEXT("--DeserializeObject take time: %fms"), (FDateTime::Now() - Time).GetTotalMilliseconds());
		Time = FDateTime::Now();
#endif

		//register component
		for (auto CompData : CreatedComponents)
		{
			if (!CompData.Component->IsRegistered())
			{
				CompData.Component->RegisterComponent();
			}
			if (auto PrimitiveComp = Cast<UPrimitiveComponent>(CompData.Component))
			{
				PrimitiveComp->BodyInstance.FixupData(PrimitiveComp);
			}
			if (auto SceneComp = Cast<USceneComponent>(CompData.Component))
			{
				SceneComp->RecreatePhysicsState();
				SceneComp->MarkRenderStateDirty();

				if (CompData.SceneComponentParentGuid.IsValid())
				{
					if (auto ParentObjectPtr = MapGuidToObject.Find(CompData.SceneComponentParentGuid))
					{
						if (auto ParentComp = Cast<USceneComponent>(*ParentObjectPtr))
						{
							SceneComp->AttachToComponent(ParentComp, FAttachmentTransformRules::KeepRelativeTransform);
						}
					}
				}
			}
		}
		//sub prefab root component
		for (auto CompData : SubPrefabRootComponents)
		{
			auto SceneComp = (USceneComponent*)CompData.Component;
			if (auto ParentObjectPtr = MapGuidToObject.Find(CompData.SceneComponentParentGuid))
			{
				if (auto ParentComp = Cast<USceneComponent>(*ParentObjectPtr))
				{
					SceneComp->AttachToComponent(ParentComp, FAttachmentTransformRules::KeepRelativeTransform);
				}
			}
		}

		//attach root actor's parent
		if (CreatedRootActor != nullptr)
		{
			if (USceneComponent* RootComp = CreatedRootActor->GetRootComponent())
			{
				auto RootUIComp = Cast<UUIItem>(RootComp);
				if (Parent)//if UIItem have parent, CheckUIActiveState will becalled when attach
				{
					RootComp->AttachToComponent(Parent, FAttachmentTransformRules::KeepRelativeTransform);
					//recreate hierarchy index
					if (RootUIComp && bSetHierarchyIndexForRootComponent)
					{
						RootUIComp->SetAsLastHierarchy();
					}
				}
				else
				{
					if (RootUIComp)//for UIItem not have parent, need to CheckUIActiveState
					{
						RootUIComp->CheckUIActiveState();
					}
				}
				RootComp->UpdateComponentToWorld();
				if (ReplaceTransform)
				{
					RootComp->SetRelativeLocationAndRotation(InLocation, InRotation);
					RootComp->SetRelativeScale3D(InScale);
				}
			}
		}

		if (CallbackBeforeAwakeForSubPrefab != nullptr)
		{
			CallbackBeforeAwakeForSubPrefab(CreatedRootActor, MapGuidToObject);
		}
		if (CallbackBeforeAwake != nullptr)
		{
			CallbackBeforeAwake(CreatedRootActor);
		}

#if LGUIPREFAB_LOG_DETAIL_TIME
		Time = FDateTime::Now();
#endif
#if WITH_EDITOR
		if (!TargetWorld->IsGameWorld())
		{
			for (auto item : CreatedActors)
			{
				ULGUIEditorManagerObject::RemoveActorForPrefabSystem(item);
			}
			if (!bIsSubPrefab)
			{
				if (LoadedRootActor != nullptr)//if any error hanppens then LoadedRootActor could be nullptr, so check it
				{
					ULGUIEditorManagerObject::EndPrefabSystemProcessingActor(TargetWorld, LoadedRootActor);
				}
			}
		}
		else
#endif
		{
			for (auto item : CreatedActors)
			{
				LGUIManagerActor->RemoveActorForPrefabSystem(item, LoadedRootActor);
			}
			if (!bIsSubPrefab)
			{
				if (LoadedRootActor != nullptr)//if any error hanppens then LoadedRootActor could be nullptr, so check it
				{
					LGUIManagerActor->EndPrefabSystemProcessingActor(LoadedRootActor);
				}
			}
		}
#if LGUIPREFAB_LOG_DETAIL_TIME
		UE_LOG(LGUI, Log, TEXT("--Call Awake (and OnEnable) take time: %fms"), (FDateTime::Now() - Time).GetTotalMilliseconds());
#endif

		return CreatedRootActor;
	}
	AActor* ActorSerializer::DeserializeActor(USceneComponent* Parent, ULGUIPrefab* InPrefab, bool ReplaceTransform, FVector InLocation, FQuat InRotation, FVector InScale)
	{
		if (!InPrefab)
		{
			UE_LOG(LGUI, Error, TEXT("Load Prefab, InPrefab is null!"));
			return nullptr;
		}
		if (!TargetWorld)
		{
			UE_LOG(LGUI, Error, TEXT("Load Prefab: '%s', World is null!"), *InPrefab->GetPathName());
			return nullptr;
		}

#if LGUIPREFAB_LOG_DETAIL_TIME
		UE_LOG(LGUI, Log, TEXT("Begin load prefab: '%s'"), *InPrefab->GetName());
#endif
		auto StartTime = FDateTime::Now();
		PrefabAssetPath = InPrefab->GetPathName();
#if WITH_EDITOR
		if (!TargetWorld->IsGameWorld())
		{

		}
		else
#endif
		{
			LGUIManagerActor = ALGUIManagerActor::GetInstance(TargetWorld, true);
		}
#if WITH_EDITOR
		if (bIsEditorOrRuntime)
		{
			//fill new reference data
			this->ReferenceAssetList = InPrefab->ReferenceAssetList;
			this->ReferenceClassList = InPrefab->ReferenceClassList;
			this->ReferenceNameList = InPrefab->ReferenceNameList;

			this->ArchiveVersion = FPackageFileVersion(InPrefab->ArchiveVersion, (EUnrealEngineObjectUE5Version)InPrefab->ArchiveVersionUE5);
			this->ArchiveLicenseeVer = InPrefab->ArchiveLicenseeVer;
			this->ArEngineNetVer = InPrefab->ArEngineNetVer;
			this->ArGameNetVer = InPrefab->ArGameNetVer;
		}
		else
#endif
		{
			//fill new reference data
			this->ReferenceAssetList = InPrefab->ReferenceAssetListForBuild;
			this->ReferenceClassList = InPrefab->ReferenceClassListForBuild;
			this->ReferenceNameList = InPrefab->ReferenceNameListForBuild;

			this->ArchiveVersion = FPackageFileVersion(InPrefab->ArchiveVersion_ForBuild, (EUnrealEngineObjectUE5Version)InPrefab->ArchiveVersionUE5_ForBuild);
			this->ArchiveLicenseeVer = InPrefab->ArchiveLicenseeVer_ForBuild;
			this->ArEngineNetVer = InPrefab->ArEngineNetVer_ForBuild;
			this->ArGameNetVer = InPrefab->ArGameNetVer_ForBuild;
		}
		this->PrefabVersion = InPrefab->PrefabVersion;
		this->ArEngineVer = FEngineVersionBase(InPrefab->EngineMajorVersion, InPrefab->EngineMinorVersion, InPrefab->EnginePatchVersion);

		FLGUIPrefabSaveData SaveData;
		{
			auto LoadedData =
#if WITH_EDITOR
				bIsEditorOrRuntime ? InPrefab->BinaryData :
#endif
				InPrefab->BinaryDataForBuild;
			if (LoadedData.Num() <= 0)
			{
				UE_LOG(LGUI, Warning, TEXT("Loaded data is empty!"));
				return nullptr;
			}
			auto FromBinary = FMemoryReader(LoadedData, false);
#if WITH_EDITOR
			if (bIsEditorOrRuntime)
			{
				FStructuredArchiveFromArchive(FromBinary).GetSlot() << SaveData;
			}
			else
#endif
			{
				FromBinary << SaveData;
			}
		}

		auto CreatedRootActor = DeserializeActorFromData(SaveData, Parent, ReplaceTransform, InLocation, InRotation, InScale);
		
		auto TimeSpan = FDateTime::Now() - StartTime;
		UE_LOG(LGUI, Log, TEXT("End load prefab: '%s', total time: %fms"), *InPrefab->GetName(), TimeSpan.GetTotalMilliseconds());

#if WITH_EDITOR
		if (IsValid(GEditor))
		{
			GEditor->BroadcastLevelActorListChanged();//UE5 will not auto refresh scene outliner and display actor label, so manually refresh it.
		}
#endif

		return CreatedRootActor;
	}




	void ActorSerializer::PreGenerateObjectArray(const TArray<FLGUIObjectSaveData>& SavedObjects, const TArray<FLGUIComponentSaveData>& SavedComponents)
	{
		//create component first, because some object may use component as outer
		for (auto& ObjectData : SavedComponents)
		{
			UActorComponent* CreatedNewComponent = nullptr;
			if (auto CompPtr = MapGuidToObject.Find(ObjectData.ObjectGuid))
			{
				//UE_LOG(LGUI, Log, TEXT("[ActorSerializer::PreGenerateObjectArray]Already generated:%s!"), *(MapGuidToObject[ObjectData.ComponentGuid]->GetPathName()));
				CreatedNewComponent = Cast<UActorComponent>(*CompPtr);
			}
			else
			{
				if (auto ObjectClass = FindClassFromListByIndex(ObjectData.ObjectClass))
				{
					if (!ObjectClass->IsChildOf(UActorComponent::StaticClass())
						)
					{
						UE_LOG(LGUI, Warning, TEXT("[ActorSerializer::PreGenerateObjectArray]Wrong component class: '%s'. Prefab: '%s'"), *(ObjectClass->GetFName().ToString()), *PrefabAssetPath);
						continue;
					}

					if (auto OuterObjectPtr = MapGuidToObject.Find(ObjectData.OuterObjectGuid))
					{
						CreatedNewComponent = NewObject<UActorComponent>(*OuterObjectPtr, ObjectClass, ObjectData.ComponentName, (EObjectFlags)ObjectData.ObjectFlags);
						MapGuidToObject.Add(ObjectData.ObjectGuid, CreatedNewComponent);
					}
					else
					{
						UE_LOG(LGUI, Warning, TEXT("[ActorSerializer::PreGenerateObjectArray]Missing Owner actor when creating component: %s. Prefab: '%s'"), *(ObjectData.ComponentName.ToString()), *PrefabAssetPath);
						continue;
					}
				}
			}

			if (CreatedNewComponent)
			{
				TArray<UObject*> DefaultSubObjects;
				CreatedNewComponent->CollectDefaultSubobjects(DefaultSubObjects);
				for (auto DefaultSubObject : DefaultSubObjects)
				{
					if (DefaultSubObject->HasAnyFlags(EObjectFlags::RF_Transient))continue;
					auto Index = ObjectData.DefaultSubObjectNameArray.IndexOfByKey(DefaultSubObject->GetFName());
					if (Index == INDEX_NONE)
					{
						UE_LOG(LGUI, Warning, TEXT("[ActorSerializer::PreGenerateObjectArray]Missing guid for default sub object: %s"), *(DefaultSubObject->GetFName().ToString()));
						continue;
					}
					MapGuidToObject.Add(ObjectData.DefaultSubObjectGuidArray[Index], DefaultSubObject);
				}
			}
		}

		for (auto& ObjectData : SavedObjects)
		{
			UObject* CreatedNewObject = nullptr;
			if (auto ObjectPtr = MapGuidToObject.Find(ObjectData.ObjectGuid))
			{
				//UE_LOG(LGUI, Log, TEXT("[ActorSerializer::PreGenerateObjectArray]Already generated:%s!"), *(MapGuidToObject[ObjectData.ObjectGuid]->GetPathName()));
				CreatedNewObject = *ObjectPtr;
			}
			else
			{
				if (auto ObjectClass = FindClassFromListByIndex(ObjectData.ObjectClass))
				{
					if (ObjectClass->IsChildOf(AActor::StaticClass())
						|| ObjectClass->IsChildOf(UActorComponent::StaticClass())
						)
					{
						UE_LOG(LGUI, Warning, TEXT("[ActorSerializer::PreGenerateObjectArray]Wrong object class: '%s'. Prefab: '%s'"), *(ObjectClass->GetFName().ToString()), *PrefabAssetPath);
						continue;
					}

					if (auto OuterObjectPtr = MapGuidToObject.Find(ObjectData.OuterObjectGuid))
					{
						CreatedNewObject = NewObject<UObject>(*OuterObjectPtr, ObjectClass, ObjectData.ObjectName, (EObjectFlags)ObjectData.ObjectFlags);
						MapGuidToObject.Add(ObjectData.ObjectGuid, CreatedNewObject);
					}
					else
					{
						UE_LOG(LGUI, Warning, TEXT("[ActorSerializer::PreGenerateObjectArray]Missing Outer object when creating object: '%s'. Prefab: '%s'"), *(ObjectData.ObjectName.ToString()), *PrefabAssetPath);
						continue;
					}
				}
			}

			if (CreatedNewObject)
			{
				TArray<UObject*> DefaultSubObjects;
				CreatedNewObject->CollectDefaultSubobjects(DefaultSubObjects);
				for (auto DefaultSubObject : DefaultSubObjects)
				{
					if (DefaultSubObject->HasAnyFlags(EObjectFlags::RF_Transient))continue;
					auto Index = ObjectData.DefaultSubObjectNameArray.IndexOfByKey(DefaultSubObject->GetFName());
					if (Index == INDEX_NONE)
					{
						UE_LOG(LGUI, Warning, TEXT("[ActorSerializer::PreGenerateObjectArray]Missing guid for default sub object: %s"), *(DefaultSubObject->GetFName().ToString()));
						continue;
					}
					MapGuidToObject.Add(ObjectData.DefaultSubObjectGuidArray[Index], DefaultSubObject);
				}
			}
		}
	}

	void ActorSerializer::DeserializeObjectArray(const TArray<FLGUIObjectSaveData>& SavedObjects, const TArray<FLGUIComponentSaveData>& SavedComponents)
	{
		//create component first, because some object may use component as outer
		for (auto ObjectData : SavedComponents)
		{
			if (auto CompPtr = MapGuidToObject.Find(ObjectData.ObjectGuid))
			{
				auto CreatedNewComponent = (UActorComponent*)(*CompPtr);
				if (auto SceneComp = Cast<USceneComponent>(CreatedNewComponent))
				{
					WriterOrReaderFunction(CreatedNewComponent, ObjectData.PropertyData, true);
				}
				else
				{
					WriterOrReaderFunction(CreatedNewComponent, ObjectData.PropertyData, false);
				}

				ComponentDataStruct CompData;
				CompData.Component = CreatedNewComponent;
				CompData.SceneComponentParentGuid = ObjectData.SceneComponentParentGuid;
				CreatedComponents.Add(CompData);
			}
		}

		for (auto ObjectData : SavedObjects)
		{
			if (auto ObjectPtr = MapGuidToObject.Find(ObjectData.ObjectGuid))
			{
				auto CreatedNewObject = *ObjectPtr;
				WriterOrReaderFunction(CreatedNewObject, ObjectData.PropertyData, false);
			}
		}
	}

	void ActorSerializer::PreGenerateActorRecursive(FLGUIActorSaveData& InActorData, USceneComponent* Parent, const FGuid& ParentComponentGuid)
	{
		if (InActorData.bIsPrefab)
		{
			auto PrefabIndex = InActorData.PrefabAssetIndex;
			if (auto PrefabAssetObject = FindAssetFromListByIndex(PrefabIndex))
			{
				if (auto SubPrefabAsset = Cast<ULGUIPrefab>(PrefabAssetObject))
				{
					AActor* SubPrefabRootActor = nullptr;
					FLGUISubPrefabData SubPrefabData;
					SubPrefabData.PrefabAsset = SubPrefabAsset;
					TMap<FGuid, UObject*>& SubMapGuidToObject = SubPrefabData.MapGuidToObject;
					if (auto ValuePtr = MapGuidToObject.Find(InActorData.ObjectGuid))
					{
						auto SubPrefabRootActorGuid = InActorData.MapObjectGuidFromParentPrefabToSubPrefab[InActorData.ObjectGuid];
						SubMapGuidToObject.Add(SubPrefabRootActorGuid, *ValuePtr);
					}
					TMap<FGuid, FGuid> MapObjectGuidFromSubPrefabToParentPrefab;
					for (auto& KeyValue : InActorData.MapObjectGuidFromParentPrefabToSubPrefab)
					{
						MapObjectGuidFromSubPrefabToParentPrefab.Add(KeyValue.Value, KeyValue.Key);
						auto ObjectPtr = MapGuidToObject.Find(KeyValue.Key);
						if (!SubMapGuidToObject.Contains(KeyValue.Value) && ObjectPtr != nullptr)
						{
							SubMapGuidToObject.Add(KeyValue.Value, *ObjectPtr);
						}
					}

					auto OnSubPrefabFinishDeserializeFunction =
						[&](AActor* InSubPrefabRootActor, const TMap<FGuid, UObject*>& InSubMapGuidToObject) {
						//collect sub prefab's object and guid to parent map, so all objects are ready when set override parameters
						for (auto& KeyValue : InSubMapGuidToObject)
						{
							auto GuidInSubPrefab = KeyValue.Key;
							auto ObjectInSubPrefab = KeyValue.Value;

							FGuid ObjectGuidInParentPrefab;
							auto ObjectGuidInParentPrefabPtr = MapObjectGuidFromSubPrefabToParentPrefab.Find(GuidInSubPrefab);
							if (ObjectGuidInParentPrefabPtr == nullptr)
							{
								ObjectGuidInParentPrefab = FGuid::NewGuid();
								MapObjectGuidFromSubPrefabToParentPrefab.Add(GuidInSubPrefab, ObjectGuidInParentPrefab);
							}
							else
							{
								ObjectGuidInParentPrefab = *ObjectGuidInParentPrefabPtr;
							}
							SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab.Add(ObjectGuidInParentPrefab, GuidInSubPrefab);

							if (!MapGuidToObject.Contains(ObjectGuidInParentPrefab))
							{
								MapGuidToObject.Add(ObjectGuidInParentPrefab, ObjectInSubPrefab);
							}
						}

						for (auto& RecordData : InActorData.ObjectOverrideParameterArray)
						{
							auto ObjectPtr = MapGuidToObject.Find(RecordData.ObjectGuid);
							if (ObjectPtr != nullptr)
							{
								auto NameSet = RecordData.OverrideParameterNameSet;
								WriterOrReaderFunctionForSubPrefab(*ObjectPtr, RecordData.OverrideParameterData, NameSet);
								FLGUIPrefabOverrideParameterData OverrideDataItem;
								OverrideDataItem.MemberPropertyName = NameSet;
								OverrideDataItem.Object = *ObjectPtr;
								SubPrefabData.ObjectOverrideParameterArray.Add(OverrideDataItem);
							}
						}
					};
#if WITH_EDITOR
					switch ((ELGUIPrefabVersion)SubPrefabAsset->PrefabVersion)
					{
					case ELGUIPrefabVersion::BuildinFArchive:
					{
						SubPrefabRootActor = LGUIPrefabSystem3::ActorSerializer::LoadSubPrefab(this->TargetWorld, SubPrefabAsset, Parent, LoadedRootActor, this->ActorIndexInPrefab, SubMapGuidToObject
							, OnSubPrefabFinishDeserializeFunction
						);
					}
					break;
					case ELGUIPrefabVersion::NestedDefaultSubObject:
					{
						SubPrefabRootActor = LGUIPrefabSystem3::ActorSerializer::LoadSubPrefab(this->TargetWorld, SubPrefabAsset, Parent, LoadedRootActor, this->ActorIndexInPrefab, SubMapGuidToObject
							, OnSubPrefabFinishDeserializeFunction
						);
					}
					break;
					case ELGUIPrefabVersion::ObjectName:
					{
#endif
						SubPrefabRootActor = LGUIPrefabSystem5::ActorSerializer::LoadSubPrefab(this->TargetWorld, SubPrefabAsset, Parent, LoadedRootActor, this->ActorIndexInPrefab, SubMapGuidToObject
							, OnSubPrefabFinishDeserializeFunction
						);
#if WITH_EDITOR
					}
					break;
					default:
					{
						auto MsgText = FText::Format(NSLOCTEXT("LGUIActorSerializer5", "Error_UnsupportOldPrefabVersion", "Detect old sub prefab version which is not support nested prefab! Prefab: '{0}'"), FText::FromString(SubPrefabAsset->GetPathName()));
						LGUIUtils::EditorNotification(MsgText, 1.0f);
					}
					break;
					}
#endif
					

					SubPrefabMap.Add(SubPrefabRootActor, SubPrefabData);
					if (Parent == nullptr && ParentComponentGuid.IsValid())
					{
						ComponentDataStruct CompData;
						CompData.Component = SubPrefabRootActor->GetRootComponent();
						CompData.SceneComponentParentGuid = ParentComponentGuid;
						SubPrefabRootComponents.Add(CompData);
					}
				}
			}
		}
		else
		{
			if (auto ActorClass = FindClassFromListByIndex(InActorData.ObjectClass))
			{
				if (!ActorClass->IsChildOf(AActor::StaticClass()))//if not the right class, use default
				{
					UE_LOG(LGUI, Warning, TEXT("[ActorSerializer::PreGenerateActorRecursive]Find class: '%s' at index: %d, but is not a Actor class, use default. Prefab: '%s'"), *(ActorClass->GetFName().ToString()), InActorData.ObjectClass, *PrefabAssetPath);
					ActorClass = AActor::StaticClass();
				}

				AActor* NewActor = nullptr;
				bool bNeedFinishSpawn = false;
				if (auto ActorPtr = MapGuidToObject.Find(InActorData.ObjectGuid))//MapGuidToObject can passed from LoadPrefabForEdit, so we need to find from map first
				{
					NewActor = (AActor*)(*ActorPtr);
				}
				else
				{
					FActorSpawnParameters Spawnparameters;
					Spawnparameters.ObjectFlags = (EObjectFlags)InActorData.ObjectFlags;
					Spawnparameters.bDeferConstruction = true;
					Spawnparameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
					NewActor = TargetWorld->SpawnActor<AActor>(ActorClass, Spawnparameters);
					bNeedFinishSpawn = true;
					MapGuidToObject.Add(InActorData.ObjectGuid, NewActor);
				}

				if (LoadedRootActor == nullptr)
				{
					LoadedRootActor = NewActor;
#if WITH_EDITOR
					if (!TargetWorld->IsGameWorld())
					{
						ULGUIEditorManagerObject::BeginPrefabSystemProcessingActor(TargetWorld, LoadedRootActor);
					}
					else
#endif
					{
						LGUIManagerActor->BeginPrefabSystemProcessingActor(LoadedRootActor);
					}
				}

#if WITH_EDITOR
				if (!TargetWorld->IsGameWorld())
				{
					ULGUIEditorManagerObject::AddActorForPrefabSystem(NewActor);
				}
				else
#endif
				{
					LGUIManagerActor->AddActorForPrefabSystem(NewActor, LoadedRootActor, ActorIndexInPrefab);
				}
				if (bNeedFinishSpawn)
				{
					NewActor->FinishSpawning(FTransform::Identity);
				}

				//Collect default sub objects
				TArray<UObject*> DefaultSubObjects;
				NewActor->CollectDefaultSubobjects(DefaultSubObjects);
				for (auto DefaultSubObject : DefaultSubObjects)
				{
					if (DefaultSubObject->HasAnyFlags(EObjectFlags::RF_Transient))continue;
					auto Index = InActorData.DefaultSubObjectNameArray.IndexOfByKey(DefaultSubObject->GetFName());
					if (Index == INDEX_NONE)
					{
						UE_LOG(LGUI, Warning, TEXT("[ActorSerializer::PreGenerateActorRecursive]Missing guid for default sub object: %s"), *(DefaultSubObject->GetFName().ToString()));
						continue;
					}
					MapGuidToObject.Add(InActorData.DefaultSubObjectGuidArray[Index], DefaultSubObject);
				}
				if (auto RootComp = NewActor->GetRootComponent())
				{
					if (!MapGuidToObject.Contains(InActorData.RootComponentGuid))//RootComponent could be a BlueprintCreatedComponent, so check it
					{
						MapGuidToObject.Add(InActorData.RootComponentGuid, RootComp);
					}
				}

				CreatedActors.Add(NewActor);
				CreatedActorsGuid.Add(InActorData.ObjectGuid);
				ActorIndexInPrefab++;

				for (auto& ChildSaveData : InActorData.ChildActorData)
				{
					PreGenerateActorRecursive(ChildSaveData, NewActor->GetRootComponent(), InActorData.RootComponentGuid);
				}
			}
			else
			{
				UE_LOG(LGUI, Warning, TEXT("[ActorSerializer::PreGenerateActorRecursive]Actor Class of index:%d not found! Prefab: '%s'"), (InActorData.ObjectClass), *PrefabAssetPath);
			}
		}
	}
	AActor* ActorSerializer::DeserializeActorRecursive(FLGUIActorSaveData& InActorData)
	{
		if (auto ObjectPtr = MapGuidToObject.Find(InActorData.ObjectGuid))
		{
			auto NewActor = (AActor*)(*ObjectPtr);
			if (!InActorData.bIsPrefab)//prefab data is stored in sub prefab and override
			{
				WriterOrReaderFunction(NewActor, InActorData.PropertyData, false);
			}

			for (auto& ChildSaveData : InActorData.ChildActorData)
			{
				DeserializeActorRecursive(ChildSaveData);
			}
			return NewActor;
		}
		else
		{
			return nullptr;
		}
	}
}
#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_ENABLE_OPTIMIZATION
#endif
