// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LGUIPrefab.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/ObjectWriter.h"
#include "Serialization/ObjectReader.h"

namespace LGUIPrefabSystem3
{
	class ActorSerializer3;

	enum class EObjectType :uint8
	{
		None,
		/** Asset resource */
		Asset,
		/** UClass */
		Class,
		/** UObject reference(Not asset), include actor/ component/ uobject */
		ObjectReference,
		/** Only for duplicate, use native ObjectWriter/ObjectReader serialization method */
		NativeSerailizeForDuplicate,
	};
	class FLGUIObjectWriter : public FObjectWriter
	{
	public:
		FLGUIObjectWriter(UObject* Object, TArray< uint8 >& Bytes, ActorSerializer3& InSerializer, TSet<FName> InSkipPropertyNames);

		virtual bool ShouldSkipProperty(const FProperty* InProperty) const override;
		virtual FArchive& operator<<(class FName& N) override;
		virtual FArchive& operator<<(UObject*& Res) override;
		virtual FArchive& operator<<(FLazyObjectPtr& LazyObjectPtr) override;
		virtual FArchive& operator<<(FSoftObjectPtr& Value) override;
		virtual FArchive& operator<<(FSoftObjectPath& Value) override;
		virtual FArchive& operator<<(FWeakObjectPtr& Value) override;
		virtual FString GetArchiveName() const override;
		bool SerializeObject(UObject* Object);
	private:
		ActorSerializer3& Serializer;
		TSet<FName> SkipPropertyNames;
	};
	class FLGUIObjectReader : public FObjectReader
	{
	public:
		FLGUIObjectReader(UObject* Object, TArray< uint8 >& Bytes, ActorSerializer3& InSerializer, TSet<FName> InSkipPropertyNames);

		virtual bool ShouldSkipProperty(const FProperty* InProperty) const override;
		virtual FArchive& operator<<(class FName& N) override;
		virtual FArchive& operator<<(UObject*& Res) override;
		virtual FArchive& operator<<(FLazyObjectPtr& LazyObjectPtr) override;
		virtual FArchive& operator<<(FSoftObjectPtr& Value) override;
		virtual FArchive& operator<<(FSoftObjectPath& Value) override;
		virtual FArchive& operator<<(FWeakObjectPtr& Value) override;
		virtual FString GetArchiveName() const override;
		bool SerializeObject(UObject*& Object, bool CanSerializeClass);
	private:
		ActorSerializer3& Serializer;
		TSet<FName> SkipPropertyNames;
	};

	class FLGUIDuplicateObjectWriter : public FObjectWriter
	{
	public:
		FLGUIDuplicateObjectWriter(UObject* Object, TArray< uint8 >& Bytes, ActorSerializer3& InSerializer, TSet<FName> InSkipPropertyNames);

		virtual bool ShouldSkipProperty(const FProperty* InProperty) const override;
		virtual FArchive& operator<<(UObject*& Res) override;
		virtual FArchive& operator<<(FWeakObjectPtr& Value) override;
		virtual FString GetArchiveName() const override;
		bool SerializeObject(UObject* Object);
	private:
		ActorSerializer3& Serializer;
		TSet<FName> SkipPropertyNames;
	};
	class FLGUIDuplicateObjectReader : public FObjectReader
	{
	public:
		FLGUIDuplicateObjectReader(UObject* Object, TArray< uint8 >& Bytes, ActorSerializer3& InSerializer, TSet<FName> InSkipPropertyNames);

		virtual bool ShouldSkipProperty(const FProperty* InProperty) const override;
		virtual FArchive& operator<<(UObject*& Res) override;
		virtual FArchive& operator<<(FWeakObjectPtr& Value) override;
		virtual FString GetArchiveName() const override;
		bool SerializeObject(UObject*& Object, bool CanSerializeClass);
	private:
		ActorSerializer3& Serializer;
		TSet<FName> SkipPropertyNames;
	};


	struct FLGUIObjectSaveData
	{
	public:
		int32 ObjectClass = -1;
		FGuid ObjectGuid;//use id to find object
		uint32 ObjectFlags;
		TArray<uint8> PropertyData;
		FGuid OuterObjectGuid;//outer object
		friend FArchive& operator<<(FArchive& Ar, FLGUIObjectSaveData& ObjectData)
		{
			Ar << ObjectData.ObjectClass;
			Ar << ObjectData.ObjectGuid;
			Ar << ObjectData.ObjectFlags;
			Ar << ObjectData.PropertyData;
			Ar << ObjectData.OuterObjectGuid;
			return Ar;
		}
	};

