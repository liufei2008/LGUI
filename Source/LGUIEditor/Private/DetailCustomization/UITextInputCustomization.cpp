// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "DetailCustomization/UITextInputCustomization.h"
#include "Interaction/UITextInput.h"

#include "LGUIEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"

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
	TargetScriptPtr = Cast<UUITextInput>(targetObjects[0].Get());
	if (TargetScriptPtr == nullptr)
	{
		UE_LOG(LGUIEditor, Log, TEXT("[%s].%d Get TargetScript is null"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		return;
	}

	IDetailCategoryBuilder& category = DetailBuilder.EditCategory("LexUI-Input");

	auto InputTypeHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUITextInput, InputType));
	InputTypeHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&DetailBuilder] {DetailBuilder.ForceRefreshDetails(); }));
	EUITextInputType InputType;
	InputTypeHandle->GetValue(*(uint8*)&InputType);
	if (InputType != EUITextInputType::Custom)
	{
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUITextInput, CustomValidation));
	}
	auto DisplayTypeHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUITextInput, DisplayType));
	DisplayTypeHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&DetailBuilder] {DetailBuilder.ForceRefreshDetails(); }));
	EUITextInputDisplayType DisplayType;
	DisplayTypeHandle->GetValue(*(uint8*)&DisplayType);
	switch (DisplayType)
	{
	case EUITextInputDisplayType::Standard:
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUITextInput, PasswordChar));
		break;
	case EUITextInputDisplayType::Password:
		break;
	}

	auto AllowMultilineHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUITextInput, bAllowMultiLine));
	AllowMultilineHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&DetailBuilder] {DetailBuilder.ForceRefreshDetails(); }));
	bool bAllowMultiLine;
	AllowMultilineHandle->GetValue(bAllowMultiLine);
	if (!bAllowMultiLine)
	{
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUITextInput, MultiLineSubmitFunctionKeys));
	}
}
void FUITextInputCustomization::ForceRefresh(IDetailLayoutBuilder* DetailBuilder)
{
	if (TargetScriptPtr.IsValid())
	{
		DetailBuilder->ForceRefreshDetails();
	}
}
#undef LOCTEXT_NAMESPACE