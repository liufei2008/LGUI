// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "Modules/ModuleInterface.h"
#include "DetailCategoryBuilder.h"

DECLARE_LOG_CATEGORY_EXTERN(LGUIComponentVisualizer, Log, All);

class FLGUIComponentVisualizerModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};