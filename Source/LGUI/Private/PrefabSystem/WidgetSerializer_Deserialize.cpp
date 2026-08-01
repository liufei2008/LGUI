// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "PrefabSystem/WidgetSerializer.h"
#include "PrefabSystem/LexUIObjectReaderAndWriter.h"
#include "Engine/World.h"
#include "LGUI.h"
#include "Core/LexUIBehaviour.h"
#include "Core/LexUIManager.h"
#include "Core/LexUISettings.h"
#include "Core/Components/LexWidget.h"
#include "Serialization/MemoryReader.h"
#include "HAL/PlatformTime.h"



#define LOCTEXT_NAMESPACE "LexUIPrefabSystem_Deserialize"

namespace LexUIPrefabSystem
{
	ULexWidget* WidgetSerializer::LoadPrefabWithExistingObjects(UWorld* InWorld, UObject* InOuter, ULexUIPrefab* InPrefab, ULexWidget* Parent
		, TMap<FGuid, TObjectPtr<UObject>>& InOutMapGuidToObjects, TMap<TObjectPtr<ULexWidget>, FLexUISubPrefabData>& OutSubPrefabMap
	)
	{
		if (!IsValid(InWorld))
		{
			UE_LOG(LGUI, Error, TEXT("[%s].%d Not valid world!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
			return nullptr;
		}
		if (!IsValid(InPrefab))
		{
			UE_LOG(LGUI, Error, TEXT("[%s].%d InPrefab is null!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
			return nullptr;
		}
		
		WidgetSerializer serializer;
		serializer.World = InWorld;
		serializer.OwnerObject = InOuter;
		for (auto& KeyValue : InOutMapGuidToObjects)//Preprocess the map, ignore invalid object
		{
			if (IsValid(KeyValue.Value))
			{
				serializer.MapGuidToObject.Add(KeyValue.Key, KeyValue.Value);
			}
		}
#if !WITH_EDITOR
		serializer.bIsEditorOrRuntime = false;
#endif
		serializer.bOverrideVersions = true;
		serializer.WriterOrReaderFunction = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer) {
			LexUIPrefabSystem::FLexUIObjectReader Reader(InOutBuffer, serializer, {});
			Reader.DoSerialize(InObject);
		};
		serializer.WriterOrReaderFunctionForSubPrefabOverride = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, const TArray<FName>& InOverridePropertyNames) {
			LexUIPrefabSystem::FLexUIOverrideParameterObjectReader Reader(InOutBuffer, serializer, InOverridePropertyNames);
			Reader.DoSerialize(InObject);
		};
		auto rootWidget = serializer.DeserializeWidget(Parent, InPrefab, nullptr, false, FVector::ZeroVector, FQuat::Identity, FVector::OneVector);
		InOutMapGuidToObjects = serializer.MapGuidToObject;
		OutSubPrefabMap = serializer.SubPrefabMap;
		return rootWidget;
	}

	ULexWidget* WidgetSerializer::LoadPrefab(UWorld* InWorld, UObject* InOuter, ULexUIPrefab* InPrefab, ULexWidget* Parent, bool SetRelativeTransformToIdentity, TFunction<void(ULexWidget*)> CallbackBeforeAwake)
	{
		if (!IsValid(InWorld))
		{
			UE_LOG(LGUI, Error, TEXT("[%s].%d Not valid world!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
			return nullptr;
		}
		if (!IsValid(InPrefab))
		{
			UE_LOG(LGUI, Error, TEXT("[%s].%d InPrefab is null!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
			return nullptr;
		}

		WidgetSerializer serializer;
		serializer.World = InWorld;
		serializer.OwnerObject = InOuter;
		serializer.CallbackBeforeAwake = CallbackBeforeAwake;
#if !WITH_EDITOR
		serializer.bIsEditorOrRuntime = false;
#endif
		serializer.bOverrideVersions = true;
		serializer.WriterOrReaderFunction = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer) {
			LexUIPrefabSystem::FLexUIObjectReader Reader(InOutBuffer, serializer, {});
			Reader.DoSerialize(InObject);
		};
		serializer.WriterOrReaderFunctionForSubPrefabOverride = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, const TArray<FName>& InOverridePropertyNames) {
			LexUIPrefabSystem::FLexUIOverrideParameterObjectReader Reader(InOutBuffer, serializer, InOverridePropertyNames);
			Reader.DoSerialize(InObject);
		};
		ULexWidget* result = nullptr;
		if (SetRelativeTransformToIdentity)
		{
			result = serializer.DeserializeWidget(Parent, InPrefab, nullptr, true);
		}
		else
		{
			result = serializer.DeserializeWidget(Parent, InPrefab, nullptr);
		}
		return result;
	}
	ULexWidget* WidgetSerializer::LoadPrefab(UWorld* InWorld, UObject* InOuter, ULexUIPrefab* InPrefab, ULexWidget* Parent, FVector RelativeLocation, FQuat RelativeRotation, FVector RelativeScale, TFunction<void(ULexWidget*)> CallbackBeforeAwake)
	{
		if (!IsValid(InWorld))
		{
			UE_LOG(LGUI, Error, TEXT("[%s].%d Not valid world!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
			return nullptr;
		}
		if (!IsValid(InPrefab))
		{
			UE_LOG(LGUI, Error, TEXT("[%s].%d InPrefab is null!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
			return nullptr;
		}

		WidgetSerializer serializer;
		serializer.World = InWorld;
		serializer.OwnerObject = InOuter;
		serializer.CallbackBeforeAwake = CallbackBeforeAwake;
#if !WITH_EDITOR
		serializer.bIsEditorOrRuntime = false;
#endif
		serializer.bOverrideVersions = true;
		serializer.WriterOrReaderFunction = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer) {
			LexUIPrefabSystem::FLexUIObjectReader Reader(InOutBuffer, serializer, {});
			Reader.DoSerialize(InObject);
		};
		serializer.WriterOrReaderFunctionForSubPrefabOverride = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, const TArray<FName>& InOverridePropertyNames) {
			LexUIPrefabSystem::FLexUIOverrideParameterObjectReader Reader(InOutBuffer, serializer, InOverridePropertyNames);
			Reader.DoSerialize(InObject);
		};
		return serializer.DeserializeWidget(Parent, InPrefab, nullptr, true, RelativeLocation, RelativeRotation, RelativeScale);
	}
	ULexWidget* WidgetSerializer::LoadSubPrefab(UWorld* InWorld,
		UObject* InOwnerObject, ULexUIPrefab* InPrefab, ULexWidget* Parent
		, TMap<FGuid, TObjectPtr<UObject>>& InMapGuidToObject
		, const TFunction<void(ULexWidget*, const TMap<FGuid, TObjectPtr<UObject>>&, const TMap<TObjectPtr<UObject>, FGuid>&, const TArray<ULexWidget*>&)>& InOnSubPrefabFinishDeserializeFunction
	)
	{
		WidgetSerializer serializer;
		serializer.World = InWorld;
		serializer.OwnerObject = InOwnerObject;
#if !WITH_EDITOR
		serializer.bIsEditorOrRuntime = false;
#endif
		serializer.bOverrideVersions = true;
		serializer.MapGuidToObject = InMapGuidToObject;
		serializer.bIsSubPrefab = true;
		serializer.WriterOrReaderFunction = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer) {
			LexUIPrefabSystem::FLexUIObjectReader Reader(InOutBuffer, serializer, {});
			Reader.DoSerialize(InObject);
		};
		serializer.WriterOrReaderFunctionForSubPrefabOverride = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, const TArray<FName>& InOverridePropertyNames) {
			LexUIPrefabSystem::FLexUIOverrideParameterObjectReader Reader(InOutBuffer, serializer, InOverridePropertyNames);
			Reader.DoSerialize(InObject);
		};
		serializer.OnSubPrefabFinishDeserializeFunction = InOnSubPrefabFinishDeserializeFunction;
		auto rootWidget = serializer.DeserializeWidget(Parent, InPrefab, nullptr, false, FVector::ZeroVector, FQuat::Identity, FVector::OneVector);
		return rootWidget;
	}

#define LGUIPREFAB_LOG_DETAIL_TIME 0
	ULexWidget* WidgetSerializer::DeserializeWidgetFromData(FLexUIPrefabSaveData& SaveData, ULexWidget* Parent, bool ReplaceTransform, FVector InLocation, FQuat InRotation, FVector InScale)
	{
#if LGUIPREFAB_LOG_DETAIL_TIME
		double Time = FPlatformTime::Seconds();
#endif
		auto CreatedRootWidget = GenerateWidgetArray(SaveData.SavedWidgets, SaveData.SavedObjects, SaveData.MapWidgetToParent, FGuid());
		if (CreatedRootWidget == nullptr)
		{
			UE_LOG(LGUI, Error, TEXT("[%s].%d No actor generated!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
			return nullptr;
		}
		GenerateObjectArray(SaveData.SavedObjects, SaveData.MapWidgetToParent);
#if LGUIPREFAB_LOG_DETAIL_TIME
		UE_LOG(LGUI, Log, TEXT("--GenerateObject take time: %fms"), (FPlatformTime::Seconds() - Time) * 1000.0);
		Time = FPlatformTime::Seconds();
#endif
		//properties
		for (auto& KeyValue : SaveData.SavedObjectData)
		{
			if (auto ObjectPtr = MapGuidToObject.Find(KeyValue.Key))
			{
				WriterOrReaderFunction(*ObjectPtr, KeyValue.Value);
			}
		}

		//sub prefab override properties
		for (auto& Item : SubPrefabOverrideParameters)
		{
			WriterOrReaderFunctionForSubPrefabOverride(Item.Object, Item.ParameterDatas, Item.ParameterNames);
		}

#if LGUIPREFAB_LOG_DETAIL_TIME
		UE_LOG(LGUI, Log, TEXT("--DeserializeObject take time: %fms"), (FPlatformTime::Seconds() - Time) * 1000.0);
		Time = FPlatformTime::Seconds();
#endif

		//component attachment
		for (auto& CompData : WidgetAttachmentArray)
		{
			auto Widget = CompData.Widget;
			ULexWidget* ParentWidget = nullptr;
			if (CompData.ParentGuid.IsValid())
			{
				if (auto ParentObjectPtr = MapGuidToObject.Find(CompData.ParentGuid))
				{
					ParentWidget = Cast<ULexWidget>(*ParentObjectPtr);
				}
			}
			if (!ParentWidget)
			{
				UE_LOG(LGUI, Error, TEXT("[%s].%d Missing parent for widget:%s at prefab:%s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *Widget->GetPathDisplayName(), *PrefabAssetPath);
				ParentWidget = CreatedRootWidget;
			}
			if (Widget->HasRegistered())
			{
				Widget->SetParent(ParentWidget, false);
			}
			else
			{
				Widget->SetParentBeforeRegister(ParentWidget);
			}
		}
		//attach root actor's parent
		if (Parent)
		{
			CreatedRootWidget->SetParent(Parent, false);
		}

#if LGUIPREFAB_LOG_DETAIL_TIME
		Time = FPlatformTime::Seconds();
#endif
		if (!bIsSubPrefab)
		{
			for (int i = 0; i < AllWidgetArray.Num(); i++)
			{
				auto& Widget = AllWidgetArray[i];
				Widget->OnRegister();
			}
		}
		
		if (ReplaceTransform)
		{
			CreatedRootWidget->SetRelativeLocationAndRotation(InLocation, InRotation);
			CreatedRootWidget->SetRelativeScale(InScale);
		}

		if (OnSubPrefabFinishDeserializeFunction != nullptr)
		{
			OnSubPrefabFinishDeserializeFunction(CreatedRootWidget, MapGuidToObject, MapObjectToOriginGuid, AllWidgetArray);
		}
		if (CallbackBeforeAwake != nullptr)
		{
			CallbackBeforeAwake(CreatedRootWidget);
		}

		if (!bIsSubPrefab)
		{
			if (auto LexUIManager = ULexUIManagerWorldSubsystem::GetInstance(World))
			{
				//originally I use World->HasBegunPlay to tell if the world has begun play but it not work well, the World->HasBegunPlay return false even i do LoadPrefab in BeginPlay
				if (LexUIManager->HasBegunPlay())
				{
					for (int i = 0; i < AllWidgetArray.Num(); i++)
					{
						auto& Widget = AllWidgetArray[i];
						if (IsValid(Widget))//widget could be destroyed during Awake, so check it
						{
							Widget->BeginPlay();
						}
					}
				}
			}
		}

#if LGUIPREFAB_LOG_DETAIL_TIME
		UE_LOG(LGUI, Log, TEXT("--Call Awake (and OnEnable) take time: %fms"), (FPlatformTime::Seconds() - Time) * 1000.0);
#endif

		return CreatedRootWidget;
	}
	ULexWidget* WidgetSerializer::DeserializeWidget(ULexWidget* Parent, ULexUIPrefab* InPrefab, const TFunction<void()>& InCallbackBeforeDeserialize, bool ReplaceTransform, FVector InLocation, FQuat InRotation, FVector InScale)
	{
		const double StartTime = FPlatformTime::Seconds();
		PrefabAssetPath = InPrefab->GetPathName();
#if WITH_EDITOR
		if (bIsEditorOrRuntime)
		{
			//fill new reference data
			this->ReferenceAssetList = InPrefab->ReferenceAssetList;
			this->ReferenceClassList = InPrefab->ReferenceClassList;
			this->ReferenceNameList = InPrefab->ReferenceNameList;
			this->ReferenceTextList = InPrefab->ReferenceTextList;

			this->ArchiveVersion = FPackageFileVersion(InPrefab->ArchiveVersion, (EUnrealEngineObjectUE5Version)InPrefab->ArchiveVersionUE5);
			this->ArchiveLicenseeVer = InPrefab->ArchiveLicenseeVer;
			this->ArEngineNetVer = InPrefab->ArEngineNetVer;
			this->ArGameNetVer = InPrefab->ArGameNetVer;
		}
		else
#endif
		{
			//fill new reference data
			this->ReferenceAssetList = InPrefab->ReferenceAssetListForBuild;
			this->ReferenceClassList = InPrefab->ReferenceClassListForBuild;
			this->ReferenceNameList = InPrefab->ReferenceNameListForBuild;
			this->ReferenceTextList = InPrefab->ReferenceTextListForBuild;

			this->ArchiveVersion = FPackageFileVersion(InPrefab->ArchiveVersion_ForBuild, (EUnrealEngineObjectUE5Version)InPrefab->ArchiveVersionUE5_ForBuild);
			this->ArchiveLicenseeVer = InPrefab->ArchiveLicenseeVer_ForBuild;
			this->ArEngineNetVer = InPrefab->ArEngineNetVer_ForBuild;
			this->ArGameNetVer = InPrefab->ArGameNetVer_ForBuild;
		}
		this->PrefabVersion = InPrefab->PrefabVersion;
		this->ArEngineVer = FEngineVersionBase(InPrefab->EngineMajorVersion, InPrefab->EngineMinorVersion, InPrefab->EnginePatchVersion);

		FLexUIPrefabSaveData SaveData;
		{
			auto& LoadedData =
#if WITH_EDITOR
				bIsEditorOrRuntime ? InPrefab->BinaryData :
#endif
				InPrefab->BinaryDataForBuild;

			auto FromBinary = FMemoryReader(LoadedData, false);
#if WITH_EDITOR
			if (bIsEditorOrRuntime)
			{
				FStructuredArchiveFromArchive(FromBinary).GetSlot() << SaveData;
			}
			else
#endif
			{
				FromBinary << SaveData;
			}
		}

		if (InCallbackBeforeDeserialize != nullptr)InCallbackBeforeDeserialize();
		auto CreatedRootWidget = DeserializeWidgetFromData(SaveData, Parent, ReplaceTransform, InLocation, InRotation, InScale);

		if (GetDefault<ULexUIEditorSettings>()->bLogPrefabLoadTime)
		{
			const double ElapsedSeconds = FPlatformTime::Seconds() - StartTime;
			UE_LOG(LGUI, Log, TEXT("Load prefab: '%s', total time: %fms"), *InPrefab->GetName(), ElapsedSeconds * 1000.0);
		}

		return CreatedRootWidget;
	}




	void WidgetSerializer::GenerateObjectArray(TMap<FGuid, FLexUIObjectSaveData>& SavedObjects, TMap<FGuid, FGuid>& MapWidgetToParent)
	{
		auto CollectDefaultSubobjects = [&](UObject* Target, const FGuid& TargetGuid, FLexUICommonObjectSaveData& ObjectData) {
			//collect default sub object
			TArray<UObject*> DefaultSubObjects;
			Target->GetDefaultSubobjects(DefaultSubObjects);
			for (auto DefaultSubObject : DefaultSubObjects)
			{
				if (DefaultSubObject->HasAnyFlags(EObjectFlags::RF_Transient))continue;
				auto Index = ObjectData.DefaultSubObjectNameArray.IndexOfByKey(DefaultSubObject->GetFName());
				if (Index == INDEX_NONE)
				{
					UE_LOG(LGUI, Warning, TEXT("[%s].%d Missing guid for default sub object: %s. Prefab: '%s'"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(DefaultSubObject->GetFName().ToString()), *PrefabAssetPath);
					continue;
				}
				auto DefaultSubObjectGuid = ObjectData.DefaultSubObjectGuidArray[Index];
				MapGuidToObject.Add(DefaultSubObjectGuid, DefaultSubObject);
				MapObjectToOriginGuid.Add(DefaultSubObject, DefaultSubObjectGuid);
			}
		};
		for (auto& KeyValuePair : SavedObjects)
		{
			auto& ObjectGuid = KeyValuePair.Key;
			auto& ObjectData = KeyValuePair.Value;
			UObject* CreatedNewObject = nullptr;
#if WITH_EDITOR
			//MapGuidToObject can passed from LoadPrefabWithExistingObjects, so we need to find from map first. This only needed in editor, because runtime never use LoadPrefabWithExistingObjects
			if (auto ObjectPtr = MapGuidToObject.Find(ObjectGuid))
			{
				CreatedNewObject = *ObjectPtr;
				MapObjectToOriginGuid.Add(CreatedNewObject, ObjectGuid);
				CollectDefaultSubobjects(CreatedNewObject, ObjectGuid, ObjectData);
			}
			else
#endif
			{
				if (auto ObjectClass = FindClassFromListByIndex(ObjectData.ObjectClass))
				{
					if (auto OuterObjectPtr = MapGuidToObject.Find(ObjectData.OuterObjectGuid))
					{
#if WITH_EDITOR
						if (auto ExitObject = FindObjectWithOuter(*OuterObjectPtr, nullptr, ObjectData.ObjectName))
						{
							if (auto Obj = Cast<UObject>(ExitObject))
							{
								if (auto Widget = Cast<ULexWidget>(Obj))
								{
									Widget->DestroyWidget();
								}
								else if (auto WidgetComponent = Cast<ULexUIBehaviour>(Obj))
								{
									WidgetComponent->DestroyComponent();
								}
								else
								{
									Obj->ConditionalBeginDestroy();
								}
								Obj->Rename(nullptr, GetTransientPackage());
							}
							UE_LOG(LGUI, Warning, TEXT("[%s].%d Object '%s' already exist on outer '%s', will destroy and rename exiting one. Prefab: '%s'"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(ObjectData.ObjectName.ToString()), *(OuterObjectPtr->GetPathName()), *PrefabAssetPath);
						}
#endif
						if (ObjectClass->HasAnyClassFlags(CLASS_Abstract))
						{
							UE_LOG(LGUI, Warning, TEXT("[%s].%d Bad class %s when creating object: '%s'. Prefab: '%s'"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(ObjectClass->GetPathName()), *(ObjectData.ObjectName.ToString()), *PrefabAssetPath);
							continue;
						}
						CreatedNewObject = NewObject<UObject>(*OuterObjectPtr, ObjectClass, ObjectData.ObjectName, (EObjectFlags)ObjectData.ObjectFlags);
						MapGuidToObject.Add(ObjectGuid, CreatedNewObject);
						MapObjectToOriginGuid.Add(CreatedNewObject, ObjectGuid);
						CollectDefaultSubobjects(CreatedNewObject, ObjectGuid, ObjectData);
					}
					else
					{
						UE_LOG(LGUI, Warning, TEXT("[%s].%d Missing Outer object when creating object: '%s'. Prefab: '%s'"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(ObjectData.ObjectName.ToString()), *PrefabAssetPath);
						continue;
					}
				}
			}
		}
	}

	ULexWidget* WidgetSerializer::GenerateWidgetArray(TArray<FLexUIWidgetSaveData>& SavedWidgets, TMap<FGuid, FLexUIObjectSaveData>& SavedObjects, TMap<FGuid, FGuid>& MapWidgetToParent, FGuid ParentGuid)
	{
		ULexWidget* RootWidget = nullptr;//first actor is the RootWidget
		for (int i = 0; i < SavedWidgets.Num(); i++)
		{
			auto& InWidgetData = SavedWidgets[i];
			if (InWidgetData.bIsPrefab)
			{
				auto PrefabIndex = InWidgetData.PrefabAssetIndex;
				if (auto PrefabAssetObject = FindAssetFromListByIndex(PrefabIndex))
				{
					if (auto SubPrefabAsset = Cast<ULexUIPrefab>(PrefabAssetObject))
					{
						ULexWidget* SubPrefabRootWidget = nullptr;
						FLexUISubPrefabData SubPrefabData;
						SubPrefabData.PrefabAsset = SubPrefabAsset;

#if WITH_EDITOR
						if (SubPrefabAsset->PrefabVersion < (uint16)ELexUIPrefabVersion::NewObjectOnNestedPrefab)
						{
							SubPrefabAsset->RecreatePrefab();//if is old version then recreate to make it new version
						}
#endif
						//sub prefab
						{
							auto& SubMapGuidToObject = SubPrefabData.MapGuidToObject;
							TMap<FGuid, FGuid> MapObjectGuidFromSubPrefabToParentPrefab;
							for (auto& KeyValue : InWidgetData.MapObjectGuidFromParentPrefabToSubPrefab)
							{
								MapObjectGuidFromSubPrefabToParentPrefab.Add(KeyValue.Value, KeyValue.Key);
							}
#if WITH_EDITOR
							//edit mode must check if the object already exist, because the deserialize process could happen when use revert-prefab
							if (bIsEditorOrRuntime)
							{
								for (auto& KeyValue : MapObjectGuidFromSubPrefabToParentPrefab)
								{
									auto ObjectPtr = MapGuidToObject.Find(KeyValue.Value);
									if (!SubMapGuidToObject.Contains(KeyValue.Key) && ObjectPtr != nullptr)
									{
										SubMapGuidToObject.Add(KeyValue.Key, *ObjectPtr);
									}
								}
							}
#endif
							bool bAnyGuidFrom_MapObjectIdToNewlyCreatedId = false;
							auto GetObjectGuidInParent = [&](const FGuid& GuidInSubPrefab, const FGuid& GuidInOriginPrefab) {
								FGuid GuidInParent;
								auto ObjectGuidInParentPrefabPtr = MapObjectGuidFromSubPrefabToParentPrefab.Find(GuidInSubPrefab);
								if (ObjectGuidInParentPrefabPtr == nullptr)
								{
									auto UniqueId = FLexUISubPrefabObjectUniqueIdSaveData{ InWidgetData.WidgetGuid, GuidInOriginPrefab };
									if (auto GuidInParentPtr = InWidgetData.MapObjectIdToNewlyCreatedId.Find(UniqueId))
									{
										GuidInParent = *GuidInParentPtr;
									}
									else
									{
										GuidInParent = FGuid::NewGuid();
										InWidgetData.MapObjectIdToNewlyCreatedId.Add(UniqueId, GuidInParent);
									}
									bAnyGuidFrom_MapObjectIdToNewlyCreatedId = true;
									MapObjectGuidFromSubPrefabToParentPrefab.Add(GuidInSubPrefab, GuidInParent);
								}
								else
								{
									GuidInParent = *ObjectGuidInParentPrefabPtr;
								}
								return GuidInParent;
								};
							auto NewOnSubPrefabFinishDeserializeFunction =
								[&](ULexWidget*, const TMap<FGuid, TObjectPtr<UObject>>& InSubPrefabMapGuidToObject, const TMap<TObjectPtr<UObject>, FGuid>& InMapObjectToOriginGuid, const TArray<ULexWidget*>& InSubWidgets) {
								//collect sub prefab's object and guid to parent map, so all objects are ready when set override parameters
								for (auto& KeyValue : InSubPrefabMapGuidToObject)
								{
									auto& GuidInSubPrefab = KeyValue.Key;
									auto& ObjectInSubPrefab = KeyValue.Value;

									auto GuidInParent = GetObjectGuidInParent(GuidInSubPrefab, InMapObjectToOriginGuid[ObjectInSubPrefab]);

									if (auto RecordDataPtr = InWidgetData.MapObjectGuidToSubPrefabOverrideParameter.Find(GuidInParent))
									{
										FLexUIPrefabOverrideParameterData OverrideDataItem;
										OverrideDataItem.MemberPropertyNames = RecordDataPtr->OverrideParameterNames;
										OverrideDataItem.Object = ObjectInSubPrefab;
										SubPrefabData.ObjectOverrideParameterArray.Add(OverrideDataItem);

										FSubPrefabObjectOverrideParameterData OverrideData;
										OverrideData.Object = ObjectInSubPrefab;
										OverrideData.ParameterDatas = RecordDataPtr->OverrideParameterData;
										OverrideData.ParameterNames = RecordDataPtr->OverrideParameterNames;
										SubPrefabOverrideParameters.Add(OverrideData);//collect override parameters, so when all objects are generated, restore these parameters will get all value back
									}

									SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab.Add(GuidInParent, GuidInSubPrefab);
									SubPrefabData.MapGuidToObject.Add(GuidInSubPrefab, ObjectInSubPrefab);
									if (!MapGuidToObject.Contains(GuidInParent))
									{
										MapGuidToObject.Add(GuidInParent, ObjectInSubPrefab);
									}
								}
								//if we don't need to get any guid from MapObjectIdToNewlyCreatedId, that means subprefab already have a persistent guid for all objects, then we can clear the data
								if (!bAnyGuidFrom_MapObjectIdToNewlyCreatedId)
								{
									if (InWidgetData.MapObjectIdToNewlyCreatedId.Num() > 0)
									{
										InWidgetData.MapObjectIdToNewlyCreatedId.Empty();
									}
								}
								else
								{
									//convert data to save
									for (auto& DataItem : InWidgetData.MapObjectIdToNewlyCreatedId)
									{
										SubPrefabData.MapObjectIdToNewlyCreatedId.Add({ DataItem.Key.RootWidgetGuidInParentPrefab, DataItem.Key.ObjectGuidInOriginPrefab }, DataItem.Value);
									}
								}
								//collect sub-prefab's actor to parent prefab
								AllWidgetArray.Append(InSubWidgets);
								MapObjectToOriginGuid.Append(InMapObjectToOriginGuid);
								};

							SubPrefabRootWidget = WidgetSerializer::LoadSubPrefab(this->World, this->OwnerObject, SubPrefabAsset, nullptr, SubMapGuidToObject
								, NewOnSubPrefabFinishDeserializeFunction
							);
						}
						
						if (SubPrefabRootWidget != nullptr)
						{
							FWidgetAttachment CompData;
							CompData.Widget = SubPrefabRootWidget;
							FGuid SubPrefabRootCompGuid;
							for (auto& KeyValue : MapGuidToObject)
							{
								if (KeyValue.Value == SubPrefabRootWidget)
								{
									SubPrefabRootCompGuid = KeyValue.Key;
									break;
								}
							}
							if (auto ParentGuidPtr = MapWidgetToParent.Find(SubPrefabRootCompGuid))
							{
								CompData.ParentGuid = *ParentGuidPtr;
								WidgetAttachmentArray.Add(CompData);
							}

							SubPrefabMap.Add(SubPrefabRootWidget, SubPrefabData);

							if (i == 0)
							{
								RootWidget = SubPrefabRootWidget;
							}
						}
					}
				}
			}
			else
			{
				if (auto WidgetClass = FindClassFromListByIndex(InWidgetData.ObjectClass))
				{
					auto CollectDefaultSubobjects = [&](ULexWidget* TargetWidget) {
						//Collect default sub objects
						TArray<UObject*> DefaultSubObjects;
						TargetWidget->GetDefaultSubobjects(DefaultSubObjects);
						for (auto DefaultSubObject : DefaultSubObjects)
						{
							if (DefaultSubObject->HasAnyFlags(EObjectFlags::RF_Transient))continue;
							auto Index = InWidgetData.DefaultSubObjectNameArray.IndexOfByKey(DefaultSubObject->GetFName());
							if (Index == INDEX_NONE)
							{
								UE_LOG(LGUI, Warning, TEXT("[%s].%d Missing guid for default sub object: %s. Prefab: '%s'"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(DefaultSubObject->GetFName().ToString()), *PrefabAssetPath);
								continue;
							}
							auto DefaultSubObjectGuid = InWidgetData.DefaultSubObjectGuidArray[Index];
							MapGuidToObject.Add(DefaultSubObjectGuid, DefaultSubObject);
							MapObjectToOriginGuid.Add(DefaultSubObject, DefaultSubObjectGuid);
						}
						};

					ULexWidget* NewWidget = nullptr;
#if WITH_EDITOR
					//MapGuidToObject can passed from LoadPrefabWithExistingObjects, so we need to find from map first. This only needed in editor, because runtime never use LoadPrefabWithExistingObjects
					if (auto WidgetPtr = MapGuidToObject.Find(InWidgetData.WidgetGuid))
					{
						NewWidget = (ULexWidget*)(*WidgetPtr);
						MapObjectToOriginGuid.Add(NewWidget, InWidgetData.WidgetGuid);
						CollectDefaultSubobjects(NewWidget);
					}
					else
#endif
					{
						NewWidget = NewObject<ULexWidget>(OwnerObject, WidgetClass, NAME_None, (EObjectFlags)InWidgetData.ObjectFlags);
						MapGuidToObject.Add(InWidgetData.WidgetGuid, NewWidget);
						MapObjectToOriginGuid.Add(NewWidget, InWidgetData.WidgetGuid);
						CollectDefaultSubobjects(NewWidget);
					}

					FWidgetAttachment CompData;
					CompData.Widget = NewWidget;
					if (auto ParentGuidPtr = MapWidgetToParent.Find(InWidgetData.WidgetGuid))
					{
						CompData.ParentGuid = *ParentGuidPtr;
						WidgetAttachmentArray.Add(CompData);
					}

					AllWidgetArray.Add(NewWidget);

					if (i == 0)
					{
						RootWidget = NewWidget;
					}
				}
				else
				{
					UE_LOG(LGUI, Warning, TEXT("[%s].%d Class of index:%d not found! Prefab: '%s'"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, (InWidgetData.ObjectClass), *PrefabAssetPath);
				}
			}
		}
		return RootWidget;
	}
}

#undef LOCTEXT_NAMESPACE


