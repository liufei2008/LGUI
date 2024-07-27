// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "PrefabSystem/LGUIObjectReaderAndWriter.h"
#include "PrefabSystem/ActorSerializerBase.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/BufferArchive.h"
#include "GameFramework/Actor.h"
#include "Engine/Blueprint.h"
#include "GameFramework/Actor.h"
#include "LGUI.h"

namespace LGUIPrefabSystem
{
	bool LGUIPrefab_ShouldSkipProperty(const FProperty* InProperty)
	{
		return
			InProperty->HasAnyPropertyFlags(CPF_Transient | CPF_NonPIEDuplicateTransient | CPF_DisableEditOnInstance)
			|| InProperty->IsA<FMulticastDelegateProperty>()
			|| InProperty->IsA<FDelegateProperty>()
			;
	}

	FLGUIObjectWriter::FLGUIObjectWriter(TArray< uint8 >& Bytes, ActorSerializerBase& InSerializer, TSet<FName> InSkipPropertyNames)
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
		if (LGUIPrefab_ShouldSkipProperty(InProperty))
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
		if (auto Function = Cast<UFunction>(Object))
		{
			auto OuterClass = Function->GetTypedOuter<UClass>();
			if (OuterClass != nullptr)
			{
				auto FunctionName = Function->GetFName();
				auto FunctionNameId = Serializer.FindOrAddNameFromList(FunctionName);
				auto OuterClassId = Serializer.FindOrAddClassFromList(OuterClass);
				auto type = (uint8)EObjectType::Function;
				*this << type;
				*this << OuterClassId;
				*this << FunctionNameId;
				return true;
			}
			return false;
		}
		//if (auto K2Node = Cast<UK2Node>(Object))//K2Node is editor only, so we just check the name
		if (Object->GetName().StartsWith(TEXT("K2Node_")))
		{
			auto OuterObject = Object->GetTypedOuter<UBlueprint>();
			if (OuterObject != nullptr)
			{
				auto NodeName = Object->GetFName();
				auto NameId = Serializer.FindOrAddNameFromList(NodeName);
				auto OuterObjectId = Serializer.FindOrAddAssetIdFromList(OuterObject);
				auto type = (uint8)EObjectType::K2Node;
				*this << type;
				*this << OuterObjectId;
				*this << NodeName;
				return true;
			}
			return false;
		}
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


	FLGUIObjectReader::FLGUIObjectReader(TArray< uint8 >& Bytes, ActorSerializerBase& InSerializer, TSet<FName> InSkipPropertyNames)
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
		if (LGUIPrefab_ShouldSkipProperty(InProperty))
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
		case LGUIPrefabSystem::EObjectType::Function:
		{
			int32 OuterClasstId = -1;
			int32 FunctionNameId = -1;
			*this << OuterClasstId;
			*this << FunctionNameId;
			if (auto OuterClass = Serializer.FindClassFromListByIndex(OuterClasstId))
			{
				auto FunctionName = Serializer.FindNameFromListByIndex(FunctionNameId);
				Object = OuterClass->FindFunctionByName(FunctionName);
				return true;
			}
		}
		break;
		case LGUIPrefabSystem::EObjectType::K2Node:
		{
			int32 OuterObjectId = -1;
			int32 NodeNameId = -1;
			*this << OuterObjectId;
			*this << NodeNameId;
			if (OuterObjectId != -1 && NodeNameId != -1)
			{
				auto OuterObject = Serializer.FindAssetFromListByIndex(OuterObjectId);
				if (OuterObject != nullptr)
				{
					auto NodeName = Serializer.FindNameFromListByIndex(NodeNameId);
					ForEachObjectWithOuter(OuterObject, [&NodeName, &Object](UObject* ItemObject) {
						if (NodeName == ItemObject->GetFName())
						{
							Object = ItemObject;
						}
						});
					return true;
				}
			}
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
	FArchive& FLGUIObjectReader::operator<<(UObject*& Value)
	{
		UObject* Res = nullptr;
		SerializeObject(Res, true);
		if (Res)
		{
			Value = Res;
		}
		return *this;
	}
	FArchive& FLGUIObjectReader::operator<<(FObjectPtr& Value)
	{
		UObject* Res = nullptr;
		SerializeObject(Res, true);
		if (Res)
		{
			Value = Res;
		}
		return *this;
	}
	FArchive& FLGUIObjectReader::operator<<(FWeakObjectPtr& Value)
	{
		UObject* Res = nullptr;
		SerializeObject(Res, false);
		if (Res)
		{
			Value = Res;
		}
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
