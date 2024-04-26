// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "PrefabSystem/ActorSerializer8.h"
#include "PrefabSystem/LGUIObjectReaderAndWriter.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SplineComponent.h"
#include "Runtime/Launch/Resources/Version.h"
#include "PrefabSystem/LGUIPrefabManager.h"
#include "LGUI.h"
#include "Misc/NetworkVersion.h"
#include "UObject/UObjectThreadContext.h"
#include "PrefabSystem/LGUIPrefabSettings.h"
#include "Serialization/MemoryReader.h"
#include "PrefabSystem/ILGUIPrefabInterface.h"
#include "PhysicsEngine/BodyInstance.h"
#if WITH_EDITOR
#include "Utils/LGUIUtils.h"
#endif

#if LGUI_CAN_DISABLE_OPTIMIZATION
UE_DISABLE_OPTIMIZATION
#endif

#define LOCTEXT_NAMESPACE "LGUIPrefabSystem8_Deserialize"

namespace LGUIPrefabSystem8
{
	AActor* ActorSerializer::LoadPrefabWithExistingObjects(UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent
		, TMap<FGuid, TObjectPtr<UObject>>& InOutMapGuidToObjects, TMap<TObjectPtr<AActor>, FLGUISubPrefabData>& OutSubPrefabMap
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

		bool bIsEditorOrRuntime = true;
#if !WITH_EDITOR
		bIsEditorOrRuntime = false;
#endif
		ActorSerializer serializer;
		serializer.TargetWorld = InWorld;
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
		serializer.WriterOrReaderFunction = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? serializer.GetSceneComponentExcludeProperties() : TSet<FName>();
			LGUIPrefabSystem::FLGUIObjectReader Reader(InOutBuffer, serializer, ExcludeProperties);
			Reader.DoSerialize(InObject);
		};
		serializer.WriterOrReaderFunctionForSubPrefabOverride = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, const TArray<FName>& InOverridePropertyNames) {
			LGUIPrefabSystem::FLGUIOverrideParameterObjectReader Reader(InOutBuffer, serializer, InOverridePropertyNames);
			Reader.DoSerialize(InObject);
		};
		auto rootActor = serializer.DeserializeActor(Parent, InPrefab, nullptr, false, FVector::ZeroVector, FQuat::Identity, FVector::OneVector);
		InOutMapGuidToObjects = serializer.MapGuidToObject;
		OutSubPrefabMap = serializer.SubPrefabMap;
		return rootActor;
	}

	AActor* ActorSerializer::LoadPrefab(UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent, bool SetRelativeTransformToIdentity, TFunction<void(AActor*)> CallbackBeforeAwake)
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

		ActorSerializer serializer;
		serializer.TargetWorld = InWorld;
		serializer.CallbackBeforeAwake = CallbackBeforeAwake;
#if !WITH_EDITOR
		serializer.bIsEditorOrRuntime = false;
#endif
		serializer.bOverrideVersions = true;
		serializer.WriterOrReaderFunction = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? serializer.GetSceneComponentExcludeProperties() : TSet<FName>();
			LGUIPrefabSystem::FLGUIObjectReader Reader(InOutBuffer, serializer, ExcludeProperties);
			Reader.DoSerialize(InObject);
		};
		serializer.WriterOrReaderFunctionForSubPrefabOverride = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, const TArray<FName>& InOverridePropertyNames) {
			LGUIPrefabSystem::FLGUIOverrideParameterObjectReader Reader(InOutBuffer, serializer, InOverridePropertyNames);
			Reader.DoSerialize(InObject);
		};
		AActor* result = nullptr;
		if (SetRelativeTransformToIdentity)
		{
			result = serializer.DeserializeActor(Parent, InPrefab, nullptr, true);
		}
		else
		{
			result = serializer.DeserializeActor(Parent, InPrefab, nullptr);
		}
		return result;
	}
	AActor* ActorSerializer::LoadPrefab(UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent, FVector RelativeLocation, FQuat RelativeRotation, FVector RelativeScale, TFunction<void(AActor*)> CallbackBeforeAwake)
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

		ActorSerializer serializer;
		serializer.TargetWorld = InWorld;
		serializer.CallbackBeforeAwake = CallbackBeforeAwake;
