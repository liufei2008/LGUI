// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "IDetailCustomization.h"
#pragma once

/**
 * 
 */
class FLGUIFontDataCustomization : public IDetailCustomization
{
public:

	static TSharedRef<IDetailCustomization> MakeInstance();
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
private:
	TWeakObjectPtr<class ULGUIFontData> TargetScriptPtr;
	FReply OnReloadButtonClicked();
	void OnPathTextChanged(const FString& InText);
	void OnPathTextCommitted(const FString& InText);
	IDetailLayoutBuilder* DetailBuilderPtr = nullptr;
	void ForceRefresh();
};