	//ActorComponent serialize and save data
	struct FLGUIComponentSaveData
	{
	public:
		int32 ComponentClass;
		FName ComponentName;
		FGuid ComponentGuid;
		uint32 ObjectFlags;
		FGuid SceneComponentParentGuid = FGuid();//invalid guid means the the SceneComponent dont have parent. @todo: For BlueprintCreatedComponent, leave this to invalid, because that component is managed by blueprint
		FGuid OuterObjectGuid;//outer object
		TArray<uint8> PropertyData;
		friend FArchive& operator<<(FArchive& Ar, FLGUIComponentSaveData& ComponentData)
		{
			Ar << ComponentData.ComponentClass;
			Ar << ComponentData.ComponentName;
			Ar << ComponentData.ComponentGuid;
			Ar << ComponentData.ObjectFlags;
			Ar << ComponentData.SceneComponentParentGuid;
			Ar << ComponentData.OuterObjectGuid;
			Ar << ComponentData.PropertyData;
			return Ar;
		}
	};

	//Actor serialize and save data
	struct FLGUIActorSaveData
	{
	public:
		int32 ActorClass;
		FGuid ActorGuid;//use id to find actor
		uint32 ObjectFlags;
		TArray<uint8> ActorPropertyData;
		FGuid RootComponentGuid;
		/**
		 * The following two array stores default sub objects which belong to this actor. Array must match index for specific component. When deserialize, use FName to find FGuid.
		 * Common UObject/UActorComponent don't need these, because actor use "CollectDefaultSubobjects(xxx, true)" to find default sub objects, with nested objects, that should include all created default sub objects.
		 */
		TArray<FGuid> DefaultSubObjectGuidArray;
		TArray<FName> DefaultSubObjectNameArray;

		TArray<FLGUIActorSaveData> ChildActorData;

		friend FArchive& operator<<(FArchive& Ar, FLGUIActorSaveData& ActorData)
		{
			Ar << ActorData.ActorGuid;
			Ar << ActorData.ActorClass;
			Ar << ActorData.ObjectFlags;
			Ar << ActorData.ActorPropertyData;
			Ar << ActorData.RootComponentGuid;
			Ar << ActorData.DefaultSubObjectGuidArray;
			Ar << ActorData.DefaultSubObjectNameArray;

			Ar << ActorData.ChildActorData;
			return Ar;
		}
	};

	struct FLGUIPrefabSaveData
	{
	public:
		FLGUIActorSaveData SavedActor;
		TArray<FLGUIObjectSaveData> SavedObjects;
		TArray<FLGUIComponentSaveData> SavedComponents;

		friend FArchive& operator<<(FArchive& Ar, FLGUIPrefabSaveData& GameData)
		{
			Ar << GameData.SavedActor;
			Ar << GameData.SavedObjects;
			Ar << GameData.SavedComponents;
			return Ar;
		}
	};

	/*
	 * serialize/deserialize actor with hierarchy
	 * Not support: LazyObject, SoftObject
	 */
	class LGUI_API ActorSerializer3
	{
	private:
		friend class FLGUIObjectReader;
		friend class FLGUIObjectWriter;
		friend class FLGUIDuplicateObjectReader;
		friend class FLGUIDuplicateObjectWriter;

		ActorSerializer3(UWorld* InTargetWorld);
	public:
		/**
		 * @param CallbackBeforeAwake	This callback function will execute before Awake event, parameter "Actor" is the loaded root actor.
		 */
		static AActor* LoadPrefab(UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent, bool SetRelativeTransformToIdentity = true, TFunction<void(AActor*)> CallbackBeforeAwake = nullptr);
		/**
		 * @param CallbackBeforeAwake	This callback function will execute before Awake event, parameter "Actor" is the loaded root actor.
		 */
		static AActor* LoadPrefab(UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent, FVector RelativeLocation, FQuat RelativeRotation, FVector RelativeScale, TFunction<void(AActor*)> CallbackBeforeAwake = nullptr);

