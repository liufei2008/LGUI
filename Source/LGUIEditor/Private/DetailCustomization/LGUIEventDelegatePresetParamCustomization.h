﻿// Copyright 2019-Present LexLiu. All Rights Reserved.
#include "LGUIEventDelegateCustomization.h"

#pragma once

/**
 * 
 */
class LGUIEventDelegatePresetParamCustomization : public FLGUIEventDelegateCustomization
{
private:
	LGUIEventDelegatePresetParamCustomization() :FLGUIEventDelegateCustomization(false) {}
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance()
	{
		return MakeShareable(new LGUIEventDelegatePresetParamCustomization());
	}
};
