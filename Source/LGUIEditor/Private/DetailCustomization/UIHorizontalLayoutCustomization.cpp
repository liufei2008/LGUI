// Copyright 2019 LexLiu. All Rights Reserved.

#include "DetailCustomization/UIHorizontalLayoutCustomization.h"
#include "LGUIEditorUtils.h"
#include "LGUIEditorModule.h"

#define LOCTEXT_NAMESPACE "UIHorizontalLayoutCustomization"

TSharedRef<IDetailCustomization> FUIHorizontalLayoutCustomization::MakeInstance()
{
	return MakeShareable(new FUIHorizontalLayoutCustomization);
}
void FUIHorizontalLayoutCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptPtr = Cast<UUIHorizontalLayout>(targetObjects[0].Get());
	if (TargetScriptPtr != nullptr)
	{

	}
	else
	{
		UE_LOG(LGUIEditor, Log, TEXT("Get TargetScript is null"));
	}
	//TargetScriptPtr->OnRebuildLayout();
	IDetailCategoryBuilder& category = DetailBuilder.EditCategory("LGUI");
	category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIHorizontalLayout, Padding));
	category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIHorizontalLayout, Spacing));
	category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIHorizontalLayout, Align));
	category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIHorizontalLayout, ExpendChildrenWidth));
	auto expendWidthHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIHorizontalLayout, ExpendChildrenWidth));
	expendWidthHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&DetailBuilder] { DetailBuilder.ForceRefreshDetails(); }));
	bool expendWidth;
	expendWidthHandle->GetValue(expendWidth);
	if (expendWidth == false)
	{
		LGUIEditorUtils::CreateSubDetail(&category, &DetailBuilder, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIHorizontalLayout, WidthFitToChildren)));
	}
	else
	{
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUIHorizontalLayout, WidthFitToChildren));
	}
	category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIHorizontalLayout, ExpendChildrenHeight));
}
#undef LOCTEXT_NAMESPACE