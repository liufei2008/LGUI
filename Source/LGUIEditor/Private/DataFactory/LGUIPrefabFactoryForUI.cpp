// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "DataFactory/LGUIPrefabFactoryForUI.h"
#include "LGUIEditorModule.h"
#include "PrefabSystem/LGUIPrefab.h"
#include "PrefabSystem/LGUIPrefabHelperObject.h"
#include "PrefabSystem/LGUIPrefabManager.h"
#include "Core/Actor/UIContainerActor.h"

#define LOCTEXT_NAMESPACE "LGUIPrefabFactoryForUI"

ULGUIPrefabFactoryForUI::ULGUIPrefabFactoryForUI()
{
	SupportedClass = ULGUIPrefabForUI::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* ULGUIPrefabFactoryForUI::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	ULGUIPrefab* NewAsset = NewObject<ULGUIPrefab>(InParent, ULGUIPrefab::StaticClass(), Name, Flags | RF_Transactional);
	ULGUIPrefabHelperObject* HelperObject = NewObject<ULGUIPrefabHelperObject>(GetTransientPackage());
	HelperObject->PrefabAsset = NewAsset;
	HelperObject->LoadedRootActor = ULGUIPrefabManagerObject::GetPreviewWorldForPrefabPackage()->SpawnActor<AUIContainerActor>();
	HelperObject->LoadedRootActor->SetActorLabel(TEXT("RootActor"));
	if (!HelperObject->LoadedRootActor->GetRootComponent())
	{
		USceneComponent* RootComponent = NewObject<USceneComponent>(HelperObject->LoadedRootActor, USceneComponent::GetDefaultSceneRootVariableName(), RF_Transactional);
		RootComponent->Mobility = EComponentMobility::Movable;
		RootComponent->bVisualizeComponent = false;

		HelperObject->LoadedRootActor->SetRootComponent(RootComponent);
		RootComponent->RegisterComponent();
		HelperObject->LoadedRootActor->AddInstanceComponent(RootComponent);
	}
	HelperObject->SavePrefab();
	HelperObject->LoadedRootActor->Destroy();
	HelperObject->ConditionalBeginDestroy();
	return NewAsset;
}

#undef LOCTEXT_NAMESPACE
