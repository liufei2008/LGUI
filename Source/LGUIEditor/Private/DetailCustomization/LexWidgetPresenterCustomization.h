// Copyright 2019-Present LexLiu. All Rights Reserved.
#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#pragma once

/**
 * 
 */
class FLexWidgetPresenterCustomization : public IDetailCustomization
{
public:
	FLexWidgetPresenterCustomization();
	~FLexWidgetPresenterCustomization();

	static TSharedRef<IDetailCustomization> MakeInstance();
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
private:
	TArray<TWeakObjectPtr<class ULexWidgetPresenterComponent>> TargetScriptArray;
	void ForceRefresh(IDetailLayoutBuilder* DetailBuilder);
};
