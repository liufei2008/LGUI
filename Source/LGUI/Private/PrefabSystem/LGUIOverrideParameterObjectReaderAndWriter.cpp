// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "PrefabSystem/LGUIObjectReaderAndWriter.h"
#include "PrefabSystem/ActorSerializerBase.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/BufferArchive.h"
#include "LGUI.h"

namespace LGUIPrefabSystem
{
	FLGUIOverrideParameterObjectWriter::FLGUIOverrideParameterObjectWriter(TArray< uint8 >& Bytes, ActorSerializerBase& InSerializer, const TArray<FName>& InOverridePropertyNames)
		: FLGUIObjectWriter(Bytes, InSerializer, {})
		, OverridePropertyNames(InOverridePropertyNames)
	{
		
	}
	bool FLGUIOverrideParameterObjectWriter::ShouldSkipProperty(const FProperty* InProperty) const
	{
		if (LGUIPrefab_ShouldSkipProperty(InProperty))
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
	bool FLGUIOverrideParameterObjectWriter::SerializeObject(UObject* Object)
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
	FString FLGUIOverrideParameterObjectWriter::GetArchiveName() const
	{
		return TEXT("FLGUIOverrideParameterObjectWriter");
	}


	FLGUIOverrideParameterObjectReader::FLGUIOverrideParameterObjectReader(TArray< uint8 >& Bytes, ActorSerializerBase& InSerializer, const TArray<FName>& InOverridePropertyNames)
		: FLGUIObjectReader(Bytes, InSerializer, {})
		, OverridePropertyNames(InOverridePropertyNames)
	{
		
	}
	bool FLGUIOverrideParameterObjectReader::ShouldSkipProperty(const FProperty* InProperty) const
	{
		if (LGUIPrefab_ShouldSkipProperty(InProperty))
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
	bool FLGUIOverrideParameterObjectReader::SerializeObject(UObject*& Object, bool CanSerializeClass)
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
		}
		return false;
	}
	FString FLGUIOverrideParameterObjectReader::GetArchiveName() const
	{
		return TEXT("FLGUIOverrideParameterObjectReader");
	}





	FLGUIImmediateOverrideParameterObjectWriter::FLGUIImmediateOverrideParameterObjectWriter(UObject* Object, TArray< uint8 >& Bytes, ActorSerializerBase& Serializer, const TArray<FName>& InOverridePropertyNames)
		: FObjectWriter(Bytes)
		, OverridePropertyNames(InOverridePropertyNames)
	{
		SetIsLoading(false);
		SetIsSaving(true);

		Serializer.SetupArchive(*this);

		Object->Serialize(*this);
	}
	bool FLGUIImmediateOverrideParameterObjectWriter::ShouldSkipProperty(const FProperty* InProperty) const
	{
		if (LGUIPrefab_ShouldSkipProperty(InProperty))
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
	FString FLGUIImmediateOverrideParameterObjectWriter::GetArchiveName() const
	{
		return TEXT("FLGUIImmediateOverrideParameterObjectWriter");
	}


	FLGUIImmediateOverrideParameterObjectReader::FLGUIImmediateOverrideParameterObjectReader(UObject* Object, TArray< uint8 >& Bytes, ActorSerializerBase& Serializer, const TArray<FName>& InOverridePropertyNames)
		: FObjectReader(Bytes)
		, OverridePropertyNames(InOverridePropertyNames)
	{
		SetIsLoading(true);
		SetIsSaving(false);

		Serializer.SetupArchive(*this);

		Object->Serialize(*this);
	}
	bool FLGUIImmediateOverrideParameterObjectReader::ShouldSkipProperty(const FProperty* InProperty) const
	{
		if (LGUIPrefab_ShouldSkipProperty(InProperty))
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
	FString FLGUIImmediateOverrideParameterObjectReader::GetArchiveName() const
	{
		return TEXT("FLGUIImmediateOverrideParameterObjectReader");
	}
}
