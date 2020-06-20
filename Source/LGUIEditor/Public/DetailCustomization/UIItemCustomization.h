// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "IDetailCustomization.h"
#pragma once

/**
 * 
 */
class FUIItemCustomization : public IDetailCustomization
{
public:
	FUIItemCustomization();
	~FUIItemCustomization();

	static TSharedRef<IDetailCustomization> MakeInstance();
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
private:
	TArray<TWeakObjectPtr<class UUIItem>> TargetScriptArray;

	void SetDepthInfo(TWeakObjectPtr<class UUIItem> InTargetScript);
	TSharedPtr<STextBlock> DepthInfoTextBlock = nullptr;

	bool GetIsAnchorsEnabled()const;
	FText GetAnchorsTooltipText()const;

	void ForceRefresh(IDetailLayoutBuilder* DetailBuilder);

	void ForceUpdateUI();
};
