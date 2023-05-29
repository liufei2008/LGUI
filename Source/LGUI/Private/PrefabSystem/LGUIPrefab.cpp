// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "PrefabSystem/LGUIPrefab.h"
#include "LGUI.h"
#if WITH_EDITOR
#include "PrefabSystem/2/ActorSerializer.h"
#include "PrefabSystem/ActorSerializer3.h"
#include "PrefabSystem/ActorSerializer4.h"
#endif
#include "PrefabSystem/ActorSerializer5.h"
#include "Utils/LGUIUtils.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "PrefabSystem/LGUIPrefabHelperObject.h"
#include "Core/Actor/UIBaseActor.h"
#include "Core/Actor/UIContainerActor.h"
#include "Engine/Engine.h"

#define LOCTEXT_NAMESPACE "LGUIPrefab"


FLGUISubPrefabData::FLGUISubPrefabData()
{
#if WITH_EDITORONLY_DATA
	EditorIdentifyColor = FLinearColor::MakeRandomColor();
#endif
}
void FLGUISubPrefabData::AddMemberProperty(UObject* InObject, FName InPropertyName)
{
	auto Index = ObjectOverrideParameterArray.IndexOfByPredicate([=](const FLGUIPrefabOverrideParameterData& Item) {
		return Item.Object == InObject;
		});
	if (Index == INDEX_NONE)
	{
		FLGUIPrefabOverrideParameterData DataItem;
		DataItem.Object = InObject;
		DataItem.MemberPropertyNames.Add(InPropertyName);
		ObjectOverrideParameterArray.Add(DataItem);
	}
	else
	{
		auto& DataItem = ObjectOverrideParameterArray[Index];
		if (!DataItem.MemberPropertyNames.Contains(InPropertyName))
		{
			DataItem.MemberPropertyNames.Add(InPropertyName);
		}
	}
}

void FLGUISubPrefabData::AddMemberProperty(UObject* InObject, const TArray<FName>& InPropertyNames)
{
	auto Index = ObjectOverrideParameterArray.IndexOfByPredicate([=](const FLGUIPrefabOverrideParameterData& Item) {
		return Item.Object == InObject;
		});
	if (Index == INDEX_NONE)
	{
		FLGUIPrefabOverrideParameterData DataItem;
		DataItem.Object = InObject;
		DataItem.MemberPropertyNames = InPropertyNames;
		ObjectOverrideParameterArray.Add(DataItem);
	}
	else
	{
		auto& DataItem = ObjectOverrideParameterArray[Index];
		for (auto& NameItem : InPropertyNames)
		{
			if (!DataItem.MemberPropertyNames.Contains(NameItem))
			{
				DataItem.MemberPropertyNames.Add(NameItem);
			}
		}
	}
}

void FLGUISubPrefabData::RemoveMemberProperty(UObject* InObject, FName InPropertyName)
{
	auto Index = ObjectOverrideParameterArray.IndexOfByPredicate([=](const FLGUIPrefabOverrideParameterData& Item) {
		return Item.Object == InObject;
		});
	if (Index != INDEX_NONE)
	{
		auto& DataItem = ObjectOverrideParameterArray[Index];
		if (DataItem.MemberPropertyNames.Contains(InPropertyName))
		{
			DataItem.MemberPropertyNames.Remove(InPropertyName);
		}
		if (DataItem.MemberPropertyNames.Num() <= 0)
		{
			ObjectOverrideParameterArray.RemoveAt(Index);
		}
	}
}

void FLGUISubPrefabData::RemoveMemberProperty(UObject* InObject)
{
	auto Index = ObjectOverrideParameterArray.IndexOfByPredicate([=](const FLGUIPrefabOverrideParameterData& Item) {
		return Item.Object == InObject;
		});
	if (Index != INDEX_NONE)
	{
		ObjectOverrideParameterArray.RemoveAt(Index);
	}
}

