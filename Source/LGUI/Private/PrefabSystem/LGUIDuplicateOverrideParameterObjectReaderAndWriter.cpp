// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "PrefabSystem/LGUIObjectReaderAndWriter.h"
#include "PrefabSystem/ActorSerializerBase.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/BufferArchive.h"
#include "GameFramework/Actor.h"
#include "LGUI.h"

namespace LGUIPrefabSystem
{
	FLGUIDuplicateOverrideParameterObjectWriter::FLGUIDuplicateOverrideParameterObjectWriter(TArray< uint8 >& Bytes, ActorSerializerBase& InSerializer, TSet<FName> InSkipPropertyNames)
		: FLGUIOverrideParameterObjectWriter(Bytes, InSerializer, InSkipPropertyNames)
	{
		
	}
	bool FLGUIDuplicateOverrideParameterObjectWriter::SerializeObject(UObject* Object)
	{
		if (Object->IsAsset() && !Object->GetClass()->IsChildOf(AActor::StaticClass()))
		{
			auto id = Serializer.FindOrAddAssetIdFromList(Object);
			auto type = (uint8)EObjectType::Asset;
			*this << type;
			*this << id;
			return true;
		}
		else
		{
			auto guidPtr = Serializer.MapObjectToGuid.Find(Object);
			if (guidPtr != nullptr)//object belongs to this actor hierarchy
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
	FString FLGUIDuplicateOverrideParameterObjectWriter::GetArchiveName() const
	{
		return TEXT("FLGUIDuplicateOverrideParameterObjectWriter");
	}



	FLGUIDuplicateOverrideParameterObjectReader::FLGUIDuplicateOverrideParameterObjectReader(TArray< uint8 >& Bytes, ActorSerializerBase& InSerializer, TSet<FName> InSkipPropertyNames)
		: FLGUIOverrideParameterObjectReader(Bytes, InSerializer, InSkipPropertyNames)
	{

	}
	bool FLGUIDuplicateOverrideParameterObjectReader::SerializeObject(UObject*& Object, bool CanSerializeClass)
	{
		uint8 typeUint8 = 0;
		*this << typeUint8;
		auto type = (EObjectType)typeUint8;
		switch (type)
		{
		case LGUIPrefabSystem::EObjectType::Class:
		{
			check(CanSerializeClass);
			int32 id = -1;
			*this << id;
			auto asset = Serializer.FindClassFromListByIndex(id);
			Object = asset;
			return true;
		}
		break;
		case LGUIPrefabSystem::EObjectType::Asset:
		{
			int32 id = -1;
			*this << id;
			auto asset = Serializer.FindAssetFromListByIndex(id);
			Object = asset;
			return true;
		}
		break;
		case LGUIPrefabSystem::EObjectType::ObjectReference:
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
		case LGUIPrefabSystem::EObjectType::NativeSerailizeForDuplicate:
		{
			ByteOrderSerialize(&Object, sizeof(Object));
			return true;
		}
		break;
		}
		return false;
	}
	FString FLGUIDuplicateOverrideParameterObjectReader::GetArchiveName() const
	{
		return TEXT("FLGUIDuplicateOverrideParameterObjectReader");
	}
}