#if !WITH_EDITOR
		serializer.bIsEditorOrRuntime = false;
#endif
		serializer.bOverrideVersions = true;
		serializer.WriterOrReaderFunction = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? serializer.GetSceneComponentExcludeProperties() : TSet<FName>();
			LGUIPrefabSystem::FLGUIObjectReader Reader(InOutBuffer, serializer, ExcludeProperties);
			Reader.DoSerialize(InObject);
		};
		serializer.WriterOrReaderFunctionForSubPrefabOverride = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, const TArray<FName>& InOverridePropertyNames) {
			LGUIPrefabSystem::FLGUIOverrideParameterObjectReader Reader(InOutBuffer, serializer, InOverridePropertyNames);
			Reader.DoSerialize(InObject);
		};
		return serializer.DeserializeActor(Parent, InPrefab, nullptr, true, RelativeLocation, RelativeRotation, RelativeScale);
	}
	AActor* ActorSerializer::LoadSubPrefab(
		UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent
		, const FGuid& InParentDeserializationSessionId
		, TMap<FGuid, TObjectPtr<UObject>>& InMapGuidToObject
		, const TFunction<void(AActor*, const TMap<FGuid, TObjectPtr<UObject>>&, const TMap<TObjectPtr<UObject>, FGuid>&, const TArray<AActor*>&, const TArray<UActorComponent*>&)>& InOnSubPrefabFinishDeserializeFunction
	)
	{
		ActorSerializer serializer;
		serializer.TargetWorld = InWorld;
#if !WITH_EDITOR
		serializer.bIsEditorOrRuntime = false;
#endif
		serializer.bOverrideVersions = true;
		serializer.MapGuidToObject = InMapGuidToObject;
		serializer.DeserializationSessionId = InParentDeserializationSessionId;
		serializer.bIsSubPrefab = true;
		serializer.WriterOrReaderFunction = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? serializer.GetSceneComponentExcludeProperties() : TSet<FName>();
			LGUIPrefabSystem::FLGUIObjectReader Reader(InOutBuffer, serializer, ExcludeProperties);
			Reader.DoSerialize(InObject);
		};
		serializer.WriterOrReaderFunctionForSubPrefabOverride = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, const TArray<FName>& InOverridePropertyNames) {
			LGUIPrefabSystem::FLGUIOverrideParameterObjectReader Reader(InOutBuffer, serializer, InOverridePropertyNames);
			Reader.DoSerialize(InObject);
		};
		serializer.OnSubPrefabFinishDeserializeFunction = InOnSubPrefabFinishDeserializeFunction;
		auto rootActor = serializer.DeserializeActor(Parent, InPrefab, nullptr, false, FVector::ZeroVector, FQuat::Identity, FVector::OneVector);
		return rootActor;
	}

	void ActorSerializer::PostSetPropertiesOnActor(UActorComponent* Comp)
	{
		//here two methods to apply the deserialized data to component
#if 0//This method is simple and robust because it use built-in funtion. But also performance-cost (about 1.5-2.0x time cost to the whole deserialize process), because a lot of unnecessary properties are set here
		auto CompInstanceData = Comp->GetComponentInstanceData();
		CompInstanceData->ApplyToComponent(Comp, ECacheApplyPhase::PostUserConstructionScript);
#else//In this method I search all "ApplyToComponent" function and get the important part (I think), so result may miss something, if it does then contact me and I will add the missing part
		if (auto PrimitiveComp = Cast<UPrimitiveComponent>(Comp))
		{
			//fix collision data. this is same as UPrimitiveComponent.UpdateCollisionProfile
			{
				PrimitiveComp->BodyInstance.LoadProfileData(false);
			}
			if (auto SplineComp = Cast<USplineComponent>(Comp))
			{
				SplineComp->UpdateSpline();
			}
		}
		Comp->ReregisterComponent();
#endif
	}

