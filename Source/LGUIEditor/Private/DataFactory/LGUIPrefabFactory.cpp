// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "DataFactory/LGUIPrefabFactory.h"
#include "LGUIEditorModule.h"
#include "PrefabSystem/LGUIPrefab.h"
#include "PrefabSystem/LGUIPrefabHelperObject.h"
#include "PrefabSystem/LGUIPrefabManager.h"
#include "Core/Actor/UIContainerActor.h"

#define LOCTEXT_NAMESPACE "LGUIPrefabFactory"


ULGUIPrefabFactory::ULGUIPrefabFactory()
{
	SupportedClass = ULGUIPrefab::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}
UObject* ULGUIPrefabFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	if (SourcePrefab != nullptr)//prefab variant
	{
		ULGUIPrefab* NewAsset = NewObject<ULGUIPrefab>(InParent, Class, Name, Flags | RF_Transactional);
		NewAsset->bIsPrefabVariant = true;
		ULGUIPrefabHelperObject* HelperObject = NewObject<ULGUIPrefabHelperObject>(GetTransientPackage());
		HelperObject->PrefabAsset = NewAsset;
		TMap<FGuid, TObjectPtr<UObject>> MapGuidToObject;
		TMap<TObjectPtr<AActor>, FLGUISubPrefabData> SubPrefabMap;
		HelperObject->LoadedRootActor = SourcePrefab->LoadPrefabWithExistingObjects(ULGUIPrefabManagerObject::GetPreviewWorldForPrefabPackage(), nullptr, MapGuidToObject, SubPrefabMap);
		FLGUISubPrefabData SubPrefabData;
		SubPrefabData.PrefabAsset = SourcePrefab;
		SubPrefabData.MapGuidToObject = MapGuidToObject;
		for (auto& KeyValue : MapGuidToObject)
		{
			auto GuidInParent = FGuid::NewGuid();
			SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab.Add(GuidInParent, KeyValue.Key);
			HelperObject->MapGuidToObject.Add(GuidInParent, KeyValue.Value);
		}
		HelperObject->SubPrefabMap.Add(HelperObject->LoadedRootActor, SubPrefabData);
		HelperObject->SavePrefab();
		HelperObject->LoadedRootActor->Destroy();
		HelperObject->ConditionalBeginDestroy();
		return NewAsset;
	}
	else
	{
		ULGUIPrefab* NewAsset = NewObject<ULGUIPrefab>(InParent, Class, Name, Flags | RF_Transactional);
		NewAsset->bIsPrefabVariant = false;
		ULGUIPrefabHelperObject* HelperObject = NewObject<ULGUIPrefabHelperObject>(GetTransientPackage());
		HelperObject->PrefabAsset = NewAsset;
		HelperObject->LoadedRootActor = ULGUIPrefabManagerObject::GetPreviewWorldForPrefabPackage()->SpawnActor<AUIContainerActor>();
		HelperObject->LoadedRootActor->SetActorLabel(TEXT("RootActor"));
		HelperObject->SavePrefab();
		HelperObject->LoadedRootActor->Destroy();
		HelperObject->ConditionalBeginDestroy();
		return NewAsset;
	}
}

#undef LOCTEXT_NAMESPACE
