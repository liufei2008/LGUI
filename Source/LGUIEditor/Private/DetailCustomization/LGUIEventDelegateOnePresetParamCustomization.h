// Copyright 2019-2021 LexLiu. All Rights Reserved.
#include "LGUIEventDelegateOneParamCustomization.h"

#pragma once

/**
 * 
 */
class LGUIEventDelegateOnePresetParamCustomization : public FLGUIEventDelegateOneParamCustomization
{
private:
	LGUIEventDelegateOnePresetParamCustomization() :FLGUIEventDelegateOneParamCustomization(false) {}
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance()
	{
		return MakeShareable(new LGUIEventDelegateOnePresetParamCustomization());
	}
};
