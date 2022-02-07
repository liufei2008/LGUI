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
#include "Core/ActorComponent/UIItem.h"

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_DISABLE_OPTIMIZATION
#endif
namespace LGUIPrefabSystem3
{
	AActor* ActorSerializer3::LoadPrefabWithExistingObjects(UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent
		, TMap<FGuid, UObject*>& InOutMapGuidToObjects, TMap<AActor*, FLGUISubPrefabData>& OutSubPrefabMap
		, bool InSetHierarchyIndexForRootComponent
	)
	{
		ActorSerializer3 serializer;
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
		serializer.WriterOrReaderFunction = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? serializer.GetSceneComponentExcludeProperties() : TSet<FName>();
			FLGUIObjectReader Reader(InOutBuffer, serializer, ExcludeProperties);
			Reader.DoSerialize(InObject);
		};
		serializer.WriterOrReaderFunctionForSubPrefab = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, const TSet<FName>& InOverridePropertyNameSet) {
			FLGUIOverrideParameterObjectReader Reader(InOutBuffer, serializer, InOverridePropertyNameSet);
			Reader.DoSerialize(InObject);
		};
		serializer.bSetHierarchyIndexForRootComponent = InSetHierarchyIndexForRootComponent;
		auto rootActor = serializer.DeserializeActor(Parent, InPrefab, false, FVector::ZeroVector, FQuat::Identity, FVector::OneVector);
		InOutMapGuidToObjects = serializer.MapGuidToObject;
		OutSubPrefabMap = serializer.SubPrefabMap;
		return rootActor;
	}

	AActor* ActorSerializer3::LoadPrefab(UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent, bool SetRelativeTransformToIdentity, TFunction<void(AActor*)> CallbackBeforeAwake)
	{
		ActorSerializer3 serializer;
		serializer.TargetWorld = InWorld;
		serializer.CallbackBeforeAwake = CallbackBeforeAwake;
#if !WITH_EDITOR
		serializer.bIsEditorOrRuntime = false;
#endif
		serializer.WriterOrReaderFunction = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? serializer.GetSceneComponentExcludeProperties() : TSet<FName>();
			FLGUIObjectReader Reader(InOutBuffer, serializer, ExcludeProperties);
			Reader.DoSerialize(InObject);
		};
		serializer.WriterOrReaderFunctionForSubPrefab = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, const TSet<FName>& InOverridePropertyNameSet) {
			FLGUIOverrideParameterObjectReader Reader(InOutBuffer, serializer, InOverridePropertyNameSet);
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
	AActor* ActorSerializer3::LoadPrefab(UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent, FVector RelativeLocation, FQuat RelativeRotation, FVector RelativeScale, TFunction<void(AActor*)> CallbackBeforeAwake)
	{
		ActorSerializer3 serializer;
		serializer.TargetWorld = InWorld;
		serializer.CallbackBeforeAwake = CallbackBeforeAwake;
#if !WITH_EDITOR
		serializer.bIsEditorOrRuntime = false;
#endif
		serializer.WriterOrReaderFunction = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? serializer.GetSceneComponentExcludeProperties() : TSet<FName>();
			FLGUIObjectReader Reader(InOutBuffer, serializer, ExcludeProperties);
			Reader.DoSerialize(InObject);
		};
		serializer.WriterOrReaderFunctionForSubPrefab = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, const TSet<FName>& InOverridePropertyNameSet) {
			FLGUIOverrideParameterObjectReader Reader(InOutBuffer, serializer, InOverridePropertyNameSet);
			Reader.DoSerialize(InObject);
		};
		return serializer.DeserializeActor(Parent, InPrefab, true, RelativeLocation, RelativeRotation, RelativeScale);
	}
	AActor* ActorSerializer3::LoadSubPrefab(
		UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent
		, AActor* InParentLoadedRootActor
		, TMap<FGuid, UObject*>& InMapGuidToObject
		, TFunction<void(AActor*, const TMap<FGuid, UObject*>&)> InOnSubPrefabFinishDeserializeFunction
	)
	{
		ActorSerializer3 serializer;
		serializer.TargetWorld = InWorld;
#if !WITH_EDITOR
		serializer.bIsEditorOrRuntime = false;
#endif
		serializer.bSetHierarchyIndexForRootComponent = false;
		serializer.MapGuidToObject = InMapGuidToObject;
		serializer.LoadedRootActor = InParentLoadedRootActor;
		serializer.bIsSubPrefab = true;
		serializer.WriterOrReaderFunction = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? serializer.GetSceneComponentExcludeProperties() : TSet<FName>();
			FLGUIObjectReader Reader(InOutBuffer, serializer, ExcludeProperties);
			Reader.DoSerialize(InObject);
		};
		serializer.WriterOrReaderFunctionForSubPrefab = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, const TSet<FName>& InOverridePropertyNameSet) {
			FLGUIOverrideParameterObjectReader Reader(InOutBuffer, serializer, InOverridePropertyNameSet);
			Reader.DoSerialize(InObject);
		};
		serializer.CallbackBeforeAwakeForSubPrefab = InOnSubPrefabFinishDeserializeFunction;
		auto rootActor = serializer.DeserializeActor(Parent, InPrefab, false, FVector::ZeroVector, FQuat::Identity, FVector::OneVector);
		return rootActor;
	}

	AActor* ActorSerializer3::DeserializeActorFromData(FLGUIPrefabSaveData& SaveData, USceneComponent* Parent, bool ReplaceTransform, FVector InLocation, FQuat InRotation, FVector InScale)
	{
		PreGenerateActorRecursive(SaveData.SavedActor, nullptr);
		PreGenerateObjectArray(SaveData.SavedObjects, SaveData.SavedComponents);
		DeserializeObjectArray(SaveData.SavedObjects, SaveData.SavedComponents);
		auto CreatedRootActor = DeserializeActorRecursive(SaveData.SavedActor);

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
					auto ParentComp = Cast<USceneComponent>(MapGuidToObject[CompData.SceneComponentParentGuid]);
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
				ALGUIManagerActor::RemoveActorForPrefabSystem(item, LoadedRootActor);
			}
			if (!bIsSubPrefab)
			{
				if (LoadedRootActor != nullptr)//if any error hanppens then LoadedRootActor could be nullptr, so check it
				{
					ALGUIManagerActor::EndPrefabSystemProcessingActor(TargetWorld, LoadedRootActor);
				}
			}
		}

		return CreatedRootActor;
	}
	AActor* ActorSerializer3::DeserializeActor(USceneComponent* Parent, ULGUIPrefab* InPrefab, bool ReplaceTransform, FVector InLocation, FQuat InRotation, FVector InScale)
	{
		if (!InPrefab)
		{
			UE_LOG(LGUI, Error, TEXT("Load Prefab, InPrefab is null!"));
			return nullptr;
		}
		if (!TargetWorld)
		{
			UE_LOG(LGUI, Error, TEXT("Load Prefab, World is null!"));
			return nullptr;
		}

		auto StartTime = FDateTime::Now();

#if WITH_EDITOR
		if (bIsEditorOrRuntime)
		{
			//fill new reference data
			this->ReferenceAssetList = InPrefab->ReferenceAssetList;
			this->ReferenceClassList = InPrefab->ReferenceClassList;
			this->ReferenceNameList = InPrefab->ReferenceNameList;
		}
		else
#endif
		{
			//fill new reference data
			this->ReferenceAssetList = InPrefab->ReferenceAssetListForBuild;
			this->ReferenceClassList = InPrefab->ReferenceClassListForBuild;
			this->ReferenceNameList = InPrefab->ReferenceNameListForBuild;
		}

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
			FromBinary << SaveData;
		}
		auto CreatedRootActor = DeserializeActorFromData(SaveData, Parent, ReplaceTransform, InLocation, InRotation, InScale);
		
		auto TimeSpan = FDateTime::Now() - StartTime;
		UE_LOG(LGUI, Log, TEXT("Take %fs loading prefab: %s"), TimeSpan.GetTotalSeconds(), *InPrefab->GetName());

