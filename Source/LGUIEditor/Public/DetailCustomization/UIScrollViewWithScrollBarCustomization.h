// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "IDetailCustomization.h"
#pragma once

/**
 * 
 */
class FUIScrollViewWithScrollBarCustomization : public IDetailCustomization
{
public:
	FUIScrollViewWithScrollBarCustomization();
	~FUIScrollViewWithScrollBarCustomization();

	static TSharedRef<IDetailCustomization> MakeInstance();
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
private:
	TWeakObjectPtr<class UUIScrollViewWithScrollbarComponent> TargetScriptPtr;
	void ForceRefresh(IDetailLayoutBuilder* DetailBuilder);
};
