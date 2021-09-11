// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "IDetailCustomization.h"
#include "Core/ActorComponent/UIText.h"
#pragma once

/**
 * 
 */
class FUITextCustomization : public IDetailCustomization
{
public:
	FUITextCustomization();
	~FUITextCustomization();

	static TSharedRef<IDetailCustomization> MakeInstance();
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
private:
	TWeakObjectPtr<class UUIText> TargetScriptPtr;
	void ForceRefresh(IDetailLayoutBuilder* DetailBuilder);
	void HandleHorizontalAlignmentCheckStateChanged(ECheckBoxState InCheckboxState, TSharedRef<IPropertyHandle> PropertyHandle, UITextParagraphHorizontalAlign ToAlignment);
	ECheckBoxState GetHorizontalAlignmentCheckState(TSharedRef<IPropertyHandle> PropertyHandle, UITextParagraphHorizontalAlign ForAlignment) const;
	void HandleVerticalAlignmentCheckStateChanged(ECheckBoxState InCheckboxState, TSharedRef<IPropertyHandle> PropertyHandle, UITextParagraphVerticalAlign ToAlignment);
	ECheckBoxState GetVerticalAlignmentCheckState(TSharedRef<IPropertyHandle> PropertyHandle, UITextParagraphVerticalAlign ForAlignment) const;

	void OnCopyAlignment();
	void OnPasteAlignment(TSharedRef<IPropertyHandle> HAlignPropertyHandle, TSharedRef<IPropertyHandle> VAlignPropertyHandle);
	bool OnCanPasteAlignment()const;
};
