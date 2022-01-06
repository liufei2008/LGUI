// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#pragma once

/**
 * 
 */
class FLGUISpriteDataCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
private:
	TWeakObjectPtr<class ULGUISpriteData> TargetScriptPtr;

	TSharedPtr<FSlateBrush> spriteSlateBrush;
	TSharedPtr<SBox> ImageBox;
	FOptionalSize GetMinDesiredHeight(IDetailLayoutBuilder* DetailBuilder)const;
	FOptionalSize GetImageWidth()const;
	FOptionalSize GetImageHeight()const;
	FOptionalSize GetBorderLeftSize()const;
	FOptionalSize GetBorderRightSize()const;
	FOptionalSize GetBorderTopSize()const;
	FOptionalSize GetBorderBottomSize()const;
};
