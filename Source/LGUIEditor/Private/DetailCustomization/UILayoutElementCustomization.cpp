// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "DetailCustomization/UILayoutElementCustomization.h"
#include "LGUIEditorUtils.h"
#include "Layout/UILayoutElement.h"

#include "LGUIEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"

#define LOCTEXT_NAMESPACE "UILayoutElementCustomization"

TSharedRef<IDetailCustomization> FUILayoutElementCustomization::MakeInstance()
{
	return MakeShareable(new FUILayoutElementCustomization);
}
void FUILayoutElementCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptPtr = Cast<UUILayoutElement>(targetObjects[0].Get());
	if (TargetScriptPtr != nullptr)
	{

	}
	else
	{
		UE_LOG(LGUIEditor, Log, TEXT("Get TargetScript is null"));
	}
	//TargetScriptPtr->OnRebuildLayout();
	IDetailCategoryBuilder& category = DetailBuilder.EditCategory("LGUI");
	category.AddProperty(GET_MEMBER_NAME_CHECKED(UUILayoutElement, LayoutElementType));
	auto elementTypeHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUILayoutElement, LayoutElementType));
	elementTypeHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&DetailBuilder] { DetailBuilder.ForceRefreshDetails(); }));
	uint8 elementType;
	elementTypeHandle->GetValue(elementType);
	switch ((ELayoutElementType)elementType)
	{
	case ELayoutElementType::AutoSize:
	case ELayoutElementType::IgnoreLayout:
	{
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUILayoutElement, ConstantSize));
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUILayoutElement, RatioSize));
	}
	break;
	case ELayoutElementType::ConstantSize:
	{
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUILayoutElement, RatioSize));
		category.AddProperty(GET_MEMBER_NAME_CHECKED(UUILayoutElement, ConstantSize));
	}
	break;
	case ELayoutElementType::RatioSize:
	{
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUILayoutElement, ConstantSize));
		category.AddProperty(GET_MEMBER_NAME_CHECKED(UUILayoutElement, RatioSize));
	}
	break;
	}
}
#undef LOCTEXT_NAMESPACE