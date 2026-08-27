// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "PrefabSystem/WidgetSerializerBase.h"
#include "Core/Components/LexWidget.h"
#include "PrefabSystem/LexUIObjectReaderAndWriter.h"
#include "PrefabSystem/LexUIPrefabOverridePropertyPathUtils.h"
#include "Misc/ConfigCacheIni.h"
#if WITH_EDITOR
#include "Tools/UEdMode.h"
#endif

namespace LexUIPrefabSystem
{
	bool WidgetSerializerBase::ObjectIsTrash(UObject* InObject)
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


	//only allow object that belongs to some widget of this prefab
	bool WidgetSerializerBase::ObjectBelongsToThisPrefab(UObject* InObject)
	{
		if (WillSerializeWidgetArray.Contains(InObject))
		{
			return true;
		}

		UObject* Outer = InObject->GetOuter();
		while (Outer != nullptr
			&& !Outer->HasAnyFlags(EObjectFlags::RF_Transient)
			)
		{
			if (WillSerializeWidgetArray.Contains(Outer))
			{
				return true;
			}
			else
			{
				if (Outer->IsA<ULexWidget>()
					)//not exist in WillSerializeWidgetArray, but is a widget, means it not belongs to this prefab
				{
					return false;
				}
			}
			Outer = Outer->GetOuter();
		}
		return false;
	}

