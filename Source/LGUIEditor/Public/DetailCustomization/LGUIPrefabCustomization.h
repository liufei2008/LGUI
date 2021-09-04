// Copyright 2019-2021 LexLiu. All Rights Reserved.

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
	FText GetEngineVersionText()const;
	FText GetPrefabVersionText()const;
	EVisibility ShouldShowFixEngineVersionButton()const;
	FSlateColor GetEngineVersionTextColorAndOpacity()const;
	FSlateColor GetPrefabVersionTextColorAndOpacity()const;
	EVisibility ShouldShowFixPrefabVersionButton()const;

	FReply OnClickRecreteButton();
	FReply OnClickRecreteAllButton();
	FReply OnClickEditPrefabButton();

	void RecreatePrefab(class ULGUIPrefab* Prefab, class UWorld* World);
};
