// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LexUIPrefabOverrideDataViewer.h"
#include "PrefabSystem/LexUIPrefab.h"
#include "PrefabSystem/LexUIPrefabHelperObject.h"
#include "PrefabSystem/LexUIPrefabOverridePropertyPathUtils.h"
#include "LexUIPrefabEditor.h"
#include "PropertyCustomizationHelpers.h"
#include "Core/LexUIBehaviour.h"
#include "Core/LexUIManager.h"
#include "Core/Components/LexWidget.h"

#define LOCTEXT_NAMESPACE "LGUIPrefabOverrideDataViewer"

void SLexUIPrefabOverrideDataViewer::Construct(const FArguments& InArgs, TFunction<ULexWidget*()> InGetSelectedWidgetFunction)
{
	AfterRevertPrefab = InArgs._AfterRevertPrefab;
	AfterApplyPrefab = InArgs._AfterApplyPrefab;

	GetSelectedWidgetFunction = InGetSelectedWidgetFunction;
	RootContentVerticalBox = SNew(SVerticalBox);
	ChildSlot
	[
		RootContentVerticalBox.ToSharedRef()
	]
	;
	RefreshDataContent();
}

void SLexUIPrefabOverrideDataViewer::RefreshDataContent()
{
	auto SelectedWidget = GetSelectedWidgetFunction();
	if (!SelectedWidget)return;
	PrefabHelperObject = ULexUIPrefabHelperObject::GetPrefabHelperObject_WhichManageThisWidget(SelectedWidget);
	if (!PrefabHelperObject.IsValid())return;
	
	bool bIsSubPrefabRoot = false;
	for (auto& KeyValue : PrefabHelperObject->SubPrefabMap)
	{
		if (KeyValue.Key == SelectedWidget)
		{
			bIsSubPrefabRoot = true;
			break;
		}
	}
	this->RefreshDataContent(PrefabHelperObject->GetSubPrefabData(SelectedWidget).ObjectOverrideParameterArray, bIsSubPrefabRoot ? nullptr : SelectedWidget);
}

