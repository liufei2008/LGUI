// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "PrefabSystem/ActorSerializer6.h"
#include "PrefabSystem/LGUIObjectReaderAndWriter.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIItem.h"
#include "Misc/NetworkVersion.h"
#include "UObject/UObjectThreadContext.h"
#include "Core/LGUISettings.h"
#if WITH_EDITOR
#include "PrefabSystem/ActorSerializer3.h"
#include "PrefabSystem/ActorSerializer4.h"
#include "PrefabSystem/ActorSerializer5.h"
#include "Utils/LGUIUtils.h"
#endif

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_DISABLE_OPTIMIZATION
#endif
namespace LGUIPrefabSystem6
{
	AActor* ActorSerializer::LoadPrefabWithExistingObjects(UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent
		, TMap<FGuid, TObjectPtr<UObject>>& InOutMapGuidToObjects, TMap<TObjectPtr<AActor>, FLGUISubPrefabData>& OutSubPrefabMap
		, bool InSetHierarchyIndexForRootComponent
	)
	{
		ActorSerializer serializer;
		serializer.TargetWorld = InWorld;
		for (auto& KeyValue : InOutMapGuidToObjects)//Preprocess the map, ignore invalid object
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
		serializer.WriterOrReaderFunctionForObjectReference = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? serializer.GetSceneComponentExcludeProperties() : TSet<FName>();
			LGUIPrefabSystem::FLGUIObjectReader Reader(InOutBuffer, serializer, ExcludeProperties);
			Reader.DoSerialize(InObject);
		};
		serializer.WriterOrReaderFunctionForSubPrefabOverride = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, const TArray<FName>& InOverridePropertyNames) {
			LGUIPrefabSystem::FLGUIOverrideParameterObjectReader Reader(InOutBuffer, serializer, InOverridePropertyNames);
			Reader.DoSerialize(InObject);
		};
		serializer.WriterOrReaderFunctionForSubPrefabOverrideForObjectReference = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, const TArray<FName>& InOverridePropertyNames) {
			LGUIPrefabSystem::FLGUIOverrideParameterObjectReader Reader(InOutBuffer, serializer, InOverridePropertyNames);
			Reader.DoSerialize(InObject);
		};
		serializer.bSetHierarchyIndexForRootComponent = InSetHierarchyIndexForRootComponent;
		auto rootActor = serializer.DeserializeActor(Parent, InPrefab, nullptr, false, FVector::ZeroVector, FQuat::Identity, FVector::OneVector);
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
		serializer.WriterOrReaderFunctionForObjectReference = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? serializer.GetSceneComponentExcludeProperties() : TSet<FName>();
			LGUIPrefabSystem::FLGUIObjectReader Reader(InOutBuffer, serializer, ExcludeProperties);
			Reader.DoSerialize(InObject);
		};
		serializer.WriterOrReaderFunctionForSubPrefabOverride = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, const TArray<FName>& InOverridePropertyNames) {
			LGUIPrefabSystem::FLGUIOverrideParameterObjectReader Reader(InOutBuffer, serializer, InOverridePropertyNames);
			Reader.DoSerialize(InObject);
		};
		serializer.WriterOrReaderFunctionForSubPrefabOverrideForObjectReference = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, const TArray<FName>& InOverridePropertyNames) {
			LGUIPrefabSystem::FLGUIOverrideParameterObjectReader Reader(InOutBuffer, serializer, InOverridePropertyNames);
			Reader.DoSerialize(InObject);
		};
		AActor* result = nullptr;
		if (SetRelativeTransformToIdentity)
		{
			result = serializer.DeserializeActor(Parent, InPrefab, nullptr, true);
		}
		else
		{
			result = serializer.DeserializeActor(Parent, InPrefab, nullptr);
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
		serializer.WriterOrReaderFunctionForObjectReference = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? serializer.GetSceneComponentExcludeProperties() : TSet<FName>();
			LGUIPrefabSystem::FLGUIObjectReader Reader(InOutBuffer, serializer, ExcludeProperties);
			Reader.DoSerialize(InObject);
		};
		serializer.WriterOrReaderFunctionForSubPrefabOverride = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, const TArray<FName>& InOverridePropertyNames) {
			LGUIPrefabSystem::FLGUIOverrideParameterObjectReader Reader(InOutBuffer, serializer, InOverridePropertyNames);
			Reader.DoSerialize(InObject);
		};
		serializer.WriterOrReaderFunctionForSubPrefabOverrideForObjectReference = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, const TArray<FName>& InOverridePropertyNames) {
			LGUIPrefabSystem::FLGUIOverrideParameterObjectReader Reader(InOutBuffer, serializer, InOverridePropertyNames);
			Reader.DoSerialize(InObject);
		};
		return serializer.DeserializeActor(Parent, InPrefab, nullptr, true, RelativeLocation, RelativeRotation, RelativeScale);
	}
	AActor* ActorSerializer::LoadPrefab(UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent, const TMap<UObject*, UObject*>& InReplaceAssetMap, const TMap<UClass*, UClass*>& InReplaceClassMap, TFunction<void(AActor*)> CallbackBeforeAwake)
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
		serializer.WriterOrReaderFunctionForObjectReference = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? serializer.GetSceneComponentExcludeProperties() : TSet<FName>();
			LGUIPrefabSystem::FLGUIObjectReader Reader(InOutBuffer, serializer, ExcludeProperties);
			Reader.DoSerialize(InObject);
		};
		serializer.WriterOrReaderFunctionForSubPrefabOverride = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, const TArray<FName>& InOverridePropertyNames) {
			LGUIPrefabSystem::FLGUIOverrideParameterObjectReader Reader(InOutBuffer, serializer, InOverridePropertyNames);
			Reader.DoSerialize(InObject);
		};
		serializer.WriterOrReaderFunctionForSubPrefabOverrideForObjectReference = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, const TArray<FName>& InOverridePropertyNames) {
			LGUIPrefabSystem::FLGUIOverrideParameterObjectReader Reader(InOutBuffer, serializer, InOverridePropertyNames);
			Reader.DoSerialize(InObject);
		};
		return serializer.DeserializeActor(Parent, InPrefab, [=, &serializer] {
			if (InReplaceAssetMap.Num() > 0)
			{
				for (int i = 0; i < serializer.ReferenceAssetList.Num(); i++)
				{
					if (auto ReplaceAssetPtr = InReplaceAssetMap.Find(serializer.ReferenceAssetList[i]))
					{
						serializer.ReferenceAssetList[i] = *ReplaceAssetPtr;
					}
				}
			}
			if (InReplaceClassMap.Num() > 0)
			{
				for (int i = 0; i < serializer.ReferenceClassList.Num(); i++)
				{
					if (auto ReplaceClassPtr = InReplaceClassMap.Find(serializer.ReferenceClassList[i]))
					{
						serializer.ReferenceClassList[i] = *ReplaceClassPtr;
					}
				}
			}
			});
	}
	AActor* ActorSerializer::LoadSubPrefab(
		UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent
		, AActor* InParentLoadedRootActor
		, int32& InOutActorIndex
		, TMap<FGuid, TObjectPtr<UObject>>& InMapGuidToObject
		, const TFunction<void(AActor*, const TMap<FGuid, TObjectPtr<UObject>>&, const TArray<AActor*>&)>& InOnSubPrefabFinishDeserializeFunction
		, const TFunction<void(FGuid, UObject*)>& InOnSubPrefabDeserializeObjectFunction
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
		serializer.WriterOrReaderFunctionForObjectReference = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? serializer.GetSceneComponentExcludeProperties() : TSet<FName>();
			LGUIPrefabSystem::FLGUIObjectReader Reader(InOutBuffer, serializer, ExcludeProperties);
			Reader.DoSerialize(InObject);
		};
		serializer.WriterOrReaderFunctionForSubPrefabOverride = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, const TArray<FName>& InOverridePropertyNames) {
			LGUIPrefabSystem::FLGUIOverrideParameterObjectReader Reader(InOutBuffer, serializer, InOverridePropertyNames);
			Reader.DoSerialize(InObject);
		};
		serializer.WriterOrReaderFunctionForSubPrefabOverrideForObjectReference = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, const TArray<FName>& InOverridePropertyNames) {
			LGUIPrefabSystem::FLGUIOverrideParameterObjectReader Reader(InOutBuffer, serializer, InOverridePropertyNames);
			Reader.DoSerialize(InObject);
		};
		serializer.OnSubPrefabFinishDeserializeFunction = InOnSubPrefabFinishDeserializeFunction;
		serializer.OnSubPrefabDeserializeObjectFunction = InOnSubPrefabDeserializeObjectFunction;
		auto rootActor = serializer.DeserializeActor(Parent, InPrefab, nullptr, false, FVector::ZeroVector, FQuat::Identity, FVector::OneVector);
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
		auto CreatedRootActor = GenerateActorRecursive(SaveData.SavedActor, SaveData.SavedObjects, nullptr, FGuid());//this must be nullptr, because we need to do the attachment later, to handle hierarchy index
		GenerateObjectArray(SaveData.SavedObjects, SaveData.MapSceneComponentToParent);
