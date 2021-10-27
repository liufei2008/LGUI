// Copyright 2019-2021 LexLiu. All Rights Reserved.
#include "LGUIDrawableEventOneParamCustomization.h"

#pragma once

/**
 * 
 */
class LGUIDrawableEventOnePresetParamCustomization : public FLGUIDrawableEventOneParamCustomization
{
private:
	LGUIDrawableEventOnePresetParamCustomization() :FLGUIDrawableEventOneParamCustomization(false) {}
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance()
	{
		return MakeShareable(new LGUIDrawableEventOnePresetParamCustomization());
	}
};
