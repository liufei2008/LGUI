// Copyright 2019 LexLiu. All Rights Reserved.

#include "DataFactory/LGUIPrefabFactory.h"
#include "PrefabSystem/LGUIPrefab.h"

#define LOCTEXT_NAMESPACE "LGUIPrefabFactory"


ULGUIPrefabFactory::ULGUIPrefabFactory()
{
	SupportedClass = ULGUIPrefab::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}
UObject* ULGUIPrefabFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	auto NewAsset = NewObject<ULGUIPrefab>(InParent, Class, Name, Flags | RF_Transactional);
	return NewAsset;
}

#undef LOCTEXT_NAMESPACE
