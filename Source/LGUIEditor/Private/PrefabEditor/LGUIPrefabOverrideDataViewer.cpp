// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LGUIPrefabOverrideDataViewer.h"
#include "PrefabSystem/LGUIPrefab.h"
#include "PrefabSystem/LGUIPrefabHelperObject.h"
#include "LGUIPrefabEditor.h"
#include "PropertyCustomizationHelpers.h"
#include "Core/ActorComponent/UIItem.h"

#define LOCTEXT_NAMESPACE "LGUIPrefabOverrideDataViewer"

void SLGUIPrefabOverrideDataViewer::Construct(const FArguments& InArgs, ULGUIPrefabHelperObject* InPrefabHelperObject)
{
	AfterRevertPrefab = InArgs._AfterRevertPrefab;
	AfterApplyPrefab = InArgs._AfterApplyPrefab;

	PrefabHelperObject = InPrefabHelperObject;
	RootContentVerticalBox = SNew(SVerticalBox);
	ChildSlot
	[
		RootContentVerticalBox.ToSharedRef()
	]
	;
}
void SLGUIPrefabOverrideDataViewer::SetPrefabHelperObject(ULGUIPrefabHelperObject* InPrefabHelperObject)
{
	PrefabHelperObject = InPrefabHelperObject; 
}
void SLGUIPrefabOverrideDataViewer::RefreshDataContent(TArray<FLGUIPrefabOverrideParameterData> ObjectOverrideParameterArray, AActor* InReferenceActor)
{
	RootContentVerticalBox->ClearChildren();
	if (ObjectOverrideParameterArray.Num() == 0)return;

	auto RootObject = ObjectOverrideParameterArray[0].Object.Get();
	if (InReferenceActor != nullptr)
	{
		for (int i = 0; i < ObjectOverrideParameterArray.Num(); i++)
		{
			auto& Item = ObjectOverrideParameterArray[i];
			if (!Item.Object->IsInOuter(InReferenceActor))
			{
				ObjectOverrideParameterArray.RemoveAt(i);
				i--;
			}
		}
	}

	const float ButtonHeight = 32;
	for (int i = 0; i < ObjectOverrideParameterArray.Num(); i++)
	{
		auto& DataItem = ObjectOverrideParameterArray[i];
		if (!DataItem.Object.IsValid())continue;
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
							PrefabHelperObject->RevertPrefabOverride(DataItem.Object.Get(), FilteredMemeberPropertyNameSet);
							AfterRevertPrefab.ExecuteIfBound(PrefabHelperObject->GetPrefabAssetBySubPrefabObject(DataItem.Object.Get()));
						})
						, LOCTEXT("RevertObjectAllParameterSet", "Click to revert all parameters of this object to prefab's default value.")
					)
				]
			]
			+SHorizontalBox::Slot()
			.Padding(FMargin(6, 0, 0, 0))
			.AutoWidth()
			.HAlign(EHorizontalAlignment::HAlign_Left)
			.VAlign(EVerticalAlignment::VAlign_Center)
			[
				SNew(SVerticalBox)
				+SVerticalBox::Slot()
				[
					PropertyCustomizationHelpers::MakeUseSelectedButton(
						FSimpleDelegate::CreateLambda([=]() {
							PrefabHelperObject->ApplyPrefabOverride(DataItem.Object.Get(), FilteredMemeberPropertyNameSet);
							AfterApplyPrefab.ExecuteIfBound(PrefabHelperObject->GetPrefabAssetBySubPrefabObject(DataItem.Object.Get()));
						})
						, LOCTEXT("ApplyObjectParameterSet", "Click to apply all parameters of this object to prefab's default value.")
					)
				]
			]
		]
		;
		for (auto& PropertyName : DataItem.MemberPropertyName)
		{
			auto Property = FindFProperty<FProperty>(DataItem.Object->GetClass(), PropertyName);
			if (!Property)continue;
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
				.Padding(FMargin(6, 0, 0, 0))
				.AutoWidth()
				.HAlign(EHorizontalAlignment::HAlign_Left)
				.VAlign(EVerticalAlignment::VAlign_Center)
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.AutoHeight()
					[
						PropertyCustomizationHelpers::MakeResetButton(
							FSimpleDelegate::CreateLambda([=]() {
								PrefabHelperObject->RevertPrefabOverride(DataItem.Object.Get(), PropertyName);
								AfterRevertPrefab.ExecuteIfBound(PrefabHelperObject->GetPrefabAssetBySubPrefabObject(DataItem.Object.Get()));
							})
							, LOCTEXT("ResetThisParameter", "Click to revert this parameter to prefab's default value.")
						)
					]
				]
				;
				HorizontalBox->AddSlot()
				.Padding(FMargin(6, 0, 0, 0))
				.AutoWidth()
				.HAlign(EHorizontalAlignment::HAlign_Left)
				.VAlign(EVerticalAlignment::VAlign_Center)
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.AutoHeight()
					[
						PropertyCustomizationHelpers::MakeUseSelectedButton(
							FSimpleDelegate::CreateLambda([=]() {
								PrefabHelperObject->ApplyPrefabOverride(DataItem.Object.Get(), PropertyName);
								AfterApplyPrefab.ExecuteIfBound(PrefabHelperObject->GetPrefabAssetBySubPrefabObject(DataItem.Object.Get()));
							})
							, LOCTEXT("ApplyThisParameter", "Click to apply this parameter to origin prefab.")
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
	if(InReferenceActor == nullptr)
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
						PrefabHelperObject->RevertAllPrefabOverride(RootObject);
						AfterRevertPrefab.ExecuteIfBound(PrefabHelperObject->GetPrefabAssetBySubPrefabObject(RootObject));
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
					.ToolTipText(LOCTEXT("ApplyAll_Tooltip", "Apply all overrides to source prefab"))
					.OnClicked_Lambda([=](){
						PrefabHelperObject->ApplyAllOverrideToPrefab(RootObject);
						AfterApplyPrefab.ExecuteIfBound(PrefabHelperObject->GetPrefabAssetBySubPrefabObject(RootObject));
						return FReply::Handled();
					})
				]
			]
		]
		;
	}
}

#undef LOCTEXT_NAMESPACE
