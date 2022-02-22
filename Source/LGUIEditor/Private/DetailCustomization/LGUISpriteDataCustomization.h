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

	void OnPackingTagTextCommited(const FText& InText, ETextCommit::Type CommitType, TSharedRef<IPropertyHandle> InProperty, IDetailLayoutBuilder* DetailBuilder);
	FText GetPackingTagText(TSharedRef<IPropertyHandle> InProperty)const;
	TSharedRef<ITableRow> GenerateComboItem(TSharedPtr<FName> InItem, const TSharedRef<STableViewBase>& OwnerTable);
	void HandleRequiredParamComboChanged(TSharedPtr<FName> Item, ESelectInfo::Type SelectInfo, TSharedRef<IPropertyHandle> InProperty, IDetailLayoutBuilder* DetailBuilder);
	TArray<TSharedPtr<FName>> NameList;
	void RefreshNameList(IDetailLayoutBuilder* DetailBuilder);
};
