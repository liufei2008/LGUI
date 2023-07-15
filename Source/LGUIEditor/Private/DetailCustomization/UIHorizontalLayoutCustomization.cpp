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
	category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIHorizontalLayout, ExpandChildrenWidth));
	auto ExpandChildrenWidthHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIHorizontalLayout, ExpandChildrenWidth));
	ExpandChildrenWidthHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&DetailBuilder] { DetailBuilder.ForceRefreshDetails(); }));
	bool ExpandChildrenWidth;
	ExpandChildrenWidthHandle->GetValue(ExpandChildrenWidth);
	if (ExpandChildrenWidth == false)
	{
		LGUIEditorUtils::CreateSubDetail(&category, &DetailBuilder, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIHorizontalLayout, WidthFitToChildren)));
	}
	else
	{
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUIHorizontalLayout, WidthFitToChildren));
	}
	category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIHorizontalLayout, ExpandChildrenHeight));
	auto ExpandChildrenHeightHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIHorizontalLayout, ExpandChildrenHeight));
	ExpandChildrenHeightHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&DetailBuilder] {DetailBuilder.ForceRefreshDetails(); }));
	bool ExpandChildrenHeight;
	ExpandChildrenHeightHandle->GetValue(ExpandChildrenHeight);
	if (ExpandChildrenHeight == false)
	{
		LGUIEditorUtils::CreateSubDetail(&category, &DetailBuilder, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIHorizontalLayout, HeightFitToChildren)));
		LGUIEditorUtils::CreateSubDetail(&category, &DetailBuilder, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIHorizontalLayout, HeightFitToChildrenFromMinToMax)));
	}
	else
	{
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUIHorizontalLayout, HeightFitToChildren));
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUIHorizontalLayout, HeightFitToChildrenFromMinToMax));
	}

	auto animationTypeHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIHorizontalLayout, AnimationType));
	animationTypeHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&DetailBuilder] { DetailBuilder.ForceRefreshDetails(); }));
	switch (TargetScriptPtr->AnimationType)
	{
	case EUILayoutChangePositionAnimationType::Immediately:
	{
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUIHorizontalLayout, AnimationDuration));
	}
	break;
	case EUILayoutChangePositionAnimationType::EaseAnimation:
	{

	}
	break;
	}
}
#undef LOCTEXT_NAMESPACE