#if WITH_EDITOR
#if ENGINE_MAJOR_VERSION >= 5
		GEditor->BroadcastLevelActorListChanged();//UE5 will not auto refresh scene outliner and display actor label, so manually refresh it.
#endif
#endif

		return CreatedRootActor;
	}




	void ActorSerializer3::PreGenerateObjectArray(const TArray<FLGUIObjectSaveData>& SavedObjects, const TArray<FLGUIComponentSaveData>& SavedComponents)
	{
		//create component first, because some object may use component as outer
		for (auto& ObjectData : SavedComponents)
		{
			UActorComponent* CreatedNewComponent = nullptr;
			if (auto CompPtr = MapGuidToObject.Find(ObjectData.ComponentGuid))
			{
				//UE_LOG(LGUI, Log, TEXT("[ActorSerializer3::PreGenerateObjectArray]Already generated:%s!"), *(MapGuidToObject[ObjectData.ComponentGuid]->GetPathName()));
				CreatedNewComponent = Cast<UActorComponent>(*CompPtr);
			}
			else
			{
				if (auto ObjectClass = FindClassFromListByIndex(ObjectData.ComponentClass))
				{
					if (!ObjectClass->IsChildOf(UActorComponent::StaticClass())
						)
					{
						UE_LOG(LGUI, Error, TEXT("[ActorSerializer3::PreGenerateObjectArray]Wrong class:%s!"), *(ObjectClass->GetFName().ToString()));
						continue;
					}

					auto Outer = MapGuidToObject[ObjectData.OuterObjectGuid];
					CreatedNewComponent = NewObject<UActorComponent>(Outer, ObjectClass, ObjectData.ComponentName, (EObjectFlags)ObjectData.ObjectFlags);
					MapGuidToObject.Add(ObjectData.ComponentGuid, CreatedNewComponent);
				}
			}
		}

		for (auto& ObjectData : SavedObjects)
		{
			UObject* CreatedNewObject = nullptr;
			if (auto ObjectPtr = MapGuidToObject.Find(ObjectData.ObjectGuid))
			{
				//UE_LOG(LGUI, Log, TEXT("[ActorSerializer3::PreGenerateObjectArray]Already generated:%s!"), *(MapGuidToObject[ObjectData.ObjectGuid]->GetPathName()));
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
						UE_LOG(LGUI, Error, TEXT("[ActorSerializer3::PreGenerateObjectArray]Wrong class:%s!"), *(ObjectClass->GetFName().ToString()));
						continue;
					}

					auto Outer = MapGuidToObject[ObjectData.OuterObjectGuid];
					CreatedNewObject = NewObject<UObject>(Outer, ObjectClass, NAME_None, (EObjectFlags)ObjectData.ObjectFlags);
					MapGuidToObject.Add(ObjectData.ObjectGuid, CreatedNewObject);
				}
			}
		}
	}

	void ActorSerializer3::DeserializeObjectArray(const TArray<FLGUIObjectSaveData>& SavedObjects, const TArray<FLGUIComponentSaveData>& SavedComponents)
	{
		//create component first, because some object may use component as outer
		for (auto ObjectData : SavedComponents)
		{
			if (auto CompPtr = MapGuidToObject.Find(ObjectData.ComponentGuid))
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

	void ActorSerializer3::PreGenerateActorRecursive(FLGUIActorSaveData& SavedActors, USceneComponent* Parent)
	{
		if (SavedActors.bIsPrefab)
		{
			auto PrefabIndex = SavedActors.PrefabAssetIndex;
			if (auto PrefabAssetObject = FindAssetFromListByIndex(PrefabIndex))
			{
				if (auto SubPrefabAsset = Cast<ULGUIPrefab>(PrefabAssetObject))
				{
					AActor* SubPrefabRootActor = nullptr;
					FLGUISubPrefabData SubPrefabData;
					SubPrefabData.PrefabAsset = SubPrefabAsset;
					TMap<FGuid, UObject*>& SubMapGuidToObject = SubPrefabData.MapGuidToObject;
					if (auto ValuePtr = MapGuidToObject.Find(SavedActors.ActorGuid))
					{
						auto SubPrefabRootActorGuid = SavedActors.MapObjectGuidFromParentPrefabToSubPrefab[SavedActors.ActorGuid];
						SubMapGuidToObject.Add(SubPrefabRootActorGuid, *ValuePtr);
					}
					TMap<FGuid, FGuid> MapObjectGuidFromSubPrefabToParentPrefab;
					for (auto& KeyValue : SavedActors.MapObjectGuidFromParentPrefabToSubPrefab)
					{
						MapObjectGuidFromSubPrefabToParentPrefab.Add(KeyValue.Value, KeyValue.Key);
						auto ObjectPtr = MapGuidToObject.Find(KeyValue.Key);
						if (!SubMapGuidToObject.Contains(KeyValue.Value) && ObjectPtr != nullptr)
						{
							SubMapGuidToObject.Add(KeyValue.Value, *ObjectPtr);
						}
					}
					SubPrefabRootActor = ActorSerializer3::LoadSubPrefab(this->TargetWorld, SubPrefabAsset, Parent
						, LoadedRootActor
						, SubMapGuidToObject
						, [&](AActor* InSubPrefabRootActor, const TMap<FGuid, UObject*>& InSubMapGuidToObject) {
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

							for (auto& RecordData : SavedActors.ObjectOverrideParameterArray)
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
						}
					);

					SubPrefabMap.Add(SubPrefabRootActor, SubPrefabData);
				}
			}
		}
		else
		{
			if (auto ActorClass = FindClassFromListByIndex(SavedActors.ActorClass))
			{
				if (!ActorClass->IsChildOf(AActor::StaticClass()))//if not the right class, use default
				{
					ActorClass = AActor::StaticClass();
					UE_LOG(LGUI, Error, TEXT("[ActorSerializer3::PreGenerateActorRecursive]Class:%s is not a Actor, use default"), *(ActorClass->GetFName().ToString()));
				}

				AActor* NewActor = nullptr;
				bool bNeedFinishSpawn = false;
				if (auto ActorPtr = MapGuidToObject.Find(SavedActors.ActorGuid))//MapGuidToObject can passed from LoadPrefabForEdit, so we need to find from map first
				{
					NewActor = (AActor*)(*ActorPtr);
				}
				else
				{
					FActorSpawnParameters Spawnparameters;
					Spawnparameters.ObjectFlags = (EObjectFlags)SavedActors.ObjectFlags;
					Spawnparameters.bDeferConstruction = true;
					Spawnparameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
					NewActor = TargetWorld->SpawnActor<AActor>(ActorClass, Spawnparameters);
					bNeedFinishSpawn = true;
					MapGuidToObject.Add(SavedActors.ActorGuid, NewActor);
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
						ALGUIManagerActor::BeginPrefabSystemProcessingActor(TargetWorld, LoadedRootActor);
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
					ALGUIManagerActor::AddActorForPrefabSystem(NewActor, LoadedRootActor);
				}
				if (bNeedFinishSpawn)
				{
					NewActor->FinishSpawning(FTransform::Identity);
				}

				//Collect default sub objects
				TArray<UObject*> DefaultSubObjects;
				NewActor->CollectDefaultSubobjects(DefaultSubObjects, true);
				for (auto DefaultSubObject : DefaultSubObjects)
				{
					auto Index = SavedActors.DefaultSubObjectNameArray.IndexOfByKey(DefaultSubObject->GetFName());
					if (Index == INDEX_NONE)
					{
						UE_LOG(LGUI, Warning, TEXT("[ActorSerializer3::PreGenerateActorRecursive]Missing guid for default sub object: %s"), *(DefaultSubObject->GetFName().ToString()));
						continue;
					}
					MapGuidToObject.Add(SavedActors.DefaultSubObjectGuidArray[Index], DefaultSubObject);
				}
				if (auto RootComp = NewActor->GetRootComponent())
				{
					if (!MapGuidToObject.Contains(SavedActors.RootComponentGuid))//RootComponent could be a BlueprintCreatedComponent, so check it
					{
						MapGuidToObject.Add(SavedActors.RootComponentGuid, RootComp);
					}
				}

				CreatedActors.Add(NewActor);
				CreatedActorsGuid.Add(SavedActors.ActorGuid);

				for (auto& ChildSaveData : SavedActors.ChildActorData)
				{
					PreGenerateActorRecursive(ChildSaveData, NewActor->GetRootComponent());
				}
			}
			else
			{
				UE_LOG(LGUI, Error, TEXT("[ActorSerializer3::PreGenerateActorRecursive]Actor Class of index:%d not found!"), (SavedActors.ActorClass));
			}
		}
	}
	AActor* ActorSerializer3::DeserializeActorRecursive(FLGUIActorSaveData& SavedActors)
	{
		auto NewActor = (AActor*)MapGuidToObject[SavedActors.ActorGuid];
		WriterOrReaderFunction(NewActor, SavedActors.ActorPropertyData, false);

		for (auto& ChildSaveData : SavedActors.ChildActorData)
		{
			DeserializeActorRecursive(ChildSaveData);
		}
		return NewActor;
	}
}
#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_ENABLE_OPTIMIZATION
#endif
