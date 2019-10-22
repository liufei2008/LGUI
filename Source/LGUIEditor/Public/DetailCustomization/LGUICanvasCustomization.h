// Copyright 2019 LexLiu. All Rights Reserved.
#include "IDetailCustomization.h"
#pragma once

/**
 * 
 */
class FLGUICanvasCustomization : public IDetailCustomization
{
public:
	FLGUICanvasCustomization();
	~FLGUICanvasCustomization();

	static TSharedRef<IDetailCustomization> MakeInstance();
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
private:
	TArray<TWeakObjectPtr<class ULGUICanvas>> TargetScriptArray;
	void ForceRefresh(IDetailLayoutBuilder* DetailBuilder);
	FText GetDrawcallInfo()const;
	FText GetDrawcallInfoTooltip()const;
};
