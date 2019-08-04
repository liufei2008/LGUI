// Copyright 2019 LexLiu. All Rights Reserved.
#include "IDetailCustomization.h"
#pragma once

/**
 * 
 */
class FLGUIPrefabHelperComponentCustomization : public IDetailCustomization
{
public:

	static TSharedRef<IDetailCustomization> MakeInstance();
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
private:
	TWeakObjectPtr<class ULGUIPrefabHelperComponent> TargetScriptPtr;
	void ForceRefresh(IDetailLayoutBuilder* DetailBuilder);
	void CreateSpriteSelector(IDetailCategoryBuilder* category, IDetailLayoutBuilder* DetailBuilder, TSharedRef<IPropertyHandle> handle);
};
