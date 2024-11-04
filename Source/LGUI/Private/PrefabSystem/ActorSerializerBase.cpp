// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "PrefabSystem/ActorSerializerBase.h"
#include "Engine/World.h"
#include "Core/LGUIManager.h"
#include "PrefabSystem/LGUIObjectReaderAndWriter.h"
#include "LGUI.h"
#include "Misc/ConfigCacheIni.h"
#if WITH_EDITOR
#include "Tools/UEdMode.h"
#include "Utils/LGUIUtils.h"
#endif

namespace LGUIPrefabSystem
{
	bool ActorSerializerBase::ObjectIsTrash(UObject* InObject)
	{
		UObject* Outer = InObject;
		while (Outer != nullptr)
		{
			if (Outer->GetName().StartsWith(TEXT("TRASH_")))
			{
				return true;
			}
			Outer = Outer->GetOuter();
		}
		return false;
	}


	//only allow object that belongs to some actor of this prefab
	bool ActorSerializerBase::ObjectBelongsToThisPrefab(UObject* InObject)
	{
		if (WillSerializeActorArray.Contains(InObject))
		{
			return true;
		}

		UObject* Outer = InObject->GetOuter();
		while (Outer != nullptr
			&& !Outer->HasAnyFlags(EObjectFlags::RF_Transient)
			)
		{
			if (WillSerializeActorArray.Contains(Outer))
			{
				return true;
			}
			else
			{
				if (Outer->GetClass()->IsChildOf(AActor::StaticClass())
					)//not exist in WillSerailizeActorArray, but is a actor, means it not belongs to this prefab
				{
					return false;
				}
			}
			Outer = Outer->GetOuter();
		}
		return false;
	}

	bool ActorSerializerBase::CollectObjectToSerailize(UObject* Object, FGuid& OutGuid)
	{
		if (!IsValid(Object))return false;
		if (!Object->IsValidLowLevel())return false;
		if (Object->IsUnreachable())return false;
#if WITH_EDITOR
		if (Object->GetClass()->IsChildOf(UEdMode::StaticClass()))return false;
		if (ObjectIsTrash(Object))return false;
		if (Object->GetClass()->IsChildOf(UActorComponent::StaticClass()) && ((UActorComponent*)Object)->IsVisualizationComponent())return false;//skip visualization component
#endif
		if (Object->IsEditorOnly() && !bIsEditorOrRuntime)return false;
		if (Object->IsAsset())return false;//skip asset, because asset is referenced directly
		if (Object->GetWorld() != TargetWorld)return false;
		if (Object->HasAnyFlags(EObjectFlags::RF_Transient))return false;//skip transient object
		if (Object->GetClass()->IsChildOf(AActor::StaticClass()))return false;//skip actor, because actor is collected in WillSerializeActorArray
		if (WillSerializeActorArray.Contains(Object))return false;//already contains it (double check)
		if (!ObjectBelongsToThisPrefab(Object))return false;
		/** 
		 * I have no idea why use these two flags, just tested for UModel and get that these two flags can filter out most classes like:
		 * XXXProperty, XXXDelegatepropertyWrapper, XXXWrapperObject, BlueprintMacros, BlueprintFunctionLibrary, XXX__PythonCallable
		 * we don't need to serialize these classes of objects, like UModel will crash LGUI's serialization process
		 */
		if (!Object->GetClass()->HasAnyClassFlags(EClassFlags::CLASS_Constructed | EClassFlags::CLASS_HasInstancedReference))return false;

		if (WillSerializeObjectArray.Contains(Object))
		{
			auto GuidPtr = MapObjectToGuid.Find(Object);
			check(GuidPtr != nullptr);
			OutGuid = *GuidPtr;
			return true;//already contains object
		}

		auto Outer = Object->GetOuter();
		check(Outer != nullptr);

		if (WillSerializeActorArray.Contains(Outer))//outer is actor
		{
			WillSerializeObjectArray.Add(Object);
			if (auto GuidPtr = MapObjectToGuid.Find(Object))
			{
				OutGuid = *GuidPtr;
			}
			else
			{
				OutGuid = FGuid::NewGuid();
				MapObjectToGuid.Add(Object, OutGuid);
			}
			return true;
		}
		else//could have nested object outer
		{
			if (auto GuidPtr = MapObjectToGuid.Find(Object))
			{
				OutGuid = *GuidPtr;
			}
			else
			{
				OutGuid = FGuid::NewGuid();
				MapObjectToGuid.Add(Object, OutGuid);
			}
			auto Index = WillSerializeObjectArray.Add(Object);
			while (Outer != nullptr
				&& !WillSerializeActorArray.Contains(Outer)//Make sure Outer is not actor, because actor is created before any other objects, they will be stored in actor's data
				&& !WillSerializeObjectArray.Contains(Outer)//Make sure Outer is not inside array
				)
			{
				WillSerializeObjectArray.Insert(Outer, Index);//insert before object
				if (!MapObjectToGuid.Contains(Outer))
				{
					MapObjectToGuid.Add(Outer, FGuid::NewGuid());
				}
				Outer = Outer->GetOuter();
			}
			return true;
		}
	}

