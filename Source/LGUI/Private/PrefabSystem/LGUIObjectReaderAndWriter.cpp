﻿// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "PrefabSystem/LGUIObjectReaderAndWriter.h"
#include "PrefabSystem/ActorSerializer3.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/BufferArchive.h"
#include "LGUI.h"

namespace LGUIPrefabSystem3
{
	FLGUIObjectWriter::FLGUIObjectWriter(TArray< uint8 >& Bytes, ActorSerializer3& InSerializer, TSet<FName> InSkipPropertyNames)
		: FObjectWriter(Bytes)
		, Serializer(InSerializer)
		, SkipPropertyNames(InSkipPropertyNames)
	{
		SetIsLoading(false);
		SetIsSaving(true);

		Serializer.SetupArchive(*this);
	}
	void FLGUIObjectWriter::DoSerialize(UObject* Object)
	{
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
				//MapObjectToGuid could be passed-in, if that the CollectObjectToSerailize will not execute which will miss some objects. so we still need to collect objects to serialize
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

			if (canSerializeObject)//object belongs to this actor hierarchy
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
		return FObjectWriter::operator<<(Value);
	}
	FArchive& FLGUIObjectWriter::operator<<(FSoftObjectPtr& Value)
	{
		return FObjectWriter::operator<<(Value);
	}
	FArchive& FLGUIObjectWriter::operator<<(FSoftObjectPath& Value)
	{
		return FObjectWriter::operator<<(Value);
	}
	FString FLGUIObjectWriter::GetArchiveName() const
	{
		return TEXT("FLGUIObjectWriter");
	}


	FLGUIObjectReader::FLGUIObjectReader(TArray< uint8 >& Bytes, ActorSerializer3& InSerializer, TSet<FName> InSkipPropertyNames)
		: FObjectReader(Bytes)
		, Serializer(InSerializer)
		, SkipPropertyNames(InSkipPropertyNames)
	{
		SetIsLoading(true);
		SetIsSaving(false);

		Serializer.SetupArchive(*this);
	}
	void FLGUIObjectReader::DoSerialize(UObject* Object)
	{
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
		return FObjectReader::operator<<(Value);
	}
	FArchive& FLGUIObjectReader::operator<<(FSoftObjectPtr& Value)
	{
		return FObjectReader::operator<<(Value);
	}
	FArchive& FLGUIObjectReader::operator<<(FSoftObjectPath& Value)
	{
		return FObjectReader::operator<<(Value);
	}
	FString FLGUIObjectReader::GetArchiveName() const
	{
		return TEXT("FLGUIObjectReader");
	}
}
