// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "PrefabSystem/LGUIObjectReaderAndWriter.h"
#include "PrefabSystem/ActorSerializer3.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/BufferArchive.h"
#include "LGUI.h"

namespace LGUIPrefabSystem3
{
	FLGUIOverrideParameterObjectWriter::FLGUIOverrideParameterObjectWriter(UObject* Object, TArray< uint8 >& Bytes, ActorSerializer3& InSerializer, const TSet<FName>& InOverridePropertyNames)
		: FObjectWriter(Bytes)
		, Serializer(InSerializer)
		, OverridePropertyNames(InOverridePropertyNames)
	{
		this->Reset();

		SetIsLoading(false);
		SetIsSaving(true);

		Serializer.SetupArchive(*this);

		Object->Serialize(*this);
	}
	bool FLGUIOverrideParameterObjectWriter::ShouldSkipProperty(const FProperty* InProperty) const
	{
		if (InProperty->HasAnyPropertyFlags(CPF_Transient)
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
			bool canSerializaObject = false;
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
#if ENGINE_MAJOR_VERSION >= 5
	FArchive& FLGUIOverrideParameterObjectWriter::operator<<(FObjectPtr& Value)
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
	FArchive& FLGUIOverrideParameterObjectWriter::operator<<(FLazyObjectPtr& LazyObjectPtr)
	{
		UE_LOG(LGUI, Warning, TEXT("[FLGUIOverrideParameterObjectWriter]Detect LazyObjectPtr property, which is not supported by LGUIPrefab!"));
		return *this;
	}
	FArchive& FLGUIOverrideParameterObjectWriter::operator<<(FSoftObjectPtr& Value)
	{
		UE_LOG(LGUI, Warning, TEXT("[FLGUIOverrideParameterObjectWriter]Detect SoftObjectPtr property, which is not supported by LGUIPrefab!"));
		return *this;
	}
	FArchive& FLGUIOverrideParameterObjectWriter::operator<<(FSoftObjectPath& Value)
	{
		UE_LOG(LGUI, Warning, TEXT("[FLGUIOverrideParameterObjectWriter]Detect SoftObjectPath property, which is not supported by LGUIPrefab!"));
		return *this;
	}
	FString FLGUIOverrideParameterObjectWriter::GetArchiveName() const
	{
		return TEXT("FLGUIOverrideParameterObjectWriter");
	}


	FLGUIOverrideParameterObjectReader::FLGUIOverrideParameterObjectReader(UObject* Object, TArray< uint8 >& Bytes, ActorSerializer3& InSerializer, const TSet<FName>& InOverridePropertyNames)
		: FObjectReader(Bytes)
		, Serializer(InSerializer)
		, OverridePropertyNames(InOverridePropertyNames)
	{
		this->Reset();

		SetIsLoading(true);
		SetIsSaving(false);

		Serializer.SetupArchive(*this);

		Object->Serialize(*this);
	}
	bool FLGUIOverrideParameterObjectReader::ShouldSkipProperty(const FProperty* InProperty) const
	{
		if (InProperty->HasAnyPropertyFlags(CPF_Transient)
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
	FArchive& FLGUIOverrideParameterObjectReader::operator<<(UObject*& Res)
	{
		SerializeObject(Res, true);
		return *this;
	}
#if ENGINE_MAJOR_VERSION >= 5
	FArchive& FLGUIOverrideParameterObjectReader::operator<<(FObjectPtr& Value)
	{
		UObject* Res = nullptr;
		SerializeObject(Res, false);
		Value = Res;
		return *this;
	}
#endif
	FArchive& FLGUIOverrideParameterObjectReader::operator<<(FWeakObjectPtr& Value)
	{
		UObject* Res = nullptr;
		SerializeObject(Res, false);
		Value = Res;
		return *this;
	}
	FArchive& FLGUIOverrideParameterObjectReader::operator<<(FLazyObjectPtr& LazyObjectPtr)
	{
		UE_LOG(LGUI, Warning, TEXT("[FLGUIOverrideParameterObjectReader]Detect LazyObjectPtr property, which is not supported by LGUIPrefab!"));
		return *this;
	}
	FArchive& FLGUIOverrideParameterObjectReader::operator<<(FSoftObjectPtr& Value)
	{
		UE_LOG(LGUI, Warning, TEXT("[FLGUIOverrideParameterObjectReader]Detect SoftObjectPtr property, which is not supported by LGUIPrefab!"));
		return *this;
	}
	FArchive& FLGUIOverrideParameterObjectReader::operator<<(FSoftObjectPath& Value)
	{
		UE_LOG(LGUI, Warning, TEXT("[FLGUIOverrideParameterObjectReader]Detect SoftObjectPath property, which is not supported by LGUIPrefab!"));
		return *this;
	}
	FString FLGUIOverrideParameterObjectReader::GetArchiveName() const
	{
		return TEXT("FLGUIOverrideParameterObjectReader");
	}





	FLGUIImmediateOverrideParameterObjectWriter::FLGUIImmediateOverrideParameterObjectWriter(UObject* Object, TArray< uint8 >& Bytes, const TSet<FName>& InOverridePropertyNames)
		: FObjectWriter(Bytes)
		, OverridePropertyNames(InOverridePropertyNames)
	{
		this->Reset();

		SetIsLoading(false);
		SetIsSaving(true);

		Object->Serialize(*this);
	}
	bool FLGUIImmediateOverrideParameterObjectWriter::ShouldSkipProperty(const FProperty* InProperty) const
	{
		if (InProperty->HasAnyPropertyFlags(CPF_Transient)
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
	FString FLGUIImmediateOverrideParameterObjectWriter::GetArchiveName() const
	{
		return TEXT("FLGUIImmediateOverrideParameterObjectWriter");
	}


	FLGUIImmediateOverrideParameterObjectReader::FLGUIImmediateOverrideParameterObjectReader(UObject* Object, TArray< uint8 >& Bytes, const TSet<FName>& InOverridePropertyNames)
		: FObjectReader(Bytes)
		, OverridePropertyNames(InOverridePropertyNames)
	{
		this->Reset();

		SetIsLoading(true);
		SetIsSaving(false);

		Object->Serialize(*this);
	}
	bool FLGUIImmediateOverrideParameterObjectReader::ShouldSkipProperty(const FProperty* InProperty) const
	{
		if (InProperty->HasAnyPropertyFlags(CPF_Transient)
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
	FString FLGUIImmediateOverrideParameterObjectReader::GetArchiveName() const
	{
		return TEXT("FLGUIImmediateOverrideParameterObjectReader");
	}
}
