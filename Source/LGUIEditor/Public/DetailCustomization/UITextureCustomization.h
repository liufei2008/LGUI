// Copyright 2019-2021 LexLiu. All Rights Reserved.
#include "IDetailCustomization.h"
#pragma once

/**
 * 
 */
class FUITextureCustomization : public IDetailCustomization
{
public:
	FUITextureCustomization();
	~FUITextureCustomization();

	static TSharedRef<IDetailCustomization> MakeInstance();
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
private:
	TWeakObjectPtr<class UUITexture> TargetScriptPtr;
	void ForceRefresh(IDetailLayoutBuilder* DetailBuilder);

};
