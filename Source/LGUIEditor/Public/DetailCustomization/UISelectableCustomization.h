// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once
#include "IDetailCustomization.h"
#include "LGUIEditorUtils.h"

/**
 * 
 */
class FUISelectableCustomization : public IDetailCustomization
{
public:
	~FUISelectableCustomization();
	static TSharedRef<IDetailCustomization> MakeInstance();
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
private:
	TWeakObjectPtr<class UUISelectableComponent> TargetScriptPtr;
	void ForceRefresh(IDetailLayoutBuilder* DetailBuilder);
};
