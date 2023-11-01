// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "Factories/Factory.h"
#include "LGUIPrefabFactoryForUI.generated.h"

UCLASS(NotBlueprintType)
class ULGUIPrefabForUI : public UObject
{
	GENERATED_BODY()
};

UCLASS()
class ULGUIPrefabFactoryForUI : public UFactory
{
	GENERATED_BODY()
public:
	ULGUIPrefabFactoryForUI();

	// UFactory interface
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	// End of UFactory interface
};
