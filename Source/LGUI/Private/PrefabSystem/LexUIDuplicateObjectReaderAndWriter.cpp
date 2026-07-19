// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "PrefabSystem/LexUIObjectReaderAndWriter.h"
#include "PrefabSystem/WidgetSerializerBase.h"
#include "Serialization/MemoryReader.h"
#include "Engine/Blueprint.h"

namespace LexUIPrefabSystem
{
	FLexUIDuplicateObjectWriter::FLexUIDuplicateObjectWriter(TArray< uint8 >& Bytes, WidgetSerializerBase& InSerializer, TSet<FName> InSkipPropertyNames)
		: FLexUIObjectWriter(Bytes, InSerializer, InSkipPropertyNames)
	{
		
	}
	bool FLexUIDuplicateObjectWriter::ShouldSkipProperty(const FProperty* InProperty) const
	{
		if (InProperty->HasAnyPropertyFlags(CPF_Transient | CPF_DuplicateTransient | CPF_NonPIEDuplicateTransient | CPF_DisableEditOnInstance)
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
	bool FLexUIDuplicateObjectWriter::SerializeObject(UObject* Object)
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
			FGuid guid;
			auto guidPtr = Serializer.MapObjectToGuid.Find(Object);
			if (guidPtr != nullptr)
			{
				canSerializeObject = true;
				guid = *guidPtr;
				//MapObjectToGuid could be passed-in, if that the CollectObjectToSerialize will not execute which will miss some objects. so we still need to collect objects to serialize
				Serializer.CollectObjectToSerialize(Object, guid);
			}
			else
			{
				canSerializeObject = Serializer.CollectObjectToSerialize(Object, guid);
			}

			if (canSerializeObject)//object belongs to this actor hierarchy
			{
				auto type = (uint8)EObjectType::ObjectReference;
				*this << type;
				*this << guid;
				return true;
			}
			else//object not belongs to this actor hierarchy, just copy pointer
			{
				auto type = (uint8)EObjectType::NativeSerializeForDuplicate;
				*this << type;
				ByteOrderSerialize(&Object, sizeof(Object));
				return true;
			}
		}
	}
	FString FLexUIDuplicateObjectWriter::GetArchiveName() const
	{
		return TEXT("FLexUIDuplicateObjectReader");
	}



	FLexUIDuplicateObjectReader::FLexUIDuplicateObjectReader(TArray< uint8 >& Bytes, WidgetSerializerBase& InSerializer, TSet<FName> InSkipPropertyNames)
		: FLexUIObjectReader(Bytes, InSerializer, InSkipPropertyNames)
	{

	}
	bool FLexUIDuplicateObjectReader::ShouldSkipProperty(const FProperty* InProperty) const
	{
		if (InProperty->HasAnyPropertyFlags(CPF_Transient | CPF_DuplicateTransient | CPF_NonPIEDuplicateTransient | CPF_DisableEditOnInstance)
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
	bool FLexUIDuplicateObjectReader::SerializeObject(UObject*& Object, bool CanSerializeClass)
	{
		uint8 typeUint8 = 0;
		*this << typeUint8;
		auto type = (EObjectType)typeUint8;
		switch (type)
		{
		case LexUIPrefabSystem::EObjectType::Class:
		{
			check(CanSerializeClass);
			int32 id = -1;
			*this << id;
			auto asset = Serializer.FindClassFromListByIndex(id);
			Object = asset;
			return true;
		}
		break;
		case LexUIPrefabSystem::EObjectType::Asset:
		{
			int32 id = -1;
			*this << id;
			auto asset = Serializer.FindAssetFromListByIndex(id);
			Object = asset;
			return true;
		}
		break;
		case LexUIPrefabSystem::EObjectType::ObjectReference:
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
		case LexUIPrefabSystem::EObjectType::NativeSerializeForDuplicate:
		{
			ByteOrderSerialize(&Object, sizeof(Object));
			return true;
		}
		break;
		}
		return false;
	}
	FString FLexUIDuplicateObjectReader::GetArchiveName() const
	{
		return TEXT("FLexUIDuplicateObjectReader");
	}
}
