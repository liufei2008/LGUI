// Copyright 2019 LexLiu. All Rights Reserved.

#include "DetailCustomization/UISelectableCustomization.h"
#include "LGUIEditorUtils.h"
#include "Core/ActorComponent/UIItem.h"
#include "Core/Actor/UIBaseActor.h"
#include "IDetailGroup.h"

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

	IDetailCategoryBuilder& category = DetailBuilder.EditCategory("LGUI-Selectable");
	auto transitionHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, Transition));
	transitionHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FUISelectableCustomization::ForceRefresh, &DetailBuilder));

	auto transitionActorHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISelectableComponent, TransitionActor));
	transitionActorHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FUISelectableCustomization::ForceRefresh, &DetailBuilder));

	UUIItem* targetUIItem = nullptr;
	UUISprite* targetUISprite = nullptr;
	UUISelectableTransitionComponent* targetTweenComp = nullptr;
	if (auto transitionActor = TargetScriptPtr->TransitionActor)
	{
		targetUIItem = transitionActor->FindComponentByClass<UUIItem>();
		targetUISprite = transitionActor->FindComponentByClass<UUISprite>();
		targetTweenComp = transitionActor->FindComponentByClass<UUISelectableTransitionComponent>();
	}

	uint8 transitionType;
	transitionHandle->GetValue(transitionType);
	TArray<FName> needToHidePropertyNameForTransition;
	IDetailGroup& transitionGroup = category.AddGroup(FName("Transition"), FText::FromString("Transition"));
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
					.Text(LOCTEXT("TransitionActorTip", "If use ColorTint, Target must have UIItem component"))
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
					.Text(LOCTEXT("TransitionActorTip", "If use SpriteSwap, Target must have UISprite component"))
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
		transitionGroup.AddPropertyRow(transitionActorHandle);
		if (!targetTweenComp)
		{
			transitionGroup.AddWidgetRow()
				.ValueContent()
				.MinDesiredWidth(500)
				[
					SNew(STextBlock)
					.AutoWrapText(true)
					.Text(LOCTEXT("TransitionActorTip", "If use TransitionComponent, Target must have UUISelectableTransitionComponent component"))
				];
		}
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
	for (auto item : needToHidePropertyNameForTransition)
	{
		DetailBuilder.HideProperty(item);
	}
}
void FUISelectableCustomization::ForceRefresh(IDetailLayoutBuilder* DetailBuilder)
{
	if (auto Script = TargetScriptPtr.Get())
	{
		DetailBuilder->ForceRefreshDetails();
	}
}
#undef LOCTEXT_NAMESPACE