// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "DetailCustomization/UITextInputCustomization.h"
#include "LGUIEditorPCH.h"

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
	ELGUITextInputType inputType = TargetScriptPtr->InputType;

	TArray<FName> needToHidePropertyName;
	switch (inputType)
	{
	case ELGUITextInputType::Password:
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
	category.AddProperty(GET_MEMBER_NAME_CHECKED(UUITextInputComponent, Text));
}
void FUITextInputCustomization::ForceRefresh(IDetailLayoutBuilder* DetailBuilder)
{
	if (TargetScriptPtr.IsValid())
	{
		DetailBuilder->ForceRefreshDetails();
	}
}
#undef LOCTEXT_NAMESPACE