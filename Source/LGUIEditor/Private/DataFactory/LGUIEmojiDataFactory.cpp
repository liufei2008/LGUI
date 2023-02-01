// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "DataFactory/LGUIEmojiDataFactory.h"
#include "Core/LGUIEmojiData.h"

#define LOCTEXT_NAMESPACE "ULGUIEmojiDataFactory"


ULGUIEmojiDataFactory::ULGUIEmojiDataFactory()
{
	SupportedClass = ULGUIEmojiData::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}
UObject* ULGUIEmojiDataFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	auto NewAsset = NewObject<ULGUIEmojiData>(InParent, Class, Name, Flags | RF_Transactional);
	return NewAsset;
}

#undef LOCTEXT_NAMESPACE