		/** Save prefab data for editor use. */
		static void SavePrefab(AActor* RootActor, ULGUIPrefab* InPrefab
			, TMap<UObject*, FGuid>& OutMapObjectToGuid);
		/** Save prefab date for runtime use. */
		static void SavePrefabForRuntime(AActor* RootActor, ULGUIPrefab* InPrefab);
		/**
		 * LoadPrefab for edit/modify, will keep reference of source prefab.
		 */
		static AActor* LoadPrefabForEdit(UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent
			, TMap<FGuid, UObject*>& InOutMapGuidToObjects
		);
		/**
		 * Duplicate actor with hierarchy
		 */
		static AActor* DuplicateActor(AActor* RootActor, USceneComponent* Parent);
	private:
		bool bIsEditorOrRuntime = true;
		TWeakObjectPtr<UWorld> TargetWorld = nullptr;//world that need to spawn actor
		TWeakObjectPtr<ULGUIPrefab> Prefab = nullptr;

		TMap<FGuid, UObject*> MapGuidToObject;
		struct ComponentDataStruct
		{
			UActorComponent* Component;
			FGuid SceneComponentParentGuid;
		};
		TArray<ComponentDataStruct> CreatedComponents;

#if WITH_EDITOR
		TArray<AActor*> SkippingActors;
#endif
		//Actor and ActorComponent that belongs to this prefab. All UObjects which get outer of these actor/component can be serailized
		TArray<UObject*> WillSerailizeActorArray;
		TArray<UObject*> WillSerailizeObjectArray;
		bool ObjectBelongsToThisPrefab(UObject* InObject);
		//Check object and it's up outer to tell if it is trash
		bool ObjectIsTrash(UObject* InObject);
		TMap<UObject*, FGuid> MapObjectToGuid;

		TArray<AActor*> CreatedActors;//collect for created actors
		TArray<FGuid> CreatedActorsGuid;//collect for created actor's guid

		void CollectActorAndComponentRecursive(AActor* Actor);
		bool CollectObjectToSerailize(UObject* Object, FGuid& OutGuid);
#if WITH_EDITOR
		class ALGUIPrefabHelperActor* GetPrefabActorThatUseTheActorAsRoot(AActor* Actor);
#endif
		//serialize actor
		void SerializeActor(AActor* RootActor, ULGUIPrefab* InPrefab);
		void SerializeActorRecursive(AActor* Actor, FLGUIActorSaveData& SavedActors);
		void SerializeObjectArray(TArray<FLGUIObjectSaveData>& ObjectSaveDataArray, TArray<FLGUIComponentSaveData>& ComponentSaveDataArray);
		void SerializeActorToData(AActor* RootActor, FLGUIPrefabSaveData& OutData);
		//deserialize actor
		AActor* DeserializeActor(USceneComponent* Parent, ULGUIPrefab* InPrefab, bool ReplaceTransform = false, FVector InLocation = FVector::ZeroVector, FQuat InRotation = FQuat::Identity, FVector InScale = FVector::OneVector);
		AActor* DeserializeActorFromData(FLGUIPrefabSaveData& SaveData, USceneComponent* Parent, bool ReplaceTransform, FVector InLocation, FQuat InRotation, FVector InScale);
		AActor* DeserializeActorRecursive(FLGUIActorSaveData& SavedActors);
		void PreGenerateActorRecursive(FLGUIActorSaveData& SavedActors);
		void PreGenerateObjectArray(const TArray<FLGUIObjectSaveData>& SavedObjects, const TArray<FLGUIComponentSaveData>& SavedComponents);
		void DeserializeObjectArray(const TArray<FLGUIObjectSaveData>& SavedObjects, const TArray<FLGUIComponentSaveData>& SavedComponents);

		//find id from list, if not will create
		int32 FindOrAddAssetIdFromList(UObject* AssetObject);
		int32 FindOrAddClassFromList(UClass* Class);
		int32 FindOrAddNameFromList(const FName& Name);
		//find object by id
		UObject* FindAssetFromListByIndex(int32 Id);
		UClass* FindClassFromListByIndex(int32 Id);
		FName FindNameFromListByIndex(int32 Id);
		TArray<UObject*> ReferenceAssetList;
		TArray<UClass*> ReferenceClassList;
		TArray<FName> ReferenceNameList;

		const TSet<FName>& GetSceneComponentExcludeProperties();

		TFunction<void(AActor*)> CallbackBeforeAwake = nullptr;

		/**
		 * get Writer and Reader for serialize or deserialize
		 * @param	UObject*	Object to serialize/deserialize
		 * @param	TArray<uint8>&	Data buffer
		 * @param	bool	is SceneComponent
		 */
		TFunction<void(UObject*, TArray<uint8>&, bool)> WriterOrReaderFunction = nullptr;
		//duplicate actor
		AActor* SerializeActor_ForDuplicate(AActor* RootActor, USceneComponent* Parent);

		void SetupArchive(FArchive& InArchive);
	};
}