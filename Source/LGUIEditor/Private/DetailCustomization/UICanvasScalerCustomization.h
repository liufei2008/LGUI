// Copyright 2019-2022 LexLiu. All Rights Reserved.
#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#pragma once

/**
 * 
 */
class FUICanvasScalerCustomization : public IDetailCustomization
{
public:

	static TSharedRef<IDetailCustomization> MakeInstance();
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
private:
	TWeakObjectPtr<class ULGUICanvasScaler> TargetScriptPtr;
	float GetMatchValue(TSharedRef<IPropertyHandle> Property)const;
	void SetMatchValue(float value, TSharedRef<IPropertyHandle> Property);
	TSharedPtr<SHorizontalBox> ValueBox;
	FOptionalSize GetValueWidth()const;
};
