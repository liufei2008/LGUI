// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "PrefabSystem/LGUIObjectReaderAndWriter.h"
#include "PrefabSystem/ActorSerializerBase.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/BufferArchive.h"
#include "LGUI.h"

namespace LGUIPrefabSystem
{
	FLGUIOverrideParameterObjectWriter::FLGUIOverrideParameterObjectWriter(TArray< uint8 >& Bytes, ActorSerializerBase& InSerializer, const TSet<FName>& InOverridePropertyNames)
		: FObjectWriter(Bytes)
		, Serializer(InSerializer)
		, OverridePropertyNames(InOverridePropertyNames)
	{
		SetIsLoading(false);
		SetIsSaving(true);

		Serializer.SetupArchive(*this);
	}
	void FLGUIOverrideParameterObjectWriter::DoSerialize(UObject* Object)
	{
		Object->Serialize(*this);
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
	FArchive& FLGUIOverrideParameterObjectWriter::operator<<(class FName& N)
	{
		auto id = Serializer.FindOrAddNameFromList(N);
		*this << id;

		return *this;
	}
	bool FLGUIOverrideParameterObjectWriter::SerializeObject(UObject* Object)
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
	FArchive& FLGUIOverrideParameterObjectWriter::operator<<(UObject*& Res)
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
	FArchive& FLGUIOverrideParameterObjectWriter::operator<<(FWeakObjectPtr& Value)
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
	FArchive& FLGUIOverrideParameterObjectWriter::operator<<(FLazyObjectPtr& Value)
	{
		return FObjectWriter::operator<<(Value);
	}
	FArchive& FLGUIOverrideParameterObjectWriter::operator<<(FSoftObjectPtr& Value)
	{
		return FObjectWriter::operator<<(Value);
	}
	FArchive& FLGUIOverrideParameterObjectWriter::operator<<(FSoftObjectPath& Value)
	{
		return FObjectWriter::operator<<(Value);
	}
	FString FLGUIOverrideParameterObjectWriter::GetArchiveName() const
	{
		return TEXT("FLGUIOverrideParameterObjectWriter");
	}


	FLGUIOverrideParameterObjectReader::FLGUIOverrideParameterObjectReader(TArray< uint8 >& Bytes, ActorSerializerBase& InSerializer, const TSet<FName>& InOverridePropertyNames)
		: FObjectReader(Bytes)
		, Serializer(InSerializer)
		, OverridePropertyNames(InOverridePropertyNames)
	{
		SetIsLoading(true);
		SetIsSaving(false);

		Serializer.SetupArchive(*this);
	}
	void FLGUIOverrideParameterObjectReader::DoSerialize(UObject* Object)
	{
		Object->Serialize(*this);
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
	FArchive& FLGUIOverrideParameterObjectReader::operator<<(class FName& N)
	{
		int32 id = -1;
		*this << id;
		N = Serializer.FindNameFromListByIndex(id);

		return *this;
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
	FArchive& FLGUIOverrideParameterObjectReader::operator<<(UObject*& Res)
	{
		SerializeObject(Res, true);
		return *this;
	}
	FArchive& FLGUIOverrideParameterObjectReader::operator<<(FWeakObjectPtr& Value)
	{
		UObject* Res = nullptr;
		SerializeObject(Res, false);
		Value = Res;
		return *this;
	}
	FArchive& FLGUIOverrideParameterObjectReader::operator<<(FLazyObjectPtr& Value)
	{
		return FObjectReader::operator<<(Value);
	}
	FArchive& FLGUIOverrideParameterObjectReader::operator<<(FSoftObjectPtr& Value)
	{
		return FObjectReader::operator<<(Value);
	}
	FArchive& FLGUIOverrideParameterObjectReader::operator<<(FSoftObjectPath& Value)
	{
		return FObjectReader::operator<<(Value);
	}
	FString FLGUIOverrideParameterObjectReader::GetArchiveName() const
	{
		return TEXT("FLGUIOverrideParameterObjectReader");
	}





	FLGUIImmediateOverrideParameterObjectWriter::FLGUIImmediateOverrideParameterObjectWriter(UObject* Object, TArray< uint8 >& Bytes, ActorSerializerBase& Serializer, const TSet<FName>& InOverridePropertyNames)
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


	FLGUIImmediateOverrideParameterObjectReader::FLGUIImmediateOverrideParameterObjectReader(UObject* Object, TArray< uint8 >& Bytes, ActorSerializerBase& Serializer, const TSet<FName>& InOverridePropertyNames)
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
