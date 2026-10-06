// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "DetailCustomization/UISelectableCustomization.h"
#include "LexUIEditorUtils.h"
#include "IDetailGroup.h"
#include "Interaction/UISelectable.h"
#include "LGUIEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "Core/LexUISettings.h"
#include "Core/Components/LexImage.h"

#define LOCTEXT_NAMESPACE "UISelectableCustomization"

TSharedRef<IDetailCustomization> FUISelectableCustomization::MakeInstance()
{
	return MakeShareable(new FUISelectableCustomization);
}
FUISelectableCustomization::~FUISelectableCustomization()
{
}
void FUISelectableCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptPtr = Cast<UUISelectable>(targetObjects[0].Get());
	if (TargetScriptPtr != nullptr)
	{
		
	}
	else
	{
		UE_LOG(LGUIEditor, Log, TEXT("[%s].%d Get TargetScript is null"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
	}

	FLexUIEditorUtils::ShowError_MultiComponentNotAllowed(&DetailBuilder, TargetScriptPtr.Get(), LOCTEXT("MultipleUISelectableComponentError", "Multiple UISelectable component in one actor is not allowed!"));

	IDetailCategoryBuilder& category = DetailBuilder.EditCategory("LexUI-Selectable");
	auto Transition_PH = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectable, TransitionType));
	Transition_PH->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FUISelectableCustomization::ForceRefresh, &DetailBuilder));

	auto TransitionTarget_PH = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectable, TransitionTarget));
	TransitionTarget_PH->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FUISelectableCustomization::ForceRefresh, &DetailBuilder));

	ULexVisual* TransitionTarget_Visual = nullptr;
	TransitionTarget_PH->GetValue(*(UObject**)&TransitionTarget_Visual);

	UUISelectableTransition* TargetTweenComp = nullptr;
	auto CustomTransition_PH = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectable, CustomTransition));
	CustomTransition_PH->GetValue(*(UObject**)&TargetTweenComp);

	uint8 TransitionType;
	Transition_PH->GetValue(TransitionType);
	TArray<FName> NeedToHidePropertyNamesForTransition;
	IDetailGroup& TransitionGroup = category.AddGroup(FName("Transition"), Transition_PH->GetPropertyDisplayName());
	TransitionGroup.HeaderProperty(Transition_PH);
	if (TransitionType == (uint8)(EUISelectableTransitionType::None))
	{
		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectable, TransitionTarget));

		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectable, NormalColor));
		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectable, HoveredColor));
		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectable, PressedColor));
		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectable, DisabledColor));
		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectable, AnimDuration));

		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectable, NormalImageBrush));
		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectable, HoveredImageBrush));
		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectable, PressedImageBrush));
		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectable, DisabledImageBrush));
		
		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectable, CustomTransition));
	}
	else if (TransitionType == (uint8)(EUISelectableTransitionType::Color))
	{
		TransitionGroup.AddPropertyRow(TransitionTarget_PH);
		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectable, NormalImageBrush));
		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectable, HoveredImageBrush));
		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectable, PressedImageBrush));
		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectable, DisabledImageBrush));

		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectable, CustomTransition));

		TransitionGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectable, NormalColor)));
		TransitionGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectable, HoveredColor)));
		TransitionGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectable, PressedColor)));
		TransitionGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectable, DisabledColor)));
		TransitionGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectable, AnimDuration)));
	}
	else if (TransitionType == (uint8)(EUISelectableTransitionType::ImageBrush))
	{
		TransitionGroup.AddPropertyRow(TransitionTarget_PH);
		if (TransitionTarget_Visual && !TransitionTarget_Visual->IsA<ULexImage>())
		{
			TransitionGroup.AddWidgetRow()
				.ValueContent()
				.MinDesiredWidth(500)
				[
					SNew(STextBlock)
					.AutoWrapText(true)
					.Text(LOCTEXT("TransitionTarget_ImageBrush_Tip", "If use ImageBrush, Target must be a LexImage"))
					.ColorAndOpacity(FLinearColor(FColor::Red))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				];
		}
		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectable, NormalColor));
		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectable, HoveredColor));
		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectable, PressedColor));
		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectable, DisabledColor));
		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectable, AnimDuration));

		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectable, CustomTransition));
		
		TransitionGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectable, NormalImageBrush)));
		TransitionGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectable, HoveredImageBrush)));
		TransitionGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectable, PressedImageBrush)));
		TransitionGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectable, DisabledImageBrush)));
	}
	else if (TransitionType == (uint8)(EUISelectableTransitionType::Custom))
	{
		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectable, TransitionTarget));

		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectable, NormalColor));
		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectable, HoveredColor));
		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectable, PressedColor));
		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectable, DisabledColor));
		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectable, AnimDuration));

		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectable, NormalImageBrush));
		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectable, HoveredImageBrush));
		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectable, PressedImageBrush));
		NeedToHidePropertyNamesForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectable, DisabledImageBrush));

		TransitionGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectable, CustomTransition)));
	}

	IDetailCategoryBuilder& NavigationCategory = DetailBuilder.EditCategory("LexUI-Selectable-Navigation");
	NavigationCategory.AddProperty(GET_MEMBER_NAME_CHECKED(UUISelectable, bCanNavigateHere));
	
	auto navigationLeftHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectable, NavigationLeft));
	auto navigationRightHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectable, NavigationRight));
	auto navigationUpHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectable, NavigationUp));
	auto navigationDownHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectable, NavigationDown));
	auto navigationPrevHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectable, NavigationPrev));
	auto navigationNextHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectable, NavigationNext));
	
	EUISelectableNavigationMode tempEnumValue;
	navigationLeftHandle->GetValue(*(uint8*)&tempEnumValue);
	navigationLeftHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FUISelectableCustomization::ForceRefresh, &DetailBuilder));
	auto navigationLeftValue = tempEnumValue;
	navigationRightHandle->GetValue(*(uint8*)&tempEnumValue);
	navigationRightHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FUISelectableCustomization::ForceRefresh, &DetailBuilder));
	auto navigationRightValue = tempEnumValue;
	navigationUpHandle->GetValue(*(uint8*)&tempEnumValue);
	navigationUpHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FUISelectableCustomization::ForceRefresh, &DetailBuilder));
	auto navigationUpValue = tempEnumValue;
	navigationDownHandle->GetValue(*(uint8*)&tempEnumValue);
	navigationDownHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FUISelectableCustomization::ForceRefresh, &DetailBuilder));
	auto navigationDownValue = tempEnumValue;

	NavigationCategory.AddProperty(navigationLeftHandle);
	if (navigationLeftValue == EUISelectableNavigationMode::Explicit)
	{
		FLexUIEditorUtils::CreateSubDetail(&NavigationCategory, &DetailBuilder, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectable, NavigationLeftSpecific)));
	}
	NavigationCategory.AddProperty(navigationRightHandle);
	if (navigationRightValue == EUISelectableNavigationMode::Explicit)
	{
		FLexUIEditorUtils::CreateSubDetail(&NavigationCategory, &DetailBuilder, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectable, NavigationRightSpecific)));
	}
	NavigationCategory.AddProperty(navigationUpHandle);
	if (navigationUpValue == EUISelectableNavigationMode::Explicit)
	{
		FLexUIEditorUtils::CreateSubDetail(&NavigationCategory, &DetailBuilder, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectable, NavigationUpSpecific)));
	}
	NavigationCategory.AddProperty(navigationDownHandle);
	if (navigationDownValue == EUISelectableNavigationMode::Explicit)
	{
		FLexUIEditorUtils::CreateSubDetail(&NavigationCategory, &DetailBuilder, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectable, NavigationDownSpecific)));
	}

	navigationNextHandle->GetValue(*(uint8*)&tempEnumValue);
	navigationNextHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FUISelectableCustomization::ForceRefresh, &DetailBuilder));
	auto navigationNextValue = (EUISelectableNavigationMode)tempEnumValue;
	navigationPrevHandle->GetValue(*(uint8*)&tempEnumValue);
	navigationPrevHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FUISelectableCustomization::ForceRefresh, &DetailBuilder));
	auto navigationPrevValue = (EUISelectableNavigationMode)tempEnumValue;
	NavigationCategory.AddProperty(navigationPrevHandle);
	if (navigationPrevValue == EUISelectableNavigationMode::Explicit)
	{
		FLexUIEditorUtils::CreateSubDetail(&NavigationCategory, &DetailBuilder, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectable, NavigationPrevSpecific)));
	}
	NavigationCategory.AddProperty(navigationNextHandle);
	if (navigationNextValue == EUISelectableNavigationMode::Explicit)
	{
		FLexUIEditorUtils::CreateSubDetail(&NavigationCategory, &DetailBuilder, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectable, NavigationNextSpecific)));
	}
	NavigationCategory.AddCustomRow(LOCTEXT("VisualizeNavigation", "VisualizeNavigation"))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("Visualize", "Visualize"))
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		.ValueContent()
		[
			SNew(SCheckBox)
			.IsChecked_Lambda([]() {
				return GetDefault<ULexUIEditorSettings>()->bDrawSelectableNavigationVisualizer ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
				})
			.OnCheckStateChanged_Lambda([=](ECheckBoxState State)
			{
				GEditor->BeginTransaction(LOCTEXT("ToggleNavigationVisualizer_Transaction", "Toggle Navigation Visualizer"));
				auto LGUIEditorSetting = GetMutableDefault<ULexUIEditorSettings>();
				LGUIEditorSetting->Modify();
				LGUIEditorSetting->bDrawSelectableNavigationVisualizer = State == ECheckBoxState::Checked;
				GEditor->EndTransaction();
			})
		]
	;
	
	if (navigationLeftValue != EUISelectableNavigationMode::Explicit)
	{
		NeedToHidePropertyNamesForTransition.Add((GET_MEMBER_NAME_CHECKED(UUISelectable, NavigationLeftSpecific)));
	}
	if (navigationRightValue != EUISelectableNavigationMode::Explicit)
	{
		NeedToHidePropertyNamesForTransition.Add((GET_MEMBER_NAME_CHECKED(UUISelectable, NavigationRightSpecific)));
	}
	if (navigationUpValue != EUISelectableNavigationMode::Explicit)
	{
		NeedToHidePropertyNamesForTransition.Add((GET_MEMBER_NAME_CHECKED(UUISelectable, NavigationUpSpecific)));
	}
	if (navigationDownValue != EUISelectableNavigationMode::Explicit)
	{
		NeedToHidePropertyNamesForTransition.Add((GET_MEMBER_NAME_CHECKED(UUISelectable, NavigationDownSpecific)));
	}
	if (navigationNextValue != EUISelectableNavigationMode::Explicit)
	{
		NeedToHidePropertyNamesForTransition.Add((GET_MEMBER_NAME_CHECKED(UUISelectable, NavigationNextSpecific)));
	}
	if (navigationPrevValue != EUISelectableNavigationMode::Explicit)
	{
		NeedToHidePropertyNamesForTransition.Add((GET_MEMBER_NAME_CHECKED(UUISelectable, NavigationPrevSpecific)));
	}
	
	for (auto item : NeedToHidePropertyNamesForTransition)
	{
		DetailBuilder.HideProperty(item);
	}
}
void FUISelectableCustomization::ForceRefresh(IDetailLayoutBuilder* DetailBuilder)
{
	if (TargetScriptPtr.IsValid())
	{
		DetailBuilder->ForceRefreshDetails();
	}
}
#undef LOCTEXT_NAMESPACE