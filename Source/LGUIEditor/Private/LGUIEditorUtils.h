// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once
#include "IDetailCustomization.h"
#include "LGUIEditorModule.h"
#include "IDetailPropertyRow.h"

DECLARE_DELEGATE_RetVal(FText, FLGUIArrowButtonDelegate);
class LGUIEditorUtils
{
public:
	static IDetailPropertyRow& CreateSubDetail(IDetailCategoryBuilder* category, IDetailLayoutBuilder* DetailBuilder, TSharedRef<IPropertyHandle> handle)
	{
		DetailBuilder->HideProperty(handle);
		IDetailPropertyRow& prop = category->AddProperty(handle);
		auto name = handle->GetPropertyDisplayName().ToString();
		name = "    " + name;
		prop.DisplayName(FText::FromString(name));
		return prop;
	}
	static void SetControlledByParentLayout(IDetailPropertyRow& prop, bool controlledByParentLayout)
	{
		prop.IsEnabled(!controlledByParentLayout);
		auto disabledByParentLayoutToolTip = FString(TEXT("This property is controlled by parent layout"));
		if (controlledByParentLayout) prop.ToolTip(FText::FromString(disabledByParentLayoutToolTip));
	}
	static void SetControlledBySelfLayout(IDetailPropertyRow& prop, bool controlledByThisLayout)
	{
		prop.IsEnabled(!controlledByThisLayout);
		auto disabledByParentLayoutToolTip = FString(TEXT("This property is controlled by self layout"));
		if (controlledByThisLayout) prop.ToolTip(FText::FromString(disabledByParentLayoutToolTip));
	}
	static TSharedRef<SWidget> GenerateArrowButtonContent(FText textContent)
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			[
				SNew(STextBlock)
				.Text(textContent)
			]
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.AutoWidth()
			.Padding(2, 4)
			[
				SNew(SImage)
				.Image(FEditorStyle::GetBrush("ComboButton.Arrow"))
				.ColorAndOpacity(FSlateColor::UseForeground())
			];
	}
	static TSharedRef<SWidget> GenerateArrowButtonContent(const FLGUIArrowButtonDelegate& getTextFunction)
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			[
				SNew(STextBlock)
				.Text_Lambda([=] {
					if(getTextFunction.IsBound())
						return getTextFunction.Execute();
					return FText();
				})
			]
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.AutoWidth()
			.Padding(2, 4)
			[
				SNew(SImage)
				.Image(FEditorStyle::GetBrush("ComboButton.Arrow"))
				.ColorAndOpacity(FSlateColor::UseForeground())
			];
	}
};