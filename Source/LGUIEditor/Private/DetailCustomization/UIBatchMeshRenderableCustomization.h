// Copyright 2019-Present LexLiu. All Rights Reserved.
#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#pragma once

/**
 * 
 */
class FUIBatchMeshRenderableCustomization : public IDetailCustomization
{
public:
	FUIBatchMeshRenderableCustomization();
	~FUIBatchMeshRenderableCustomization();

	static TSharedRef<IDetailCustomization> MakeInstance();
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
private:
	TWeakObjectPtr<class UUIBatchMeshRenderable> TargetScriptPtr;
};
