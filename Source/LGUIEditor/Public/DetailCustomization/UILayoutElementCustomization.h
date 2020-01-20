// Copyright 2019-2020 LexLiu. All Rights Reserved.
#include "IDetailCustomization.h"
#pragma once

/**
 * 
 */
class FUILayoutElementCustomization : public IDetailCustomization
{
public:

	static TSharedRef<IDetailCustomization> MakeInstance();
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
private:
	TWeakObjectPtr<class UUILayoutElement> TargetScriptPtr;
};
