// Copyright 2019-2022 LexLiu. All Rights Reserved.
#include "CoreMinimal.h"
#include "DetailCategoryBuilder.h"
#include "IDetailCustomization.h"
#pragma once

/**
 * 
 */
class FUIToggleCustomization : public IDetailCustomization
{
public:

	static TSharedRef<IDetailCustomization> MakeInstance();
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
private:
	TWeakObjectPtr<class UUIToggleComponent> TargetScriptPtr;
	void ForceRefresh(IDetailLayoutBuilder* DetailBuilder);
	void CreateSpriteSelector(IDetailCategoryBuilder* category, IDetailLayoutBuilder* DetailBuilder, TSharedRef<IPropertyHandle> handle);
};
