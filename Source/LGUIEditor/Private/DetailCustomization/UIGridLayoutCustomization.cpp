// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "DetailCustomization/UIGridLayoutCustomization.h"
#include "LGUIEditorUtils.h"
#include "Layout/UIGridLayout.h"

#include "LGUIEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"

#define LOCTEXT_NAMESPACE "UIGridLayoutCustomization"

TSharedRef<IDetailCustomization> FUIGridLayoutCustomization::MakeInstance()
{
	return MakeShareable(new FUIGridLayoutCustomization);
}
void FUIGridLayoutCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptPtr = Cast<UUIGridLayout>(targetObjects[0].Get());
	if (TargetScriptPtr != nullptr)
	{

	}
	else
	{
		UE_LOG(LGUIEditor, Log, TEXT("Get TargetScript is null"));
	}
	IDetailCategoryBuilder& category = DetailBuilder.EditCategory("LGUI");
	category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIGridLayout, Padding));
	category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIGridLayout, Spacing));
	category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIGridLayout, HorizontalOrVertical));
	category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIGridLayout, DependOnSizeOrCount));
	auto dependOnSizeOrCountHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIGridLayout, DependOnSizeOrCount));
	dependOnSizeOrCountHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&DetailBuilder] { DetailBuilder.ForceRefreshDetails(); }));
	bool dependOnSizeOrCount;
	dependOnSizeOrCountHandle->GetValue(dependOnSizeOrCount);
	auto expendChildSizeHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIGridLayout, ExpendChildSize));
	expendChildSizeHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&DetailBuilder] { DetailBuilder.ForceRefreshDetails(); }));
	bool expendChildSize;
	expendChildSizeHandle->GetValue(expendChildSize);
	if (dependOnSizeOrCount)
	{
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUIGridLayout, LineCount));
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUIGridLayout, ExpendChildSize));
	}
	else
	{
		if (expendChildSize)
		{
			DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUIGridLayout, WidthFitToChildren));
			DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUIGridLayout, HeightFitToChildren));
		}
	}
	auto horizontalOrVerticalHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIGridLayout, HorizontalOrVertical));
	horizontalOrVerticalHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&DetailBuilder] { DetailBuilder.ForceRefreshDetails(); }));
	bool horizontalOrVertical;
	horizontalOrVerticalHandle->GetValue(horizontalOrVertical);
	if (horizontalOrVertical)
	{
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUIGridLayout, WidthFitToChildren));
	}
	else
	{
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUIGridLayout, HeightFitToChildren));
	}

	auto animationTypeHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIGridLayout, AnimationType));
	animationTypeHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&DetailBuilder] { DetailBuilder.ForceRefreshDetails(); }));
	switch (TargetScriptPtr->AnimationType)
	{
	case EUILayoutAnimationType::Immediately:
	{
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUIGridLayout, AnimationDuration));
	}
	break;
	case EUILayoutAnimationType::EaseAnimation:
	{

	}
	break;
	}
}
#undef LOCTEXT_NAMESPACE