#define LGUIPREFAB_LOG_DETAIL_TIME 0
	AActor* ActorSerializer::DeserializeActorFromData(FLGUIPrefabSaveData& SaveData, USceneComponent* Parent, bool ReplaceTransform, FVector InLocation, FQuat InRotation, FVector InScale)
	{
#if LGUIPREFAB_LOG_DETAIL_TIME
		auto Time = FDateTime::Now();
#endif
		if (LGUIPrefabManager == nullptr)
		{
			LGUIPrefabManager = ULGUIPrefabWorldSubsystem::GetInstance(TargetWorld);
		}
		if (!bIsSubPrefab)
		{
			if (!DeserializationSessionId.IsValid())
			{
				DeserializationSessionId = FGuid::NewGuid();
				LGUIPrefabManager->BeginPrefabSystemProcessingActor(DeserializationSessionId);
			}
		}
		auto CreatedRootActor = GenerateActorArray(SaveData.SavedActors, SaveData.SavedObjects, SaveData.MapSceneComponentToParent, FGuid());
		if (CreatedRootActor == nullptr)
		{
			UE_LOG(LGUI, Error, TEXT("[%s].%d No actor generated!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);

			if (!bIsSubPrefab)
			{
				check(DeserializationSessionId.IsValid());
				LGUIPrefabManager->EndPrefabSystemProcessingActor(DeserializationSessionId);
			}
			return nullptr;
		}
		GenerateObjectArray(SaveData.SavedObjects, SaveData.MapSceneComponentToParent);
#if LGUIPREFAB_LOG_DETAIL_TIME
		UE_LOG(LGUI, Log, TEXT("--GenerateObject take time: %fms"), (FDateTime::Now() - Time).GetTotalMilliseconds());
		Time = FDateTime::Now();
#endif
		//properties
		for (auto& KeyValue : SaveData.SavedObjectData)
		{
			if (auto ObjectPtr = MapGuidToObject.Find(KeyValue.Key))
			{
				WriterOrReaderFunction(*ObjectPtr, KeyValue.Value, Cast<USceneComponent>(*ObjectPtr) != nullptr);
			}
		}

		//sub prefab override properties
		for (auto& Item : SubPrefabOverrideParameters)
		{
			WriterOrReaderFunctionForSubPrefabOverride(Item.Object, Item.ParameterDatas, Item.ParameterNames);
		}

#if LGUIPREFAB_LOG_DETAIL_TIME
		UE_LOG(LGUI, Log, TEXT("--DeserializeObject take time: %fms"), (FDateTime::Now() - Time).GetTotalMilliseconds());
		Time = FDateTime::Now();
#endif

		//component attachment
		for (auto& CompData : ComponentsInThisPrefab)
		{
			if (auto SceneComp = Cast<USceneComponent>(CompData.Component))
			{
				if (CompData.SceneComponentParentGuid.IsValid())
				{
					USceneComponent* ParentComp = nullptr;
					auto ParentObjectPtr = MapGuidToObject.Find(CompData.SceneComponentParentGuid);
					if (ParentObjectPtr != nullptr)
					{
						ParentComp = Cast<USceneComponent>(*ParentObjectPtr);
					}
					if (!ParentComp)
					{
#if WITH_EDITOR
						if (TargetWorld != ULGUIPrefabManagerObject::GetPreviewWorldForPrefabPackage())//skip preview world, only show this in PrefabEditor or LevelEditor
						{
							auto MissingParentMsg = FText::Format(LOCTEXT("MissingParentMsg", "Prefab '{0}' fail to find parent for component '{1}.{2}', do you delete it? The component will attach to root")
								, FText::FromString(PrefabAssetPath), FText::FromString(SceneComp->GetOwner()->GetActorLabel()), FText::FromString(SceneComp->GetName()));
							LGUIUtils::EditorNotification(MissingParentMsg, 10);
							UE_LOG(LGUI, Error, TEXT("[%s].%d %s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *MissingParentMsg.ToString());
						}
#endif
						ParentComp = CreatedRootActor->GetRootComponent();
					}
					if (SceneComp->IsRegistered())
					{
						SceneComp->AttachToComponent(ParentComp, FAttachmentTransformRules::KeepRelativeTransform);
					}
					else
					{
						SceneComp->SetupAttachment(ParentComp);
					}

				}
			}
			if (!CompData.Component->IsRegistered())
			{
				CompData.Component->RegisterComponent();
			}
		}
		for (auto& CompData : SubPrefabRootComponents)
		{
			auto SceneComp = (USceneComponent*)CompData.Component;
			if (auto ParentObjectPtr = MapGuidToObject.Find(CompData.SceneComponentParentGuid))
			{
				if (auto ParentComp = Cast<USceneComponent>(*ParentObjectPtr))
				{
					SceneComp->AttachToComponent(ParentComp, FAttachmentTransformRules::KeepRelativeTransform);
				}
			}
		}

		if (!bIsSubPrefab)//sub-prefab's re-register should handle in parent after all override property
		{
			//mark component reregister to use new property value
			for (auto& Comp : AllComponents)
			{
				PostSetPropertiesOnActor(Comp);
			}
		}

		//attach root actor's parent
		if (USceneComponent* RootComp = CreatedRootActor->GetRootComponent())
		{
			if (Parent)
			{
				RootComp->AttachToComponent(Parent, FAttachmentTransformRules::KeepRelativeTransform);
			}
			if (!bIsSubPrefab)//need to do this in root actor and it will propogate to children. If do this in subprefab and parent prefab override transform data on subprefab's actor, then transform goes wrong
			{
				RootComp->UpdateComponentToWorld();
			}
			if (ReplaceTransform)
			{
				RootComp->SetRelativeLocationAndRotation(InLocation, InRotation);
				RootComp->SetRelativeScale3D(InScale);
			}
		}

#if WITH_EDITOR
		if (!bIsSubPrefab)//sub-prefab's RerunConstructionScripts should handle in parent after all override property, and after root actor attach to parent
		{
			if (!TargetWorld->IsGameWorld())
			{
				ULGUIPrefabManagerObject::OnDeserialize_ProcessComponentsBeforeRerunConstructionScript.ExecuteIfBound(AllComponents);
				//refresh it
				for (auto& Actor : AllActors)
				{
					Actor->RerunConstructionScripts();
				}
			}
		}
#endif

		if (OnSubPrefabFinishDeserializeFunction != nullptr)
		{
			OnSubPrefabFinishDeserializeFunction(CreatedRootActor, MapGuidToObject, MapObjectToOriginGuid, AllActors, AllComponents);
		}
		if (CallbackBeforeAwake != nullptr)
		{
			CallbackBeforeAwake(CreatedRootActor);
		}

#if LGUIPREFAB_LOG_DETAIL_TIME
		Time = FDateTime::Now();
#endif
		if (!bIsSubPrefab)
		{
			check(DeserializationSessionId.IsValid());
			for (auto item : AllActors)
			{
				LGUIPrefabManager->RemoveActorForPrefabSystem(item, DeserializationSessionId);
			}
			LGUIPrefabManager->EndPrefabSystemProcessingActor(DeserializationSessionId);

#if WITH_EDITOR
			if (!TargetWorld->IsGameWorld())
			{
				for (int i = 0; i < AllActors.Num(); i++)
				{
					auto& Actor = AllActors[i];
					if (Actor->GetClass()->ImplementsInterface(ULGUIPrefabInterface::StaticClass()))
					{
						ILGUIPrefabInterface::Execute_EditorAwake(Actor);
					}
					auto Components = Actor->GetComponents();
					for (auto& Comp : Components)
					{
						if (Comp->GetClass()->ImplementsInterface(ULGUIPrefabInterface::StaticClass()))
						{
							ILGUIPrefabInterface::Execute_EditorAwake(Comp);
						}
					}
				}
		}
			else
#endif
			{
				for (int i = 0; i < AllActors.Num(); i++)
				{
					auto& Actor = AllActors[i];
					if (Actor->GetClass()->ImplementsInterface(ULGUIPrefabInterface::StaticClass()))
					{
						ILGUIPrefabInterface::Execute_Awake(Actor);
					}
					auto Components = Actor->GetComponents();
					for (auto& Comp : Components)
					{
						if (Comp->GetClass()->ImplementsInterface(ULGUIPrefabInterface::StaticClass()))
						{
							ILGUIPrefabInterface::Execute_Awake(Comp);
						}
					}
				}
			}
		}

#if LGUIPREFAB_LOG_DETAIL_TIME
		UE_LOG(LGUI, Log, TEXT("--Call Awake (and OnEnable) take time: %fms"), (FDateTime::Now() - Time).GetTotalMilliseconds());
#endif

		return CreatedRootActor;
	}
	AActor* ActorSerializer::DeserializeActor(USceneComponent* Parent, ULGUIPrefab* InPrefab, const TFunction<void()>& InCallbackBeforeDeserialize, bool ReplaceTransform, FVector InLocation, FQuat InRotation, FVector InScale)
	{
		auto StartTime = FDateTime::Now();
		PrefabAssetPath = InPrefab->GetPathName();
#if WITH_EDITOR
		if (bIsEditorOrRuntime)
		{
			//fill new reference data
			this->ReferenceAssetList = InPrefab->ReferenceAssetList;
			this->ReferenceClassList = InPrefab->ReferenceClassList;
			this->ReferenceNameList = InPrefab->ReferenceNameList;

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

			this->ArchiveVersion = FPackageFileVersion(InPrefab->ArchiveVersion_ForBuild, (EUnrealEngineObjectUE5Version)InPrefab->ArchiveVersionUE5_ForBuild);
			this->ArchiveLicenseeVer = InPrefab->ArchiveLicenseeVer_ForBuild;
			this->ArEngineNetVer = InPrefab->ArEngineNetVer_ForBuild;
			this->ArGameNetVer = InPrefab->ArGameNetVer_ForBuild;
		}
		this->PrefabVersion = InPrefab->PrefabVersion;
		this->ArEngineVer = FEngineVersionBase(InPrefab->EngineMajorVersion, InPrefab->EngineMinorVersion, InPrefab->EnginePatchVersion);

		FLGUIPrefabSaveData SaveData;
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
		auto CreatedRootActor = DeserializeActorFromData(SaveData, Parent, ReplaceTransform, InLocation, InRotation, InScale);

		if (ULGUIPrefabSettings::GetLogPrefabLoadTime())
		{
			auto TimeSpan = FDateTime::Now() - StartTime;
			UE_LOG(LGUI, Log, TEXT("Load prefab: '%s', total time: %fms"), *InPrefab->GetName(), TimeSpan.GetTotalMilliseconds());
		}

#if WITH_EDITOR
		ULGUIPrefabManagerObject::MarkBroadcastLevelActorListChanged();//UE5 will not auto refresh scene outliner and display actor label, so manually refresh it.
#endif

		return CreatedRootActor;
	}




	void ActorSerializer::GenerateObjectArray(TMap<FGuid, FLGUIObjectSaveData>& SavedObjects, TMap<FGuid, FGuid>& MapSceneComponentToParent)
	{
		auto CollectDefaultSubobjects = [&](UObject* Target, const FGuid& TargetGuid, FLGUICommonObjectSaveData& ObjectData) {
			//collect default sub object
			TArray<UObject*> DefaultSubObjects;
			ForEachObjectWithOuter(Target, [&DefaultSubObjects](UObject* SubObj) {
				DefaultSubObjects.Add(SubObj);
				});
			for (auto DefaultSubObject : DefaultSubObjects)
			{
				if (DefaultSubObject->HasAnyFlags(EObjectFlags::RF_Transient))continue;
				auto Index = ObjectData.DefaultSubObjectNameArray.IndexOfByKey(DefaultSubObject->GetFName());
				if (Index == INDEX_NONE)
				{
#if WITH_EDITOR
					if (auto Comp = Cast<UActorComponent>(DefaultSubObject))
					{
						if (Comp->IsVisualizationComponent())//visualization component no need to serialize
							continue;
					}
#endif
					UE_LOG(LGUI, Warning, TEXT("[%s].%d Missing guid for default sub object: %s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(DefaultSubObject->GetFName().ToString()));
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
					if (ObjectClass->IsChildOf(AActor::StaticClass()))
					{
						UE_LOG(LGUI, Warning, TEXT("[%s].%d Wrong object class: '%s'. Prefab: '%s'"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(ObjectClass->GetFName().ToString()), *PrefabAssetPath);
						continue;
					}

					if (auto OuterObjectPtr = MapGuidToObject.Find(ObjectData.OuterObjectGuid))
					{
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
			if (auto CreatedNewComponent = Cast<UActorComponent>(CreatedNewObject))
			{
				FComponentDataStruct CompData;
				CompData.Component = CreatedNewComponent;
				if (auto ParentGuidPtr = MapSceneComponentToParent.Find(ObjectGuid))
				{
					CompData.SceneComponentParentGuid = *ParentGuidPtr;
				}
				ComponentsInThisPrefab.Add(CompData);
				AllComponents.Add(CreatedNewComponent);
			}
		}
	}

	AActor* ActorSerializer::GenerateActorArray(TArray<FLGUIActorSaveData>& SavedActors, TMap<FGuid, FLGUIObjectSaveData>& SavedObjects, TMap<FGuid, FGuid>& MapSceneComponentToParent, FGuid ParentGuid)
	{
		AActor* RootActor = nullptr;//first actor is the RootActor
		for (int i = 0; i < SavedActors.Num(); i++)
		{
			auto& InActorData = SavedActors[i];
			if (InActorData.bIsPrefab)
			{
				auto PrefabIndex = InActorData.PrefabAssetIndex;
				if (auto PrefabAssetObject = FindAssetFromListByIndex(PrefabIndex))
				{
					if (auto SubPrefabAsset = Cast<ULGUIPrefab>(PrefabAssetObject))
					{
						AActor* SubPrefabRootActor = nullptr;
						FLGUISubPrefabData SubPrefabData;
						SubPrefabData.PrefabAsset = SubPrefabAsset;

#if WITH_EDITOR
						if (SubPrefabAsset->PrefabVersion < (uint16)ELGUIPrefabVersion::NewObjectOnNestedPrefab)
						{
							SubPrefabAsset->RecreatePrefab();//if is old version then recreate to make it new version
						}
#endif
						//sub prefab
						{
							auto& SubMapGuidToObject = SubPrefabData.MapGuidToObject;
							TMap<FGuid, FGuid> MapObjectGuidFromSubPrefabToParentPrefab;
							for (auto& KeyValue : InActorData.MapObjectGuidFromParentPrefabToSubPrefab)
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
									auto UniqueId = FLGUISubPrefabObjectUniqueIdSaveData{ InActorData.ActorGuid, GuidInOriginPrefab };
									if (auto GuidInParentPtr = InActorData.MapObjectIdToNewlyCreatedId.Find(UniqueId))
									{
										GuidInParent = *GuidInParentPtr;
									}
									else
									{
										GuidInParent = FGuid::NewGuid();
										InActorData.MapObjectIdToNewlyCreatedId.Add(UniqueId, GuidInParent);
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
								[&](AActor*, const TMap<FGuid, TObjectPtr<UObject>>& InSubPrefabMapGuidToObject, const TMap<TObjectPtr<UObject>, FGuid>& InMapObjectToOriginGuid, const TArray<AActor*>& InSubActors, const TArray<UActorComponent*>& InSubComponents) {
								//collect sub prefab's object and guid to parent map, so all objects are ready when set override parameters
								for (auto& KeyValue : InSubPrefabMapGuidToObject)
								{
									auto& GuidInSubPrefab = KeyValue.Key;
									auto& ObjectInSubPrefab = KeyValue.Value;

									auto GuidInParent = GetObjectGuidInParent(GuidInSubPrefab, InMapObjectToOriginGuid[ObjectInSubPrefab]);

									if (auto RecordDataPtr = InActorData.MapObjectGuidToSubPrefabOverrideParameter.Find(GuidInParent))
									{
										FLGUIPrefabOverrideParameterData OverrideDataItem;
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
									if (InActorData.MapObjectIdToNewlyCreatedId.Num() > 0)
									{
										InActorData.MapObjectIdToNewlyCreatedId.Empty();
									}
								}
								else
								{
									//convert data to save
									for (auto& DataItem : InActorData.MapObjectIdToNewlyCreatedId)
									{
										SubPrefabData.MapObjectIdToNewlyCreatedId.Add({ DataItem.Key.RootActorGuidInParentPrefab, DataItem.Key.ObjectGuidInOrignPrefab }, DataItem.Value);
									}
								}
								//collect sub-prefab's actor to parent prefab
								AllActors.Append(InSubActors);
								AllComponents.Append(InSubComponents);
								MapObjectToOriginGuid.Append(InMapObjectToOriginGuid);
								};

							SubPrefabRootActor = ActorSerializer::LoadSubPrefab(this->TargetWorld, SubPrefabAsset, nullptr, DeserializationSessionId, SubMapGuidToObject
								, NewOnSubPrefabFinishDeserializeFunction
							);
						}
						
						if (SubPrefabRootActor != nullptr)
						{
							FComponentDataStruct CompData;
							CompData.Component = SubPrefabRootActor->GetRootComponent();
							FGuid SubPrefabRootCompGuid;
							for (auto& KeyValue : MapGuidToObject)
							{
								if (KeyValue.Value == CompData.Component)
								{
									SubPrefabRootCompGuid = KeyValue.Key;
									break;
								}
							}
							if (auto ParentGuidPtr = MapSceneComponentToParent.Find(SubPrefabRootCompGuid))
							{
								CompData.SceneComponentParentGuid = *ParentGuidPtr;
								SubPrefabRootComponents.Add(CompData);
							}

							SubPrefabMap.Add(SubPrefabRootActor, SubPrefabData);

							if (i == 0)
							{
								RootActor = SubPrefabRootActor;
							}
						}
					}
				}
			}
			else
			{
				if (auto ActorClass = FindClassFromListByIndex(InActorData.ObjectClass))
				{
					if (!ActorClass->IsChildOf(AActor::StaticClass()))//if not the right class, use default
					{
						UE_LOG(LGUI, Warning, TEXT("[%s].%d Find class: '%s' at index: %d, but is not a Actor class, use default. Prefab: '%s'"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(ActorClass->GetFName().ToString()), InActorData.ObjectClass, *PrefabAssetPath);
						ActorClass = AActor::StaticClass();
					}

					auto CollectDefaultSubobjects = [&](AActor* TargetActor) {
						//Collect default sub objects
						TArray<UObject*> DefaultSubObjects;
						ForEachObjectWithOuter(TargetActor, [&DefaultSubObjects](UObject* SubObj) {
							DefaultSubObjects.Add(SubObj);
							});
						for (auto DefaultSubObject : DefaultSubObjects)
						{
							if (DefaultSubObject->HasAnyFlags(EObjectFlags::RF_Transient))continue;
							auto Index = InActorData.DefaultSubObjectNameArray.IndexOfByKey(DefaultSubObject->GetFName());
							if (Index == INDEX_NONE)
							{
								UE_LOG(LGUI, Warning, TEXT("[%s].%d Missing guid for default sub object: %s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(DefaultSubObject->GetFName().ToString()));
								continue;
							}
							auto DefaultSubObjectGuid = InActorData.DefaultSubObjectGuidArray[Index];
							MapGuidToObject.Add(DefaultSubObjectGuid, DefaultSubObject);
							MapObjectToOriginGuid.Add(DefaultSubObject, DefaultSubObjectGuid);
						}
						};

					AActor* NewActor = nullptr;
					bool bNeedFinishSpawn = false;
#if WITH_EDITOR
					//MapGuidToObject can passed from LoadPrefabWithExistingObjects, so we need to find from map first. This only needed in editor, because runtime never use LoadPrefabWithExistingObjects
					if (auto ActorPtr = MapGuidToObject.Find(InActorData.ActorGuid))
					{
						NewActor = (AActor*)(*ActorPtr);
						MapObjectToOriginGuid.Add(NewActor, InActorData.ActorGuid);
						CollectDefaultSubobjects(NewActor);
					}
					else
#endif
					{
						FActorSpawnParameters Spawnparameters;
						Spawnparameters.ObjectFlags = (EObjectFlags)InActorData.ObjectFlags;
						Spawnparameters.bDeferConstruction = true;
						Spawnparameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
#if WITH_EDITOR
						//ref: LevelActor.cpp::SpawnActor 
						//LGUI's editor preview world (or other simple world (not UE5's open world)) don't need external actor, so we need to remove the flag, or game will crash when check external package.
						if ((Spawnparameters.ObjectFlags & EObjectFlags::RF_HasExternalPackage) != 0
							&& !TargetWorld->GetCurrentLevel()->IsUsingExternalActors()
							)
						{
							Spawnparameters.ObjectFlags = Spawnparameters.ObjectFlags & (~EObjectFlags::RF_HasExternalPackage);
						}
#endif
						NewActor = TargetWorld->SpawnActor<AActor>(ActorClass, Spawnparameters);
						MapGuidToObject.Add(InActorData.ActorGuid, NewActor);
						MapObjectToOriginGuid.Add(NewActor, InActorData.ActorGuid);
						CollectDefaultSubobjects(NewActor);
						bNeedFinishSpawn = true;
					}
					//add actor before FinishSpawing, so it's good for component (or other default subobject) to check if actor is processing by prefab system
					LGUIPrefabManager->AddActorForPrefabSystem(NewActor, DeserializationSessionId);
					if (bNeedFinishSpawn)
					{
						NewActor->FinishSpawning(FTransform::Identity, true);
					}

					if (auto RootComp = NewActor->GetRootComponent())
					{
						if (!MapGuidToObject.Contains(InActorData.RootComponentGuid))
						{
							MapGuidToObject.Add(InActorData.RootComponentGuid, RootComp);
							MapObjectToOriginGuid.Add(RootComp, InActorData.RootComponentGuid);
						}

						if (ParentGuid.IsValid())
						{
							FComponentDataStruct CompData;
							CompData.Component = RootComp;
							CompData.SceneComponentParentGuid = ParentGuid;
							ComponentsInThisPrefab.Add(CompData);
						}
					}

					AllActors.Add(NewActor);

					if (i == 0)
					{
						RootActor = NewActor;
					}
				}
				else
				{
					UE_LOG(LGUI, Warning, TEXT("[%s].%d Actor Class of index:%d not found! Prefab: '%s'"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, (InActorData.ObjectClass), *PrefabAssetPath);
				}
			}
		}
		return RootActor;
	}
}

#undef LOCTEXT_NAMESPACE

#if LGUI_CAN_DISABLE_OPTIMIZATION
UE_ENABLE_OPTIMIZATION
#endif
