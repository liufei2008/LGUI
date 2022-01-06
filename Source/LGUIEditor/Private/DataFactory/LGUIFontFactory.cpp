// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "DataFactory/LGUIFontFactory.h"
#include "Core/LGUIFontData.h"

#define LOCTEXT_NAMESPACE "ULGUIFontFactory"


ULGUIFontFactory::ULGUIFontFactory()
{
	SupportedClass = ULGUIFontData::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}
UObject* ULGUIFontFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	ULGUIFontData* NewAsset = NewObject<ULGUIFontData>(InParent, Class, Name, Flags | RF_Transactional);
	return NewAsset;
}

#undef LOCTEXT_NAMESPACE