bool FLGUISubPrefabData::CheckParameters()
{
	bool AnythingChanged = false;
	for (int i = 0; i < ObjectOverrideParameterArray.Num(); i++)
	{
		auto DataItem = ObjectOverrideParameterArray[i];
		if (!DataItem.Object.IsValid())
		{
			ObjectOverrideParameterArray.RemoveAt(i);
			i--;
			AnythingChanged = true;
		}
		else
		{
			TSet<FName> PropertyNamesToRemove;
			auto Object = DataItem.Object;
			for (auto PropertyName : DataItem.MemberPropertyNames)
			{
				auto Property = FindFProperty<FProperty>(Object->GetClass(), PropertyName);
				if (Property == nullptr)
				{
					PropertyNamesToRemove.Add(PropertyName);
				}
			}
			for (auto PropertyName : PropertyNamesToRemove)
			{
				DataItem.MemberPropertyNames.Remove(PropertyName);
				AnythingChanged = true;
			}
		}
	}
	return AnythingChanged;
}

ULGUIPrefab::ULGUIPrefab()
{

}

#if WITH_EDITOR
AActor* ULGUIPrefab::GetContainerActor()
{
	if (ContainerActor.IsValid())return ContainerActor.Get();
	auto World = ULGUIEditorManagerObject::GetPreviewWorldForPrefabPackage();
	if (ReferenceClassList.Num() > 0 && ReferenceClassList[0]->IsChildOf(AUIBaseActor::StaticClass()))
	{
		auto RootUICanvasActor = (AUIContainerActor*)(World->SpawnActor<AActor>(AUIContainerActor::StaticClass(), FTransform::Identity));
		RootUICanvasActor->GetRootComponent()->SetWorldLocationAndRotationNoPhysics(FVector::ZeroVector, FRotator(0, 0, 0));

		RootUICanvasActor->GetUIItem()->SetWidth(1920);
		RootUICanvasActor->GetUIItem()->SetHeight(1080);
		ContainerActor = RootUICanvasActor;
	}
	else
	{
		auto RootActor = World->SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
		ContainerActor = RootActor;
	}
	return ContainerActor.Get();
}
void ULGUIPrefab::RefreshAgentObjectsInPreviewWorld()
{
	ClearAgentObjectsInPreviewWorld();
	MakeAgentObjectsInPreviewWorld();
}
void ULGUIPrefab::MakeAgentObjectsInPreviewWorld()
{
	if (PrefabVersion >= (uint16)ELGUIPrefabVersion::BuildinFArchive)
	{
		if (PrefabHelperObject == nullptr)
		{
			PrefabHelperObject = NewObject<ULGUIPrefabHelperObject>(this, "PrefabHelper");
			PrefabHelperObject->PrefabAsset = this;
		}
		if (!IsValid(PrefabHelperObject->LoadedRootActor))
		{
			auto World = ULGUIEditorManagerObject::GetPreviewWorldForPrefabPackage();
			PrefabHelperObject->LoadPrefab(World, GetContainerActor()->GetRootComponent());
		}
	}
}
void ULGUIPrefab::ClearAgentObjectsInPreviewWorld()
{
	if (PrefabHelperObject != nullptr)
	{
		PrefabHelperObject->ClearLoadedPrefab();
	}
}

struct LGUIVersionScope
{
public:
	uint16 PrefabVersion = 0;
	uint16 EngineMajorVersion = 0;
	uint16 EngineMinorVersion = 0;
	uint16 EnginePatchVersion = 0;
	int32 ArchiveVersion = 0;
	int32 ArchiveLicenseeVer = 0;
	uint32 ArEngineNetVer = 0;
	uint32 ArGameNetVer = 0;

