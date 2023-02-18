// Copyright 2019-present LexLiu. All Rights Reserved.

#pragma once

#include "Factories/Factory.h"
#include "LGUISDFFontDataFactory.generated.h"

UCLASS()
class ULGUISDFFontDataFactory : public UFactory
{
	GENERATED_BODY()
public:
	ULGUISDFFontDataFactory();

	// UFactory interface
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	// End of UFactory interface
};
