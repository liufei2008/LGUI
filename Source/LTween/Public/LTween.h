// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once
#include "Stats/Stats.h"
#include "Modules/ModuleInterface.h"
DECLARE_LOG_CATEGORY_EXTERN(LTween, Log, All);
DECLARE_STATS_GROUP(TEXT("LTween"), STATGROUP_LTween, STATCAT_Advanced);
class FLTweenModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};