#if LGUIPREFAB_LOG_DETAIL_TIME
		UE_LOG(LGUI, Log, TEXT("--GenerateObject take time: %fms"), (FDateTime::Now() - Time).GetTotalMilliseconds());
		Time = FDateTime::Now();
#endif
#if LGUIPREFAB_LOG_DETAIL_TIME
		UE_LOG(LGUI, Log, TEXT("--DeserializeObject take time: %fms"), (FDateTime::Now() - Time).GetTotalMilliseconds());
		Time = FDateTime::Now();
#endif
		//object reference
		for (auto& KeyValue : SaveData.SavedObjectReferences)
		{
			if (auto ObjectPtr = MapGuidToObject.Find(KeyValue.Key))
			{
				WriterOrReaderFunctionForObjectReference(*ObjectPtr, KeyValue.Value, Cast<USceneComponent>(*ObjectPtr) != nullptr);
			}
		}

		//sub prefab object reference
		for (auto& Item : SubPrefabOverrideParameters)
		{
			WriterOrReaderFunctionForSubPrefabOverrideForObjectReference(Item.Object, Item.ParameterDatas, Item.ParameterNames);
		}

		//register component
		for (auto& CompData : CreatedComponents)
		{
			if (!CompData.Component->IsRegistered())
			{
				CompData.Component->RegisterComponent();
			}
			if (auto SceneComp = Cast<USceneComponent>(CompData.Component))
			{
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
		for (auto& CompData : SubPrefabRootComponents)
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

#if WITH_EDITOR
		if (!TargetWorld->IsGameWorld())
		{
			//in edit mode, component may already exist, so recreate
			for (auto& CompData : CreatedComponents)
			{
				CompData.Component->RecreatePhysicsState();
				CompData.Component->MarkRenderStateDirty();
				CompData.Component->MarkRenderDynamicDataDirty();
				CompData.Component->MarkRenderTransformDirty();
				CompData.Component->MarkRenderInstancesDirty();
			}
			for (auto& CompData : SubPrefabRootComponents)
			{
				CompData.Component->RecreatePhysicsState();
				CompData.Component->MarkRenderStateDirty();
				CompData.Component->MarkRenderDynamicDataDirty();
				CompData.Component->MarkRenderTransformDirty();
				CompData.Component->MarkRenderInstancesDirty();
			}
			//refresh it
			for (auto& Actor : CreatedActors)
			{
				Actor->RerunConstructionScripts();
			}
		}
#endif

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

		if (OnSubPrefabFinishDeserializeFunction != nullptr)
		{
			OnSubPrefabFinishDeserializeFunction(CreatedRootActor, MapGuidToObject, CreatedActors);
		}
		if (CallbackBeforeAwake != nullptr)
		{
			CallbackBeforeAwake(CreatedRootActor);
		}

#if LGUIPREFAB_LOG_DETAIL_TIME
		Time = FDateTime::Now();
#endif
		if (!bIsSubPrefab)
		{
			for (auto item : CreatedActors)
			{
				LGUIManagerActor->RemoveActorForPrefabSystem(item, LoadedRootActor);
			}
			if (LoadedRootActor != nullptr)//if any error hanppens then LoadedRootActor could be nullptr, so check it
			{
				LGUIManagerActor->EndPrefabSystemProcessingActor(LoadedRootActor);
			}
		}
#if LGUIPREFAB_LOG_DETAIL_TIME
		UE_LOG(LGUI, Log, TEXT("--Call Awake (and OnEnable) take time: %fms"), (FDateTime::Now() - Time).GetTotalMilliseconds());
#endif

		return CreatedRootActor;
	}
	AActor* ActorSerializer::DeserializeActor(USceneComponent* Parent, ULGUIPrefab* InPrefab, const TFunction<void()>& InCallbackBeforeDeserialize, bool ReplaceTransform, FVector InLocation, FQuat InRotation, FVector InScale)
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

		auto StartTime = FDateTime::Now();
		PrefabAssetPath = InPrefab->GetPathName();
		LGUIManagerActor = ALGUIManagerActor::GetInstance(TargetWorld, true);
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
			auto& LoadedData =
#if WITH_EDITOR
				bIsEditorOrRuntime ? InPrefab->BinaryData :
#endif
				InPrefab->BinaryDataForBuild;

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

		if (InCallbackBeforeDeserialize != nullptr)InCallbackBeforeDeserialize();
		auto CreatedRootActor = DeserializeActorFromData(SaveData, Parent, ReplaceTransform, InLocation, InRotation, InScale);

		if (ULGUISettings::GetLogPrefabLoadTime())
		{
			auto TimeSpan = FDateTime::Now() - StartTime;
			UE_LOG(LGUI, Log, TEXT("Load prefab: '%s', total time: %fms"), *InPrefab->GetName(), TimeSpan.GetTotalMilliseconds());
		}

#if WITH_EDITOR
		ULGUIEditorManagerObject::MarkBroadcastLevelActorListChanged();//UE5 will not auto refresh scene outliner and display actor label, so manually refresh it.
#endif

		return CreatedRootActor;
	}




	void ActorSerializer::GenerateObjectArray(TMap<FGuid, FLGUIObjectSaveData>& SavedObjects, TMap<FGuid, FGuid>& MapSceneComponentToParent)
	{
		auto RestoreObjectData = [&](UObject* Target, const FGuid& TargetGuid, FLGUICommonObjectSaveData& ObjectData) {
			//collect default sub object
			TArray<UObject*> DefaultSubObjects;
			Target->CollectDefaultSubobjects(DefaultSubObjects);
			for (auto DefaultSubObject : DefaultSubObjects)
			{
				if (DefaultSubObject->HasAnyFlags(EObjectFlags::RF_Transient))continue;
				auto Index = ObjectData.DefaultSubObjectNameArray.IndexOfByKey(DefaultSubObject->GetFName());
				if (Index == INDEX_NONE)
				{
					UE_LOG(LGUI, Warning, TEXT("[%s].%d Missing guid for default sub object: %s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(DefaultSubObject->GetFName().ToString()));
					continue;
				}
				auto DefaultSubObjectGuid = ObjectData.DefaultSubObjectGuidArray[Index];
				MapGuidToObject.Add(DefaultSubObjectGuid, DefaultSubObject);
				AlreadyRestoredObject.Add(Target);
				WriterOrReaderFunction(DefaultSubObject, SavedObjects[DefaultSubObjectGuid].PropertyData, false);
				if (OnSubPrefabDeserializeObjectFunction != nullptr)
				{
					OnSubPrefabDeserializeObjectFunction(DefaultSubObjectGuid, DefaultSubObject);
				}
			}
			if (!AlreadyRestoredObject.Contains(Target))
			{
				AlreadyRestoredObject.Add(Target);
				WriterOrReaderFunction(Target, ObjectData.PropertyData, Cast<USceneComponent>(Target) != nullptr);
				if (OnSubPrefabDeserializeObjectFunction != nullptr)
				{
					OnSubPrefabDeserializeObjectFunction(TargetGuid, Target);
				}
			}
		};
		for (auto& KeyValuePair : SavedObjects)
		{
			auto& ObjectGuid = KeyValuePair.Key;
			auto& ObjectData = KeyValuePair.Value;
			UObject* CreatedNewObject = nullptr;
			if (auto ObjectPtr = MapGuidToObject.Find(ObjectGuid))
			{
				CreatedNewObject = *ObjectPtr;
				RestoreObjectData(CreatedNewObject, ObjectGuid, ObjectData);//already created, so it must be other actor/component/object's default-sub-object, then the data is already restored, so no need to restore self again
			}
			else
			{
				if (auto ObjectClass = FindClassFromListByIndex(ObjectData.ObjectClass))
				{
					if (ObjectClass->IsChildOf(AActor::StaticClass()))
					{
						UE_LOG(LGUI, Warning, TEXT("[%s].%d Wrong object class: '%s'. Prefab: '%s'"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(ObjectClass->GetFName().ToString()), *PrefabAssetPath);
						continue;
					}

					if (auto OuterObjectPtr = MapGuidToObject.Find(ObjectData.OuterObjectGuid))
					{
						EComponentCreationMethod OrignCreationMethod;
						bool bIsComponent = ObjectClass->IsChildOf(UActorComponent::StaticClass());
						FStaticConstructObjectParameters Params(ObjectClass);
						Params.Outer = *OuterObjectPtr;
						Params.Name = ObjectData.ObjectName;
						Params.SetFlags = (EObjectFlags)ObjectData.ObjectFlags;
						Params.Template = nullptr;
						Params.bCopyTransientsFromClassDefaults = false;
						Params.InstanceGraph = nullptr;
						Params.ExternalPackage = nullptr;
						Params.PropertyInitCallback = [&] {
							auto& ThreadContext = FUObjectThreadContext::Get();
							auto& ObjectInitialer = ThreadContext.TopInitializerChecked();
							auto Obj = ObjectInitialer.GetObj();
							MapGuidToObject.Add(ObjectGuid, Obj);//add this before RestoreObjectData, so if object property is referencing this object then it's good to use
							RestoreObjectData(Obj, ObjectGuid, ObjectData);
							if (bIsComponent)
							{
								auto Comp = (UActorComponent*)Obj;
								OrignCreationMethod = Comp->CreationMethod;
								Comp->CreationMethod = EComponentCreationMethod::Native;
							}
						};

						CreatedNewObject = static_cast<UObject*>(StaticConstructObject_Internal(Params));
						if (bIsComponent)
						{
							auto Comp = (UActorComponent*)CreatedNewObject;
							Comp->CreationMethod = OrignCreationMethod;
						}
						//CreatedNewObject = NewObject<UObject>(*OuterObjectPtr, ObjectClass, ObjectData.ObjectName, (EObjectFlags)ObjectData.ObjectFlags);
					}
					else
					{
						UE_LOG(LGUI, Warning, TEXT("[%s].%d Missing Outer object when creating object: '%s'. Prefab: '%s'"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(ObjectData.ObjectName.ToString()), *PrefabAssetPath);
						continue;
					}
				}
			}
			if (auto CreatedNewComponent = Cast<UActorComponent>(CreatedNewObject))
			{
				FComponentDataStruct CompData;
				CompData.Component = CreatedNewComponent;
				if (auto ParentGuidPtr = MapSceneComponentToParent.Find(ObjectGuid))
				{
					CompData.SceneComponentParentGuid = *ParentGuidPtr;
				}
				CreatedComponents.Add(CompData);
			}
		}
	}

	AActor* ActorSerializer::GenerateActorRecursive(FLGUIActorSaveData& InActorData, TMap<FGuid, FLGUIObjectSaveData>& SavedObjects, USceneComponent* Parent, FGuid ParentGuid)
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

#if WITH_EDITOR
					if (SubPrefabAsset->PrefabVersion < (uint16)ELGUIPrefabVersion::CommonActor)
					{
						SubPrefabAsset->RecreatePrefab();//if is old version then recreate to make it new version
					}
#endif
					//sub prefab
					{
						auto& SubMapGuidToObject = SubPrefabData.MapGuidToObject;
						TMap<FGuid, FGuid> MapObjectGuidFromSubPrefabToParentPrefab;
						for (auto& KeyValue : InActorData.MapObjectGuidFromParentPrefabToSubPrefab)
						{
							MapObjectGuidFromSubPrefabToParentPrefab.Add(KeyValue.Value, KeyValue.Key);
						}
#if WITH_EDITOR
						//edit mode must check if the object already exist, because the deserialize process could happen when use revert-prefab
						if (bIsEditorOrRuntime)
						{
							for (auto& KeyValue : MapObjectGuidFromSubPrefabToParentPrefab)
							{
								auto ObjectPtr = MapGuidToObject.Find(KeyValue.Value);
								if (!SubMapGuidToObject.Contains(KeyValue.Key) && ObjectPtr != nullptr)
								{
									SubMapGuidToObject.Add(KeyValue.Key, *ObjectPtr);
								}
							}
						}
#endif

						auto GetObjectGuidInParent = [&](const FGuid& GuidInSubPrefab) {
							FGuid GuidInParent;
							auto ObjectGuidInParentPrefabPtr = MapObjectGuidFromSubPrefabToParentPrefab.Find(GuidInSubPrefab);
							if (ObjectGuidInParentPrefabPtr == nullptr)
							{
								GuidInParent = FGuid::NewGuid();
								MapObjectGuidFromSubPrefabToParentPrefab.Add(GuidInSubPrefab, GuidInParent);
							}
							else
							{
								GuidInParent = *ObjectGuidInParentPrefabPtr;
							}
							return GuidInParent;
							};
						auto NewOnSubPrefabFinishDeserializeFunction =
							[&](AActor*, const TMap<FGuid, TObjectPtr<UObject>>& InSubPrefabMapGuidToObject, const TArray<AActor*>& InSubCreatedActors) {
							//collect sub prefab's object and guid to parent map, so all objects are ready when set override parameters
							for (auto& KeyValue : InSubPrefabMapGuidToObject)
							{
								auto& GuidInSubPrefab = KeyValue.Key;
								auto& ObjectInSubPrefab = KeyValue.Value;

								auto GuidInParent = GetObjectGuidInParent(GuidInSubPrefab);

								SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab.Add(GuidInParent, GuidInSubPrefab);
								SubPrefabData.MapGuidToObject.Add(GuidInSubPrefab, ObjectInSubPrefab);
								if (!MapGuidToObject.Contains(GuidInParent))
								{
									MapGuidToObject.Add(GuidInParent, ObjectInSubPrefab);
								}
							}
							//collect sub-prefab's actor to parent prefab
							CreatedActors.Append(InSubCreatedActors);
						};

						//restore sub-prefab's override parameters
						auto NewOnSubPrefabDeserializeObjectFunction =
							[&](const FGuid& GuidInSubPrefab, UObject* ObjectInSubPrefab) {

							auto GuidInParent = GetObjectGuidInParent(GuidInSubPrefab);

							if (auto RecordDataPtr = InActorData.MapObjectGuidToSubPrefabOverrideParameter.Find(GuidInParent))
							{
								WriterOrReaderFunctionForSubPrefabOverride(ObjectInSubPrefab, RecordDataPtr->OverrideParameterData, RecordDataPtr->OverrideParameterNames);
								
								FLGUIPrefabOverrideParameterData OverrideDataItem;
								OverrideDataItem.MemberPropertyNames = RecordDataPtr->OverrideParameterNames;
								OverrideDataItem.Object = ObjectInSubPrefab;
								SubPrefabData.ObjectOverrideParameterArray.Add(OverrideDataItem);

								FSubPrefabObjectOverrideParameterData OverrideData;
								OverrideData.Object = ObjectInSubPrefab;
								OverrideData.ParameterDatas = RecordDataPtr->OverrideObjectReferenceParameterData;
								OverrideData.ParameterNames = RecordDataPtr->OverrideParameterNames;
								SubPrefabOverrideParameters.Add(OverrideData);//collect override parameters, so when all objects are generated, restore these parameters will get all value back
							}
							};
						SubPrefabRootActor = LGUIPrefabSystem6::ActorSerializer::LoadSubPrefab(this->TargetWorld, SubPrefabAsset, Parent, LoadedRootActor, this->ActorIndexInPrefab, SubMapGuidToObject
							, NewOnSubPrefabFinishDeserializeFunction, NewOnSubPrefabDeserializeObjectFunction
						);
					}
					FComponentDataStruct CompData;
					CompData.Component = SubPrefabRootActor->GetRootComponent();
					CompData.SceneComponentParentGuid = ParentGuid;
					SubPrefabRootComponents.Add(CompData);

					SubPrefabMap.Add(SubPrefabRootActor, SubPrefabData);

					return SubPrefabRootActor;
				}
			}
		}
		else
		{
			if (auto ActorClass = FindClassFromListByIndex(InActorData.ObjectClass))
			{
				if (!ActorClass->IsChildOf(AActor::StaticClass()))//if not the right class, use default
				{
					UE_LOG(LGUI, Warning, TEXT("[%s].%d Find class: '%s' at index: %d, but is not a Actor class, use default. Prefab: '%s'"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(ActorClass->GetFName().ToString()), InActorData.ObjectClass, *PrefabAssetPath);
					ActorClass = AActor::StaticClass();
				}

				auto RestoreActorData = [&](AActor* TargetActor) {
					//Collect default sub objects
					TArray<UObject*> DefaultSubObjects;
					TargetActor->CollectDefaultSubobjects(DefaultSubObjects);
					for (auto DefaultSubObject : DefaultSubObjects)
					{
						if (DefaultSubObject->HasAnyFlags(EObjectFlags::RF_Transient))continue;
						auto Index = InActorData.DefaultSubObjectNameArray.IndexOfByKey(DefaultSubObject->GetFName());
						if (Index == INDEX_NONE)
						{
							UE_LOG(LGUI, Warning, TEXT("[%s].%d Missing guid for default sub object: %s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(DefaultSubObject->GetFName().ToString()));
							continue;
						}
						auto DefaultSubObjectGuid = InActorData.DefaultSubObjectGuidArray[Index];
						MapGuidToObject.Add(DefaultSubObjectGuid, DefaultSubObject);
						WriterOrReaderFunction(DefaultSubObject, SavedObjects[DefaultSubObjectGuid].PropertyData, Cast<USceneComponent>(DefaultSubObject) != nullptr);
						AlreadyRestoredObject.Add(DefaultSubObject);
						if (OnSubPrefabDeserializeObjectFunction != nullptr)
						{
							OnSubPrefabDeserializeObjectFunction(DefaultSubObjectGuid, DefaultSubObject);
						}
					}
					//restore object data after default-sub-object, so if property referencing default-sub-object it's good to use
					WriterOrReaderFunction(TargetActor, InActorData.PropertyData, false);
					if (OnSubPrefabDeserializeObjectFunction != nullptr)
					{
						OnSubPrefabDeserializeObjectFunction(InActorData.ActorGuid, TargetActor);
					}
				};

				AActor* NewActor = nullptr;
				if (auto ActorPtr = MapGuidToObject.Find(InActorData.ActorGuid))//MapGuidToObject can passed from LoadPrefabForEdit, so we need to find from map first
				{
					NewActor = (AActor*)(*ActorPtr);
					RestoreActorData(NewActor);
				}
				else
				{
					FActorSpawnParameters Spawnparameters;
					Spawnparameters.ObjectFlags = (EObjectFlags)InActorData.ObjectFlags;
					Spawnparameters.bDeferConstruction = false;
					Spawnparameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
					bool bIsBlueprintActor = ActorClass->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !ActorClass->HasAnyClassFlags(CLASS_Native);
					if (!bIsBlueprintActor)
					{
						Spawnparameters.CustomPreSpawnInitalization = [&](AActor* Actor) {
							MapGuidToObject.Add(InActorData.ActorGuid, Actor);//add this before RestoreActorData, so if object property is referencing this object then it's good to use
							RestoreActorData(Actor);
						};
					}
#if WITH_EDITOR
					//ref: LevelActor.cpp::SpawnActor 
					//LGUI's editor preview world (or other simple world (not UE5's open world)) don't need external actor, so we need to remove the flag, or game will crash when check external package.
					if ((Spawnparameters.ObjectFlags & EObjectFlags::RF_HasExternalPackage) != 0
						&& !TargetWorld->GetCurrentLevel()->IsUsingExternalActors()
						)
					{
						Spawnparameters.ObjectFlags = Spawnparameters.ObjectFlags & (~EObjectFlags::RF_HasExternalPackage);
					}
#endif
					NewActor = TargetWorld->SpawnActor<AActor>(ActorClass, Spawnparameters);
					if (bIsBlueprintActor)//blueprint should restore data after spawn, because components are created here
					{
						MapGuidToObject.Add(InActorData.ActorGuid, NewActor);//add this before RestoreActorData, so if object property is referencing this object then it's good to use
						RestoreActorData(NewActor);
					}
				}

				if (LoadedRootActor == nullptr)
				{
					LoadedRootActor = NewActor;
					LGUIManagerActor->BeginPrefabSystemProcessingActor(LoadedRootActor);
				}

				LGUIManagerActor->AddActorForPrefabSystem(NewActor, LoadedRootActor, ActorIndexInPrefab);

				if (auto RootComp = NewActor->GetRootComponent())
				{
					if (!MapGuidToObject.Contains(InActorData.RootComponentGuid))//RootComponent could be a BlueprintCreatedComponent, so check it
					{
						MapGuidToObject.Add(InActorData.RootComponentGuid, RootComp);
					}
				}

				CreatedActors.Add(NewActor);
				ActorIndexInPrefab++;

				NewActor->AttachToComponent(Parent, FAttachmentTransformRules::KeepRelativeTransform);

				for (auto& ChildSaveData : InActorData.ChildrenActorDataArray)
				{
					GenerateActorRecursive(ChildSaveData, SavedObjects, NewActor->GetRootComponent(), InActorData.RootComponentGuid);
				}

				return NewActor;
			}
			else
			{
				UE_LOG(LGUI, Warning, TEXT("[%s].%d Actor Class of index:%d not found! Prefab: '%s'"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, (InActorData.ObjectClass), *PrefabAssetPath);
			}
			}
			return nullptr;
	}
}
#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_ENABLE_OPTIMIZATION
#endif