// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "PrefabSystem/ActorSerializer3.h"
#include "PrefabSystem/LGUIPrefabHelperComponent.h"
#include "PrefabSystem/LGUIPrefabHelperActor.h"
#include "Utils/BitConverter.h"
#include "Engine/World.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/BufferArchive.h"
#include "Components/PrimitiveComponent.h"
#include "UObject/TextProperty.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Core/Actor/LGUIManagerActor.h"

namespace LGUIPrefabSystem3
{
	FLGUIObjectWriter::FLGUIObjectWriter(UObject* Object, TArray< uint8 >& Bytes, ActorSerializer3& InSerializer, TSet<FName> InSkipPropertyNames)
		: FObjectWriter(Bytes)
		, Serializer(InSerializer)
		, SkipPropertyNames(InSkipPropertyNames)
	{
		this->Reset();

		SetIsLoading(false);
		SetIsSaving(true);

		Serializer.SetupArchive(*this);

		Object->Serialize(*this);
	}
	bool FLGUIObjectWriter::ShouldSkipProperty(const FProperty* InProperty) const
	{
		if (InProperty->HasAnyPropertyFlags(CPF_Transient)
			|| InProperty->IsA<FMulticastDelegateProperty>()
			|| InProperty->IsA<FDelegateProperty>()
			)
		{
			return true;
		}
		if (SkipPropertyNames.Contains(InProperty->GetFName()))
		{
			return true;
		}

		return false;
	}
	FArchive& FLGUIObjectWriter::operator<<(class FName& N)
	{
		auto id = Serializer.FindOrAddNameFromList(N);
		*this << id;

		return *this;
	}
	bool FLGUIObjectWriter::SerializeObject(UObject* Object)
	{
		if (Object->IsAsset())
		{
			auto id = Serializer.FindOrAddAssetIdFromList(Object);
			auto type = (uint8)EObjectType::Asset;
			*this << type;
			*this << id;
			return true;
		}
		else
		{
			bool canSerializaObject = false;
			auto guidPtr = Serializer.MapObjectToGuid.Find(Object);
			if (guidPtr != nullptr)
			{
				canSerializaObject = true;
			}
			else
			{
				FGuid guid;
				canSerializaObject = Serializer.CollectObjectToSerailize(Object, guid);
				if (canSerializaObject)
				{
					guidPtr = &guid;
				}
			}

			if (canSerializaObject)
			{
				auto type = (uint8)EObjectType::ObjectReference;
				*this << type;
				*this << *guidPtr;
				return true;
			}
			else
			{
				return false;
			}
		}
	}
	FArchive& FLGUIObjectWriter::operator<<(UObject*& Res)
	{
		if (Res != nullptr)
		{
			auto Property = this->GetSerializedProperty();
			if (CastField<FClassProperty>(Property) != nullptr)//class property
			{
				auto id = Serializer.FindOrAddClassFromList((UClass*)Res);
				auto type = (uint8)EObjectType::Class;
				*this << type;
				*this << id;
				return *this;
			}
			else
			{
				if (SerializeObject(Res))
				{
					return *this;
				}
			}
		}

		auto noneType = (uint8)EObjectType::None;
		*this << noneType;

		return *this;
	}
	FArchive& FLGUIObjectWriter::operator<<(FWeakObjectPtr& Value)
	{
		if (Value.IsValid())
		{
			//no need to concern UClass, because UClass cannot use weakptr
			if (SerializeObject(Value.Get()))
			{
				return *this;
			}
		}
		auto noneType = (uint8)EObjectType::None;
		*this << noneType;

		return *this;
	}
	FArchive& FLGUIObjectWriter::operator<<(FLazyObjectPtr& LazyObjectPtr)
	{
		UE_LOG(LGUI, Warning, TEXT("[FLGUIObjectWriter]Detect LazyObjectPtr property, which is not supported by LGUIPrefab!"));
		return *this;
	}
	FArchive& FLGUIObjectWriter::operator<<(FSoftObjectPtr& Value)
	{
		UE_LOG(LGUI, Warning, TEXT("[FLGUIObjectWriter]Detect SoftObjectPtr property, which is not supported by LGUIPrefab!"));
		return *this;
	}
	FArchive& FLGUIObjectWriter::operator<<(FSoftObjectPath& Value)
	{
		UE_LOG(LGUI, Warning, TEXT("[FLGUIObjectWriter]Detect SoftObjectPath property, which is not supported by LGUIPrefab!"));
		return *this;
	}
	FString FLGUIObjectWriter::GetArchiveName() const
	{
		return TEXT("FLGUIObjectWriter");
	}


