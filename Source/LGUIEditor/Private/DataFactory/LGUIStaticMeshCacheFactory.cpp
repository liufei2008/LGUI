// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "DataFactory/LGUIStaticMeshCacheFactory.h"
#include "Extensions/UIStaticMesh.h"

#define LOCTEXT_NAMESPACE "ULGUIStaticMeshCacheFactory"


ULGUIStaticMeshCacheFactory::ULGUIStaticMeshCacheFactory()
{
	SupportedClass = ULGUIStaticMeshCacheData::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}
UObject* ULGUIStaticMeshCacheFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	ULGUIStaticMeshCacheData* NewAsset = NewObject<ULGUIStaticMeshCacheData>(InParent, Class, Name, Flags | RF_Transactional);
	return NewAsset;
}

#undef LOCTEXT_NAMESPACE
