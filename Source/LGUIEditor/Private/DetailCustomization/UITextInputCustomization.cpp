// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "DetailCustomization/UITextInputCustomization.h"

#define LOCTEXT_NAMESPACE "UITextComponentDetails"
FUITextInputCustomization::FUITextInputCustomization()
{
}

FUITextInputCustomization::~FUITextInputCustomization()
{
}

TSharedRef<IDetailCustomization> FUITextInputCustomization::MakeInstance()
{
	return MakeShareable(new FUITextInputCustomization);
}
void FUITextInputCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptPtr = Cast<UUITextInputComponent>(targetObjects[0].Get());
	if (TargetScriptPtr == nullptr)
	{
		UE_LOG(LGUIEditor, Log, TEXT("[UITextInputCustomization]Get TargetScript is null"));
		return;
	}

	IDetailCategoryBuilder& category = DetailBuilder.EditCategory("LGUI-Input");
	auto inputTypeHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUITextInputComponent, InputType));
	inputTypeHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FUITextInputCustomization::ForceRefresh, &DetailBuilder));
	ELGUIInputType inputType = TargetScriptPtr->InputType;

	TArray<FName> needToHidePropertyName;
	switch (inputType)
	{
	case ELGUIInputType::Password:
	{

	}
	break;
	default:
	{
		needToHidePropertyName.Add(GET_MEMBER_NAME_CHECKED(UUITextInputComponent, PasswordChar));
	}
	break;
	}
	for (auto item : needToHidePropertyName)
	{
		DetailBuilder.HideProperty(item);
	}

	category.AddProperty(GET_MEMBER_NAME_CHECKED(UUITextInputComponent, TextActor));
	auto textHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUITextInputComponent, Text));
	DetailBuilder.HideProperty(textHandle);
	category.AddCustomRow(LOCTEXT("Text", "Text"))
		.NameContent()
		[
			textHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		.MinDesiredWidth(500)
		[
			SNew(SEditableTextBox)
			.Text(this, &FUITextInputCustomization::GetText)
			.OnTextChanged(this, &FUITextInputCustomization::OnTextChanged)
		];
}
void FUITextInputCustomization::ForceRefresh(IDetailLayoutBuilder* DetailBuilder)
{
	if (auto Script = TargetScriptPtr.Get())
	{
		DetailBuilder->ForceRefreshDetails();
	}
}
FText FUITextInputCustomization::GetText()const
{
	return FText::FromString(TargetScriptPtr->Text);
}
void FUITextInputCustomization::OnTextChanged(const FText& InText)
{
	TargetScriptPtr->SetText(InText.ToString());
}
#undef LOCTEXT_NAMESPACE