	FLGUIObjectReader::FLGUIObjectReader(UObject* Object, TArray< uint8 >& Bytes, ActorSerializer3& InSerializer, TSet<FName> InSkipPropertyNames)
		: FObjectReader(Bytes)
		, Serializer(InSerializer)
		, SkipPropertyNames(InSkipPropertyNames)
	{
		this->Reset();

		SetIsLoading(true);
		SetIsSaving(false);

		Serializer.SetupArchive(*this);

		Object->Serialize(*this);
	}
	bool FLGUIObjectReader::ShouldSkipProperty(const FProperty* InProperty) const
	{
		if (InProperty->HasAnyPropertyFlags(CPF_Transient)
			|| InProperty->IsA<FMulticastDelegateProperty>()
			|| InProperty->IsA<FDelegateProperty>()
			)
		{
			return true;
		}
		if (SkipPropertyNames.Contains(InProperty->GetFName()))
		{
			return true;
		}

		return false;
	}
	FArchive& FLGUIObjectReader::operator<<(class FName& N)
	{
		int32 id = -1;
		*this << id;
		N = Serializer.FindNameFromListByIndex(id);

		return *this;
	}
	bool FLGUIObjectReader::SerializeObject(UObject*& Object, bool CanSerializeClass)
	{
		uint8 typeUint8 = 0;
		*this << typeUint8;
		auto type = (EObjectType)typeUint8;
		switch (type)
		{
		case LGUIPrefabSystem3::EObjectType::Class:
		{
			check(CanSerializeClass);
			int32 id = -1;
			*this << id;
			auto asset = Serializer.FindClassFromListByIndex(id);
			Object = asset;
			return true;
		}
		break;
		case LGUIPrefabSystem3::EObjectType::Asset:
		{
			int32 id = -1;
			*this << id;
			auto asset = Serializer.FindAssetFromListByIndex(id);
			Object = asset;
			return true;
		}
		break;
		case LGUIPrefabSystem3::EObjectType::ObjectReference:
		{
			FGuid guid;
			*this << guid;
			if (auto ObjectPtr = Serializer.MapGuidToObject.Find(guid))
			{
				Object = *ObjectPtr;
				return true;
			}
		}
		break;
		}
		return false;
	}
	FArchive& FLGUIObjectReader::operator<<(UObject*& Res)
	{
		SerializeObject(Res, true);
		return *this;
	}
	FArchive& FLGUIObjectReader::operator<<(FWeakObjectPtr& Value)
	{
		UObject* Res = nullptr;
		SerializeObject(Res, false);
		Value = Res;
		return *this;
	}
	FArchive& FLGUIObjectReader::operator<<(FLazyObjectPtr& LazyObjectPtr)
	{
		UE_LOG(LGUI, Warning, TEXT("[FLGUIObjectWriter]Detect LazyObjectPtr property, which is not supported by LGUIPrefab!"));
		return *this;
	}
	FArchive& FLGUIObjectReader::operator<<(FSoftObjectPtr& Value)
	{
		UE_LOG(LGUI, Warning, TEXT("[FLGUIObjectWriter]Detect SoftObjectPtr property, which is not supported by LGUIPrefab!"));
		return *this;
	}
	FArchive& FLGUIObjectReader::operator<<(FSoftObjectPath& Value)
	{
		UE_LOG(LGUI, Warning, TEXT("[FLGUIObjectWriter]Detect SoftObjectPath property, which is not supported by LGUIPrefab!"));
		return *this;
	}
	FString FLGUIObjectReader::GetArchiveName() const
	{
		return TEXT("FLGUIObjectReader");
	}



