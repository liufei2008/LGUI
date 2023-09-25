// Copyright 2019-Present LexLiu. All Rights Reserved.
#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#pragma once

/**
 * 
 */
class FUIBatchGeometryRenderableCustomization : public IDetailCustomization
{
public:
	FUIBatchGeometryRenderableCustomization();
	~FUIBatchGeometryRenderableCustomization();

	static TSharedRef<IDetailCustomization> MakeInstance();
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
private:
	TWeakObjectPtr<class UUIBatchGeometryRenderable> TargetScriptPtr;
};
