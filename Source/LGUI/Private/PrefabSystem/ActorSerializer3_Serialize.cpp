// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "PrefabSystem/ActorSerializer3.h"
#include "PrefabSystem/LGUIObjectReaderAndWriter.h"
#include "PrefabSystem/LGUIPrefabHelperActor.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/BufferArchive.h"
#include "Components/PrimitiveComponent.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIItem.h"
#if WITH_EDITOR
#include "Tools/UEdMode.h"
#include "Utils/LGUIUtils.h"
#endif

PRAGMA_DISABLE_OPTIMIZATION
namespace LGUIPrefabSystem3
{
	void ActorSerializer3::SavePrefab(AActor* RootActor, ULGUIPrefab* InPrefab
		, TMap<UObject*, FGuid>& InOutMapObjectToGuid, TMap<AActor*, FLGUISubPrefabData>& InSubPrefabMap
		, bool InForEditorOrRuntimeUse
	)
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
		ActorSerializer3 serializer;
		serializer.TargetWorld = RootActor->GetWorld();
		for (auto KeyValue : InOutMapObjectToGuid)//Preprocess the map, ignore invalid object
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
			FLGUIObjectWriter Writer(InObject, InOutBuffer, serializer, ExcludeProperties);
		};
		serializer.WriterOrReaderFunctionForSubPrefab = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, const TSet<FName>& InOverridePropertyNameSet) {
			FLGUIOverrideParameterObjectWriter Writer(InObject, InOutBuffer, serializer, InOverridePropertyNameSet);
		};
		serializer.SerializeActor(RootActor, InPrefab);
		InOutMapObjectToGuid = serializer.MapObjectToGuid;
	}

	void ActorSerializer3::SerializeActorRecursive(AActor* Actor, FLGUIActorSaveData& OutActorSaveData)
	{
		auto ActorGuid = MapObjectToGuid[Actor];

		OutActorSaveData.ActorClass = FindOrAddClassFromList(Actor->GetClass());
		OutActorSaveData.ActorGuid = ActorGuid;
		OutActorSaveData.ObjectFlags = (uint32)Actor->GetFlags();
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
		//sort on hierarchy, so hierarchy order will be good when deserialize it. Actually normal UIItem's hierarchyIndex property can do the job, but sub prefab's root actor not, so sort it to make sure.
		Algo::Sort(ChildrenActors, [](const AActor* A, const AActor* B) {
			auto ARoot = A->GetRootComponent();
			auto BRoot = B->GetRootComponent();
			if (ARoot != nullptr && BRoot != nullptr)
			{
				auto AUIRoot = Cast<UUIItem>(ARoot);
				auto BUIRoot = Cast<UUIItem>(BRoot);
				return AUIRoot->GetHierarchyIndex() < BUIRoot->GetHierarchyIndex();
			}
			return false;
			});
		TArray<FLGUIActorSaveData> ChildSaveDataList;
		for (auto ChildActor : ChildrenActors)
		{
			if (auto SubPrefabDataPtr = SubPrefabMap.Find(ChildActor))//sub prefab
			{
				FLGUIActorSaveData ChildActorSaveData;
				ChildActorSaveData.bIsPrefab = true;
				ChildActorSaveData.PrefabAssetIndex = FindOrAddAssetIdFromList(SubPrefabDataPtr->PrefabAsset);
				ChildActorSaveData.ActorGuid = MapObjectToGuid[ChildActor];
				ChildActorSaveData.MapObjectGuidFromParentPrefabToSubPrefab = SubPrefabDataPtr->MapObjectGuidFromParentPrefabToSubPrefab;

				//serialize override parameter data
				for (auto& DataItem : SubPrefabDataPtr->ObjectOverrideParameterArray)
				{
					TArray<uint8> SubPrefabOverrideData;
					auto SubPrefabObject = DataItem.Object.Get();
					WriterOrReaderFunctionForSubPrefab(SubPrefabObject, SubPrefabOverrideData, DataItem.MemberPropertyName);
					FLGUIPrefabOverrideParameterRecordData RecordDataItem;
					RecordDataItem.ObjectGuid = MapObjectToGuid[SubPrefabObject];
					RecordDataItem.OverrideParameterData = SubPrefabOverrideData;
					RecordDataItem.OverrideParameterNameSet = DataItem.MemberPropertyName;
					ChildActorSaveData.ObjectOverrideParameterArray.Add(RecordDataItem);
				}
				ChildSaveDataList.Add(ChildActorSaveData);
			}
			else
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
		CollectActorRecursive(RootActor);
		//serailize actor
		SerializeActorRecursive(RootActor, OutData.SavedActor);
		//serialize objects and components
		SerializeObjectArray(OutData.SavedObjects, OutData.SavedComponents);
	}
	void ActorSerializer3::SerializeActor(AActor* RootActor, ULGUIPrefab* InPrefab)
	{
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
			InPrefab->BinaryData = ToBinary;
			InPrefab->ThumbnailDirty = true;
			InPrefab->CreateTime = FDateTime::Now();

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
		}
		else
#endif
		{
			InPrefab->BinaryDataForBuild = ToBinary;

			//fill new reference data
			InPrefab->ReferenceAssetListForBuild = this->ReferenceAssetList;
			InPrefab->ReferenceClassListForBuild = this->ReferenceClassList;
			InPrefab->ReferenceNameListForBuild = this->ReferenceNameList;
		}

		InPrefab->EngineMajorVersion = ENGINE_MAJOR_VERSION;
		InPrefab->EngineMinorVersion = ENGINE_MINOR_VERSION;
		InPrefab->PrefabVersion = LGUI_CURRENT_PREFAB_VERSION;

		InPrefab->MarkPackageDirty();
		auto TimeSpan = FDateTime::Now() - StartTime;
		UE_LOG(LGUI, Log, TEXT("Take %fs saving prefab: %s"), TimeSpan.GetTotalSeconds(), *InPrefab->GetName());
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

	void ActorSerializer3::CollectActorRecursive(AActor* Actor)
	{
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

	bool ActorSerializer3::CollectObjectToSerailize(UObject* Object, FGuid& OutGuid)
	{
#if WITH_EDITOR
		if (Object->GetClass()->IsChildOf(UEdMode::StaticClass()))return false;
		if (ObjectIsTrash(Object))return false;
#endif
		if (!Object->IsAsset()//skip asset, because asset is referenced directly
			&& Object->GetWorld() == TargetWorld
			&& !Object->HasAnyFlags(EObjectFlags::RF_Transient)
			&& !WillSerailizeActorArray.Contains(Object)
			&& !Object->GetClass()->IsChildOf(AActor::StaticClass())//skip actor
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
				if (auto GuidPtr = MapObjectToGuid.Find(Object))
				{
					OutGuid = *GuidPtr;
				}
				else
				{
					OutGuid = FGuid::NewGuid();
					MapObjectToGuid.Add(Object, OutGuid);
				}
				return true;
			}
			else//could have nested object outer
			{
				if (auto GuidPtr = MapObjectToGuid.Find(Object))
				{
					OutGuid = *GuidPtr;
				}
				else
				{
					OutGuid = FGuid::NewGuid();
					MapObjectToGuid.Add(Object, OutGuid);
				}
				auto Index = WillSerailizeObjectArray.Add(Object);
				while (Outer != nullptr
					&& !WillSerailizeActorArray.Contains(Outer)//Make sure Outer is not actor, because actor is created before any other objects, they will be stored in actor's data
					&& !WillSerailizeObjectArray.Contains(Outer)//Make sure Outer is not inside array
					)
				{
					WillSerailizeObjectArray.Insert(Outer, Index);//insert before object
					if (!MapObjectToGuid.Contains(Outer))
					{
						MapObjectToGuid.Add(Outer, FGuid::NewGuid());
					}
					Outer = Outer->GetOuter();
				}
				return true;
			}
		}
		return false;
	}
	void ActorSerializer3::SerializeObjectArray(TArray<FLGUIObjectSaveData>& ObjectSaveDataArray, TArray<FLGUIComponentSaveData>& ComponentSaveDataArray)
	{
		for (int i = 0; i < WillSerailizeObjectArray.Num(); i++)
		{
			auto Object = WillSerailizeObjectArray[i];
			auto Class = Object->GetClass();
			if (Class->IsChildOf(UActorComponent::StaticClass()))
			{
				FLGUIComponentSaveData ComponentSaveDataItem;
				ComponentSaveDataItem.ComponentClass = FindOrAddClassFromList(Class);
				ComponentSaveDataItem.ComponentName = Object->GetFName();
				ComponentSaveDataItem.ComponentGuid = MapObjectToGuid[Object];
				ComponentSaveDataItem.ObjectFlags = (uint32)Object->GetFlags();
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
				ObjectSaveDataItem.ObjectFlags = (uint32)Object->GetFlags();
				ObjectSaveDataItem.OuterObjectGuid = MapObjectToGuid[Object->GetOuter()];
				WriterOrReaderFunction(Object, ObjectSaveDataItem.PropertyData, false);
				ObjectSaveDataArray.Add(ObjectSaveDataItem);
			}
		}
	}
}
PRAGMA_ENABLE_OPTIMIZATION
