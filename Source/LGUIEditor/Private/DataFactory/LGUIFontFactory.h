// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "Factories/Factory.h"
#include "LGUIFontFactory.generated.h"

UCLASS()
class ULGUIFontFactory : public UFactory
{
	GENERATED_BODY()
public:
	ULGUIFontFactory();

	// UFactory interface
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	// End of UFactory interface
};