	FLGUIDuplicateObjectWriter::FLGUIDuplicateObjectWriter(UObject* Object, TArray< uint8 >& Bytes, ActorSerializer3& InSerializer, TSet<FName> InSkipPropertyNames)
		: FObjectWriter(Bytes)
		, Serializer(InSerializer)
		, SkipPropertyNames(InSkipPropertyNames)
	{
		this->Reset();

		SetIsLoading(false);
		SetIsSaving(true);

		Serializer.SetupArchive(*this);

		Object->Serialize(*this);
	}
	bool FLGUIDuplicateObjectWriter::ShouldSkipProperty(const FProperty* InProperty) const
	{
		if (InProperty->HasAnyPropertyFlags(CPF_Transient)
			|| InProperty->IsA<FMulticastDelegateProperty>()
			|| InProperty->IsA<FDelegateProperty>()
			)
		{
			return true;
		}
		if (SkipPropertyNames.Contains(InProperty->GetFName()))
		{
			return true;
		}

		return false;
	}
	bool FLGUIDuplicateObjectWriter::SerializeObject(UObject* Object)
	{
		if (Object->IsAsset())
		{
			auto id = Serializer.FindOrAddAssetIdFromList(Object);
			auto type = (uint8)EObjectType::Asset;
			*this << type;
			*this << id;
			return true;
		}
		else
		{
			bool canSerializaObject = false;
			auto guidPtr = Serializer.MapObjectToGuid.Find(Object);
			if (guidPtr != nullptr)
			{
				canSerializaObject = true;
			}
			else
			{
				FGuid guid;
				canSerializaObject = Serializer.CollectObjectToSerailize(Object, guid);
				if (canSerializaObject)
				{
					guidPtr = &guid;
				}
			}

			if (canSerializaObject)//object belongs to this actor hierarchy
			{
				auto type = (uint8)EObjectType::ObjectReference;
				*this << type;
				*this << *guidPtr;
				return true;
			}
			else//object not belongs to this actor hierarchy, just copy pointer
			{
				auto type = (uint8)EObjectType::NativeSerailizeForDuplicate;
				*this << type;
				ByteOrderSerialize(&Object, sizeof(Object));
				return true;
			}
		}
	}
	FArchive& FLGUIDuplicateObjectWriter::operator<<(UObject*& Res)
	{
		if (Res != nullptr)
		{
			auto Property = this->GetSerializedProperty();
			if (CastField<FClassProperty>(Property) != nullptr)//class property
			{
				auto id = Serializer.FindOrAddClassFromList((UClass*)Res);
				auto type = (uint8)EObjectType::Class;
				*this << type;
				*this << id;
				return *this;
			}
			else
			{
				if (SerializeObject(Res))
				{
					return *this;
				}
			}
		}

		auto noneType = (uint8)EObjectType::None;
		*this << noneType;

		return *this;
	}
	FArchive& FLGUIDuplicateObjectWriter::operator<<(FWeakObjectPtr& Value)
	{
		if (Value.IsValid())
		{
			//no need to concern UClass, because UClass cannot use weakptr
			if (SerializeObject(Value.Get()))
			{
				return *this;
			}
		}
		auto noneType = (uint8)EObjectType::None;
		*this << noneType;

		return *this;
	}
	FString FLGUIDuplicateObjectWriter::GetArchiveName() const
	{
		return TEXT("FLGUIDuplicateObjectReader");
	}



	FLGUIDuplicateObjectReader::FLGUIDuplicateObjectReader(UObject* Object, TArray< uint8 >& Bytes, ActorSerializer3& InSerializer, TSet<FName> InSkipPropertyNames)
		: FObjectReader(Bytes)
		, Serializer(InSerializer)
		, SkipPropertyNames(InSkipPropertyNames)
	{
		this->Reset();

		SetIsLoading(true);
		SetIsSaving(false);

		Serializer.SetupArchive(*this);

		Object->Serialize(*this);
	}
	bool FLGUIDuplicateObjectReader::ShouldSkipProperty(const FProperty* InProperty) const
	{
		if (InProperty->HasAnyPropertyFlags(CPF_Transient)
			|| InProperty->IsA<FMulticastDelegateProperty>()
			|| InProperty->IsA<FDelegateProperty>()
			)
		{
			return true;
		}
		if (SkipPropertyNames.Contains(InProperty->GetFName()))
		{
			return true;
		}

		return false;
	}
	bool FLGUIDuplicateObjectReader::SerializeObject(UObject*& Object, bool CanSerializeClass)
	{
		uint8 typeUint8 = 0;
		*this << typeUint8;
		auto type = (EObjectType)typeUint8;
		switch (type)
		{
		case LGUIPrefabSystem3::EObjectType::Class:
		{
			check(CanSerializeClass);
			int32 id = -1;
			*this << id;
			auto asset = Serializer.FindClassFromListByIndex(id);
			Object = asset;
			return true;
		}
		break;
		case LGUIPrefabSystem3::EObjectType::Asset:
		{
			int32 id = -1;
			*this << id;
			auto asset = Serializer.FindAssetFromListByIndex(id);
			Object = asset;
			return true;
		}
		break;
		case LGUIPrefabSystem3::EObjectType::ObjectReference:
		{
			FGuid guid;
			*this << guid;
			if (auto ObjectPtr = Serializer.MapGuidToObject.Find(guid))
			{
				Object = *ObjectPtr;
				return true;
			}
		}
		break;
		case LGUIPrefabSystem3::EObjectType::NativeSerailizeForDuplicate:
		{
			ByteOrderSerialize(&Object, sizeof(Object));
			return true;
		}
		break;
		}
		return false;
	}
	FArchive& FLGUIDuplicateObjectReader::operator<<(UObject*& Res)
	{
		SerializeObject(Res, true);
		return *this;
	}
	FArchive& FLGUIDuplicateObjectReader::operator<<(FWeakObjectPtr& Value)
	{
		UObject* Res = nullptr;
		SerializeObject(Res, false);
		Value = Res;
		return *this;
	}
	FString FLGUIDuplicateObjectReader::GetArchiveName() const
	{
		return TEXT("FLGUIDuplicateObjectReader");
	}




