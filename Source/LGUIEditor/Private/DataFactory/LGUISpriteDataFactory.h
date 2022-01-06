// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "Factories/Factory.h"
#include "LGUISpriteDataFactory.generated.h"

UCLASS()
class ULGUISpriteDataFactory : public UFactory
{
	GENERATED_BODY()
public:
	ULGUISpriteDataFactory();

	class UTexture2D* SpriteTexture = nullptr;
	// UFactory interface
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	// End of UFactory interface
};
