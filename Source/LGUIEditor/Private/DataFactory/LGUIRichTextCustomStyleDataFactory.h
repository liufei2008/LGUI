// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "Factories/Factory.h"
#include "LGUIRichTextCustomStyleDataFactory.generated.h"

UCLASS()
class ULGUIRichTextCustomStyleDataFactory : public UFactory
{
	GENERATED_BODY()
public:
	ULGUIRichTextCustomStyleDataFactory();

	// UFactory interface
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	// End of UFactory interface
};
