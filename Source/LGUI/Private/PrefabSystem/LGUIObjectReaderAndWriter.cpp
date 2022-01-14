// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "PrefabSystem/LGUIObjectReaderAndWriter.h"
#include "PrefabSystem/ActorSerializer3.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/BufferArchive.h"
#include "LGUI.h"

namespace LGUIPrefabSystem3
{
	FLGUIObjectWriter::FLGUIObjectWriter(UObject* Object, TArray< uint8 >& Bytes, ActorSerializer3& InSerializer, TSet<FName> InSkipPropertyNames)
		: FObjectWriter(Bytes)
		, Serializer(InSerializer)
		, SkipPropertyNames(InSkipPropertyNames)
	{
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
		if (SkipPropertyNames.Contains(InProperty->GetFName())
			&& CurrentIsMemberProperty(*this)//Skip property only support UObject's member property
			)
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
			bool canSerializeObject = false;
			auto guidPtr = Serializer.MapObjectToGuid.Find(Object);
			if (guidPtr != nullptr)
			{
				canSerializeObject = true;
				//MapObjectToGuid could be passed-in, so we still need to collect objects to serialize
				FGuid guid;
				Serializer.CollectObjectToSerailize(Object, guid);
			}
			else
			{
				FGuid guid;
				canSerializeObject = Serializer.CollectObjectToSerailize(Object, guid);
				if (canSerializeObject)
				{
					guidPtr = &guid;
				}
			}

			if (canSerializeObject)
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
#if ENGINE_MAJOR_VERSION >= 5
	FArchive& FLGUIObjectWriter::operator<<(FObjectPtr& Value)
	{
		auto Res = Value.Get();
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
				if (SerializeObject(Value.Get()))
				{
					return *this;
				}
			}
		}

		auto noneType = (uint8)EObjectType::None;
		*this << noneType;

		return *this;
	}
#endif
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
	FArchive& FLGUIObjectWriter::operator<<(FLazyObjectPtr& Value)
	{
		UE_LOG(LGUI, Warning, TEXT("[FLGUIObjectWriter]Detect Value property, which is not supported by LGUIPrefab!"));
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
		if (SkipPropertyNames.Contains(InProperty->GetFName())
			&& CurrentIsMemberProperty(*this)//Skip property only support UObject's member property
			)
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
#if ENGINE_MAJOR_VERSION >= 5
	FArchive& FLGUIObjectReader::operator<<(FObjectPtr& Value)
	{
		UObject* Res = nullptr;
		SerializeObject(Res, true);
		Value = Res;
		return *this;
	}
#endif
	FArchive& FLGUIObjectReader::operator<<(FWeakObjectPtr& Value)
	{
		UObject* Res = nullptr;
		SerializeObject(Res, false);
		Value = Res;
		return *this;
	}
	FArchive& FLGUIObjectReader::operator<<(FLazyObjectPtr& Value)
	{
		UE_LOG(LGUI, Warning, TEXT("[FLGUIObjectReader]Detect Value property, which is not supported by LGUIPrefab!"));
		return *this;
	}
	FArchive& FLGUIObjectReader::operator<<(FSoftObjectPtr& Value)
	{
		UE_LOG(LGUI, Warning, TEXT("[FLGUIObjectReader]Detect SoftObjectPtr property, which is not supported by LGUIPrefab!"));
		return *this;
	}
	FArchive& FLGUIObjectReader::operator<<(FSoftObjectPath& Value)
	{
		UE_LOG(LGUI, Warning, TEXT("[FLGUIObjectReader]Detect SoftObjectPath property, which is not supported by LGUIPrefab!"));
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
		if (SkipPropertyNames.Contains(InProperty->GetFName())
			&& CurrentIsMemberProperty(*this)//Skip property only support UObject's member property
			)
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
			bool canSerializeObject = false;
			auto guidPtr = Serializer.MapObjectToGuid.Find(Object);
			if (guidPtr != nullptr)
			{
				canSerializeObject = true;
			}
			else
			{
				FGuid guid;
				canSerializeObject = Serializer.CollectObjectToSerailize(Object, guid);
				if (canSerializeObject)
				{
					guidPtr = &guid;
				}
			}

			if (canSerializeObject)//object belongs to this actor hierarchy
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
#if ENGINE_MAJOR_VERSION >= 5
	FArchive& FLGUIDuplicateObjectWriter::operator<<(FObjectPtr& Value)
	{
		auto Res = Value.Get();
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
#endif
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
		if (SkipPropertyNames.Contains(InProperty->GetFName())
			&& CurrentIsMemberProperty(*this)//Skip property only support UObject's member property
			)
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
#if ENGINE_MAJOR_VERSION >= 5
	FArchive& FLGUIDuplicateObjectReader::operator<<(FObjectPtr& Value)
	{
		UObject* Res = nullptr;
		SerializeObject(Res, false);
		Value = Res;
		return *this;
	}
#endif
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
}
