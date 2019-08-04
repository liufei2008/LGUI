// Copyright 2019 LexLiu. All Rights Reserved.
#include "IDetailCustomization.h"
#pragma once

/**
 * 
 */
class FEditorToolsCustomization : public IDetailCustomization
{
public:

	static TSharedRef<IDetailCustomization> MakeInstance();
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
private:

};
