// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "DataFactory/LGUIStaticSpriteAtalsDataFactory.h"
#include "Core/LGUIStaticSpriteAtlasData.h"

#define LOCTEXT_NAMESPACE "LGUIStaticSpriteAtalsDataFactory"


ULGUIStaticSpriteAtalsDataFactory::ULGUIStaticSpriteAtalsDataFactory()
{
	SupportedClass = ULGUIStaticSpriteAtlasData::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}
UObject* ULGUIStaticSpriteAtalsDataFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	auto NewAsset = NewObject<ULGUIStaticSpriteAtlasData>(InParent, Class, Name, Flags | RF_Transactional);
	return NewAsset;
}

#undef LOCTEXT_NAMESPACE
