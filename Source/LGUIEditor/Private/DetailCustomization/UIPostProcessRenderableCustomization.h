// Copyright 2019-2022 LexLiu. All Rights Reserved.
#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#pragma once

/**
 * 
 */
class FUIPostProcessRenderableCustomization : public IDetailCustomization
{
public:
	FUIPostProcessRenderableCustomization();
	~FUIPostProcessRenderableCustomization();

	static TSharedRef<IDetailCustomization> MakeInstance();
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
private:
	TArray<TWeakObjectPtr<class UUIPostProcessRenderable>> TargetScriptArray;
};
