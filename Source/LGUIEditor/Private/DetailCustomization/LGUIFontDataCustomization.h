// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#pragma once

/**
 * 
 */
class FLGUIFontDataCustomization : public IDetailCustomization
{
public:

	static TSharedRef<IDetailCustomization> MakeInstance();
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
private:
	TWeakObjectPtr<class ULGUIFontData> TargetScriptPtr;
	FReply OnReloadButtonClicked(IDetailLayoutBuilder* DetailBuilderPtr);
	FText OnGetFontFilePath()const;
	void OnPathTextChanged(const FString& InText, TSharedRef<IPropertyHandle> InPathProperty);
	void OnPathTextCommitted(const FString& InText, TSharedRef<IPropertyHandle> InPathProperty, IDetailLayoutBuilder* DetailBuilderPtr);
	void ForceRefresh(IDetailLayoutBuilder* DetailBuilderPtr);
	TArray<TSharedPtr<FString>> faceSelections;

	FText GetCurrentValue() const;
	void OnFontFaceComboSelectionChanged(TSharedPtr<FString> InSelectedItem, ESelectInfo::Type SelectInfo, TSharedRef<IPropertyHandle> fontFaceHandle);
	void OnFontFaceComboMenuOpening();

	void OnPackingTagTextCommited(const FText& InText, ETextCommit::Type CommitType, TSharedRef<IPropertyHandle> InProperty, IDetailLayoutBuilder* DetailBuilder);
	FText GetPackingTagText(TSharedRef<IPropertyHandle> InProperty)const;
	TSharedRef<ITableRow> GenerateComboItem(TSharedPtr<FName> InItem, const TSharedRef<STableViewBase>& OwnerTable);
	void HandleRequiredParamComboChanged(TSharedPtr<FName> Item, ESelectInfo::Type SelectInfo, TSharedRef<IPropertyHandle> InProperty, IDetailLayoutBuilder* DetailBuilder);
	TArray<TSharedPtr<FName>> NameList;
	void RefreshNameList(IDetailLayoutBuilder* DetailBuilder);
};
