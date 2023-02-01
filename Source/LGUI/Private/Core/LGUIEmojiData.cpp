// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Core/LGUIEmojiData.h"
#include "LGUI.h"

void ULGUIEmojiData::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	OnDataChange.Broadcast();
}
