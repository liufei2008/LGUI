// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "PrefabSystem/LexUIObjectReaderAndWriter.h"
#include "PrefabSystem/WidgetSerializerBase.h"
#include "Serialization/MemoryReader.h"
#include "Engine/Blueprint.h"

namespace LexUIPrefabSystem
{
	bool LexUIPrefab_ShouldSkipProperty(const FProperty* InProperty)
	{
		return
			InProperty->HasAnyPropertyFlags(CPF_Transient | CPF_NonPIEDuplicateTransient | CPF_DisableEditOnInstance)
			|| InProperty->IsA<FMulticastDelegateProperty>()
			|| InProperty->IsA<FDelegateProperty>()
			;
	}

	FLexUIObjectWriter::FLexUIObjectWriter(TArray< uint8 >& Bytes, WidgetSerializerBase& InSerializer, TSet<FName> InSkipPropertyNames)
		: FObjectWriter(Bytes)
		, Serializer(InSerializer)
		, SkipPropertyNames(InSkipPropertyNames)
	{
		SetIsLoading(false);
		SetIsSaving(true);

		Serializer.SetupArchive(*this);
	}
	void FLexUIObjectWriter::DoSerialize(UObject* Object)
	{
		Object->Serialize(*this);
	}
	bool FLexUIObjectWriter::ShouldSkipProperty(const FProperty* InProperty) const
	{
		if (LexUIPrefab_ShouldSkipProperty(InProperty))
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
	FArchive& FLexUIObjectWriter::operator<<(FName& N)
	{
		auto id = Serializer.FindOrAddNameFromList(N);
		*this << id;

		return *this;
	}

	FArchive& FLexUIObjectWriter::operator<<(FText& Value)
	{
#if WITH_EDITOR
		if (Serializer.PrefabVersion < (uint16)ELexUIPrefabVersion::FTextAsReference)
		{
			return FArchive::operator<<(Value);
		}
		else
#endif
		{
			auto id = Serializer.FindOrAddTextFromList(Value);
			*this << id;
			return *this;
		}
	}

	bool FLexUIObjectWriter::SerializeObject(UObject* Object)
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
				Serializer.CollectObjectToSerialize(Object, guid);
			}
			else
			{
				FGuid guid;
				canSerializeObject = Serializer.CollectObjectToSerialize(Object, guid);
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
	FArchive& FLexUIObjectWriter::operator<<(UObject*& Res)
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
	FArchive& FLexUIObjectWriter::operator<<(FObjectPtr& Value)
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
	FArchive& FLexUIObjectWriter::operator<<(FWeakObjectPtr& Value)
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
	FArchive& FLexUIObjectWriter::operator<<(FLazyObjectPtr& Value)
	{
		return FObjectWriter::operator<<(Value);
	}
	FArchive& FLexUIObjectWriter::operator<<(FSoftObjectPtr& Value)
	{
		return FObjectWriter::operator<<(Value);
	}
	FArchive& FLexUIObjectWriter::operator<<(FSoftObjectPath& Value)
	{
		return FObjectWriter::operator<<(Value);
	}
	FString FLexUIObjectWriter::GetArchiveName() const
	{
		return TEXT("FLexUIObjectWriter");
	}


	FLexUIObjectReader::FLexUIObjectReader(TArray< uint8 >& Bytes, WidgetSerializerBase& InSerializer, TSet<FName> InSkipPropertyNames)
		: FObjectReader(Bytes)
		, Serializer(InSerializer)
		, SkipPropertyNames(InSkipPropertyNames)
	{
		SetIsLoading(true);
		SetIsSaving(false);

		Serializer.SetupArchive(*this);
	}
	void FLexUIObjectReader::DoSerialize(UObject* Object)
	{
		Object->Serialize(*this);
	}
	bool FLexUIObjectReader::ShouldSkipProperty(const FProperty* InProperty) const
	{
		if (LexUIPrefab_ShouldSkipProperty(InProperty))
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
	FArchive& FLexUIObjectReader::operator<<(FName& N)
	{
		int32 id = -1;
		*this << id;
		N = Serializer.FindNameFromListByIndex(id);

		return *this;
	}

	FArchive& FLexUIObjectReader::operator<<(FText& Value)
	{
#if WITH_EDITOR
		if (Serializer.PrefabVersion < (uint16)ELexUIPrefabVersion::FTextAsReference)
		{
			return FArchive::operator<<(Value);
		}
		else
#endif
		{
			int32 id = -1;
			*this << id;
			Value = Serializer.FindTextFromListByIndex(id);
			
			return *this;
		}
	}

	bool FLexUIObjectReader::SerializeObject(UObject*& Object, bool CanSerializeClass)
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
	FArchive& FLexUIObjectReader::operator<<(UObject*& Value)
	{
		UObject* Res = nullptr;
		SerializeObject(Res, true);
		if (Res)
		{
			Value = Res;
		}
		return *this;
	}
	FArchive& FLexUIObjectReader::operator<<(FObjectPtr& Value)
	{
		UObject* Res = nullptr;
		SerializeObject(Res, true);
		if (Res)
		{
			Value = Res;
		}
		return *this;
	}
	FArchive& FLexUIObjectReader::operator<<(FWeakObjectPtr& Value)
	{
		UObject* Res = nullptr;
		SerializeObject(Res, false);
		if (Res)
		{
			Value = Res;
		}
		return *this;
	}
	FArchive& FLexUIObjectReader::operator<<(FLazyObjectPtr& Value)
	{
		return FObjectReader::operator<<(Value);
	}
	FArchive& FLexUIObjectReader::operator<<(FSoftObjectPtr& Value)
	{
		return FObjectReader::operator<<(Value);
	}
	FArchive& FLexUIObjectReader::operator<<(FSoftObjectPath& Value)
	{
		return FObjectReader::operator<<(Value);
	}
	FString FLexUIObjectReader::GetArchiveName() const
	{
		return TEXT("FLexUIObjectReader");
	}
}