	ULGUIPrefab* Prefab = nullptr;
	LGUIVersionScope(ULGUIPrefab* InPrefab)
	{
		Prefab = InPrefab;
		this->EngineMajorVersion = Prefab->EngineMajorVersion;
		this->EngineMinorVersion = Prefab->EngineMinorVersion;
		this->PrefabVersion = Prefab->PrefabVersion;
		this->ArchiveVersion = Prefab->ArchiveVersion;
		this->ArchiveLicenseeVer = Prefab->ArchiveLicenseeVer;
		this->ArEngineNetVer = Prefab->ArEngineNetVer;
		this->ArGameNetVer = Prefab->ArGameNetVer;
	}
	~LGUIVersionScope()
	{
		Prefab->EngineMajorVersion = this->EngineMajorVersion;
		Prefab->EngineMinorVersion = this->EngineMinorVersion;
		Prefab->PrefabVersion = this->PrefabVersion;
		Prefab->ArchiveVersion = this->ArchiveVersion;
		Prefab->ArchiveLicenseeVer = this->ArchiveLicenseeVer;
		Prefab->ArEngineNetVer = this->ArEngineNetVer;
		Prefab->ArGameNetVer = this->ArGameNetVer;
	}
};

ULGUIPrefabHelperObject* ULGUIPrefab::GetPrefabHelperObject()
{
	if (!IsValid(PrefabHelperObject))
	{
		PrefabHelperObject = NewObject<ULGUIPrefabHelperObject>(this, "PrefabHelper");
		PrefabHelperObject->PrefabAsset = this;
	}
	if (!IsValid(PrefabHelperObject->LoadedRootActor))
	{
		auto World = ULGUIEditorManagerObject::GetPreviewWorldForPrefabPackage();
		PrefabHelperObject->LoadPrefab(World, nullptr);
	}
	return PrefabHelperObject;
}

