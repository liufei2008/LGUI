// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "DataFactory/LGUIRichTextImageDataFactory.h"
#include "Core/LGUIRichTextImageData.h"

#define LOCTEXT_NAMESPACE "ULGUIRichTextImageDataFactory"


ULGUIRichTextImageDataFactory::ULGUIRichTextImageDataFactory()
{
	SupportedClass = ULGUIRichTextImageData::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}
UObject* ULGUIRichTextImageDataFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	auto NewAsset = NewObject<ULGUIRichTextImageData>(InParent, Class, Name, Flags | RF_Transactional);
	return NewAsset;
}

#undef LOCTEXT_NAMESPACE
