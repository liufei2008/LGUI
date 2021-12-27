// Copyright 2019-2021 LexLiu. All Rights Reserved.

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
#include "PrefabSystem/LGUIPrefabOverrideParameter.h"
#include "Core/ActorComponent/UIItem.h"

//PRAGMA_DISABLE_OPTIMIZATION
namespace LGUIPrefabSystem3
{
	AActor* ActorSerializer3::LoadPrefabForEdit(UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent
		, TMap<FGuid, TWeakObjectPtr<UObject>>& InOutMapGuidToObjects, TMap<TWeakObjectPtr<AActor>, FLGUISubPrefabData>& OutSubPrefabMap
		, const TArray<uint8>& InOverrideParameterData, TWeakObjectPtr<ULGUIPrefabOverrideParameterObject>& OutOverrideParameterObject
	)
	{
		ActorSerializer3 serializer(InWorld);
		for (auto KeyValue : InOutMapGuidToObjects)//Preprocess the map, ignore invalid object
		{
			if (KeyValue.Value.IsValid())
			{
				serializer.MapGuidToObject.Add(KeyValue.Key, KeyValue.Value);
			}
		}
		serializer.WriterOrReaderFunction = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? serializer.GetSceneComponentExcludeProperties() : TSet<FName>();
			FLGUIObjectReader Reader(InObject, InOutBuffer, serializer, ExcludeProperties);
		};
		serializer.bIsLoadForEdit = true;
		serializer.OverrideParameterData = InOverrideParameterData;
		auto rootActor = serializer.DeserializeActor(Parent, InPrefab, false, FVector::ZeroVector, FQuat::Identity, FVector::OneVector);
		InOutMapGuidToObjects = serializer.MapGuidToObject;
		OutSubPrefabMap = serializer.SubPrefabMap;
		OutOverrideParameterObject = serializer.OverrideParameterObject;
		return rootActor;
	}

	AActor* ActorSerializer3::LoadPrefab(UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent, bool SetRelativeTransformToIdentity, TFunction<void(AActor*)> CallbackBeforeAwake)
	{
		ActorSerializer3 serializer(InWorld);
		serializer.CallbackBeforeAwake = CallbackBeforeAwake;
#if !WITH_EDITOR
		serializer.bIsEditorOrRuntime = false;
#endif
		serializer.bIsLoadForEdit = false;
		serializer.WriterOrReaderFunction = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? serializer.GetSceneComponentExcludeProperties() : TSet<FName>();
			FLGUIObjectReader Reader(InObject, InOutBuffer, serializer, ExcludeProperties);
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
		ActorSerializer3 serializer(InWorld);
		serializer.CallbackBeforeAwake = CallbackBeforeAwake;
#if !WITH_EDITOR
		serializer.bIsEditorOrRuntime = false;
#endif
		serializer.bIsLoadForEdit = false;
		serializer.WriterOrReaderFunction = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? serializer.GetSceneComponentExcludeProperties() : TSet<FName>();
			FLGUIObjectReader Writer(InObject, InOutBuffer, serializer, ExcludeProperties);
		};
		return serializer.DeserializeActor(Parent, InPrefab, true, RelativeLocation, RelativeRotation, RelativeScale);
	}
	AActor* ActorSerializer3::LoadSubPrefab(
		UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent
		, TMap<FGuid, TWeakObjectPtr<UObject>>& InMapGuidToObject
		, TFunction<ULGUIPrefabOverrideParameterObject* (AActor*)> InGetDeserializedOverrideParameterObjectFunction
		, bool InIsLoadForEdit
	)
	{
		ActorSerializer3 serializer(InWorld);
		serializer.bApplyOverrideParameters = true;
		serializer.bIsLoadForEdit = InIsLoadForEdit;
		serializer.MapGuidToObject = InMapGuidToObject;
		serializer.WriterOrReaderFunction = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? serializer.GetSceneComponentExcludeProperties() : TSet<FName>();
			FLGUIObjectReader Reader(InObject, InOutBuffer, serializer, ExcludeProperties);
		};
		serializer.GetDeserializedOverrideParameterObjectFunction = InGetDeserializedOverrideParameterObjectFunction;
		auto rootActor = serializer.DeserializeActor(Parent, InPrefab, false, FVector::ZeroVector, FQuat::Identity, FVector::OneVector);
		return rootActor;
	}

	AActor* ActorSerializer3::DeserializeActorFromData(FLGUIPrefabSaveData& SaveData, USceneComponent* Parent, bool ReplaceTransform, FVector InLocation, FQuat InRotation, FVector InScale)
	{
#if WITH_EDITOR
		if (!TargetWorld->IsGameWorld())
		{
			ULGUIEditorManagerObject::BeginPrefabSystemProcessingActor(TargetWorld.Get());
		}
		else
#endif
		{
			ALGUIManagerActor::BeginPrefabSystemProcessingActor(TargetWorld.Get());
		}

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
				if (Parent)
				{
					RootComp->AttachToComponent(Parent, FAttachmentTransformRules::KeepRelativeTransform);
					//recreate hierarchy index, for sub prefab root UIItem, with sorted hierarchy order when serialize
					if (auto RootUIComp = Cast<UUIItem>(RootComp))
					{
						RootUIComp->SetAsLastHierarchy();
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
		
		if (GetDeserializedOverrideParameterObjectFunction != nullptr)
		{
			OverrideParameterObject = GetDeserializedOverrideParameterObjectFunction(CreatedRootActor);//sub prefab's override parameter is store/restore from parent prefab (which is nessary), so reference object inside sub prefab will lose
		}
		else
		{
			OverrideParameterObject = NewObject<ULGUIPrefabOverrideParameterObject>((UObject*)(Prefab.IsValid() ? Prefab.Get() : (UObject*)GetTransientPackage()));//prefab is null when use duplicate
			if (OverrideParameterData.Num() > 0)
			{
				WriterOrReaderFunction(OverrideParameterObject.Get(), OverrideParameterData, false);
			}
		}
		if (bApplyOverrideParameters)
		{
			auto TemplateOverrideParameterObject = NewObject<ULGUIPrefabOverrideParameterObject>();//@todo could use a persistent static object for all template
			WriterOrReaderFunction(TemplateOverrideParameterObject, Prefab->OverrideParameterData, false);
			OverrideParameterObject->SetParameterReferenceFromTemplate(TemplateOverrideParameterObject);//since sub prefab's reference object is lost, we should copy reference from template, which is created inside sub prefab so all reference is good
			OverrideParameterObject->ApplyParameter();
			TemplateOverrideParameterObject->ConditionalBeginDestroy();
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
			ULGUIEditorManagerObject::EndPrefabSystemProcessingActor();
		}
		else
#endif
		{
			for (auto item : CreatedActors)
			{
				ALGUIManagerActor::RemoveActorForPrefabSystem(item);
			}
			ALGUIManagerActor::EndPrefabSystemProcessingActor(TargetWorld.Get());
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
		if (!TargetWorld.IsValid())
		{
			UE_LOG(LGUI, Error, TEXT("Load Prefab, World is null!"));
			return nullptr;
		}

		auto StartTime = FDateTime::Now();

		Prefab = TWeakObjectPtr<ULGUIPrefab>(InPrefab);
		//fill new reference data
		this->ReferenceAssetList = Prefab->ReferenceAssetList;
		this->ReferenceClassList = Prefab->ReferenceClassList;
		this->ReferenceNameList = Prefab->ReferenceNameList;

		FLGUIPrefabSaveData SaveData;
		{
			auto LoadedData =
#if WITH_EDITOR
				bIsEditorOrRuntime ? Prefab->BinaryData :
#endif
				Prefab->BinaryDataForBuild;
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
		UE_LOG(LGUI, Log, TEXT("Take %fs loading prefab: %s"), TimeSpan.GetTotalSeconds(), *Prefab->GetName());

		return CreatedRootActor;
	}




	void ActorSerializer3::PreGenerateObjectArray(const TArray<FLGUIObjectSaveData>& SavedObjects, const TArray<FLGUIComponentSaveData>& SavedComponents)
	{
		//create component first, because some object may use component as outer
		for (auto ObjectData : SavedComponents)
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
					CreatedNewComponent = NewObject<UActorComponent>(Outer.Get(), ObjectClass, ObjectData.ComponentName, (EObjectFlags)ObjectData.ObjectFlags);
					MapGuidToObject.Add(ObjectData.ComponentGuid, CreatedNewComponent);
				}
			}
		}

		for (auto ObjectData : SavedObjects)
		{
			UObject* CreatedNewObject = nullptr;
			if (auto ObjectPtr = MapGuidToObject.Find(ObjectData.ObjectGuid))
			{
				//UE_LOG(LGUI, Log, TEXT("[ActorSerializer3::PreGenerateObjectArray]Already generated:%s!"), *(MapGuidToObject[ObjectData.ObjectGuid]->GetPathName()));
				CreatedNewObject = ObjectPtr->Get();
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
					CreatedNewObject = NewObject<UObject>(Outer.Get(), ObjectClass, NAME_None, (EObjectFlags)ObjectData.ObjectFlags);
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
				auto CreatedNewComponent = (UActorComponent*)(CompPtr->Get());
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
				WriterOrReaderFunction(CreatedNewObject.Get(), ObjectData.PropertyData, false);
			}
		}
	}

	void ActorSerializer3::PreGenerateActorRecursive(FLGUIActorSaveData& SavedActors, AActor* ParentActor)
	{
		if (SavedActors.bIsPrefab)
		{
			auto PrefabIndex = SavedActors.PrefabAssetIndex;
			if (auto PrefabAssetObject = FindAssetFromListByIndex(PrefabIndex))
			{
				if (auto PrefabAsset = Cast<ULGUIPrefab>(PrefabAssetObject))
				{
					AActor* SubPrefabRootActor = nullptr;
					FLGUISubPrefabData SubPrefabData;
					SubPrefabData.PrefabAsset = PrefabAsset;
					TMap<FGuid, TWeakObjectPtr<UObject>> SubMapGuidToObject;
					if (auto ValuePtr = MapGuidToObject.Find(SavedActors.ActorGuid))
					{
						SubMapGuidToObject.Add(SavedActors.ActorGuid, *ValuePtr);
					}
					ULGUIPrefabOverrideParameterObject* SubOverrideParameterObject = nullptr;
					SubPrefabRootActor = ActorSerializer3::LoadSubPrefab(this->TargetWorld.Get(), PrefabAsset, ParentActor->GetRootComponent()
						, SubMapGuidToObject
						, [&](AActor* InSubPrefabRootActor) {
							SubPrefabRootActor = InSubPrefabRootActor;
							//set MapGuidToObject before overrideParameter, or the actor reference will not be found
							if (MapGuidToObject.Contains(SavedActors.ActorGuid))
							{
								MapGuidToObject[SavedActors.ActorGuid] = SubPrefabRootActor;
							}
							else
							{
								MapGuidToObject.Add(SavedActors.ActorGuid, SubPrefabRootActor);
							}
							//map guid to components
							for (auto& BlueprintComp : InSubPrefabRootActor->BlueprintCreatedComponents)
							{
								auto FoundIndex = SavedActors.PrefabRootActorComponentNameArray.IndexOfByKey(BlueprintComp->GetFName());
								if (FoundIndex != INDEX_NONE)
								{
									MapGuidToObject.Add(SavedActors.PrefabRootActorComponentGuidArray[FoundIndex], BlueprintComp);
								}
							}
							for (auto& InstanceComp : InSubPrefabRootActor->GetInstanceComponents())
							{
								auto FoundIndex = SavedActors.PrefabRootActorComponentNameArray.IndexOfByKey(InstanceComp->GetFName());
								if (FoundIndex != INDEX_NONE)
								{
									MapGuidToObject.Add(SavedActors.PrefabRootActorComponentGuidArray[FoundIndex], InstanceComp);
								}
							}
							auto RootComp = SubPrefabRootActor->GetRootComponent();
							if (!MapGuidToObject.Contains(SavedActors.RootComponentGuid) && RootComp != nullptr)
							{
								MapGuidToObject.Add(SavedActors.RootComponentGuid, RootComp);
							}

							SubOverrideParameterObject = NewObject<ULGUIPrefabOverrideParameterObject>(Prefab.Get());
							WriterOrReaderFunction(SubOverrideParameterObject, SavedActors.PrefabOverrideParameterData, false);//use WriterOrReaderFunction from parent prefab, because when serailize(save) nested prefab, the subprefab's override parameter use parent's WriterOrReaderFunction
							return SubOverrideParameterObject;
						}
						, bIsLoadForEdit
					);
					SubPrefabData.OverrideParameterObject = SubOverrideParameterObject;
					SubPrefabData.OverrideParameterData = SavedActors.PrefabOverrideParameterData;

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
				if (auto ActorPtr = MapGuidToObject.Find(SavedActors.ActorGuid))//MapGuidToObject can passed from LoadPrefabForEdit, so we need to find from map first
				{
					NewActor = (AActor*)(ActorPtr->Get());
				}
				else
				{
					FActorSpawnParameters Spawnparameters;
					Spawnparameters.ObjectFlags = (EObjectFlags)SavedActors.ObjectFlags;
					NewActor = TargetWorld->SpawnActor<AActor>(ActorClass, Spawnparameters);
					MapGuidToObject.Add(SavedActors.ActorGuid, NewActor);
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

#if WITH_EDITOR
				if (!TargetWorld->IsGameWorld())
				{
					ULGUIEditorManagerObject::AddActorForPrefabSystem(NewActor);
				}
				else
#endif
				{
					ALGUIManagerActor::AddActorForPrefabSystem(NewActor);
				}
				CreatedActors.Add(NewActor);
				CreatedActorsGuid.Add(SavedActors.ActorGuid);

				for (auto ChildSaveData : SavedActors.ChildActorData)
				{
					PreGenerateActorRecursive(ChildSaveData, NewActor);
				}
			}
			else
			{
				UE_LOG(LGUI, Warning, TEXT("[ActorSerializer3::PreGenerateActorRecursive]Actor Class of index:%d not found!"), (SavedActors.ActorClass));
			}
		}
	}
	AActor* ActorSerializer3::DeserializeActorRecursive(FLGUIActorSaveData& SavedActors)
	{
		auto NewActor = (AActor*)MapGuidToObject[SavedActors.ActorGuid].Get();
		WriterOrReaderFunction(NewActor, SavedActors.ActorPropertyData, false);

		for (auto ChildSaveData : SavedActors.ChildActorData)
		{
			DeserializeActorRecursive(ChildSaveData);
		}
		return NewActor;
	}
}
//PRAGMA_ENABLE_OPTIMIZATION
