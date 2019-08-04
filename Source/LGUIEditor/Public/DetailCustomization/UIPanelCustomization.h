// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

/**
 * 
 */
class FUIPanelCustomization : public IDetailCustomization
{
public:
	FUIPanelCustomization();
	~FUIPanelCustomization();

	static TSharedRef<IDetailCustomization> MakeInstance();
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
private:
	TArray<class UUIPanel*> TargetScriptArray;
	void ForceRefresh(IDetailLayoutBuilder* DetailBuilder);
	FText GetDrawcallInfo()const;
};
