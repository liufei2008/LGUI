// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "DetailCustomization/UIToggleCustomization.h"
#include "LexUIEditorUtils.h"
#include "Core/Components/LexWidget.h"
#include "IDetailGroup.h"
#include "Interaction/UIToggleGroup.h"
#include "Interaction/UIToggle.h"
#include "LGUIEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "Core/Components/LexImage.h"

#define LOCTEXT_NAMESPACE "UIToggleCustomization"

TSharedRef<IDetailCustomization> FUIToggleCustomization::MakeInstance()
{
	return MakeShareable(new FUIToggleCustomization);
}
void FUIToggleCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptPtr = Cast<UUIToggle>(targetObjects[0].Get());
	if (TargetScriptPtr != nullptr)
	{
		
	}
	else
	{
		UE_LOG(LGUIEditor, Log, TEXT("[%s].%d Get TargetScript is null"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
	}

	IDetailCategoryBuilder& category = DetailBuilder.EditCategory("LexUI-Toggle");
	auto ToggleTransition_PH = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIToggle, ToggleTransitionType));
	ToggleTransition_PH->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FUIToggleCustomization::ForceRefresh, &DetailBuilder));

	auto ToggleTransitionTarget_PH = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIToggle, ToggleTransitionTarget));
	ToggleTransitionTarget_PH->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FUIToggleCustomization::ForceRefresh, &DetailBuilder));
	ULexVisual* ToggleTransitionTarget_Visual = nullptr;
	ToggleTransitionTarget_PH->GetValue(*(UObject**)&ToggleTransitionTarget_Visual);

	UUISelectableTransition* CustomTransition = nullptr;
	auto CustomTransition_PH = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIToggle, CustomToggleTransition));
	CustomTransition_PH->GetValue(*(UObject**)&CustomTransition);

	uint8 TransitionType;
	ToggleTransition_PH->GetValue(TransitionType);
	TArray<FName> NeedToHidePropertyNamesForTransition;
	IDetailGroup& TransitionGroup = category.AddGroup(FName("Transition"), LOCTEXT("Transition", "Transition"));
	TransitionGroup.HeaderProperty(ToggleTransition_PH);
	if (TransitionType == (uint8)(EUISelectableTransitionType::None))
	{
		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUIToggle, ToggleTransitionTarget));
		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUIToggle, OffImageBrush));
		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUIToggle, OnImageBrush));
		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUIToggle, OffColor));
		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUIToggle, OnColor));
		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUIToggle, ToggleDuration));
		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUIToggle, CustomToggleTransition));
	}
	else if (TransitionType == (uint8)(EUISelectableTransitionType::ImageBrush))
	{
		TransitionGroup.AddPropertyRow(ToggleTransitionTarget_PH);
		if (ToggleTransitionTarget_Visual && !ToggleTransitionTarget_Visual->IsA<ULexImage>())
		{
			TransitionGroup.AddWidgetRow()
				.ValueContent()
				.MinDesiredWidth(500)
				[
					SNew(STextBlock)
					.AutoWrapText(true)
					.Text(LOCTEXT("TransitionTarget_ImageBrush_Tip", "If use ImageBrush, Target must be LexImage"))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				];
		}
		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUIToggle, OffColor));
		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUIToggle, OnColor));
		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUIToggle, CustomToggleTransition));

		TransitionGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIToggle, OffImageBrush)));
		TransitionGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIToggle, OnImageBrush)));
		TransitionGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIToggle, ToggleDuration)));
	}
	else if (TransitionType == (uint8)(EUISelectableTransitionType::Color))
	{
		TransitionGroup.AddPropertyRow(ToggleTransitionTarget_PH);
		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUIToggle, OffImageBrush));
		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUIToggle, OnImageBrush));
		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUIToggle, CustomToggleTransition));

		TransitionGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIToggle, OffColor)));
		TransitionGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIToggle, OnColor)));
		TransitionGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIToggle, ToggleDuration)));
	}
	else if (TransitionType == (uint8)(EUISelectableTransitionType::Custom))
	{
		TransitionGroup.AddPropertyRow(CustomTransition_PH);

		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUIToggle, ToggleTransitionTarget));
		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUIToggle, OffImageBrush));
		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUIToggle, OnImageBrush));
		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUIToggle, OffColor));
		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUIToggle, OnColor));
	}
	for (auto item : NeedToHidePropertyNamesForTransition)
	{
		DetailBuilder.HideProperty(item);
	}
}
void FUIToggleCustomization::ForceRefresh(IDetailLayoutBuilder* DetailBuilder)
{
	if (TargetScriptPtr.IsValid())
	{
		DetailBuilder->ForceRefreshDetails();
	}
}
#undef LOCTEXT_NAMESPACE