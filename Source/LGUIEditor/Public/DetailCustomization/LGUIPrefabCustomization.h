// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "IDetailCustomization.h"
#pragma once

/**
 * 
 */
class FLGUIPrefabCustomization : public IDetailCustomization
{
public:

	static TSharedRef<IDetailCustomization> MakeInstance();
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
private:
	TWeakObjectPtr<class ULGUIPrefab> TargetScriptPtr;
	FReply OnClickUseBuildDataButton(bool AllUseBuildData);
	FReply OnClickRecreteButton();
	FReply OnClickRecreteAllButton();
	FText GetPrefabVersionText()const;
	FSlateColor GetPrefabVersionTextColorAndOpacity()const;
};