void SLexUIPrefabOverrideDataViewer::RefreshDataContent(TArray<FLexUIPrefabOverrideParameterData> ObjectOverrideParameterArray, ULexWidget* InReferenceWidget)
{
	const float ButtonHeight = 32;
	
	RootContentVerticalBox->ClearChildren();
	if (ObjectOverrideParameterArray.Num() == 0)return;

	auto RootObject = ObjectOverrideParameterArray[0].Object.Get();
	if (InReferenceWidget != nullptr)
	{
		for (int i = 0; i < ObjectOverrideParameterArray.Num(); i++)
		{
			auto& Item = ObjectOverrideParameterArray[i];
			if (!Item.Object->IsInOuter(InReferenceWidget)
				&& Item.Object != InReferenceWidget
				)
			{
				ObjectOverrideParameterArray.RemoveAt(i);
				i--;
			}
		}
	}
	if (ObjectOverrideParameterArray.Num() == 0)
	{
		RootContentVerticalBox->AddSlot()
		.AutoHeight()
		[
			SNew(SBox)
			.HeightOverride(ButtonHeight)
			.Padding(FMargin(4, 2))
			.HAlign(EHorizontalAlignment::HAlign_Center)
			.VAlign(EVerticalAlignment::VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("NoParamOverrideHint", "(No Override)"))
			]
		];
		return;
	}

	for (int i = 0; i < ObjectOverrideParameterArray.Num(); i++)
	{
		auto& DataItem = ObjectOverrideParameterArray[i];
		if (!DataItem.Object.IsValid())continue;
		FString DisplayName;
		auto Widget = Cast<ULexWidget>(DataItem.Object.Get());
		auto Component = Cast<ULexUIBehaviour>(DataItem.Object.Get());
		if (Widget)
		{
			DisplayName = Widget->GetDisplayName();
		}
		else if (Component)
		{
			Widget = Component->GetWidget();
			DisplayName = Widget->GetDisplayName() + TEXT(".") + Component->GetName();
		}
		else
		{
			DisplayName = DataItem.Object->GetName();
			for (UObject* NextOuter = DataItem.Object->GetOuter(); NextOuter != NULL; NextOuter = NextOuter->GetOuter())
			{
				if (NextOuter->IsA(ULexWidget::StaticClass()))
				{
					DisplayName = ((ULexWidget*)NextOuter)->GetDisplayName() + "." + DisplayName;
					break;
				}
				else
				{
					DisplayName = NextOuter->GetName() + "." + DisplayName;
				}
			}
		}

		auto FilteredMemberPropertyNames = DataItem.MemberPropertyNames;
		auto FilteredSubPropertyPaths = DataItem.SubPropertyPaths;

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
					.ToolTipText(LOCTEXT("ObjectButtonTooltipText", "Widget.Component, click to select target"))
					.ButtonStyle(FAppStyle::Get(), "PropertyEditor.AssetComboStyle" )
					.ForegroundColor(FAppStyle::GetColor("PropertyEditor.AssetName.ColorAndOpacity"))
					.OnClicked_Lambda([=](){
						ULexUISelection::GetInstance(Widget->GetWorld())->SelectNone();
						ULexUISelection::GetInstance(Widget->GetWorld())->SelectWidget(Widget);
						if(Component)ULexUISelection::GetInstance(Widget->GetWorld())->SelectComponent(Component);
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
						FSimpleDelegate::CreateLambda([=, this]() {
							PrefabHelperObject->RevertPrefabOverride(DataItem.Object.Get(), FilteredMemberPropertyNames, FilteredSubPropertyPaths);
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
						FSimpleDelegate::CreateLambda([=, this]() {
							PrefabHelperObject->ApplyPrefabOverride(DataItem.Object.Get(), FilteredMemberPropertyNames, FilteredSubPropertyPaths);
							AfterApplyPrefab.ExecuteIfBound(PrefabHelperObject->GetPrefabAssetBySubPrefabObject(DataItem.Object.Get()));
						})
						, LOCTEXT("ApplyObjectParameterSet", "Click to apply all parameters of this object to prefab's default value.")
					)
				]
			]
		]
		;
		//One row per override: a whole member property, or a path addressing a leaf nested inside a struct member.
		auto AddOverrideRow = [=, this](const FText& InLabel, const TArray<FName>& InNames, const TArray<FLexUIPrefabOverridePropertyPath>& InPaths, float InIndent)
		{
			auto HorizontalBox = SNew(SHorizontalBox);
			HorizontalBox->AddSlot()
			.AutoWidth()
			[
				SNew(SBox)
				.Padding(FMargin(InIndent, 2, 2, 2))
				.HAlign(EHorizontalAlignment::HAlign_Left)
				.VAlign(EVerticalAlignment::VAlign_Center)
				[
					SNew(STextBlock)
					.Text(InLabel)
					.ToolTipText(LOCTEXT("ModifiedPropertyName", "Modified property name"))
				]
			]
			;
			//apply and revert
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
						FSimpleDelegate::CreateLambda([=, this]() {
							PrefabHelperObject->RevertPrefabOverride(DataItem.Object.Get(), InNames, InPaths);
							RefreshDataContent();
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
						FSimpleDelegate::CreateLambda([=, this]() {
							PrefabHelperObject->ApplyPrefabOverride(DataItem.Object.Get(), InNames, InPaths);
							RefreshDataContent();
							AfterApplyPrefab.ExecuteIfBound(PrefabHelperObject->GetPrefabAssetBySubPrefabObject(DataItem.Object.Get()));
						})
						, LOCTEXT("ApplyThisParameter", "Click to apply this parameter to origin prefab.")
					)
				]
			]
			;
			RootContentVerticalBox->AddSlot()
			[
				HorizontalBox
			]
			;
		};
		for (auto& PropertyName : DataItem.MemberPropertyNames)
		{
			auto Property = FindFProperty<FProperty>(DataItem.Object->GetClass(), PropertyName);
			if (!Property)continue;
			AddOverrideRow(Property->GetDisplayNameText(), { PropertyName }, {}, 20);
		}
		for (auto& Path : DataItem.SubPropertyPaths)
		{
			using FPathUtils = LexUIPrefabSystem::FLexUIPrefabOverridePropertyPathUtils;
			FProperty* LeafProperty = nullptr;
			if (FPathUtils::ResolveProperty(DataItem.Object->GetClass(), Path.Segments, LeafProperty) != LexUIPrefabSystem::EPropertyPathResolveResult::Success)continue;
			AddOverrideRow(FPathUtils::GetDisplayText(DataItem.Object->GetClass(), Path.Segments), {}, { Path }, 32);
		}
	}
	//revert all, apply all
	if(InReferenceWidget == nullptr)
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
					.OnClicked_Lambda([=, this](){
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
					.ToolTipText(LOCTEXT("ApplyAll_Tooltip", "Apply all overrides to source prefab, except root widget's transform"))
					.OnClicked_Lambda([=, this](){
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
