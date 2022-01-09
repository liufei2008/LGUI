// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "LGUIPrefabOverrideDataViewer.h"
#include "PrefabSystem/LGUIPrefab.h"
#include "LGUIPrefabEditor.h"
#include "PropertyCustomizationHelpers.h"
#include "Core/ActorComponent/UIItem.h"

#define LOCTEXT_NAMESPACE "LGUIPrefabOverrideDataViewer"

void SLGUIPrefabOverrideDataViewer::Construct(const FArguments& InArgs)
{
	RevertPrefabWithParameterSet = InArgs._RevertPrefabWithParameterSet;
	RevertPrefabWithParameter = InArgs._RevertPrefabWithParameter;
	RevertPrefabAllParameters = InArgs._RevertPrefabAllParameters;
	ApplyPrefabAllParameters = InArgs._ApplyPrefabAllParameters;

	RootContentVerticalBox = SNew(SVerticalBox);
	ChildSlot
	[
		RootContentVerticalBox.ToSharedRef()
	]
	;
}
void SLGUIPrefabOverrideDataViewer::RefreshDataContent(const TArray<FLGUIPrefabOverrideParameterData>& ObjectOverrideParameterArray)
{
	RootContentVerticalBox->ClearChildren();
	const float ButtonHeight = 32;
	for (int i = 0; i < ObjectOverrideParameterArray.Num(); i++)
	{
		auto& DataItem = ObjectOverrideParameterArray[i];
		FString DisplayName;
		AActor* Actor = Cast<AActor>(DataItem.Object.Get());
		UActorComponent* Component = Cast<UActorComponent>(DataItem.Object.Get());
		if (Actor)
		{
			DisplayName = Actor->GetActorLabel();
		}
		else if (Component)
		{
			Actor = Component->GetOwner();
			DisplayName = Actor->GetActorLabel() + TEXT(".") + Component->GetName();
		}

		TSet<FName> FilteredMemeberPropertyNameSet = DataItem.MemberPropertyName;
		if (i == 0)
		{
			if (auto UIItem = Cast<UUIItem>(DataItem.Object.Get()))
			{
				for (auto& Name : UUIItem::PersistentOverridePropertyNameSet)
				{
					FilteredMemeberPropertyNameSet.Remove(Name);
				}
			}
		}

		RootContentVerticalBox->AddSlot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SBox)
				.HeightOverride(ButtonHeight)
				.Padding(FMargin(4, 2))
				.HAlign(EHorizontalAlignment::HAlign_Left)
				.VAlign(EVerticalAlignment::VAlign_Center)
				[
					SNew(SButton)
					.Text(FText::FromString(DisplayName))
					.ToolTipText(LOCTEXT("ObjectButtonTooltipText", "Actor.Component, click to select target"))
					.ButtonStyle( FEditorStyle::Get(), "PropertyEditor.AssetComboStyle" )
					.ForegroundColor(FEditorStyle::GetColor("PropertyEditor.AssetName.ColorAndOpacity"))
					.OnClicked_Lambda([=](){
						GEditor->SelectNone(true, true);
						GEditor->SelectActor(Actor, true, true);
						if(Component)GEditor->SelectComponent(Component, true, true);
						return FReply::Handled();
					})
				]
			]
			+SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(EHorizontalAlignment::HAlign_Left)
			.VAlign(EVerticalAlignment::VAlign_Center)
			[
				SNew(SVerticalBox)
				+SVerticalBox::Slot()
				[
					PropertyCustomizationHelpers::MakeResetButton(
						FSimpleDelegate::CreateLambda([=]() {
							RevertPrefabWithParameterSet.ExecuteIfBound(DataItem.Object.Get(), FilteredMemeberPropertyNameSet);
						})
						, LOCTEXT("ResetAllParameters", "Click to revert all parameters of this object to prefab's default value.")
					)
				]
			]
		]
		;
		for (auto& PropertyName : DataItem.MemberPropertyName)
		{
			auto Property = FindFProperty<FProperty>(DataItem.Object->GetClass(), PropertyName);
			auto HorizontalBox = SNew(SHorizontalBox);
			HorizontalBox->AddSlot()
			.AutoWidth()
			[
				SNew(SBox)
				.Padding(FMargin(20, 2, 2, 2))
				.HAlign(EHorizontalAlignment::HAlign_Left)
				.VAlign(EVerticalAlignment::VAlign_Center)
				[
					SNew(STextBlock)
					.Text(Property->GetDisplayNameText())
					.ToolTipText(LOCTEXT("ModifiedPropertyName", "Modified property name"))
				]
			]
			;
			bool bCanDrawReset = true;
			if (i == 0)
			{
				if (auto UIItem = Cast<UUIItem>(DataItem.Object.Get()))
				{
					if (UUIItem::PersistentOverridePropertyNameSet.Contains(PropertyName))
					{
						bCanDrawReset = false;
					}
				}
			}
			if (bCanDrawReset)
			{
				HorizontalBox->AddSlot()
				.AutoWidth()
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.AutoHeight()
					[
						PropertyCustomizationHelpers::MakeResetButton(
							FSimpleDelegate::CreateLambda([=]() {
								RevertPrefabWithParameter.ExecuteIfBound(DataItem.Object.Get(), PropertyName);
							})
							, LOCTEXT("ResetThisParameter", "Click to revert this parameter to prefab's default value.")
						)
					]
				]
				;
			}
			RootContentVerticalBox->AddSlot()
			[
				HorizontalBox
			]
			;
		}
	}
	//revert all, apply all
	if(ObjectOverrideParameterArray.Num() > 0)
	{
		RootContentVerticalBox->AddSlot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			[
				SNew(SBox)
				.HeightOverride(ButtonHeight)
				.Padding(FMargin(4, 2))
				.HAlign(EHorizontalAlignment::HAlign_Left)
				.VAlign(EVerticalAlignment::VAlign_Center)
				[
					SNew(SButton)
					.Text(LOCTEXT("RevertAll", "Revert All"))
					.ToolTipText(LOCTEXT("RevertAll_Tooltip", "Revert all overrides"))
					.OnClicked_Lambda([=](){
						RevertPrefabAllParameters.ExecuteIfBound();
						return FReply::Handled();
					})
				]
			]
			+SHorizontalBox::Slot()
			[
				SNew(SBox)
				.HeightOverride(ButtonHeight)
				.Padding(FMargin(4, 2))
				.HAlign(EHorizontalAlignment::HAlign_Left)
				.VAlign(EVerticalAlignment::VAlign_Center)
				[
					SNew(SButton)
					.Text(LOCTEXT("ApplyAll", "Apply All"))
					.ToolTipText(LOCTEXT("ApplyAll_Tooltip", "(Future support) Apply all overrides to source prefab"))
					.IsEnabled(false)
					.OnClicked_Lambda([=](){
						ApplyPrefabAllParameters.ExecuteIfBound();
						return FReply::Handled();
					})
				]
			]
		]
		;
	}
}

#undef LOCTEXT_NAMESPACE
