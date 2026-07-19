// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "PrefabSystem/LexUIObjectReaderAndWriter.h"
#include "PrefabSystem/WidgetSerializerBase.h"
#include "Serialization/MemoryReader.h"
#include "Engine/Blueprint.h"

namespace LexUIPrefabSystem
{
	FLexUIOverrideParameterObjectWriter::FLexUIOverrideParameterObjectWriter(TArray< uint8 >& Bytes, WidgetSerializerBase& InSerializer, const TArray<FName>& InOverridePropertyNames)
		: FLexUIObjectWriter(Bytes, InSerializer, {})
		, OverridePropertyNames(InOverridePropertyNames)
	{
		
	}
	bool FLexUIOverrideParameterObjectWriter::ShouldSkipProperty(const FProperty* InProperty) const
	{
		if (LexUIPrefab_ShouldSkipProperty(InProperty))
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
	bool FLexUIOverrideParameterObjectWriter::SerializeObject(UObject* Object)
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
			if (guidPtr != nullptr)
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
	FString FLexUIOverrideParameterObjectWriter::GetArchiveName() const
	{
		return TEXT("FLexUIOverrideParameterObjectWriter");
	}


	FLexUIOverrideParameterObjectReader::FLexUIOverrideParameterObjectReader(TArray< uint8 >& Bytes, WidgetSerializerBase& InSerializer, const TArray<FName>& InOverridePropertyNames)
		: FLexUIObjectReader(Bytes, InSerializer, {})
		, OverridePropertyNames(InOverridePropertyNames)
	{
		
	}
	bool FLexUIOverrideParameterObjectReader::ShouldSkipProperty(const FProperty* InProperty) const
	{
		if (LexUIPrefab_ShouldSkipProperty(InProperty))
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
	bool FLexUIOverrideParameterObjectReader::SerializeObject(UObject*& Object, bool CanSerializeClass)
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
		}
		return false;
	}
	FString FLexUIOverrideParameterObjectReader::GetArchiveName() const
	{
		return TEXT("FLexUIOverrideParameterObjectReader");
	}





	FLexUIImmediateOverrideParameterObjectWriter::FLexUIImmediateOverrideParameterObjectWriter(UObject* Object, TArray< uint8 >& Bytes, WidgetSerializerBase& Serializer, const TArray<FName>& InOverridePropertyNames)
		: FObjectWriter(Bytes)
		, OverridePropertyNames(InOverridePropertyNames)
	{
		SetIsLoading(false);
		SetIsSaving(true);

		Serializer.SetupArchive(*this);

		Object->Serialize(*this);
	}
	bool FLexUIImmediateOverrideParameterObjectWriter::ShouldSkipProperty(const FProperty* InProperty) const
	{
		if (LexUIPrefab_ShouldSkipProperty(InProperty))
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
	FString FLexUIImmediateOverrideParameterObjectWriter::GetArchiveName() const
	{
		return TEXT("FLexUIImmediateOverrideParameterObjectWriter");
	}


	FLexUIImmediateOverrideParameterObjectReader::FLexUIImmediateOverrideParameterObjectReader(UObject* Object, TArray< uint8 >& Bytes, WidgetSerializerBase& Serializer, const TArray<FName>& InOverridePropertyNames)
		: FObjectReader(Bytes)
		, OverridePropertyNames(InOverridePropertyNames)
	{
		SetIsLoading(true);
		SetIsSaving(false);

		Serializer.SetupArchive(*this);

		Object->Serialize(*this);
	}
	bool FLexUIImmediateOverrideParameterObjectReader::ShouldSkipProperty(const FProperty* InProperty) const
	{
		if (LexUIPrefab_ShouldSkipProperty(InProperty))
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
	FString FLexUIImmediateOverrideParameterObjectReader::GetArchiveName() const
	{
		return TEXT("FLexUIImmediateOverrideParameterObjectReader");
	}
}
