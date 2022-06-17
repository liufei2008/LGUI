// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "DataFactory/LGUIPrefabFactory.h"
#include "LGUIEditorModule.h"
#include "PrefabSystem/LGUIPrefab.h"
#include "PrefabSystem/LGUIPrefabHelperObject.h"
#include "Core/Actor/LGUIManagerActor.h"
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
	ULGUIPrefab* NewAsset = NewObject<ULGUIPrefab>(InParent, Class, Name, Flags | RF_Transactional);
	ULGUIPrefabHelperObject* HelperObject = NewObject<ULGUIPrefabHelperObject>(GetTransientPackage());
	HelperObject->PrefabAsset = NewAsset;
	HelperObject->LoadedRootActor = ULGUIEditorManagerObject::GetPreviewWorldForPrefabPackage()->SpawnActor<AUIContainerActor>();
	HelperObject->LoadedRootActor->SetActorLabel(TEXT("RootActor"));
	HelperObject->SavePrefab();
	HelperObject->LoadedRootActor->Destroy();
	HelperObject->ConditionalBeginDestroy();
	return NewAsset;
}

#undef LOCTEXT_NAMESPACE
