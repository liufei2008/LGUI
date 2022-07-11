// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "PrefabSystem/ActorSerializerBase.h"
#include "Engine/World.h"
#include "Core/Actor/LGUIManagerActor.h"
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


	bool ActorSerializerBase::ObjectBelongsToThisPrefab(UObject* InObject)
	{
		if (WillSerailizeActorArray.Contains(InObject))
		{
			return true;
		}

		UObject* Outer = InObject->GetOuter();
		while (Outer != nullptr
			&& !Outer->HasAnyFlags(EObjectFlags::RF_Transient)
			)
		{
			if (WillSerailizeActorArray.Contains(Outer))
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
#if WITH_EDITOR
		if (Object->GetClass()->IsChildOf(UEdMode::StaticClass()))return false;
		if (ObjectIsTrash(Object))return false;
#endif
		if (!Object->IsAsset()//skip asset, because asset is referenced directly
			&& Object->GetWorld() == TargetWorld
			&& !Object->IsPendingKillOrUnreachable()
			&& !Object->HasAnyFlags(EObjectFlags::RF_Transient)
			&& !WillSerailizeActorArray.Contains(Object)
			&& !Object->GetClass()->IsChildOf(AActor::StaticClass())//skip actor
			&& ObjectBelongsToThisPrefab(Object)
			)
		{
			if (WillSerailizeObjectArray.Contains(Object))
			{
				auto GuidPtr = MapObjectToGuid.Find(Object);
				check(GuidPtr != nullptr);
				OutGuid = *GuidPtr;
				return true;//already contains object
			}

			auto Outer = Object->GetOuter();
			check(Outer != nullptr);

			if (WillSerailizeActorArray.Contains(Outer))//outer is actor
			{
				WillSerailizeObjectArray.Add(Object);
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
				auto Index = WillSerailizeObjectArray.Add(Object);
				while (Outer != nullptr
					&& !WillSerailizeActorArray.Contains(Outer)//Make sure Outer is not actor, because actor is created before any other objects, they will be stored in actor's data
					&& !WillSerailizeObjectArray.Contains(Outer)//Make sure Outer is not inside array
					)
				{
					WillSerailizeObjectArray.Insert(Outer, Index);//insert before object
					if (!MapObjectToGuid.Contains(Outer))
					{
						MapObjectToGuid.Add(Outer, FGuid::NewGuid());
					}
					Outer = Outer->GetOuter();
				}
				return true;
			}
		}
		return false;
	}

	TMap<UObject*, TArray<uint8>> ActorSerializerBase::SaveOverrideParameterToData(TArray<FLGUIPrefabOverrideParameterData> InData)
	{
		this->bIsEditorOrRuntime = true;
		TMap<UObject*, TArray<uint8>> MapObjectToOverrideDatas;
		for (auto& DataItem : InData)
		{
			TArray<uint8> ObjectOverrideData;
			FLGUIImmediateOverrideParameterObjectWriter Writer(DataItem.Object.Get(), ObjectOverrideData, *this, DataItem.MemberPropertyName);
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
					FLGUIImmediateOverrideParameterObjectReader Reader(KeyValue.Key, KeyValue.Value, *this, InNameSetData[Index].MemberPropertyName);
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
		static TSet<FName> result = { FName("AttachParent") };
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
			InArchive.SetUE4Ver(this->ArchiveVersion);
			InArchive.SetLicenseeUE4Ver(this->ArchiveLicenseeVer);
			if (!this->ArEngineVer.IsEmpty()) InArchive.SetEngineVer(this->ArEngineVer);
			InArchive.SetEngineNetVer(this->ArEngineNetVer);
			InArchive.SetGameNetVer(this->ArGameNetVer);
		}
	}
}