	bool WidgetSerializerBase::CollectObjectToSerialize(UObject* Object, FGuid& OutGuid)
	{
		if (!IsValid(Object))return false;
		if (!Object->IsValidLowLevel())return false;
		if (Object->IsUnreachable())return false;
		if (Object->GetFName() == NAME_None)return false;
#if WITH_EDITOR
		if (Object->GetClass()->IsChildOf(UEdMode::StaticClass()))return false;
		if (ObjectIsTrash(Object))return false;
#endif
		if (Object->IsEditorOnly() && !bIsEditorOrRuntime)return false;
		if (Object->IsAsset())return false;//skip asset, because asset is referenced directly
		if (Object->HasAnyFlags(EObjectFlags::RF_Transient))return false;//skip transient object
		if (Object->IsA<ULexWidget>())return false;//skip Widget, because Widget is collected in WillSerializeWidgetArray
		if (WillSerializeWidgetArray.Contains(Object))return false;//already contains it (double check)
		if (!ObjectBelongsToThisPrefab(Object))return false;

		if (WillSerializeObjectArray.Contains(Object))
		{
			auto GuidPtr = MapObjectToGuid.Find(Object);
			check(GuidPtr != nullptr);
			OutGuid = *GuidPtr;
			return true;//already contains object
		}

		auto Outer = Object->GetOuter();
		check(Outer != nullptr);

		if (WillSerializeWidgetArray.Contains(Outer))//outer is actor
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
				&& !WillSerializeWidgetArray.Contains(Outer)//Make sure Outer is not actor, because actor is created before any other objects, they will be stored in actor's data
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

	TMap<UObject*, TArray<uint8>> WidgetSerializerBase::SaveOverrideParameterToData(const TArray<FLexUIPrefabOverrideParameterData>& InData)
	{
		this->bIsEditorOrRuntime = true;
		TMap<UObject*, TArray<uint8>> MapObjectToOverrideDatas;
		for (auto& DataItem : InData)
		{
			TArray<uint8> ObjectOverrideData;
			FLexUIImmediateOverrideParameterObjectWriter Writer(DataItem.Object.Get(), ObjectOverrideData, *this, DataItem.MemberPropertyNames);
			MapObjectToOverrideDatas.Add(DataItem.Object.Get(), ObjectOverrideData);
		}
		return MapObjectToOverrideDatas;
	}

	void WidgetSerializerBase::RestoreOverrideParameterFromData(TMap<UObject*, TArray<uint8>>& InData, const TArray<FLexUIPrefabOverrideParameterData>& InNameSetData)
	{
		this->bIsEditorOrRuntime = true;
		for (auto& KeyValue : InData)
		{
			if (IsValid(KeyValue.Key))
			{
				auto Index = InNameSetData.IndexOfByPredicate([&](const FLexUIPrefabOverrideParameterData& Item) {
					return Item.Object.Get() == KeyValue.Key;
					});
				if (Index != INDEX_NONE)
				{
					FLexUIImmediateOverrideParameterObjectReader Reader(KeyValue.Key, KeyValue.Value, *this, InNameSetData[Index].MemberPropertyNames);
				}
			}
		}
	}


	TArray<WidgetSerializerBase::FSubPropertyOverrideValue> WidgetSerializerBase::SaveSubPropertyOverrideToData(const TArray<FLexUIPrefabOverrideParameterData>& InData)
	{
		this->bIsEditorOrRuntime = true;
		TArray<FSubPropertyOverrideValue> Result;
		for (auto& DataItem : InData)
		{
			auto Object = DataItem.Object.Get();
			if (!IsValid(Object))continue;
			for (auto& Path : DataItem.SubPropertyPaths)
			{
				FProperty* LeafProperty = nullptr;
				void* LeafContainer = nullptr;
				if (FLexUIPrefabOverridePropertyPathUtils::Resolve(Object->GetClass(), Object, Path.Segments, LeafProperty, LeafContainer) != EPropertyPathResolveResult::Success)continue;
				FSubPropertyOverrideValue Item;
				Item.Object = Object;
				Item.Path = Path;
				Item.LeafProperty = LeafProperty;
				Item.Value = FDefaultConstructedPropertyElement(LeafProperty);
				LeafProperty->CopyCompleteValue(Item.Value.GetObjAddress(), LeafProperty->ContainerPtrToValuePtr<void>(LeafContainer));
				Result.Add(MoveTemp(Item));
			}
		}
		return Result;
	}

	void WidgetSerializerBase::RestoreSubPropertyOverrideFromData(const TArray<FSubPropertyOverrideValue>& InData)
	{
		this->bIsEditorOrRuntime = true;
		for (auto& Item : InData)
		{
			auto Object = Item.Object.Get();
			if (!IsValid(Object))continue;
			FProperty* LeafProperty = nullptr;
			void* LeafContainer = nullptr;
			if (FLexUIPrefabOverridePropertyPathUtils::Resolve(Object->GetClass(), Object, Item.Path.Segments, LeafProperty, LeafContainer) != EPropertyPathResolveResult::Success)continue;
			if (LeafProperty != Item.LeafProperty)continue;//the type changed while the sub-prefab was reloading
			LeafProperty->CopyCompleteValue(LeafProperty->ContainerPtrToValuePtr<void>(LeafContainer), Item.Value.GetObjAddress());
		}
	}

	int32 WidgetSerializerBase::FindOrAddAssetIdFromList(UObject* AssetObject)
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

	int32 WidgetSerializerBase::FindOrAddClassFromList(UClass* Class)
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
	int32 WidgetSerializerBase::FindOrAddNameFromList(const FName& Name)
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

	int32 WidgetSerializerBase::FindOrAddTextFromList(const FText& Text)
	{
		if (Text.IsEmpty())return -1;
		auto resultIndex = ReferenceTextList.IndexOfByPredicate([&Text](const FText& Item)
			{
				return Item.EqualTo(Text);
			});
		if (resultIndex != INDEX_NONE)
		{
			return resultIndex;
		}
		else
		{
			ReferenceTextList.Add(Text);
			return ReferenceTextList.Num() - 1;
		}
	}

	FName WidgetSerializerBase::FindNameFromListByIndex(int32 Id)
	{
		return ReferenceNameList.IsValidIndex(Id) ? ReferenceNameList[Id] : NAME_None;
	}

	FText WidgetSerializerBase::FindTextFromListByIndex(int32 Id)
	{
		return ReferenceTextList.IsValidIndex(Id) ? ReferenceTextList[Id] : FText::GetEmpty();
	}

	UObject* WidgetSerializerBase::FindAssetFromListByIndex(int32 Id)
	{
		return ReferenceAssetList.IsValidIndex(Id) ? ReferenceAssetList[Id] : nullptr;
	}

	UClass* WidgetSerializerBase::FindClassFromListByIndex(int32 Id)
	{
		return ReferenceClassList.IsValidIndex(Id) ? ReferenceClassList[Id] : nullptr;
	}

	bool WidgetSerializerBase::CanUseUnversionedPropertySerialization()
	{
		bool bTemp;
		static bool bAllow = GConfig->GetBool(TEXT("Core.System"), TEXT("CanUseUnversionedPropertySerialization"), bTemp, GEngineIni) && bTemp;
		return bAllow;
	}

	void WidgetSerializerBase::SetupArchive(FArchive& InArchive)
	{
		if (!bIsEditorOrRuntime)
		{
			InArchive.SetUseUnversionedPropertySerialization(true);
		}
		InArchive.SetFilterEditorOnly(!bIsEditorOrRuntime);
		InArchive.SetWantBinaryPropertySerialization(false);

		InArchive.ArNoDelta = false;
		InArchive.ArNoIntraPropertyDelta = false;

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
