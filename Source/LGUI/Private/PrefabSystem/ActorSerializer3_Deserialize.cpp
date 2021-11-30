// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "PrefabSystem/ActorSerializer3.h"
#include "PrefabSystem/LGUIPrefabHelperComponent.h"
#include "PrefabSystem/LGUIPrefabHelperActor.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/BufferArchive.h"
#include "Components/PrimitiveComponent.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Core/Actor/LGUIManagerActor.h"

//PRAGMA_DISABLE_OPTIMIZATION
namespace LGUIPrefabSystem3
{
#if WITH_EDITOR
	AActor* ActorSerializer3::LoadPrefabForEdit(UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent
		, TMap<FGuid, UObject*>& InOutMapGuidToObjects
	)
	{
		ActorSerializer3 serializer(InWorld);
		serializer.MapGuidToObject = InOutMapGuidToObjects;
		serializer.WriterOrReaderFunction = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? serializer.GetSceneComponentExcludeProperties() : TSet<FName>();
			FLGUIObjectReader Writer(InObject, InOutBuffer, serializer, ExcludeProperties);
		};
		auto rootActor = serializer.DeserializeActor(Parent, InPrefab, false, FVector::ZeroVector, FQuat::Identity, FVector::OneVector);
		InOutMapGuidToObjects = serializer.MapGuidToObject;
		return rootActor;
	}
#endif
	AActor* ActorSerializer3::LoadPrefab(UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent, bool SetRelativeTransformToIdentity, TFunction<void(AActor*)> CallbackBeforeAwake)
	{
		ActorSerializer3 serializer(InWorld);
		serializer.CallbackBeforeAwake = CallbackBeforeAwake;
#if !WITH_EDITOR
		serializer.bIsEditorOrRuntime = false;
#endif
		serializer.WriterOrReaderFunction = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? serializer.GetSceneComponentExcludeProperties() : TSet<FName>();
			FLGUIObjectReader Writer(InObject, InOutBuffer, serializer, ExcludeProperties);
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
		serializer.WriterOrReaderFunction = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? serializer.GetSceneComponentExcludeProperties() : TSet<FName>();
			FLGUIObjectReader Writer(InObject, InOutBuffer, serializer, ExcludeProperties);
		};
		return serializer.DeserializeActor(Parent, InPrefab, true, RelativeLocation, RelativeRotation, RelativeScale);
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

		PreGenerateActorRecursive(SaveData.SavedActor);
		PreGenerateObjectArray(SaveData.SavedObjects, SaveData.SavedComponents);
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

		if (CreatedRootActor != nullptr)
		{
			if (USceneComponent* RootComp = CreatedRootActor->GetRootComponent())
			{
				if (Parent)
				{
					RootComp->AttachToComponent(Parent, FAttachmentTransformRules::KeepRelativeTransform);
				}
				RootComp->UpdateComponentToWorld();
				if (ReplaceTransform)
				{
					RootComp->SetRelativeLocationAndRotation(InLocation, InRotation);
					RootComp->SetRelativeScale3D(InScale);
				}
			}
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
				CreatedNewComponent = (UActorComponent*)*CompPtr;
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
					CreatedNewComponent = NewObject<UActorComponent>(Outer, ObjectClass, ObjectData.ComponentName, RF_Transactional);//@todo: what about object flag?
					MapGuidToObject.Add(ObjectData.ComponentGuid, CreatedNewComponent);
				}
			}

			if (CreatedNewComponent)
			{
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
					check(Outer != nullptr);
					CreatedNewObject = NewObject<UObject>(Outer, ObjectClass, NAME_None, RF_Transactional);//@todo: what about object flag?
					MapGuidToObject.Add(ObjectData.ObjectGuid, CreatedNewObject);
				}
			}

			if (CreatedNewObject)
			{
				WriterOrReaderFunction(CreatedNewObject, ObjectData.PropertyData, false);
			}
		}
	}
	void ActorSerializer3::PreGenerateActorRecursive(FLGUIActorSaveData& SaveData)
	{
		if (auto ActorClass = FindClassFromListByIndex(SaveData.ActorClass))
		{
			if (!ActorClass->IsChildOf(AActor::StaticClass()))//if not the right class, use default
			{
				ActorClass = AActor::StaticClass();
				UE_LOG(LGUI, Error, TEXT("[ActorSerializer3::PreGenerateActorRecursive]Class:%s is not a Actor, use default"), *(ActorClass->GetFName().ToString()));
			}

			AActor* NewActor = nullptr;
			if (auto ActorPtr = MapGuidToObject.Find(SaveData.ActorGuid))//MapGuidToObject can passed from LoadPrefabForEdit, so we need to find from map first
			{
				NewActor = (AActor*)*ActorPtr;
			}
			else
			{
				NewActor = TargetWorld->SpawnActorDeferred<AActor>(ActorClass, FTransform::Identity);
				NewActor->FinishSpawning(FTransform::Identity, false);
				MapGuidToObject.Add(SaveData.ActorGuid, NewActor);
			}

			//Collect default sub objects after spawn actor
			TArray<UObject*> DefaultSubObjects;
			NewActor->CollectDefaultSubobjects(DefaultSubObjects, true);
			for (auto DefaultSubObject : DefaultSubObjects)
			{
				auto Index = SaveData.DefaultSubObjectNameArray.IndexOfByKey(DefaultSubObject->GetFName());
				if (Index == INDEX_NONE)
				{
					UE_LOG(LGUI, Warning, TEXT("[ActorSerializer3::PreGenerateActorRecursive]This blueprint (%s) seems broken"), *(NewActor->GetPathName()));
					continue;
				}
				MapGuidToObject.Add(SaveData.DefaultSubObjectGuidArray[Index], DefaultSubObject);
			}
			if (auto RootComp = NewActor->GetRootComponent())
			{
				if (!MapGuidToObject.Contains(SaveData.RootComponentGuid))//RootComponent could be a BlueprintCreatedComponent, so check it
				{
					MapGuidToObject.Add(SaveData.RootComponentGuid, RootComp);
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
			CreatedActorsGuid.Add(SaveData.ActorGuid);

			for (auto ChildSaveData : SaveData.ChildActorData)
			{
				PreGenerateActorRecursive(ChildSaveData);
			}
		}
		else
		{
			UE_LOG(LGUI, Warning, TEXT("[ActorSerializer3::PreGenerateActorRecursive]Actor Class of index:%d not found!"), (SaveData.ActorClass));
		}
	}
	AActor* ActorSerializer3::DeserializeActorRecursive(FLGUIActorSaveData& SaveData)
	{
		auto NewActor = (AActor*)MapGuidToObject[SaveData.ActorGuid];
		WriterOrReaderFunction(NewActor, SaveData.ActorPropertyData, false);

		for (auto ChildSaveData : SaveData.ChildActorData)
		{
			DeserializeActorRecursive(ChildSaveData);
		}
		return NewActor;
	}
}
//PRAGMA_ENABLE_OPTIMIZATION
