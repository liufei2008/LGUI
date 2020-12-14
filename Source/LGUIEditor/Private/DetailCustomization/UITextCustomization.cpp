// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "DetailCustomization/UITextCustomization.h"
#include "LGUIEditorPCH.h"

#define LOCTEXT_NAMESPACE "UITextComponentDetails"
FUITextCustomization::FUITextCustomization()
{
}

FUITextCustomization::~FUITextCustomization()
{
}

TSharedRef<IDetailCustomization> FUITextCustomization::MakeInstance()
{
	return MakeShareable(new FUITextCustomization);
}
void FUITextCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptPtr = Cast<UUIText>(targetObjects[0].Get());
	if (TargetScriptPtr == nullptr)
	{
		UE_LOG(LGUIEditor, Log, TEXT("[UITextCustomization]Get TargetScript is null"));
		return;
	}
	
	IDetailCategoryBuilder& category = DetailBuilder.EditCategory("LGUI");
	category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIText, font));
	auto textHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIText, text));
	DetailBuilder.HideProperty(textHandle);
	category.AddCustomRow(LOCTEXT("Text", "Text"))
		.NameContent()
		[
			textHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		.MinDesiredWidth(500)
		[
			SNew(SMultiLineEditableTextBox)
			.Text(this, &FUITextCustomization::GetText)
			.OnTextChanged(this, &FUITextCustomization::OnTextChanged)
		];

	TArray<FName> needToHidePropertyName;
	auto overflowTypeHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIText, overflowType));
	overflowTypeHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FUITextCustomization::ForceRefresh, &DetailBuilder));
	uint8 overflowType;
	overflowTypeHandle->GetValue(overflowType);
	if (overflowType == (uint8)(UITextOverflowType::HorizontalOverflow))
	{
		needToHidePropertyName.Add(GET_MEMBER_NAME_CHECKED(UUIText, adjustHeight));
	}
	else if (overflowType == (uint8)(UITextOverflowType::VerticalOverflow))
	{
		needToHidePropertyName.Add(GET_MEMBER_NAME_CHECKED(UUIText, adjustWidth));
	}
	else
	{
		needToHidePropertyName.Add(GET_MEMBER_NAME_CHECKED(UUIText, adjustHeight));
		needToHidePropertyName.Add(GET_MEMBER_NAME_CHECKED(UUIText, adjustWidth));
	}

	for (auto item : needToHidePropertyName)
	{
		DetailBuilder.HideProperty(item);
	}

	auto fontHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIText, font));
	fontHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([=] {
		TargetScriptPtr->OnPostChangeFontProperty();
	}));
	fontHandle->SetOnPropertyValuePreChange(FSimpleDelegate::CreateLambda([=]{
		TargetScriptPtr->OnPreChangeFontProperty();
	}));
}
void FUITextCustomization::ForceRefresh(IDetailLayoutBuilder* DetailBuilder)
{
	if (auto Script = TargetScriptPtr.Get())
	{
		DetailBuilder->ForceRefreshDetails();
	}
}
FText FUITextCustomization::GetText()const
{
	return FText::FromString(TargetScriptPtr->text);
}
void FUITextCustomization::OnTextChanged(const FText& InText)
{
	TargetScriptPtr->SetText(InText.ToString());
	TargetScriptPtr->EditorForceUpdateImmediately();
}
#undef LOCTEXT_NAMESPACE