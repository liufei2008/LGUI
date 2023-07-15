// Copyright 2019-Present LexLiu. All Rights Reserved.
#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#pragma once

/**
 * 
 */
class FLGUICanvasCustomClip_RoundedRect_Customization : public IDetailCustomization
{
public:
	FLGUICanvasCustomClip_RoundedRect_Customization();
	~FLGUICanvasCustomClip_RoundedRect_Customization();

	static TSharedRef<IDetailCustomization> MakeInstance();
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
private:
	TArray<TWeakObjectPtr<class ULGUICanvasCustomClip_RoundedRect>> TargetScriptArray;
	void ForceRefresh(IDetailLayoutBuilder* DetailBuilder);
};