	TMap<UObject*, TArray<uint8>> ActorSerializerBase::SaveOverrideParameterToData(TArray<FLGUIPrefabOverrideParameterData> InData)
	{
		this->bIsEditorOrRuntime = true;
		TMap<UObject*, TArray<uint8>> MapObjectToOverrideDatas;
		for (auto& DataItem : InData)
		{
			TArray<uint8> ObjectOverrideData;
			FLGUIImmediateOverrideParameterObjectWriter Writer(DataItem.Object.Get(), ObjectOverrideData, *this, DataItem.MemberPropertyNames);
			MapObjectToOverrideDatas.Add(DataItem.Object.Get(), ObjectOverrideData);
		}
		return MapObjectToOverrideDatas;
	}

	void ActorSerializerBase::RestoreOverrideParameterFromData(TMap<UObject*, TArray<uint8>>& InData, TArray<FLGUIPrefabOverrideParameterData> InNameSetData)
	{
		this->bIsEditorOrRuntime = true;
		for (auto& KeyValue : InData)
		{
			if (IsValid(KeyValue.Key))
			{
				auto Index = InNameSetData.IndexOfByPredicate([&](const FLGUIPrefabOverrideParameterData& Item) {
					return Item.Object.Get() == KeyValue.Key;
					});
				if (Index != INDEX_NONE)
				{
					FLGUIImmediateOverrideParameterObjectReader Reader(KeyValue.Key, KeyValue.Value, *this, InNameSetData[Index].MemberPropertyNames);
				}
			}
		}
	}


	int32 ActorSerializerBase::FindOrAddAssetIdFromList(UObject* AssetObject)
	{
		if (!AssetObject)return -1;
		int32 resultIndex;
		if (ReferenceAssetList.Find(AssetObject, resultIndex))
		{
			return resultIndex;//return index if found
		}
		else//add to list if not found
		{
			ReferenceAssetList.Add(AssetObject);
			return ReferenceAssetList.Num() - 1;
		}
	}

	int32 ActorSerializerBase::FindOrAddClassFromList(UClass* Class)
	{
		if (!Class)return -1;
		int32 resultIndex;
		if (ReferenceClassList.Find(Class, resultIndex))
		{
			return resultIndex;
		}
		else
		{
			ReferenceClassList.Add(Class);
			return ReferenceClassList.Num() - 1;
		}
	}
	int32 ActorSerializerBase::FindOrAddNameFromList(const FName& Name)
	{
		if (!Name.IsValid())return -1;
		int32 resultIndex;
		if (ReferenceNameList.Find(Name, resultIndex))
		{
			return resultIndex;
		}
		else
		{
			ReferenceNameList.Add(Name);
			return ReferenceNameList.Num() - 1;
		}
	}
	FName ActorSerializerBase::FindNameFromListByIndex(int32 Id)
	{
		return ReferenceNameList.IsValidIndex(Id) ? ReferenceNameList.GetData()[Id] : NAME_None;
	}

	UObject* ActorSerializerBase::FindAssetFromListByIndex(int32 Id)
	{
		return ReferenceAssetList.IsValidIndex(Id) ? ReferenceAssetList.GetData()[Id] : nullptr;
	}

	UClass* ActorSerializerBase::FindClassFromListByIndex(int32 Id)
	{
		return ReferenceClassList.IsValidIndex(Id) ? ReferenceClassList.GetData()[Id] : nullptr;
	}

	const TSet<FName>& ActorSerializerBase::GetSceneComponentExcludeProperties()
	{
		static TSet<FName> result = {
			FName("AttachParent") //exclude AttachParent property, because we need to mannually do the attachment so that AttachChildren property is good
		};
		return result;
	}

	bool ActorSerializerBase::CanUseUnversionedPropertySerialization()
	{
		bool bTemp;
		static bool bAllow = GConfig->GetBool(TEXT("Core.System"), TEXT("CanUseUnversionedPropertySerialization"), bTemp, GEngineIni) && bTemp;
		return bAllow;
	}

	void ActorSerializerBase::SetupArchive(FArchive& InArchive)
	{
		if (!bIsEditorOrRuntime && CanUseUnversionedPropertySerialization())
		{
			InArchive.SetUseUnversionedPropertySerialization(true);
		}
		InArchive.SetFilterEditorOnly(!bIsEditorOrRuntime);
		InArchive.SetWantBinaryPropertySerialization(!bIsEditorOrRuntime);

		InArchive.ArNoDelta = true;
		InArchive.ArNoIntraPropertyDelta = true;

		if (InArchive.IsLoading() && bOverrideVersions)
		{
			InArchive.SetUEVer(this->ArchiveVersion);
			InArchive.SetLicenseeUEVer(this->ArchiveLicenseeVer);
			if (!this->ArEngineVer.IsEmpty()) InArchive.SetEngineVer(this->ArEngineVer);
			InArchive.SetEngineNetVer(this->ArEngineNetVer);
			InArchive.SetGameNetVer(this->ArGameNetVer);
		}
	}
}
