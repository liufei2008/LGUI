// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "PrefabSystem/LGUIPrefabSettings.h"
#include "LGUI.h"

#if WITH_EDITOR
void ULGUIPrefabSettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

bool ULGUIPrefabSettings::GetLogPrefabLoadTime()
{
	return GetDefault<ULGUIPrefabSettings>()->bLogPrefabLoadTime;
}
