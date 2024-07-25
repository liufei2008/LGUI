// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "DetailCustomization/UISelectableCustomization.h"
#include "LGUIEditorUtils.h"
#include "Core/ActorComponent/UIItem.h"
#include "Core/Actor/UIBaseActor.h"
#include "IDetailGroup.h"
#include "Interaction/UISelectableComponent.h"
#include "Interaction/UISelectableTransitionComponent.h"
#include "Core/ActorComponent/UISprite.h"
#include "LGUIEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "Core/LGUISettings.h"

#define LOCTEXT_NAMESPACE "UISelectableCustomization"

TSharedRef<IDetailCustomization> FUISelectableCustomization::MakeInstance()
{
	return MakeShareable(new FUISelectableCustomization);
}
FUISelectableCustomization::~FUISelectableCustomization()
{
	//SLGUISpriteSelector::CloseTab();
}
void FUISelectableCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptPtr = Cast<UUISelectableComponent>(targetObjects[0].Get());
	if (TargetScriptPtr != nullptr)
	{
		
	}
	else
	{
		UE_LOG(LGUIEditor, Log, TEXT("Get TargetScript is null"));
	}

	LGUIEditorUtils::ShowError_MultiComponentNotAllowed(&DetailBuilder, TargetScriptPtr.Get(), LOCTEXT("MultipleUISelectableComponentError", "Multiple UISelectable component in one actor is not allowed!"));

	IDetailCategoryBuilder& category = DetailBuilder.EditCategory("LGUI-Selectable");
	auto transitionHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, Transition));
	transitionHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FUISelectableCustomization::ForceRefresh, &DetailBuilder));

	auto transitionActorHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, TransitionActor));
	transitionActorHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FUISelectableCustomization::ForceRefresh, &DetailBuilder));

	UUIItem* targetUIItem = nullptr;
	UUISprite* targetUISprite = nullptr;
	UUISelectableTransitionComponent* targetTweenComp = nullptr;
	if (auto transitionActor = TargetScriptPtr->TransitionActor.Get())
	{
		targetUIItem = transitionActor->FindComponentByClass<UUIItem>();
		targetUISprite = transitionActor->FindComponentByClass<UUISprite>();
	}
	if (auto Actor = TargetScriptPtr->GetOwner())
	{
		targetTweenComp = TargetScriptPtr->GetOwner()->FindComponentByClass<UUISelectableTransitionComponent>();
	}

	uint8 transitionType;
	transitionHandle->GetValue(transitionType);
	TArray<FName> needToHidePropertyNameForTransition;
	IDetailGroup& transitionGroup = category.AddGroup(FName("Transition"), transitionHandle->GetPropertyDisplayName());
	transitionGroup.HeaderProperty(transitionHandle);
	if (transitionType == (uint8)(UISelectableTransitionType::None))
	{
		needToHidePropertyNameForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, TransitionActor));

		needToHidePropertyNameForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, NormalColor));
		needToHidePropertyNameForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, HighlightedColor));
		needToHidePropertyNameForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, PressedColor));
		needToHidePropertyNameForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, DisabledColor));
		needToHidePropertyNameForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, FadeDuration));

		needToHidePropertyNameForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, NormalSprite));
		needToHidePropertyNameForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, HighlightedSprite));
		needToHidePropertyNameForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, PressedSprite));
		needToHidePropertyNameForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, DisabledSprite));
	}
	else if (transitionType == (uint8)(UISelectableTransitionType::ColorTint))
	{
		transitionGroup.AddPropertyRow(transitionActorHandle);
		if (!targetUIItem)
		{
			transitionGroup.AddWidgetRow()
				.ValueContent()
				.MinDesiredWidth(500)
				[
					SNew(STextBlock)
					.AutoWrapText(true)
					.Text(LOCTEXT("TransitionActor_ColorTint_Tip", "If use ColorTint, Target must be a UIBaseRenderable Actor (UISprite, UITexture, UIText)"))
					.ColorAndOpacity(FLinearColor(FColor::Red))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				];
		}
		needToHidePropertyNameForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, NormalSprite));
		needToHidePropertyNameForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, HighlightedSprite));
		needToHidePropertyNameForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, PressedSprite));
		needToHidePropertyNameForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, DisabledSprite));

		transitionGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, NormalColor)));
		transitionGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, HighlightedColor)));
		transitionGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, PressedColor)));
		transitionGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, DisabledColor)));
		transitionGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, FadeDuration)));
	}
	else if (transitionType == (uint8)(UISelectableTransitionType::SpriteSwap))
	{
		transitionGroup.AddPropertyRow(transitionActorHandle);
		if (!targetUISprite)
		{
			transitionGroup.AddWidgetRow()
				.ValueContent()
				.MinDesiredWidth(500)
				[
					SNew(STextBlock)
					.AutoWrapText(true)
					.Text(LOCTEXT("TransitionActor_SpriteSwap_Tip", "If use SpriteSwap, Target must be a UISprite Actor"))
					.ColorAndOpacity(FLinearColor(FColor::Red))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				];
		}
		needToHidePropertyNameForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, NormalColor));
		needToHidePropertyNameForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, HighlightedColor));
		needToHidePropertyNameForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, PressedColor));
		needToHidePropertyNameForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, DisabledColor));
		needToHidePropertyNameForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, FadeDuration));

		transitionGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, NormalSprite)));
		transitionGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, HighlightedSprite)));
		transitionGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, PressedSprite)));
		transitionGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, DisabledSprite)));
	}
	else if (transitionType == (uint8)(UISelectableTransitionType::TransitionComponent))
	{
		if (!targetTweenComp)
		{
			transitionGroup.AddWidgetRow()
				.ValueContent()
				.MinDesiredWidth(500)
				[
					SNew(STextBlock)
					.AutoWrapText(true)
					.Text(LOCTEXT("TransitionActor_TransitionComponent_Tip", "If use TransitionComponent, This actor must have UISelectableTransitionComponent"))
					.ColorAndOpacity(FLinearColor(FColor::Red))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				];
		}
		needToHidePropertyNameForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, TransitionActor));

		needToHidePropertyNameForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, NormalColor));
		needToHidePropertyNameForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, HighlightedColor));
		needToHidePropertyNameForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, PressedColor));
		needToHidePropertyNameForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, DisabledColor));
		needToHidePropertyNameForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, FadeDuration));

		needToHidePropertyNameForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, NormalSprite));
		needToHidePropertyNameForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, HighlightedSprite));
		needToHidePropertyNameForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, PressedSprite));
		needToHidePropertyNameForTransition.Add(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, DisabledSprite));
	}

	IDetailCategoryBuilder& NavigationCategory = DetailBuilder.EditCategory("LGUI-Selectable-Navigation");

	auto navigationLeftHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, NavigationLeft));
	auto navigationRightHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, NavigationRight));
	auto navigationUpHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, NavigationUp));
	auto navigationDownHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, NavigationDown));
	auto navigationPrevHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, NavigationPrev));
	auto navigationNextHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, NavigationNext));
	
	uint8 tempEnumValue;
	navigationLeftHandle->GetValue(tempEnumValue);
	navigationLeftHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FUISelectableCustomization::ForceRefresh, &DetailBuilder));
	auto navigationLeftValue = (EUISelectableNavigationMode)tempEnumValue;
	navigationRightHandle->GetValue(tempEnumValue);
	navigationRightHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FUISelectableCustomization::ForceRefresh, &DetailBuilder));
	auto navigationRightValue = (EUISelectableNavigationMode)tempEnumValue;
	navigationUpHandle->GetValue(tempEnumValue);
	navigationUpHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FUISelectableCustomization::ForceRefresh, &DetailBuilder));
	auto navigationUpValue = (EUISelectableNavigationMode)tempEnumValue;
	navigationDownHandle->GetValue(tempEnumValue);
	navigationDownHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FUISelectableCustomization::ForceRefresh, &DetailBuilder));
	auto navigationDownValue = (EUISelectableNavigationMode)tempEnumValue;

	NavigationCategory.AddProperty(navigationLeftHandle);
	if (navigationLeftValue == EUISelectableNavigationMode::Explicit)
	{
		LGUIEditorUtils::CreateSubDetail(&NavigationCategory, &DetailBuilder, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, NavigationLeftSpecific)));
	}
	NavigationCategory.AddProperty(navigationRightHandle);
	if (navigationRightValue == EUISelectableNavigationMode::Explicit)
	{
		LGUIEditorUtils::CreateSubDetail(&NavigationCategory, &DetailBuilder, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, NavigationRightSpecific)));
	}
	NavigationCategory.AddProperty(navigationUpHandle);
	if (navigationUpValue == EUISelectableNavigationMode::Explicit)
	{
		LGUIEditorUtils::CreateSubDetail(&NavigationCategory, &DetailBuilder, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, NavigationUpSpecific)));
	}
	NavigationCategory.AddProperty(navigationDownHandle);
	if (navigationDownValue == EUISelectableNavigationMode::Explicit)
	{
		LGUIEditorUtils::CreateSubDetail(&NavigationCategory, &DetailBuilder, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, NavigationDownSpecific)));
	}

	navigationNextHandle->GetValue(tempEnumValue);
	navigationNextHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FUISelectableCustomization::ForceRefresh, &DetailBuilder));
	auto navigationNextValue = (EUISelectableNavigationMode)tempEnumValue;
	navigationPrevHandle->GetValue(tempEnumValue);
	navigationPrevHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FUISelectableCustomization::ForceRefresh, &DetailBuilder));
	auto navigationPrevValue = (EUISelectableNavigationMode)tempEnumValue;
	NavigationCategory.AddProperty(navigationPrevHandle);
	if (navigationPrevValue == EUISelectableNavigationMode::Explicit)
	{
		LGUIEditorUtils::CreateSubDetail(&NavigationCategory, &DetailBuilder, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, NavigationPrevSpecific)));
	}
	NavigationCategory.AddProperty(navigationNextHandle);
	if (navigationNextValue == EUISelectableNavigationMode::Explicit)
	{
		LGUIEditorUtils::CreateSubDetail(&NavigationCategory, &DetailBuilder, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, NavigationNextSpecific)));
	}
	NavigationCategory.AddProperty(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, bCanNavigateHere));
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
				return GetDefault<ULGUIEditorSettings>()->bDrawSelectableNavigationVisualizer ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
				})
			.OnCheckStateChanged_Lambda([=](ECheckBoxState State)
			{
				GEditor->BeginTransaction(LOCTEXT("ToggleNavigationVisualizer_Transaction", "Toggle Navigation Visualizer"));
				auto LGUIEditorSetting = GetMutableDefault<ULGUIEditorSettings>();
				LGUIEditorSetting->Modify();
				LGUIEditorSetting->bDrawSelectableNavigationVisualizer = State == ECheckBoxState::Checked;
				GEditor->EndTransaction();
			})
		]
	;
	
	if (navigationLeftValue != EUISelectableNavigationMode::Explicit)
	{
		needToHidePropertyNameForTransition.Add((GET_MEMBER_NAME_CHECKED(UUISelectableComponent, NavigationLeftSpecific)));
	}
	if (navigationRightValue != EUISelectableNavigationMode::Explicit)
	{
		needToHidePropertyNameForTransition.Add((GET_MEMBER_NAME_CHECKED(UUISelectableComponent, NavigationRightSpecific)));
	}
	if (navigationUpValue != EUISelectableNavigationMode::Explicit)
	{
		needToHidePropertyNameForTransition.Add((GET_MEMBER_NAME_CHECKED(UUISelectableComponent, NavigationUpSpecific)));
	}
	if (navigationDownValue != EUISelectableNavigationMode::Explicit)
	{
		needToHidePropertyNameForTransition.Add((GET_MEMBER_NAME_CHECKED(UUISelectableComponent, NavigationDownSpecific)));
	}
	if (navigationNextValue != EUISelectableNavigationMode::Explicit)
	{
		needToHidePropertyNameForTransition.Add((GET_MEMBER_NAME_CHECKED(UUISelectableComponent, NavigationNextSpecific)));
	}
	if (navigationPrevValue != EUISelectableNavigationMode::Explicit)
	{
		needToHidePropertyNameForTransition.Add((GET_MEMBER_NAME_CHECKED(UUISelectableComponent, NavigationPrevSpecific)));
	}
	
	for (auto item : needToHidePropertyNameForTransition)
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