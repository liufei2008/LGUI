// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "PrefabSystem/LexUIObjectReaderAndWriter.h"
#include "PrefabSystem/WidgetSerializerBase.h"
#include "Serialization/StructuredArchiveAdapters.h"

namespace LexUIPrefabSystem
{
	FLexUIOverrideParameterObjectWriter::FLexUIOverrideParameterObjectWriter(TArray< uint8 >& Bytes, WidgetSerializerBase& InSerializer, const TArray<FName>& InOverridePropertyNames)
		: FLexUIObjectWriter(Bytes, InSerializer)
		, OverridePropertyNames(InOverridePropertyNames)
	{
		//An override must be written even when it equals the archetype value, otherwise the sub-prefab's own value wins on load and the override is silently lost.
		ArNoDelta = true;
	}
	bool FLexUIOverrideParameterObjectWriter::ShouldSkipProperty(const FProperty* InProperty) const
	{
		if (LexUIPrefab_ShouldSkipProperty(InProperty))
		{
			return true;
		}

		if (LexUIPrefab_CurrentIsMemberProperty(this))
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
		: FLexUIObjectReader(Bytes, InSerializer)
		, OverridePropertyNames(InOverridePropertyNames)
	{
		
	}
	bool FLexUIOverrideParameterObjectReader::ShouldSkipProperty(const FProperty* InProperty) const
	{
		if (LexUIPrefab_ShouldSkipProperty(InProperty))
		{
			return true;
		}

		if (LexUIPrefab_CurrentIsMemberProperty(this))
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





	//The sub-property archives address one exact leaf, so they have no name list to filter against.
	static const TArray<FName> EmptyOverridePropertyNames;

	FLexUIOverrideSubPropertyValueWriter::FLexUIOverrideSubPropertyValueWriter(TArray< uint8 >& Bytes, WidgetSerializerBase& InSerializer)
		: FLexUIOverrideParameterObjectWriter(Bytes, InSerializer, EmptyOverridePropertyNames)
	{

	}
	bool FLexUIOverrideSubPropertyValueWriter::ShouldSkipProperty(const FProperty* InProperty) const
	{
		//No name filtering: the caller already picked the exact leaf. Keeping the base implementation would filter
		//everything away, because a plain USTRUCT leaf recurses into SerializeTaggedProperties with an empty property
		//chain, which the base reads as "a member property that is not in the override list".
		return LexUIPrefab_ShouldSkipProperty(InProperty);
	}
	void FLexUIOverrideSubPropertyValueWriter::SerializeValue(FProperty* InLeafProperty, void* InLeafValuePtr)
	{
		//FLexUIObjectWriter::operator<<(UObject*&) inspects GetSerializedProperty() to recognise an FClassProperty.
		//Calling SerializeItem directly would leave it unset, and a UClass* leaf would be written as EObjectType::None.
		SetSerializedProperty(InLeafProperty);
		{
			FStructuredArchiveFromArchive Adapter(*this);
			//Defaults = nullptr, so there is no delta against the archetype and a value equal to the CDO still writes.
			InLeafProperty->SerializeItem(Adapter.GetSlot(), InLeafValuePtr, nullptr);
		}
		SetSerializedProperty(nullptr);
	}
	FString FLexUIOverrideSubPropertyValueWriter::GetArchiveName() const
	{
		return TEXT("FLexUIOverrideSubPropertyValueWriter");
	}


	FLexUIOverrideSubPropertyValueReader::FLexUIOverrideSubPropertyValueReader(TArray< uint8 >& Bytes, WidgetSerializerBase& InSerializer)
		: FLexUIOverrideParameterObjectReader(Bytes, InSerializer, EmptyOverridePropertyNames)
	{

	}
	bool FLexUIOverrideSubPropertyValueReader::ShouldSkipProperty(const FProperty* InProperty) const
	{
		return LexUIPrefab_ShouldSkipProperty(InProperty);
	}
	void FLexUIOverrideSubPropertyValueReader::SerializeValue(FProperty* InLeafProperty, void* InLeafValuePtr)
	{
		SetSerializedProperty(InLeafProperty);
		{
			FStructuredArchiveFromArchive Adapter(*this);
			InLeafProperty->SerializeItem(Adapter.GetSlot(), InLeafValuePtr, nullptr);
		}
		SetSerializedProperty(nullptr);
	}
	FString FLexUIOverrideSubPropertyValueReader::GetArchiveName() const
	{
		return TEXT("FLexUIOverrideSubPropertyValueReader");
	}


	FLexUIImmediateOverrideParameterObjectWriter::FLexUIImmediateOverrideParameterObjectWriter(UObject* Object, TArray< uint8 >& Bytes, WidgetSerializerBase& Serializer, const TArray<FName>& InOverridePropertyNames)
		: FObjectWriter(Bytes)
		, OverridePropertyNames(InOverridePropertyNames)
	{
		SetIsLoading(false);
		SetIsSaving(true);

		Serializer.SetupArchive(*this);
		//An override must be written even when it equals the archetype value, otherwise the sub-prefab's own value wins on restore and the override is silently lost.
		ArNoDelta = true;

		Object->Serialize(*this);
	}
	bool FLexUIImmediateOverrideParameterObjectWriter::ShouldSkipProperty(const FProperty* InProperty) const
	{
		if (LexUIPrefab_ShouldSkipProperty(InProperty))
		{
			return true;
		}

		if (LexUIPrefab_CurrentIsMemberProperty(this))
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

		if (LexUIPrefab_CurrentIsMemberProperty(this))
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
