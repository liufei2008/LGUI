// Copyright 2019-2021 LexLiu. All Rights Reserved.

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
	void OnComboSelectionChanged(TSharedPtr<FString> InSelectedItem, ESelectInfo::Type SelectInfo, TSharedRef<IPropertyHandle> fontFaceHandle);
	void OnComboMenuOpening();
};
