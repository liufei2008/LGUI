// Copyright 2019 LexLiu. All Rights Reserved.

#include "LGUI.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FLGUIModule"
DEFINE_LOG_CATEGORY(LGUI);

void FLGUIModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FLGUIModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FLGUIModule, LGUI)//if second param is wrong, an error like "EmptyLinkFunctionForStaticInitialization(XXX)" will occor when package project