// Copyright 2019-present LexLiu. All Rights Reserved.

#pragma once

#include "Modules/ModuleInterface.h"

DECLARE_LOG_CATEGORY_EXTERN(LGUISDFFontEditor, Log, All)

class FLGUISDFFontEditorModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
private:
	TSharedPtr<class FAssetTypeActions_Base> SDFFontDataTypeAction;
};