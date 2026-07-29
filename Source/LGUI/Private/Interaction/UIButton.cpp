// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Interaction/UIButton.h"
#include "LGUI.h"

bool UUIButton::OnPointerClick_Implementation(ULexPointerEventData* EventData)
{
	OnClickCPP.Broadcast();
	OnClick.Broadcast();
	OnClickED.FireEvent();
	return AllowEventBubbleUp;
}
