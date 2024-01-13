// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "DetailCustomization/UIHorizontalLayoutCustomization.h"
#include "LGUIEditorUtils.h"
#include "Layout/UIHorizontalLayout.h"

#include "LGUIEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"

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

	category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIHorizontalLayout, ExpandChildWidthArea));
	auto ExpendChildrenWidthHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIHorizontalLayout, ExpandChildWidthArea));
	ExpendChildrenWidthHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&DetailBuilder] { DetailBuilder.ForceRefreshDetails(); }));
	bool ExpendChildrenWidth;
	ExpendChildrenWidthHandle->GetValue(ExpendChildrenWidth);
	if (ExpendChildrenWidth == false)
	{
		LGUIEditorUtils::CreateSubDetail(&category, &DetailBuilder, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIHorizontalLayout, WidthFitToChildren)));
	}
	else
	{
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUIHorizontalLayout, WidthFitToChildren));
	}
	category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIHorizontalLayout, ExpandChildHeightArea));
	auto ExpendChildrenHeightHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIHorizontalLayout, ExpandChildHeightArea));
	ExpendChildrenHeightHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&DetailBuilder] {DetailBuilder.ForceRefreshDetails(); }));
	bool ExpendChildrenHeight;
	ExpendChildrenHeightHandle->GetValue(ExpendChildrenHeight);
	if (ExpendChildrenHeight == false)
	{
		LGUIEditorUtils::CreateSubDetail(&category, &DetailBuilder, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIHorizontalLayout, HeightFitToChildren)));
		LGUIEditorUtils::CreateSubDetail(&category, &DetailBuilder, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIHorizontalLayout, HeightFitToChildrenFromMinToMax)));
	}
	else
	{
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUIHorizontalLayout, HeightFitToChildren));
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUIHorizontalLayout, HeightFitToChildrenFromMinToMax));
	}

	category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIHorizontalLayout, ControlChildWidth));
	category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIHorizontalLayout, ControlChildHeight));
	category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIHorizontalLayout, UseChildScaleOnWidth));

	auto AnimationTypeHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIHorizontalLayout, AnimationType), UUILayoutWithAnimation::StaticClass());
	category.AddProperty(AnimationTypeHandle);
	AnimationTypeHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&DetailBuilder] { DetailBuilder.ForceRefreshDetails(); }));
	switch (TargetScriptPtr->AnimationType)
	{
	case EUILayoutAnimationType::Immediately:
	{
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUIHorizontalLayout, AnimationDuration), UUILayoutWithAnimation::StaticClass());
	}
	break;
	case EUILayoutAnimationType::EaseAnimation:
	{
		LGUIEditorUtils::CreateSubDetail(&category, &DetailBuilder, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIHorizontalLayout, AnimationDuration), UUILayoutWithAnimation::StaticClass()));
	}
	break;
	}
}
#undef LOCTEXT_NAMESPACE