// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "PrefabSystem/LGUIObjectReaderAndWriter.h"
#include "PrefabSystem/ActorSerializerBase.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/BufferArchive.h"
#include "GameFramework/Actor.h"
#include "LGUI.h"

namespace LGUIPrefabSystem
{
	FLGUIDuplicateOverrideParameterObjectWriter::FLGUIDuplicateOverrideParameterObjectWriter(TArray< uint8 >& Bytes, ActorSerializerBase& InSerializer, const TArray<FName>& InOverridePropertyNames)
		: FLGUIOverrideParameterObjectWriter(Bytes, InSerializer, InOverridePropertyNames)
	{
		
	}
	bool FLGUIDuplicateOverrideParameterObjectWriter::SerializeObject(UObject* Object)
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



	FLGUIDuplicateOverrideParameterObjectReader::FLGUIDuplicateOverrideParameterObjectReader(TArray< uint8 >& Bytes, ActorSerializerBase& InSerializer, const TArray<FName>& InOverridePropertyNames)
		: FLGUIOverrideParameterObjectReader(Bytes, InSerializer, InOverridePropertyNames)
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
		case LGUIPrefabSystem::EObjectType::Function:
		{
			int32 OuterClasstId = -1;
			int32 FunctionNameId = -1;
			*this << OuterClasstId;
			*this << FunctionNameId;
			auto OuterClass = Serializer.FindClassFromListByIndex(OuterClasstId);
			auto FunctionName = Serializer.FindNameFromListByIndex(FunctionNameId);
			Object = OuterClass->FindFunctionByName(FunctionName);
			return true;
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
