// Copyright 2019-Present LexLiu. All Rights Reserved.

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

	category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIVerticalLayout, ExpandChildWidthArea));
	auto ExpendChildrenWidthHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIVerticalLayout, ExpandChildWidthArea));
	ExpendChildrenWidthHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&DetailBuilder] {DetailBuilder.ForceRefreshDetails(); }));
	bool ExpendChildrenWidth;
	ExpendChildrenWidthHandle->GetValue(ExpendChildrenWidth);
	if (ExpendChildrenWidth == false)
	{
		LGUIEditorUtils::CreateSubDetail(&category, &DetailBuilder, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIVerticalLayout, WidthFitToChildren)));
		LGUIEditorUtils::CreateSubDetail(&category, &DetailBuilder, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIVerticalLayout, WidthFitToChildrenFromMinToMax)));
	}
	else
	{
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUIVerticalLayout, WidthFitToChildren));
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUIVerticalLayout, WidthFitToChildrenFromMinToMax));
	}

	category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIVerticalLayout, ExpandChildHeightArea));
	auto ExpendChildrenHeightHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIVerticalLayout, ExpandChildHeightArea));
	ExpendChildrenHeightHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&DetailBuilder] { DetailBuilder.ForceRefreshDetails(); }));
	bool ExpendChildrenHeight;
	ExpendChildrenHeightHandle->GetValue(ExpendChildrenHeight);
	if (ExpendChildrenHeight == false)
	{
		LGUIEditorUtils::CreateSubDetail(&category, &DetailBuilder, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIVerticalLayout, HeightFitToChildren)));
	}
	else
	{
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUIVerticalLayout, HeightFitToChildren));
	}

	category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIVerticalLayout, ControlChildWidth));
	category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIVerticalLayout, ControlChildHeight));
	category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIVerticalLayout, UseChildScaleOnHeight));

	auto AnimationTypeHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIVerticalLayout, AnimationType), UUILayoutWithAnimation::StaticClass());
	category.AddProperty(AnimationTypeHandle);
	AnimationTypeHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&DetailBuilder] { DetailBuilder.ForceRefreshDetails(); }));
	EUILayoutAnimationType AnimationType;
	AnimationTypeHandle->GetValue(*(uint8*)&AnimationType);
	switch (AnimationType)
	{
	case EUILayoutAnimationType::Immediately:
	{
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUIVerticalLayout, AnimationDuration), UUILayoutWithAnimation::StaticClass());
	}
	break;
	case EUILayoutAnimationType::EaseAnimation:
	{
		LGUIEditorUtils::CreateSubDetail(&category, &DetailBuilder, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIVerticalLayout, AnimationDuration), UUILayoutWithAnimation::StaticClass()));
	}
	break;
	}
}
#undef LOCTEXT_NAMESPACE