	ActorSerializer3::ActorSerializer3(UWorld* InTargetWorld)
	{
		TargetWorld = TWeakObjectPtr<UWorld>(InTargetWorld);
	}


	int32 ActorSerializer3::FindOrAddAssetIdFromList(UObject* AssetObject)
	{
		if (!AssetObject)return -1;
		int32 resultIndex;
		if (ReferenceAssetList.Find(AssetObject, resultIndex))
		{
			return resultIndex;//return index if found
		}
		else//add to list if not found
		{
			ReferenceAssetList.Add(AssetObject);
			return ReferenceAssetList.Num() - 1;
		}
	}

	int32 ActorSerializer3::FindOrAddClassFromList(UClass* Class)
	{
		if (!Class)return -1;
		int32 resultIndex;
		if (ReferenceClassList.Find(Class, resultIndex))
		{
			return resultIndex;
		}
		else
		{
			ReferenceClassList.Add(Class);
			return ReferenceClassList.Num() - 1;
		}
	}
	int32 ActorSerializer3::FindOrAddNameFromList(const FName& Name)
	{
		if (!Name.IsValid())return -1;
		int32 resultIndex;
		if (ReferenceNameList.Find(Name, resultIndex))
		{
			return resultIndex;
		}
		else
		{
			ReferenceNameList.Add(Name);
			return ReferenceNameList.Num() - 1;
		}
	}
	FName ActorSerializer3::FindNameFromListByIndex(int32 Id)
	{
		int32 count = ReferenceNameList.Num();
		if (Id >= count || Id < 0)
		{
			return NAME_None;
		}
		return ReferenceNameList[Id];
	}

	UObject* ActorSerializer3::FindAssetFromListByIndex(int32 Id)
	{
		int32 count = ReferenceAssetList.Num();
		if (Id >= count || Id < 0)
		{
			return nullptr;
		}
		return ReferenceAssetList[Id];
	}

	UClass* ActorSerializer3::FindClassFromListByIndex(int32 Id)
	{
		int32 count = ReferenceClassList.Num();
		if (Id >= count || Id < 0)
		{
			return nullptr;
		}
		return ReferenceClassList[Id];
	}

	const TSet<FName>& ActorSerializer3::GetSceneComponentExcludeProperties()
	{
		static TSet<FName> result = { FName("AttachParent") };
		return result;
	}

	static bool CanUseUnversionedPropertySerialization()
	{
		bool bTemp;
		static bool bAllow = GConfig->GetBool(TEXT("Core.System"), TEXT("CanUseUnversionedPropertySerialization"), bTemp, GEngineIni) && bTemp;
		return bAllow;
	}

	void ActorSerializer3::SetupArchive(FArchive& InArchive)
	{
		if (!bIsEditorOrRuntime && CanUseUnversionedPropertySerialization())
		{
			InArchive.SetUseUnversionedPropertySerialization(true);
		}
		InArchive.SetFilterEditorOnly(!bIsEditorOrRuntime);
		InArchive.SetWantBinaryPropertySerialization(!bIsEditorOrRuntime);
	}
}
