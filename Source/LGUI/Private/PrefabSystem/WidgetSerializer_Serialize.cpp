// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "PrefabSystem/WidgetSerializer.h"
#include "PrefabSystem/LexUIObjectReaderAndWriter.h"
#include "LGUI.h"
#include "Core/Components/LexWidget.h"
#include "Misc/NetworkVersion.h"
#include "Runtime/Launch/Resources/Version.h"
#include "HAL/PlatformTime.h"


namespace LexUIPrefabSystem
{
	bool WidgetSerializer::SavePrefab(ULexWidget* OriginRootWidget, ULexUIPrefab* InPrefab
		, TMap<UObject*, FGuid>& InOutMapObjectToGuid, TMap<TObjectPtr<ULexWidget>, FLexUISubPrefabData>& InSubPrefabMap
		, bool InForEditorOrRuntimeUse
	)
	{
		if (!OriginRootWidget || !InPrefab)
		{
			UE_LOG(LGUI, Error, TEXT("[%s].%d OriginRootWidget Or InPrefab is null!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
			return false;
		}
		if (!IsValid(OriginRootWidget))
		{
			UE_LOG(LGUI, Error, TEXT("[%s].%d OriginRootWidget is not valid!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
			return false;
		}
		if (OriginRootWidget->HasAnyFlags(EObjectFlags::RF_Transient))
		{
			UE_LOG(LGUI, Error, TEXT("[%s].%d OriginRootWidget is transient!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
			return false;
		}
		if (!InForEditorOrRuntimeUse && OriginRootWidget->IsEditorOnly())
		{
			UE_LOG(LGUI, Error, TEXT("[%s].%d OriginRootWidget is editor only!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
			return false;
		}
		WidgetSerializer serializer;
		for (auto& KeyValue : InOutMapObjectToGuid)//Preprocess the map, ignore invalid object
		{
			if (IsValid(KeyValue.Key))
			{
				serializer.MapObjectToGuid.Add(KeyValue.Key, KeyValue.Value);
			}
		}
		serializer.SubPrefabMap = InSubPrefabMap;
		for (auto& SubPrefabKeyValue : InSubPrefabMap)
		{
			for (auto& GuidToObjectKeyValue : SubPrefabKeyValue.Value.MapGuidToObject)
			{
				if (auto SubPrefabWidget = Cast<ULexWidget>(GuidToObjectKeyValue.Value))
				{
					serializer.SubPrefabWidgetArray.Add(SubPrefabWidget);
				}
			}
		}
		serializer.bIsEditorOrRuntime = InForEditorOrRuntimeUse;
		serializer.WriterOrReaderFunction = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer) {
			LexUIPrefabSystem::FLexUIObjectWriter Writer(InOutBuffer, serializer, {});
			Writer.DoSerialize(InObject);
		};
		serializer.WriterOrReaderFunctionForSubPrefabOverride = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, const TArray<FName>& InOverridePropertyNames) {
			LexUIPrefabSystem::FLexUIOverrideParameterObjectWriter Writer(InOutBuffer, serializer, InOverridePropertyNames);
			Writer.DoSerialize(InObject);
		};
		bool saveResult = serializer.SerializeWidget(OriginRootWidget, InPrefab);
		InOutMapObjectToGuid = serializer.MapObjectToGuid;
		return saveResult;
	}

	void WidgetSerializer::SerializeWidgetArray(TMap<FGuid, FGuid>& MapWidgetToParent, TArray<FLexUIWidgetSaveData>& SavedWidgets, TMap<FGuid, TArray<uint8>>& SavedObjectData)
	{
		for (int i = 0; i < TrySerializeWidgetArray.Num(); i++)
		{
			auto& Widget = TrySerializeWidgetArray[i];
			FLexUIWidgetSaveData WidgetSaveData;
			if (auto SubPrefabDataPtr = SubPrefabMap.Find(Widget))//sub prefab's Widget is not collected in WillSerializeWidgetArray
			{
				WidgetSaveData.bIsPrefab = true;
				WidgetSaveData.PrefabAssetIndex = FindOrAddAssetIdFromList(SubPrefabDataPtr->PrefabAsset);
				WidgetSaveData.WidgetGuid = MapObjectToGuid[Widget];
				WidgetSaveData.MapObjectGuidFromParentPrefabToSubPrefab = SubPrefabDataPtr->MapObjectGuidFromParentPrefabToSubPrefab;

				//serialize override parameter data
				for (auto& DataItem : SubPrefabDataPtr->ObjectOverrideParameterArray)
				{
					TArray<uint8> SubPrefabOverrideData;
					auto SubPrefabObject = DataItem.Object.Get();
					if (MapObjectToGuid.Contains(SubPrefabObject))
					{
						FLexUIPrefabOverrideParameterSaveData RecordDataItem;
						RecordDataItem.OverrideParameterNames = DataItem.MemberPropertyNames;
						WriterOrReaderFunctionForSubPrefabOverride(SubPrefabObject, RecordDataItem.OverrideParameterData, DataItem.MemberPropertyNames);
						WidgetSaveData.MapObjectGuidToSubPrefabOverrideParameter.Add(MapObjectToGuid[SubPrefabObject], RecordDataItem);
					}
				}
				for (auto& DataItem : SubPrefabDataPtr->MapObjectIdToNewlyCreatedId)
				{
					WidgetSaveData.MapObjectIdToNewlyCreatedId.Add({ DataItem.Key.RootWidgetGuidInParentPrefab, DataItem.Key.ObjectGuidInOriginPrefab }, DataItem.Value);
				}

				if (auto Parent = Widget->GetParent())
				{
					if (MapObjectToGuid.Contains(Parent))//check if parent component belongs to this prefab
					{
						MapWidgetToParent.Add(MapObjectToGuid[Widget], MapObjectToGuid[Parent]);
					}
				}
			}
			else
			{
				auto WidgetGuid = MapObjectToGuid[Widget];

				WidgetSaveData.ObjectClass = FindOrAddClassFromList(Widget->GetClass());
				WidgetSaveData.WidgetGuid = WidgetGuid;
				WidgetSaveData.ObjectFlags = (uint32)Widget->GetFlags();
				WriterOrReaderFunction(Widget, SavedObjectData.Add(WidgetGuid));
				TArray<UObject*> DefaultSubObjects;
				Widget->GetDefaultSubobjects(DefaultSubObjects);
				for (auto DefaultSubObject : DefaultSubObjects)
				{
					FGuid DefaultSubObjectGuid;
					if (!CollectObjectToSerialize(DefaultSubObject, DefaultSubObjectGuid))continue;
					WidgetSaveData.DefaultSubObjectGuidArray.Add(MapObjectToGuid[DefaultSubObject]);
					WidgetSaveData.DefaultSubObjectNameArray.Add(DefaultSubObject->GetFName());
				}
				
				if (auto Parent = Widget->GetParent())
				{
					if (MapObjectToGuid.Contains(Parent))//check if parent component belongs to this prefab
					{
						MapWidgetToParent.Add(MapObjectToGuid[Widget], MapObjectToGuid[Parent]);
					}
				}
			}
			SavedWidgets.Add(WidgetSaveData);
		}
	}
	void WidgetSerializer::SerializeWidgetToData(ULexWidget* OriginRootWidget, FLexUIPrefabSaveData& OutData)
	{
		CollectWidgetRecursive(OriginRootWidget);
		//serialize Widget
		SerializeWidgetArray(OutData.MapWidgetToParent, OutData.SavedWidgets, OutData.SavedObjectData);
		//serialize objects and components
		SerializeObjectArray(OutData.SavedObjects, OutData.SavedObjectData);
	}
	bool WidgetSerializer::SerializeWidget(ULexWidget* OriginRootWidget, ULexUIPrefab* InPrefab)
	{
		const double StartTime = FPlatformTime::Seconds();

		this->PrefabVersion = LEXUI_CURRENT_PREFAB_VERSION;

		FLexUIPrefabSaveData SaveData;
		SerializeWidgetToData(OriginRootWidget, SaveData);

		FBufferArchive ToBinary;
#if WITH_EDITOR
		if (bIsEditorOrRuntime)
		{
			FStructuredArchiveFromArchive(ToBinary).GetSlot() << SaveData;
		}
		else
#endif
		{
			ToBinary << SaveData;
		}

		if (ToBinary.Num() <= 0)
		{
			UE_LOG(LGUI, Warning, TEXT("Save binary length is 0!"));
			return false;
		}
#if WITH_EDITOR
		if (bIsEditorOrRuntime)
		{
			InPrefab->BinaryData = ToBinary;
			InPrefab->bThumbnailDirty = true;
			InPrefab->CreateTime = FDateTime::UtcNow();

			//clear old reference data
			InPrefab->ReferenceAssetList.Empty();
			InPrefab->ReferenceClassList.Empty();
			InPrefab->ReferenceNameList.Empty();
			InPrefab->ReferenceTextList.Empty();
			//fill new reference data
			InPrefab->ReferenceAssetList = this->ReferenceAssetList;
			InPrefab->ReferenceClassList = this->ReferenceClassList;
			InPrefab->ReferenceNameList = this->ReferenceNameList;
			InPrefab->ReferenceTextList = this->ReferenceTextList;

			InPrefab->ArchiveVersion = GPackageFileUEVersion.FileVersionUE4;
			InPrefab->ArchiveVersionUE5 = GPackageFileUEVersion.FileVersionUE5;
			InPrefab->ArchiveLicenseeVer = GPackageFileLicenseeUEVersion;
			InPrefab->ArEngineNetVer = FNetworkVersion::GetNetworkProtocolVersion(FEngineNetworkCustomVersion::Guid);
			InPrefab->ArGameNetVer = FNetworkVersion::GetNetworkProtocolVersion(FGameNetworkCustomVersion::Guid);
		}
		else
#endif
		{
			InPrefab->BinaryDataForBuild = ToBinary;

			//fill new reference data
			InPrefab->ReferenceAssetListForBuild = this->ReferenceAssetList;
			InPrefab->ReferenceClassListForBuild = this->ReferenceClassList;
			InPrefab->ReferenceNameListForBuild = this->ReferenceNameList;
			InPrefab->ReferenceTextListForBuild = this->ReferenceTextList;

			InPrefab->ArchiveVersion_ForBuild = GPackageFileUEVersion.FileVersionUE4;
			InPrefab->ArchiveVersionUE5_ForBuild = GPackageFileUEVersion.FileVersionUE5;
			InPrefab->ArchiveLicenseeVer_ForBuild = GPackageFileLicenseeUEVersion;
			InPrefab->ArEngineNetVer_ForBuild = FNetworkVersion::GetNetworkProtocolVersion(FEngineNetworkCustomVersion::Guid);
			InPrefab->ArGameNetVer_ForBuild = FNetworkVersion::GetNetworkProtocolVersion(FGameNetworkCustomVersion::Guid);
		}

		InPrefab->EngineMajorVersion = ENGINE_MAJOR_VERSION;
		InPrefab->EngineMinorVersion = ENGINE_MINOR_VERSION;
		InPrefab->PrefabVersion = LEXUI_CURRENT_PREFAB_VERSION;

		const double ElapsedSeconds = FPlatformTime::Seconds() - StartTime;
		UE_LOG(LGUI, Log, TEXT("Take %fs saving prefab: %s"), ElapsedSeconds, *InPrefab->GetName());
		
		return true;
	}

	void WidgetSerializer::CollectWidgetRecursive(ULexWidget* Widget)
	{
		if (!IsValid(Widget))return;
		if (Widget->HasAnyFlags(EObjectFlags::RF_Transient))return;
		//collect actor
		bool bIsSubPrefabWidget = SubPrefabWidgetArray.Contains(Widget);
		if (!bIsSubPrefabWidget)//sub prefab's Widget should not put to the list
		{
			WillSerializeWidgetArray.Add(Widget);//sub-prefab just keep a reference, no need to serialize
			TrySerializeWidgetArray.Add(Widget);
		}
		else
		{
			if (SubPrefabMap.Contains(Widget))//sub-prefab's root Widget
			{
				TrySerializeWidgetArray.Add(Widget);
			}
		}
		//collect all Widgets include sub-prefab's Widget, because some property could reference it
		if (!MapObjectToGuid.Contains(Widget))
		{
			MapObjectToGuid.Add(Widget, FGuid::NewGuid());
		}

		auto& Children = Widget->GetChildren();
		for (auto& ChildWidget : Children)
		{
			CollectWidgetRecursive(ChildWidget);
		}
	}

	void WidgetSerializer::SerializeObjectArray(TMap<FGuid, FLexUIObjectSaveData>& ObjectSaveDataArray, TMap<FGuid, TArray<uint8>>& SavedObjectData)
	{
		for (int i = 0; i < WillSerializeObjectArray.Num(); i++)
		{
			auto Object = WillSerializeObjectArray[i];
			auto Class = Object->GetClass();
			FLexUIObjectSaveData ObjectSaveDataItem;
			ObjectSaveDataItem.ObjectClass = FindOrAddClassFromList(Class);
			ObjectSaveDataItem.ObjectName = Object->GetFName();
			ObjectSaveDataItem.ObjectFlags = (uint32)Object->GetFlags();
			ObjectSaveDataItem.OuterObjectGuid = MapObjectToGuid[Object->GetOuter()];
			WriterOrReaderFunction(Object, SavedObjectData.Add(MapObjectToGuid[Object]));
			TArray<UObject*> DefaultSubObjects;
			Object->GetDefaultSubobjects(DefaultSubObjects);
			for (auto DefaultSubObject : DefaultSubObjects)
			{
				FGuid DefaultSubObjectGuid;
				if (!CollectObjectToSerialize(DefaultSubObject, DefaultSubObjectGuid))continue;
				ObjectSaveDataItem.DefaultSubObjectGuidArray.Add(MapObjectToGuid[DefaultSubObject]);
				ObjectSaveDataItem.DefaultSubObjectNameArray.Add(DefaultSubObject->GetFName());
			}
			ObjectSaveDataArray.Add(MapObjectToGuid[Object], ObjectSaveDataItem);
		}
	}
}

