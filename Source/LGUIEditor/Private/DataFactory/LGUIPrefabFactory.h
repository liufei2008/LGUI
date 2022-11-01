// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "Factories/Factory.h"
#include "LGUIPrefabFactory.generated.h"

UCLASS()
class ULGUIPrefabFactory : public UFactory
{
	GENERATED_BODY()
public:
	ULGUIPrefabFactory();

	class ULGUIPrefab* SourcePrefab = nullptr;
	// UFactory interface
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	// End of UFactory interface
};
