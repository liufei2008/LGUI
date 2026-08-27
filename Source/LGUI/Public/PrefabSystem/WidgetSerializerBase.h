// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LexUIPrefab.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/ObjectWriter.h"
#include "Serialization/ObjectReader.h"
#include "UObject/ObjectVersion.h"
#include "UObject/UnrealType.h"

class ULexWidget;
class ULexUIPrefabWorldSubsystem;

namespace LexUIPrefabSystem
{
	/*
	 * serialize/deserialize actor with hierarchy
	 */
	class LGUI_API WidgetSerializerBase
	{
		friend class FLexUIObjectReader;
		friend class FLexUIObjectWriter;
		friend class FLexUIDuplicateObjectReader;
		friend class FLexUIDuplicateObjectWriter;
		friend class FLexUIOverrideParameterObjectWriter;
		friend class FLexUIOverrideParameterObjectReader;
		friend class FLexUIDuplicateOverrideParameterObjectWriter;
		friend class FLexUIDuplicateOverrideParameterObjectReader;

	public:
		virtual ~WidgetSerializerBase() {}

		TMap<UObject*, TArray<uint8>> SaveOverrideParameterToData(const TArray<FLexUIPrefabOverrideParameterData>& InData);
		void RestoreOverrideParameterFromData(TMap<UObject*, TArray<uint8>>& InData, const TArray<FLexUIPrefabOverrideParameterData>& InNameSetData);

		/** A nested-path override value, captured before the sub-prefab gets reloaded over the existing objects. */
		struct FSubPropertyOverrideValue
		{
			TWeakObjectPtr<UObject> Object;
			FLexUIPrefabOverridePropertyPath Path;
			FProperty* LeafProperty = nullptr;
			FDefaultConstructedPropertyElement Value;
		};
		/**
		 * Snapshot nested-path override values with a plain FProperty copy instead of an archive: an archive would
		 * delta-compare against the archetype and silently drop a value that equals the CDO, and the buffer never
		 * leaves this process so raw object pointers are fine (same as the whole-member immediate archives).
		 */
		TArray<FSubPropertyOverrideValue> SaveSubPropertyOverrideToData(const TArray<FLexUIPrefabOverrideParameterData>& InData);
		void RestoreSubPropertyOverrideFromData(const TArray<FSubPropertyOverrideValue>& InData);

		virtual void SetupArchive(FArchive& InArchive);

		//Actor that belongs to this prefab. All UObjects which get outer of these actor can be serialized
		TArray<ULexWidget*> WillSerializeWidgetArray;
		//Common UObjects that need to serialize. Outer object should stay at lower index then sub object, so when deserialize the outer object will created ealier, then the sub object can use the correct outer.
		TArray<UObject*> WillSerializeObjectArray;
		bool ObjectBelongsToThisPrefab(UObject* InObject);

		bool CollectObjectToSerialize(UObject* Object, FGuid& OutGuid);
		//Check object and it's up outer to tell if it is trash
		bool ObjectIsTrash(UObject* InObject);
		//find id from list, if not then create
		int32 FindOrAddAssetIdFromList(UObject* AssetObject);
		int32 FindOrAddClassFromList(UClass* Class);
		int32 FindOrAddNameFromList(const FName& Name);
		int32 FindOrAddTextFromList(const FText& Text);
		//find object by id
		UObject* FindAssetFromListByIndex(int32 Id);
		UClass* FindClassFromListByIndex(int32 Id);
		FName FindNameFromListByIndex(int32 Id);
		FText FindTextFromListByIndex(int32 Id);
		TArray<UObject*> ReferenceAssetList;
		TArray<UClass*> ReferenceClassList;
		TArray<FName> ReferenceNameList;
		TArray<FText> ReferenceTextList;

		bool bOverrideVersions = false;
		uint16 PrefabVersion = 0;
		FPackageFileVersion ArchiveVersion;
		int32 ArchiveLicenseeVer = 0;
		FEngineVersionBase ArEngineVer;
		uint32 ArEngineNetVer = 0;
		uint32 ArGameNetVer = 0;
		TMap<FGuid, TObjectPtr<UObject>> MapGuidToObject;
		TMap<UObject*, FGuid> MapObjectToGuid;

	protected:
		UObject* OwnerObject = nullptr;//Outer object that hold our widget
		UWorld* World = nullptr;
		bool bIsEditorOrRuntime = true;
		static bool CanUseUnversionedPropertySerialization();
	};
}