void ULGUIPrefab::BeginCacheForCookedPlatformData(const ITargetPlatform* TargetPlatform)
{
	BinaryDataForBuild.Empty();
	if (!IsValid(PrefabHelperObject) || !IsValid(PrefabHelperObject->LoadedRootActor))
	{
		UE_LOG(LGUI, Log, TEXT("[%s].%d AgentObjects not valid, recreate it! prefab: '%s'"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(this->GetPathName()));
		MakeAgentObjectsInPreviewWorld();
	}

	//serialize to runtime data
	{
		LGUIVersionScope VersionProtect(this);
		//check override parameter. although parameter is refreshed when sub prefab change, but what if sub prefab is changed outside of editor?
		bool AnythingChange = false;
		for (auto& KeyValue : PrefabHelperObject->SubPrefabMap)
		{
			if (KeyValue.Value.CheckParameters())
			{
				AnythingChange = true;
			}
		}
		if (AnythingChange)
		{
			UE_LOG(LGUI, Log, TEXT("[%s].%d Something changed in sub prefab override parameter, refresh it. Prefab: '%s'."), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(this->GetPathName()));
		}

		TMap<UObject*, FGuid> MapObjectToGuid;
		for (auto& KeyValue : PrefabHelperObject->MapGuidToObject)
		{
			if (IsValid(KeyValue.Value))
			{
				MapObjectToGuid.Add(KeyValue.Value, KeyValue.Key);
			}
		}
		this->SavePrefab(PrefabHelperObject->LoadedRootActor
			, MapObjectToGuid, PrefabHelperObject->SubPrefabMap
			, false
		);
		PrefabHelperObject->MapGuidToObject.Empty();
		for (auto KeyValue : MapObjectToGuid)
		{
			PrefabHelperObject->MapGuidToObject.Add(KeyValue.Value, KeyValue.Key);
		}
	}
}
void ULGUIPrefab::WillNeverCacheCookedPlatformDataAgain()
{
	if (PrefabVersion >= (uint16)ELGUIPrefabVersion::BuildinFArchive)
	{
		BinaryDataForBuild.Empty();
		ReferenceAssetListForBuild.Empty();
		ReferenceClassListForBuild.Empty();
		ReferenceNameListForBuild.Empty();
	}
}
void ULGUIPrefab::ClearCachedCookedPlatformData(const ITargetPlatform* TargetPlatform)
{
	if (PrefabVersion >= (uint16)ELGUIPrefabVersion::BuildinFArchive)
	{
		BinaryDataForBuild.Empty();
		ReferenceAssetListForBuild.Empty();
		ReferenceClassListForBuild.Empty();
		ReferenceNameListForBuild.Empty();
	}
}

void ULGUIPrefab::PostInitProperties()
{
	Super::PostInitProperties();
}
void ULGUIPrefab::PostCDOContruct()
{
	Super::PostCDOContruct();
}

void ULGUIPrefab::PostRename(UObject* OldOuter, const FName OldName)
{
	Super::PostRename(OldOuter, OldName);
}
void ULGUIPrefab::PreDuplicate(FObjectDuplicationParameters& DupParams)
{
	Super::PreDuplicate(DupParams);
}

void ULGUIPrefab::PostDuplicate(bool bDuplicateForPIE)
{
	Super::PostDuplicate(bDuplicateForPIE);
	if (PrefabVersion >= (uint16)ELGUIPrefabVersion::BuildinFArchive)
	{

	}
	else
	{
		//generate new guid for actor data
		LGUIPrefabSystem::ActorSerializer::RenewActorGuidForDuplicate(this);
	}
}

void ULGUIPrefab::PostLoad()
{
	Super::PostLoad();
}

void ULGUIPrefab::BeginDestroy()
{
#if WITH_EDITOR
	if (IsValid(PrefabHelperObject))
	{
		ClearAgentObjectsInPreviewWorld();
		PrefabHelperObject->ConditionalBeginDestroy();
	}
#endif
	Super::BeginDestroy();
}

void ULGUIPrefab::FinishDestroy()
{
	Super::FinishDestroy();
}

void ULGUIPrefab::PostEditUndo()
{
	Super::PostEditUndo();
	RefreshAgentObjectsInPreviewWorld();
}
bool ULGUIPrefab::IsEditorOnly()const
{
	auto PathName = this->GetPathName();
	if (PathName.StartsWith(TEXT("/LGUI/Prefabs/"))//LGUI's preset prefab no need to use in runtime
		|| PathName.Contains(TEXT("/EditorOnly/"))//if prefab stays in a folder named "EditorOnly" then skip it too
		)
	{
		return true;
	}
	return false;
}

#endif

AActor* ULGUIPrefab::LoadPrefab(UWorld* InWorld, USceneComponent* InParent, bool SetRelativeTransformToIdentity)
{
	AActor* LoadedRootActor = nullptr;
	if (InWorld)
	{
#if WITH_EDITOR
		switch ((ELGUIPrefabVersion)PrefabVersion)
		{
		case ELGUIPrefabVersion::ObjectName:
		{
			LoadedRootActor = LGUIPrefabSystem5::ActorSerializer::LoadPrefab(InWorld, this, InParent, SetRelativeTransformToIdentity);
		}
		break;
		case ELGUIPrefabVersion::NestedDefaultSubObject:
		{
			LoadedRootActor = LGUIPrefabSystem4::ActorSerializer::LoadPrefab(InWorld, this, InParent, SetRelativeTransformToIdentity);
		}
		break;
		case ELGUIPrefabVersion::BuildinFArchive:
		{
			LoadedRootActor = LGUIPrefabSystem3::ActorSerializer::LoadPrefab(InWorld, this, InParent, SetRelativeTransformToIdentity);
		}
		break;
		default:
		{
			LoadedRootActor = LGUIPrefabSystem::ActorSerializer::LoadPrefab(InWorld, this, InParent, SetRelativeTransformToIdentity);
		}
		break;
		}
#else
		LoadedRootActor = LGUIPrefabSystem5::ActorSerializer::LoadPrefab(InWorld, this, InParent, SetRelativeTransformToIdentity);
#endif
	}
	return LoadedRootActor;
}

AActor* ULGUIPrefab::LoadPrefab(UObject* WorldContextObject, USceneComponent* InParent, bool SetRelativeTransformToIdentity)
{
	auto World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (World)
	{
		return LoadPrefab(World, InParent, SetRelativeTransformToIdentity);
	}
	return nullptr;
}
AActor* ULGUIPrefab::LoadPrefabWithTransform(UObject* WorldContextObject, USceneComponent* InParent, FVector Location, FRotator Rotation, FVector Scale)
{
	AActor* LoadedRootActor = nullptr;
	auto World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (World)
	{
#if WITH_EDITOR
		switch ((ELGUIPrefabVersion)PrefabVersion)
		{
		case ELGUIPrefabVersion::ObjectName:
		{
			LoadedRootActor = LGUIPrefabSystem5::ActorSerializer::LoadPrefab(World, this, InParent, Location, Rotation.Quaternion(), Scale);
		}
		break;
		case ELGUIPrefabVersion::NestedDefaultSubObject:
		{
			LoadedRootActor = LGUIPrefabSystem4::ActorSerializer::LoadPrefab(World, this, InParent, Location, Rotation.Quaternion(), Scale);
		}
		break;
		case ELGUIPrefabVersion::BuildinFArchive:
		{
			LoadedRootActor = LGUIPrefabSystem3::ActorSerializer::LoadPrefab(World, this, InParent, Location, Rotation.Quaternion(), Scale);
		}
		break;
		default:
		{
			LoadedRootActor = LGUIPrefabSystem::ActorSerializer::LoadPrefab(World, this, InParent, Location, Rotation.Quaternion(), Scale);
		}
		break;
		}
#else
		LoadedRootActor = LGUIPrefabSystem5::ActorSerializer::LoadPrefab(World, this, InParent, Location, Rotation.Quaternion(), Scale);
#endif
	}
	return LoadedRootActor;
}
AActor* ULGUIPrefab::LoadPrefabWithReplacement(UObject* WorldContextObject, USceneComponent* InParent, const TMap<UObject*, UObject*>& InReplaceAssetMap, const TMap<UClass*, UClass*>& InReplaceClassMap)
{
	AActor* LoadedRootActor = nullptr;
	auto World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (World)
	{
#if WITH_EDITOR
		switch ((ELGUIPrefabVersion)PrefabVersion)
		{
		case ELGUIPrefabVersion::ObjectName:
		{
			LoadedRootActor = LGUIPrefabSystem5::ActorSerializer::LoadPrefab(World, this, InParent, InReplaceAssetMap, InReplaceClassMap);
		}
		break;
		default:
		{
			UE_LOG(LGUI, Error, TEXT("[%s].%d This prefab version is too old to support this function, open this prefab and hit \"Apply\" button to fix it. Prefab: '%s'"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *this->GetPathName());
		}
		break;
		}
#else
		LoadedRootActor = LGUIPrefabSystem5::ActorSerializer::LoadPrefab(World, this, InParent, InReplaceAssetMap, InReplaceClassMap);
#endif
	}
	return LoadedRootActor;
}
AActor* ULGUIPrefab::LoadPrefabWithTransform(UObject* WorldContextObject, USceneComponent* InParent, FVector Location, FQuat Rotation, FVector Scale)
{
	AActor* LoadedRootActor = nullptr;
	auto World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (World)
	{
#if WITH_EDITOR
		switch ((ELGUIPrefabVersion)PrefabVersion)
		{
		case ELGUIPrefabVersion::ObjectName:
		{
			LoadedRootActor = LGUIPrefabSystem5::ActorSerializer::LoadPrefab(World, this, InParent, Location, Rotation, Scale);
		}
		break;
		case ELGUIPrefabVersion::NestedDefaultSubObject:
		{
			LoadedRootActor = LGUIPrefabSystem4::ActorSerializer::LoadPrefab(World, this, InParent, Location, Rotation, Scale);
		}
		break;
		case ELGUIPrefabVersion::BuildinFArchive:
		{
			LoadedRootActor = LGUIPrefabSystem3::ActorSerializer::LoadPrefab(World, this, InParent, Location, Rotation, Scale);
		}
		break;
		default:
		{
			LoadedRootActor = LGUIPrefabSystem::ActorSerializer::LoadPrefab(World, this, InParent, Location, Rotation, Scale);
		}
		break;
		}
#else
		LoadedRootActor = LGUIPrefabSystem5::ActorSerializer::LoadPrefab(World, this, InParent, Location, Rotation, Scale);
#endif
	}
	return LoadedRootActor;
}

#if WITH_EDITOR
AActor* ULGUIPrefab::LoadPrefabWithExistingObjects(UWorld* InWorld, USceneComponent* InParent
	, TMap<FGuid, UObject*>& InOutMapGuidToObject, TMap<AActor*, FLGUISubPrefabData>& OutSubPrefabMap
	, bool InSetHierarchyIndexForRootComponent
)
{
	AActor* LoadedRootActor = nullptr;
	switch ((ELGUIPrefabVersion)PrefabVersion)
	{
	case ELGUIPrefabVersion::ObjectName:
	{
		LoadedRootActor = LGUIPrefabSystem5::ActorSerializer::LoadPrefabWithExistingObjects(InWorld, this, InParent
			, InOutMapGuidToObject, OutSubPrefabMap
			, InSetHierarchyIndexForRootComponent
		);
	}
	break;
	case ELGUIPrefabVersion::NestedDefaultSubObject:
	{
		LoadedRootActor = LGUIPrefabSystem4::ActorSerializer::LoadPrefabWithExistingObjects(InWorld, this, InParent
			, InOutMapGuidToObject, OutSubPrefabMap
			, InSetHierarchyIndexForRootComponent
		);
	}
	break;
	case ELGUIPrefabVersion::BuildinFArchive:
	{
		LoadedRootActor = LGUIPrefabSystem3::ActorSerializer::LoadPrefabWithExistingObjects(InWorld, this, InParent
			, InOutMapGuidToObject, OutSubPrefabMap
			, InSetHierarchyIndexForRootComponent
		);
	}
	break;
	default:
	{
		TArray<AActor*> OutCreatedActors;
		TArray<FGuid> OutCreatedActorsGuid;
		LoadedRootActor = LGUIPrefabSystem::ActorSerializer::LoadPrefabForEdit(InWorld, this, InParent
			, nullptr, nullptr, OutCreatedActors, OutCreatedActorsGuid);
		for (int i = 0; i < OutCreatedActors.Num(); i++)
		{
			InOutMapGuidToObject.Add(OutCreatedActorsGuid[i], OutCreatedActors[i]);
		}
	}
	break;
	}
	return LoadedRootActor;
}

bool ULGUIPrefab::IsPrefabBelongsToThisSubPrefab(ULGUIPrefab* InPrefab, bool InRecursive)
{
	MakeAgentObjectsInPreviewWorld();
	if (!PrefabHelperObject)return false;
	if (this == InPrefab)return false;
	for (auto& KeyValue : PrefabHelperObject->SubPrefabMap)
	{
		if (KeyValue.Value.PrefabAsset == InPrefab)
		{
			return true;
		}
	}
	if (InRecursive)
	{
		for (auto& KeyValue : PrefabHelperObject->SubPrefabMap)
		{
			if (KeyValue.Value.PrefabAsset->IsPrefabBelongsToThisSubPrefab(InPrefab, InRecursive))
			{
				return true;
			}
		}
	}
	return false;
}

#if WITH_EDITOR
void ULGUIPrefab::CopyDataTo(ULGUIPrefab* TargetPrefab)
{
	TargetPrefab->ReferenceAssetList = this->ReferenceAssetList;
	TargetPrefab->ReferenceClassList = this->ReferenceClassList;
	TargetPrefab->ReferenceNameList = this->ReferenceNameList;
	TargetPrefab->ReferenceStringList = this->ReferenceStringList;
	TargetPrefab->ReferenceTextList = this->ReferenceTextList;
	TargetPrefab->BinaryData = this->BinaryData;
	TargetPrefab->PrefabVersion = this->PrefabVersion;
	TargetPrefab->EngineMajorVersion = this->EngineMajorVersion;
	TargetPrefab->EngineMinorVersion = this->EngineMinorVersion;
	TargetPrefab->EnginePatchVersion = this->EnginePatchVersion;
	TargetPrefab->ArchiveVersion = this->ArchiveVersion;
	TargetPrefab->ArchiveLicenseeVer = this->ArchiveLicenseeVer;
	TargetPrefab->ArEngineNetVer = this->ArEngineNetVer;
	TargetPrefab->ArGameNetVer = this->ArGameNetVer;
	TargetPrefab->PrefabDataForPrefabEditor = this->PrefabDataForPrefabEditor;
}
#endif

void ULGUIPrefab::SavePrefab(AActor* RootActor
	, TMap<UObject*, FGuid>& InOutMapObjectToGuid, TMap<AActor*, FLGUISubPrefabData>& InSubPrefabMap
	, bool InForEditorOrRuntimeUse
)
{
	LGUIPrefabSystem5::ActorSerializer::SavePrefab(RootActor, this
		, InOutMapObjectToGuid, InSubPrefabMap
		, InForEditorOrRuntimeUse
	);
}

AActor* ULGUIPrefab::LoadPrefabInEditor(UWorld* InWorld, USceneComponent* InParent, bool SetRelativeTransformToIdentity)
{
	AActor* LoadedRootActor = nullptr;
	switch ((ELGUIPrefabVersion)PrefabVersion)
	{
	case ELGUIPrefabVersion::ObjectName:
	{
		TMap<FGuid, UObject*> MapGuidToObject;
		TMap<AActor*, FLGUISubPrefabData> SubPrefabMap;
		LoadedRootActor = LGUIPrefabSystem5::ActorSerializer::LoadPrefabWithExistingObjects(InWorld, this
			, InParent, MapGuidToObject, SubPrefabMap
		);
	}
	break;
	case ELGUIPrefabVersion::NestedDefaultSubObject:
	{
		TMap<FGuid, UObject*> MapGuidToObject;
		TMap<AActor*, FLGUISubPrefabData> SubPrefabMap;
		LoadedRootActor = LGUIPrefabSystem4::ActorSerializer::LoadPrefabWithExistingObjects(InWorld, this
			, InParent, MapGuidToObject, SubPrefabMap
		);
	}
	break;
	case ELGUIPrefabVersion::BuildinFArchive:
	{
		TMap<FGuid, UObject*> MapGuidToObject;
		TMap<AActor*, FLGUISubPrefabData> SubPrefabMap;
		LoadedRootActor = LGUIPrefabSystem3::ActorSerializer::LoadPrefabWithExistingObjects(InWorld, this
			, InParent, MapGuidToObject, SubPrefabMap
		);
	}
	break;
	default:
	{
		LoadedRootActor = LGUIPrefabSystem::ActorSerializer::LoadPrefabInEditor(InWorld, this
			, InParent);
	}
	break;
	}
	return LoadedRootActor;
}

AActor* ULGUIPrefab::LoadPrefabInEditor(UWorld* InWorld, USceneComponent* InParent, TMap<AActor*, FLGUISubPrefabData>& OutSubPrefabMap, TMap<FGuid, UObject*>& OutMapGuidToObject, bool SetRelativeTransformToIdentity)
{
	AActor* LoadedRootActor = nullptr;
	switch ((ELGUIPrefabVersion)PrefabVersion)
	{
	case ELGUIPrefabVersion::ObjectName:
	{
		LoadedRootActor = LGUIPrefabSystem5::ActorSerializer::LoadPrefabWithExistingObjects(InWorld, this
			, InParent, OutMapGuidToObject, OutSubPrefabMap
		);
	}
	break;
	case ELGUIPrefabVersion::NestedDefaultSubObject:
	{
		LoadedRootActor = LGUIPrefabSystem4::ActorSerializer::LoadPrefabWithExistingObjects(InWorld, this
			, InParent, OutMapGuidToObject, OutSubPrefabMap
		);
	}
	break;
	case ELGUIPrefabVersion::BuildinFArchive:
	{
		LoadedRootActor = LGUIPrefabSystem3::ActorSerializer::LoadPrefabWithExistingObjects(InWorld, this
			, InParent, OutMapGuidToObject, OutSubPrefabMap
		);
	}
	break;
	default:
	{
		LoadedRootActor = LGUIPrefabSystem::ActorSerializer::LoadPrefabInEditor(InWorld, this
			, InParent);
	}
	break;
	}
	return LoadedRootActor;
}

#endif

#undef LOCTEXT_NAMESPACE