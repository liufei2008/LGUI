// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "DetailCustomization/UIVerticalLayoutCustomization.h"
#include "LGUIEditorUtils.h"
#include "Layout/UIVerticalLayout.h"

#include "LGUIEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"

#define LOCTEXT_NAMESPACE "UIVerticalLayoutCustomization"

TSharedRef<IDetailCustomization> FUIVerticalLayoutCustomization::MakeInstance()
{
	return MakeShareable(new FUIVerticalLayoutCustomization);
}
void FUIVerticalLayoutCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptPtr = Cast<UUIVerticalLayout>(targetObjects[0].Get());
	if (TargetScriptPtr != nullptr)
	{

	}
	else
	{
		UE_LOG(LGUIEditor, Log, TEXT("Get TargetScript is null"));
	}
	//TargetScriptPtr->OnRebuildLayout();
	IDetailCategoryBuilder& category = DetailBuilder.EditCategory("LGUI");
	category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIVerticalLayout, Padding));
	category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIVerticalLayout, Spacing));
	category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIVerticalLayout, Align));
	category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIVerticalLayout, ExpendChildrenWidth));
	category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIVerticalLayout, ExpendChildrenHeight));
	auto expendHeightHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIVerticalLayout, ExpendChildrenHeight));
	expendHeightHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&DetailBuilder] { DetailBuilder.ForceRefreshDetails(); }));
	bool expendHeight;
	expendHeightHandle->GetValue(expendHeight);
	if (expendHeight == false)
	{
		LGUIEditorUtils::CreateSubDetail(&category, &DetailBuilder, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIVerticalLayout, HeightFitToChildren)));
	}
	else
	{
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUIVerticalLayout, HeightFitToChildren));
	}

	auto animationTypeHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIVerticalLayout, AnimationType));
	animationTypeHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&DetailBuilder] { DetailBuilder.ForceRefreshDetails(); }));
	switch (TargetScriptPtr->AnimationType)
	{
	case EUILayoutChangePositionAnimationType::Immediately:
	{
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUIVerticalLayout, AnimationDuration));
	}
	break;
	case EUILayoutChangePositionAnimationType::EaseAnimation:
	{

	}
	break;
	}
}
#undef LOCTEXT_NAMESPACE