// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "PrefabSystem/LexUIObjectReaderAndWriter.h"
#include "PrefabSystem/WidgetSerializerBase.h"
#include "Serialization/MemoryReader.h"
#include "Engine/Blueprint.h"

namespace LexUIPrefabSystem
{
	FLexUIDuplicateOverrideParameterObjectWriter::FLexUIDuplicateOverrideParameterObjectWriter(TArray< uint8 >& Bytes, WidgetSerializerBase& InSerializer, const TArray<FName>& InOverridePropertyNames)
		: FLexUIOverrideParameterObjectWriter(Bytes, InSerializer, InOverridePropertyNames)
	{
		
	}
	bool FLexUIDuplicateOverrideParameterObjectWriter::ShouldSkipProperty(const FProperty* InProperty) const
	{
		if (InProperty->HasAnyPropertyFlags(CPF_Transient | CPF_DuplicateTransient | CPF_NonPIEDuplicateTransient | CPF_DisableEditOnInstance)
			|| InProperty->IsA<FMulticastDelegateProperty>()
			|| InProperty->IsA<FDelegateProperty>()
			)
		{
			return true;
		}
		if (CurrentIsMemberProperty(*this))
		{
			if (OverridePropertyNames.Contains(InProperty->GetFName()))
			{
				return false;
			}
			else
			{
				return true;
			}
		}

		return false;
	}
	bool FLexUIDuplicateOverrideParameterObjectWriter::SerializeObject(UObject* Object)
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
				auto type = (uint8)EObjectType::NativeSerializeForDuplicate;
				*this << type;
				ByteOrderSerialize(&Object, sizeof(Object));
				return true;
			}
		}
	}
	FString FLexUIDuplicateOverrideParameterObjectWriter::GetArchiveName() const
	{
		return TEXT("FLexUIDuplicateOverrideParameterObjectWriter");
	}



	FLexUIDuplicateOverrideParameterObjectReader::FLexUIDuplicateOverrideParameterObjectReader(TArray< uint8 >& Bytes, WidgetSerializerBase& InSerializer, const TArray<FName>& InOverridePropertyNames)
		: FLexUIOverrideParameterObjectReader(Bytes, InSerializer, InOverridePropertyNames)
	{

	}
	bool FLexUIDuplicateOverrideParameterObjectReader::ShouldSkipProperty(const FProperty* InProperty) const
	{
		if (InProperty->HasAnyPropertyFlags(CPF_Transient | CPF_DuplicateTransient | CPF_NonPIEDuplicateTransient | CPF_DisableEditOnInstance)
			|| InProperty->IsA<FMulticastDelegateProperty>()
			|| InProperty->IsA<FDelegateProperty>()
			)
		{
			return true;
		}
		if (CurrentIsMemberProperty(*this))
		{
			if (OverridePropertyNames.Contains(InProperty->GetFName()))
			{
				return false;
			}
			else
			{
				return true;
			}
		}

		return false;
	}
	bool FLexUIDuplicateOverrideParameterObjectReader::SerializeObject(UObject*& Object, bool CanSerializeClass)
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
	FString FLexUIDuplicateOverrideParameterObjectReader::GetArchiveName() const
	{
		return TEXT("FLexUIDuplicateOverrideParameterObjectReader");
	}
}
