// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "DetailCustomization/UIToggleCustomization.h"
#include "LGUIEditorUtils.h"
#include "Core/ActorComponent/UIItem.h"
#include "Core/Actor/UIBaseActor.h"
#include "IDetailGroup.h"
#include "Interaction/UIToggleGroupComponent.h"
#include "Interaction/UISelectableTransitionComponent.h"

#define LOCTEXT_NAMESPACE "UIToggleCustomization"

TSharedRef<IDetailCustomization> FUIToggleCustomization::MakeInstance()
{
	return MakeShareable(new FUIToggleCustomization);
}
void FUIToggleCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptPtr = Cast<UUIToggleComponent>(targetObjects[0].Get());
	if (TargetScriptPtr != nullptr)
	{
		
	}
	else
	{
		UE_LOG(LGUIEditor, Log, TEXT("Get TargetScript is null"));
	}

	IDetailCategoryBuilder& category = DetailBuilder.EditCategory("LGUI-Toggle");
	auto transitionHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIToggleComponent, ToggleTransition));
	transitionHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FUIToggleCustomization::ForceRefresh, &DetailBuilder));

	auto toggleActorHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIToggleComponent, ToggleActor));
	toggleActorHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FUIToggleCustomization::ForceRefresh, &DetailBuilder));

	UUIItem* targetUIItem = nullptr;
	USceneComponent* targetComp = nullptr;
	UUISelectableTransitionComponent* targetTweenComp = nullptr;
	if (auto toggleActor = TargetScriptPtr->ToggleActor)
	{
		targetUIItem = toggleActor->FindComponentByClass<UUIItem>();
		targetComp = toggleActor->FindComponentByClass<USceneComponent>();
		targetTweenComp = toggleActor->FindComponentByClass<UUISelectableTransitionComponent>();
	}

	uint8 transitionType;
	transitionHandle->GetValue(transitionType);
	TArray<FName> needToHidePropertyNameForTransition;
	IDetailGroup& transitionGroup = category.AddGroup(FName("Transition"), FText::FromString("Transition"));
	transitionGroup.HeaderProperty(transitionHandle);
	if (transitionType == (uint8)(UIToggleTransitionType::None))
	{
		needToHidePropertyNameForTransition.Add(GET_MEMBER_NAME_CHECKED(UUIToggleComponent, ToggleActor));
		needToHidePropertyNameForTransition.Add(GET_MEMBER_NAME_CHECKED(UUIToggleComponent, OffAlpha));
		needToHidePropertyNameForTransition.Add(GET_MEMBER_NAME_CHECKED(UUIToggleComponent, OnAlpha));
		needToHidePropertyNameForTransition.Add(GET_MEMBER_NAME_CHECKED(UUIToggleComponent, OffColor));
		needToHidePropertyNameForTransition.Add(GET_MEMBER_NAME_CHECKED(UUIToggleComponent, OnColor));
		needToHidePropertyNameForTransition.Add(GET_MEMBER_NAME_CHECKED(UUIToggleComponent, OnTransitionName));
		needToHidePropertyNameForTransition.Add(GET_MEMBER_NAME_CHECKED(UUIToggleComponent, OffTransitionName));
		needToHidePropertyNameForTransition.Add(GET_MEMBER_NAME_CHECKED(UUIToggleComponent, ToggleDuration));
	}
	else if (transitionType == (uint8)(UIToggleTransitionType::Fade))
	{
		transitionGroup.AddPropertyRow(toggleActorHandle);
		if (!targetUIItem)
		{
			transitionGroup.AddWidgetRow()
				.ValueContent()
				.MinDesiredWidth(500)
				[
					SNew(STextBlock)
					.AutoWrapText(true)
					.Text(LOCTEXT("TransitionActorTip", "If use Fade, Target must have UIItem component"))
					
				];
		}
		needToHidePropertyNameForTransition.Add(GET_MEMBER_NAME_CHECKED(UUIToggleComponent, OffColor));
		needToHidePropertyNameForTransition.Add(GET_MEMBER_NAME_CHECKED(UUIToggleComponent, OnColor));
		needToHidePropertyNameForTransition.Add(GET_MEMBER_NAME_CHECKED(UUIToggleComponent, OnTransitionName));
		needToHidePropertyNameForTransition.Add(GET_MEMBER_NAME_CHECKED(UUIToggleComponent, OffTransitionName));

		transitionGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIToggleComponent, OffAlpha)));
		transitionGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIToggleComponent, OnAlpha)));
		transitionGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIToggleComponent, ToggleDuration)));
	}
	else if (transitionType == (uint8)(UIToggleTransitionType::ColorTint))
	{
		transitionGroup.AddPropertyRow(toggleActorHandle);
		if (!targetUIItem)
		{
			transitionGroup.AddWidgetRow()
				.ValueContent()
				.MinDesiredWidth(500)
				[
					SNew(STextBlock)
					.AutoWrapText(true)
					.Text(LOCTEXT("TransitionActorTip", "If use ColorTint, Target must have UIItem component"))
					.ColorAndOpacity(FLinearColor(FColor::Red))
				];
		}
		needToHidePropertyNameForTransition.Add(GET_MEMBER_NAME_CHECKED(UUIToggleComponent, OffAlpha));
		needToHidePropertyNameForTransition.Add(GET_MEMBER_NAME_CHECKED(UUIToggleComponent, OnAlpha));
		needToHidePropertyNameForTransition.Add(GET_MEMBER_NAME_CHECKED(UUIToggleComponent, OnTransitionName));
		needToHidePropertyNameForTransition.Add(GET_MEMBER_NAME_CHECKED(UUIToggleComponent, OffTransitionName));

		transitionGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIToggleComponent, OffColor)));
		transitionGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIToggleComponent, OnColor)));
		transitionGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIToggleComponent, ToggleDuration)));
	}
	else if (transitionType == (uint8)(UIToggleTransitionType::TransitionComponent))
	{
		transitionGroup.AddPropertyRow(toggleActorHandle);
		if (!targetTweenComp)
		{
			transitionGroup.AddWidgetRow()
				.ValueContent()
				.MinDesiredWidth(500)
				[
					SNew(STextBlock)
					.AutoWrapText(true)
					.Text(LOCTEXT("TransitionActorTip", "If use TransitionComponent, Target must have UUISelectableTransitionComponent component"))
					.ColorAndOpacity(FLinearColor(FColor::Red))
				];
		}
		needToHidePropertyNameForTransition.Add(GET_MEMBER_NAME_CHECKED(UUIToggleComponent, OffAlpha));
		needToHidePropertyNameForTransition.Add(GET_MEMBER_NAME_CHECKED(UUIToggleComponent, OnAlpha));
		needToHidePropertyNameForTransition.Add(GET_MEMBER_NAME_CHECKED(UUIToggleComponent, OffColor));
		needToHidePropertyNameForTransition.Add(GET_MEMBER_NAME_CHECKED(UUIToggleComponent, OnColor));
	}
	for (auto item : needToHidePropertyNameForTransition)
	{
		DetailBuilder.HideProperty(item);
	}

	auto groupActorHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIToggleComponent, UIToggleGroupActor));
	groupActorHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FUIToggleCustomization::ForceRefresh, &DetailBuilder));
	if (TargetScriptPtr->UIToggleGroupActor)
	{
		auto toggleGroupComp = TargetScriptPtr->UIToggleGroupActor->FindComponentByClass<UUIToggleGroupComponent>();
		if (!toggleGroupComp)
		{
			category.AddProperty(groupActorHandle);
			category.AddCustomRow(LOCTEXT("Group", "Group"))
				.ValueContent()
				.MinDesiredWidth(500)
				[
					SNew(STextBlock)
					.AutoWrapText(true)
					.Text(LOCTEXT("GroupActorTip", "If use Group, Target must have UUIToggleGroupComponent"))
					.ColorAndOpacity(FLinearColor(FColor::Red))
				];
		}
	}
}
void FUIToggleCustomization::CreateSpriteSelector(IDetailCategoryBuilder* category, IDetailLayoutBuilder* DetailBuilder, TSharedRef<IPropertyHandle> handle)
{
	DetailBuilder->HideProperty(handle);
	category->AddCustomRow(handle->GetPropertyDisplayName())
		.NameContent()
		[
			SNew(SBox)
			.Padding(FMargin(10, 0, 0, 0))
			[
				handle->CreatePropertyNameWidget()
			]
		]
		.ValueContent()
		[
			handle->CreatePropertyValueWidget()
			/*SNew(SButton)
			.Text(FText::FromName(sprite))
			.OnClicked_Lambda([&]()
			{
				TargetScriptPtr->TransitionSpriteIndex = index;
				SLGUISpriteSelector::TargetScript = TargetScriptPtr.Get();
				SLGUISpriteSelector::TargetSprite = sprite;
				if (TargetScriptPtr->TargetSpriteComp != nullptr)
					SLGUISpriteSelector::TargetAtlas = TargetScriptPtr->TargetSpriteComp->GetAtlasData();
				FGlobalTabmanager::Get()->InvokeTab(FLGUIEditorModule::LGUISpriteSelectorTabName);
				return FReply::Handled();
			})*/
		];
}
void FUIToggleCustomization::ForceRefresh(IDetailLayoutBuilder* DetailBuilder)
{
	if (auto Script = TargetScriptPtr.Get())
	{
		DetailBuilder->ForceRefreshDetails();
	}
}
#undef LOCTEXT_NAMESPACE