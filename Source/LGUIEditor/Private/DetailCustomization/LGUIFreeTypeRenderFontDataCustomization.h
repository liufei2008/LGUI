﻿// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#pragma once

/**
 * 
 */
class FLGUIFreeTypeRenderFontDataCustomization : public IDetailCustomization
{
public:

	static TSharedRef<IDetailCustomization> MakeInstance();
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
private:
	TWeakObjectPtr<class ULGUIFreeTypeRenderFontData> TargetScriptPtr;
	FReply OnReloadButtonClicked(IDetailLayoutBuilder* DetailBuilderPtr);
	FText OnGetFontFilePath()const;
	void OnPathTextChanged(const FString& InText, TSharedRef<IPropertyHandle> InPathProperty);
	void OnPathTextCommitted(const FString& InText, TSharedRef<IPropertyHandle> InPathProperty, IDetailLayoutBuilder* DetailBuilderPtr);
	void ForceRefresh(IDetailLayoutBuilder* DetailBuilderPtr);
	TArray<TSharedPtr<FString>> FontFaceOptions;
	TSharedRef<ITableRow> FontFaceOptions_GenerateComboItem(TSharedPtr<FString> InItem, const TSharedRef<STableViewBase>& OwnerTable, IDetailLayoutBuilder* DetailBuilder);
	void FontFaceOptions_OnComboChanged(TSharedPtr<FString> Item, ESelectInfo::Type SelectInfo, TSharedRef<IPropertyHandle> InProperty, IDetailLayoutBuilder* DetailBuilder);
	FText FontFaceOptions_GetCurrentFace()const;

	FText GetCurrentValue() const;
	void OnFontFaceComboSelectionChanged(TSharedPtr<FString> InSelectedItem, ESelectInfo::Type SelectInfo, TSharedRef<IPropertyHandle> fontFaceHandle);
};
