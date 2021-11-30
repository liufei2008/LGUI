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
#if WITH_EDITOR
#include "Tools/UEdMode.h"
#include "Utils/LGUIUtils.h"
#endif

PRAGMA_DISABLE_OPTIMIZATION
namespace LGUIPrefabSystem3
{
	void ActorSerializer3::SavePrefab(AActor* RootActor, ULGUIPrefab* InPrefab
		, TMap<UObject*, FGuid>& OutMapObjectToGuid)
	{
		if (!RootActor || !InPrefab)
		{
			UE_LOG(LGUI, Error, TEXT("[ActorSerializer3::SerializeActor]RootActor Or InPrefab is null!"));
			return;
		}
		if (!RootActor->GetWorld())
		{
			UE_LOG(LGUI, Error, TEXT("[ActorSerializer3::SerializeActor]Cannot get World from RootActor!"));
			return;
		}
		ActorSerializer3 serializer(RootActor->GetWorld());
		serializer.WriterOrReaderFunction = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? serializer.GetSceneComponentExcludeProperties() : TSet<FName>();
			FLGUIObjectWriter Writer(InObject, InOutBuffer, serializer, ExcludeProperties);
		};
		serializer.SerializeActor(RootActor, InPrefab);
		OutMapObjectToGuid = serializer.MapObjectToGuid;
	}

	void ActorSerializer3::SavePrefabForRuntime(AActor* RootActor, ULGUIPrefab* InPrefab)
	{
		if (!RootActor || !InPrefab)
		{
			UE_LOG(LGUI, Error, TEXT("[ActorSerializer3::SerializeActor]RootActor Or InPrefab is null!"));
			return;
		}
		if (!RootActor->GetWorld())
		{
			UE_LOG(LGUI, Error, TEXT("[ActorSerializer3::SerializeActor]Cannot get World from RootActor!"));
			return;
		}
		ActorSerializer3 serializer(RootActor->GetWorld());
		serializer.bIsEditorOrRuntime = false;
		serializer.WriterOrReaderFunction = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? serializer.GetSceneComponentExcludeProperties() : TSet<FName>();
			FLGUIObjectWriter Writer(InObject, InOutBuffer, serializer, ExcludeProperties);
		};
		serializer.SerializeActor(RootActor, InPrefab);
	}

	void ActorSerializer3::SerializeActorRecursive(AActor* Actor, FLGUIActorSaveData& OutActorSaveData)
	{
		auto ActorGuid = MapObjectToGuid[Actor];

		OutActorSaveData.ActorClass = FindOrAddClassFromList(Actor->GetClass());
		OutActorSaveData.ActorGuid = ActorGuid;
		WriterOrReaderFunction(Actor, OutActorSaveData.ActorPropertyData, false);
		if (auto RootComp = Actor->GetRootComponent())
		{
			OutActorSaveData.RootComponentGuid = MapObjectToGuid[RootComp];
		}
		TArray<UObject*> DefaultSubObjects;
		Actor->CollectDefaultSubobjects(DefaultSubObjects, true);
		for (auto DefaultSubObject : DefaultSubObjects)
		{
			OutActorSaveData.DefaultSubObjectGuidArray.Add(MapObjectToGuid[DefaultSubObject]);
			OutActorSaveData.DefaultSubObjectNameArray.Add(DefaultSubObject->GetFName());
		}

		TArray<AActor*> ChildrenActors;
		Actor->GetAttachedActors(ChildrenActors);
		TArray<FLGUIActorSaveData> ChildSaveDataList;
		for (auto ChildActor : ChildrenActors)
		{
#if WITH_EDITOR
			if (!SkippingActors.Contains(ChildActor))
#endif
			{
				FLGUIActorSaveData ChildActorSaveData;
				SerializeActorRecursive(ChildActor, ChildActorSaveData);
				ChildSaveDataList.Add(ChildActorSaveData);
			}
		}
		OutActorSaveData.ChildActorData = ChildSaveDataList;
	}
	void ActorSerializer3::SerializeActorToData(AActor* RootActor, FLGUIPrefabSaveData& OutData)
	{
		CollectActorAndComponentRecursive(RootActor);
		//serailize actor
		SerializeActorRecursive(RootActor, OutData.SavedActor);
		//serialize objects and components
		SerializeObjectArray(OutData.SavedObjects, OutData.SavedComponents);
	}
	void ActorSerializer3::SerializeActor(AActor* RootActor, ULGUIPrefab* InPrefab)
	{
		Prefab = TWeakObjectPtr<ULGUIPrefab>(InPrefab);

		auto StartTime = FDateTime::Now();

		FLGUIPrefabSaveData SaveData;
		SerializeActorToData(RootActor, SaveData);

		FBufferArchive ToBinary;
		ToBinary << SaveData;

		if (ToBinary.Num() <= 0)
		{
			UE_LOG(LGUI, Warning, TEXT("Save binary length is 0!"));
			return;
		}

#if WITH_EDITOR
		if (bIsEditorOrRuntime)
		{
			Prefab->BinaryData = ToBinary;
			Prefab->ThumbnailDirty = true;
			Prefab->CreateTime = FDateTime::Now();
		}
		else
#endif
		{
			Prefab->BinaryDataForBuild = ToBinary;
		}

		//clear old reference data
		Prefab->ReferenceAssetList.Empty();
		Prefab->ReferenceClassList.Empty();
		Prefab->ReferenceTextList.Empty();
		Prefab->ReferenceNameList.Empty();
		Prefab->ReferenceStringList.Empty();
		//fill new reference data
		Prefab->ReferenceAssetList = this->ReferenceAssetList;
		Prefab->ReferenceClassList = this->ReferenceClassList;
		Prefab->ReferenceNameList = this->ReferenceNameList;

		Prefab->EngineMajorVersion = ENGINE_MAJOR_VERSION;
		Prefab->EngineMinorVersion = ENGINE_MINOR_VERSION;
		Prefab->PrefabVersion = LGUI_PREFAB_VERSION;

		Prefab->MarkPackageDirty();
		auto TimeSpan = FDateTime::Now() - StartTime;
		UE_LOG(LGUI, Log, TEXT("Take %fs saving prefab: %s"), TimeSpan.GetTotalSeconds(), *Prefab->GetName());
	}

	bool ActorSerializer3::ObjectBelongsToThisPrefab(UObject* InObject)
	{
		if (WillSerailizeActorArray.Contains(InObject))
		{
			return true;
		}

		UObject* Outer = InObject->GetOuter();
		while (Outer != nullptr
			&& !Outer->HasAnyFlags(EObjectFlags::RF_Transient)
			)
		{
			if (WillSerailizeActorArray.Contains(Outer))
			{
				return true;
			}
			else
			{
				if (Outer->GetClass()->IsChildOf(AActor::StaticClass())
					)//not exist in WillSerailizeActorArray, but is a actor, means it not belongs to this prefab
				{
					return false;
				}
			}
			Outer = Outer->GetOuter();
		}
		return false;
	}
	bool ActorSerializer3::ObjectIsTrash(UObject* InObject)
	{
		UObject* Outer = InObject;
		while (Outer != nullptr)
		{
			if (Outer->GetName().StartsWith(TEXT("TRASH_")))
			{
				return true;
			}
			Outer = Outer->GetOuter();
		}
		return false;
	}

	void ActorSerializer3::CollectActorAndComponentRecursive(AActor* Actor)
	{
		//collect actor and components
		WillSerailizeActorArray.Add(Actor);
		MapObjectToGuid.Add(Actor, FGuid::NewGuid());

		TArray<AActor*> ChildrenActors;
		Actor->GetAttachedActors(ChildrenActors);
		for (auto ChildActor : ChildrenActors)
		{
#if WITH_EDITOR//Editor have no such thing of "SubPrefab"
			if (GetPrefabActorThatUseTheActorAsRoot(ChildActor) != nullptr)
			{
				SkippingActors.Add(ChildActor);
			}
			else
#endif
			{
				CollectActorAndComponentRecursive(ChildActor);
			}
		}
	}

	bool ActorSerializer3::CollectObjectToSerailize(UObject* Object, FGuid& OutGuid)
	{
#if WITH_EDITOR
		if (Object->GetClass()->IsChildOf(UEdMode::StaticClass()))return false;
		if (ObjectIsTrash(Object))return false;
#endif
		if (!Object->IsAsset()//skip asset, because asset is referenced directly
			&& Object->GetWorld() == TargetWorld.Get()
			&& !Object->HasAnyFlags(EObjectFlags::RF_Transient)
			&& !WillSerailizeActorArray.Contains(Object)
			&& !Object->GetClass()->IsChildOf(AActor::StaticClass())//skip actor
			&& !Object->GetClass()->IsChildOf(ULGUIPrefabHelperComponent::StaticClass())//skip LGUIPrefabHelperComponent, this component is only for editor helper
			&& ObjectBelongsToThisPrefab(Object)
			)
		{
			if (WillSerailizeObjectArray.Contains(Object))
			{
				auto GuidPtr = MapObjectToGuid.Find(Object);
				check(GuidPtr != nullptr);
				OutGuid = *GuidPtr;
				return true;//already contains object
			}

			auto Outer = Object->GetOuter();
			check(Outer != nullptr);

			if (WillSerailizeActorArray.Contains(Outer))//outer is actor
			{
				WillSerailizeObjectArray.Add(Object);
				OutGuid = FGuid::NewGuid();
				MapObjectToGuid.Add(Object, OutGuid);
				return true;
			}
			else//could have nested object outer
			{
				OutGuid = FGuid::NewGuid();
				MapObjectToGuid.Add(Object, OutGuid);
				auto Index = WillSerailizeObjectArray.Add(Object);
				while (Outer != nullptr
					&& !WillSerailizeActorArray.Contains(Outer)//Make sure Outer is not actor, because actor is created before any other objects, they will be stored in actor's data
					&& !WillSerailizeObjectArray.Contains(Outer)//Make sure Outer is not inside array
					)
				{
					WillSerailizeObjectArray.Insert(Outer, Index);//insert before object
					MapObjectToGuid.Add(Outer, FGuid::NewGuid());
					Outer = Outer->GetOuter();
				}
				return true;
			}
		}
		return false;
	}
	void ActorSerializer3::SerializeObjectArray(TArray<FLGUIObjectSaveData>& ObjectSaveDataArray, TArray<FLGUIComponentSaveData>& ComponentSaveDataArray)
	{
		for (auto Object : WillSerailizeObjectArray)
		{
			auto Class = Object->GetClass();
			if (Class->IsChildOf(UActorComponent::StaticClass()))
			{
				FLGUIComponentSaveData ComponentSaveDataItem;
				ComponentSaveDataItem.ComponentClass = FindOrAddClassFromList(Class);
				ComponentSaveDataItem.ComponentName = Object->GetFName();
				ComponentSaveDataItem.ComponentGuid = MapObjectToGuid[Object];
				ComponentSaveDataItem.OuterObjectGuid = MapObjectToGuid[Object->GetOuter()];
				if (auto SceneComp = Cast<USceneComponent>(Object))
				{
					if (auto ParentComp = SceneComp->GetAttachParent())
					{
						if (WillSerailizeActorArray.Contains(ParentComp->GetOwner()))//check if parent component belongs to this prefab
						{
							ComponentSaveDataItem.SceneComponentParentGuid = MapObjectToGuid[ParentComp];//@todo: better way to store SceneComponent's parent?
						}
					}
					WriterOrReaderFunction(Object, ComponentSaveDataItem.PropertyData, true);
				}
				else
				{
					WriterOrReaderFunction(Object, ComponentSaveDataItem.PropertyData, false);
				}
				ComponentSaveDataArray.Add(ComponentSaveDataItem);
			}
			else
			{
				FLGUIObjectSaveData ObjectSaveDataItem;
				ObjectSaveDataItem.ObjectClass = FindOrAddClassFromList(Class);
				ObjectSaveDataItem.ObjectGuid = MapObjectToGuid[Object];
				ObjectSaveDataItem.OuterObjectGuid = MapObjectToGuid[Object->GetOuter()];
				WriterOrReaderFunction(Object, ObjectSaveDataItem.PropertyData, false);
				ObjectSaveDataArray.Add(ObjectSaveDataItem);
			}
		}
	}

#include "EngineUtils.h"
#if WITH_EDITOR
	ALGUIPrefabHelperActor* ActorSerializer3::GetPrefabActorThatUseTheActorAsRoot(AActor* InActor)
	{
		for (TActorIterator<ALGUIPrefabHelperActor> ActorItr(InActor->GetWorld()); ActorItr; ++ActorItr)
		{
			auto PrefabActor = *ActorItr;
			if (IsValid(PrefabActor))
			{
				if (PrefabActor->LoadedRootActor == InActor)
				{
					return PrefabActor;
				}
			}
		}
		return nullptr;
	}
#endif
}
PRAGMA_ENABLE_OPTIMIZATION
