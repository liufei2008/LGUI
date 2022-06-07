// Copyright 2019-present LexLiu. All Rights Reserved.

#pragma once

#include "Modules/ModuleInterface.h"

DECLARE_LOG_CATEGORY_EXTERN(LGUISDFFont, Log, All);

class FLGUISDFFontModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};