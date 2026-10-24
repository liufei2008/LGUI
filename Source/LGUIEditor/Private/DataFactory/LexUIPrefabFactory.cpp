// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "DataFactory/LexUIPrefabFactory.h"

#include "Core/Components/LexWidget.h"
#include "PrefabSystem/LexUIPrefab.h"
#include "PrefabSystem/LexUIPrefabHelperObject.h"

#define LOCTEXT_NAMESPACE "LexUIPrefabFactory"


ULexUIPrefabFactory::ULexUIPrefabFactory()
{
	SupportedClass = ULexUIPrefab::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* ULexUIPrefabFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	if (SourcePrefab != nullptr)//prefab variant
	{
		ULexUIPrefab* NewAsset = NewObject<ULexUIPrefab>(InParent, Class, Name, Flags | RF_Transactional);
		NewAsset->bIsPrefabVariant = true;
		auto HelperObject = NewAsset->GetPrefabHelperObject();
		HelperObject->PrefabAsset = NewAsset;
		TMap<FGuid, TObjectPtr<UObject>> MapGuidToObject;
		TMap<TObjectPtr<ULexWidget>, FLexUISubPrefabData> SubPrefabMap;
		auto PrefabScene = NewAsset->GetPrefabInstanceScene();
		auto World = PrefabScene->GetWorld();
		HelperObject->LoadedRootWidget = SourcePrefab->LoadPrefabWithExistingObjects(World, World, PrefabScene->GetParentForLoadPrefab(NewAsset), MapGuidToObject, SubPrefabMap);
		FLexUISubPrefabData SubPrefabData;
		SubPrefabData.PrefabAsset = SourcePrefab;
		SubPrefabData.MapGuidToObject = MapGuidToObject;
		for (auto& KeyValue : MapGuidToObject)
		{
			auto GuidInParent = FGuid::NewGuid();
			SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab.Add(GuidInParent, KeyValue.Key);
			HelperObject->MapGuidToObject.Add(GuidInParent, KeyValue.Value);
		}
		HelperObject->SubPrefabMap.Add(HelperObject->LoadedRootWidget, SubPrefabData);
		HelperObject->SavePrefab();
		return NewAsset;
	}
	else
	{
		ULexUIPrefab* NewAsset = NewObject<ULexUIPrefab>(InParent, Class, Name, Flags | RF_Transactional);
		NewAsset->bIsPrefabVariant = false;
		NewAsset->PrefabVersion = LEXUI_CURRENT_PREFAB_VERSION;
		auto HelperObject = NewAsset->GetPrefabHelperObject();
		HelperObject->PrefabAsset = NewAsset;
		auto PrefabScene = NewAsset->GetPrefabInstanceScene();
		auto World = PrefabScene->GetWorld();
		HelperObject->LoadedRootWidget = NewObject<ULexWidget>(World);
		HelperObject->LoadedRootWidget->SetParent(PrefabScene->GetParentForLoadPrefab(NewAsset));
		HelperObject->LoadedRootWidget->SetDisplayName(NewAsset->GetName());
		HelperObject->LoadedRootWidget->SetAnchorMin(FVector2D::Zero());
		HelperObject->LoadedRootWidget->SetAnchorMax(FVector2D::One());
		HelperObject->LoadedRootWidget->SetSizeDelta(FVector2D::Zero());
		HelperObject->SavePrefab();
		return NewAsset;
	}
}

#undef LOCTEXT_NAMESPACE
