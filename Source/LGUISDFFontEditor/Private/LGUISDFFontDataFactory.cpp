// Copyright 2019-present LexLiu. All Rights Reserved.

#include "LGUISDFFontDataFactory.h"
#include "LGUISDFFontData.h"

#define LOCTEXT_NAMESPACE "LGUISDFFontDataFactory"

ULGUISDFFontDataFactory::ULGUISDFFontDataFactory()
{
	SupportedClass = ULGUISDFFontData::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}
UObject* ULGUISDFFontDataFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<ULGUISDFFontData>(InParent, Class, Name, Flags | RF_Transactional);
}

#undef LOCTEXT_NAMESPACE
