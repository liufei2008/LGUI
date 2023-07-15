// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "DetailCustomization/LGUICanvasCustomClip_RoundedRect_Customization.h"
#include "LGUIEditorUtils.h"
#include "Core/LGUICanvasCustomClip.h"
#include "Utils/LGUIUtils.h"
#include "LGUIEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "IDetailGroup.h"

#define LOCTEXT_NAMESPACE "LGUICanvasCustomClip_RoundedRect_Customization"
FLGUICanvasCustomClip_RoundedRect_Customization::FLGUICanvasCustomClip_RoundedRect_Customization()
{
}

FLGUICanvasCustomClip_RoundedRect_Customization::~FLGUICanvasCustomClip_RoundedRect_Customization()
{
	
}

TSharedRef<IDetailCustomization> FLGUICanvasCustomClip_RoundedRect_Customization::MakeInstance()
{
	return MakeShareable(new FLGUICanvasCustomClip_RoundedRect_Customization);
}
void FLGUICanvasCustomClip_RoundedRect_Customization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptArray.Empty();
	for (auto item : targetObjects)
	{
		if (auto validItem = Cast<ULGUICanvasCustomClip_RoundedRect>(item.Get()))
		{
			TargetScriptArray.Add(TWeakObjectPtr<ULGUICanvasCustomClip_RoundedRect>(validItem));
		}
	}
	if (TargetScriptArray.Num() == 0)
	{
		UE_LOG(LGUIEditor, Log, TEXT("[LGUICanvasCustomClip_RoundedRect_Customization]Get TargetScript is null"));
		return;
	}

	auto CreateUnitSelector = [=](TSharedRef<IPropertyHandle> PropertyHandle) {
		const FMargin OuterPadding(2, 0);
		const FMargin ContentPadding(2);
		return
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(EVerticalAlignment::VAlign_Center)
		.Padding(OuterPadding)
		[
			SNew( SCheckBox )
			.Style(FAppStyle::Get(), "ToggleButtonCheckbox")
			.ToolTipText(LOCTEXT("Value_Tooltip", "Use direct value"))
			.Padding(ContentPadding)
			.OnCheckStateChanged_Lambda([=](ECheckBoxState InCheckboxState){
				PropertyHandle->SetValue((uint8)ELGUICanvasCustomClip_RoundedRect_UnitMode::Value);
				})
			.IsChecked_Lambda([=] {
				uint8 Value;
				PropertyHandle->GetValue(Value);
				return Value == (uint8)ELGUICanvasCustomClip_RoundedRect_UnitMode::Value ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
				})
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Value", "V"))
				.Font(IDetailLayoutBuilder::GetDetailFont())
			]
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(EVerticalAlignment::VAlign_Center)
		.Padding(OuterPadding)
		[
			SNew(SCheckBox)
			.Style(FAppStyle::Get(), "ToggleButtonCheckbox")
			.ToolTipText(LOCTEXT("Percentage_Tooltip", "Use percentage of rect width and height"))
			.Padding(ContentPadding)
			.OnCheckStateChanged_Lambda([=](ECheckBoxState InCheckboxState) {
			PropertyHandle->SetValue((uint8)ELGUICanvasCustomClip_RoundedRect_UnitMode::Percentage);
				})
			.IsChecked_Lambda([=] {
				uint8 Value;
				PropertyHandle->GetValue(Value);
				return Value == (uint8)ELGUICanvasCustomClip_RoundedRect_UnitMode::Percentage ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
				})
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Percentage", "%"))
				.Font(IDetailLayoutBuilder::GetDetailFont())
			]
		]
		;
	};

	IDetailCategoryBuilder& LGUICategory = DetailBuilder.EditCategory("LGUI");
	
	DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(ULGUICanvasCustomClip_RoundedRect, bUniformSetCornerRadius));

	auto UniformSetCornerRadiusHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUICanvasCustomClip_RoundedRect, bUniformSetCornerRadius));
	auto CornerRadiusUnitModeHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUICanvasCustomClip_RoundedRect, CornerRadiusUnitMode));
	auto CornerRadiusHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUICanvasCustomClip_RoundedRect, CornerRadius));
	auto CornerRadiusXHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUICanvasCustomClip_RoundedRect, CornerRadius.X));
	auto CornerRadiusYHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUICanvasCustomClip_RoundedRect, CornerRadius.Y));
	auto CornerRadiusZHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUICanvasCustomClip_RoundedRect, CornerRadius.Z));
	auto CornerRadiusWHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUICanvasCustomClip_RoundedRect, CornerRadius.W));
	auto CornerRadiusPropertyIsEnabledFunction = [=] {
		bool bUniformSetCornerRadius = false;
		UniformSetCornerRadiusHandle->GetValue(bUniformSetCornerRadius);
		return !bUniformSetCornerRadius;
	};

	CornerRadiusXHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([=] {
		bool bUniformSetCornerRadius = false;
		UniformSetCornerRadiusHandle->GetValue(bUniformSetCornerRadius);
		if (bUniformSetCornerRadius)
		{
			float CornerRadiusX;
			CornerRadiusXHandle->GetValue(CornerRadiusX);
			CornerRadiusYHandle->SetValue(CornerRadiusX);
			CornerRadiusZHandle->SetValue(CornerRadiusX);
			CornerRadiusWHandle->SetValue(CornerRadiusX);
		}
		}));
	LGUICategory.AddCustomRow(LOCTEXT("CornerRadius", "CornerRadius"), false)
	.PropertyHandleList({ CornerRadiusHandle })
	.NameContent()
	[
		SNew(SBox)
		.MinDesiredWidth(1000)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(CornerRadiusHandle->GetPropertyDisplayName())
				.Font(IDetailLayoutBuilder::GetDetailFont())
			]
			+SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(FMargin(4.0f, 0.0f, 0.0f, 0.0f))
			[
				SNew(SCheckBox)
				.IsChecked_Lambda([=] {
					bool bUniformSetCornerRadius = false;
					UniformSetCornerRadiusHandle->GetValue(bUniformSetCornerRadius);
					return bUniformSetCornerRadius ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
					})
				.OnCheckStateChanged_Lambda([=](ECheckBoxState NewState){
					bool bUniformSetCornerRadius = (NewState == ECheckBoxState::Checked) ? true : false;
					UniformSetCornerRadiusHandle->SetValue(bUniformSetCornerRadius);
					})
				.Style(FAppStyle::Get(), "TransparentCheckBox")
				.ToolTipText(LOCTEXT("UniformSetCornerRadiusToolTip", "When locked, corner radius will all set with x value"))
				[
					SNew(SImage)
					.Image_Lambda([=] {
						bool bUniformSetCornerRadius = false;
						UniformSetCornerRadiusHandle->GetValue(bUniformSetCornerRadius);
						return bUniformSetCornerRadius ? FAppStyle::GetBrush(TEXT("Icons.Lock")) : FAppStyle::GetBrush(TEXT("Icons.Unlock"));
						})
					.ColorAndOpacity(FSlateColor::UseForeground())
				]
			]
			+SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Right)
			.Padding(FMargin(4.0f, 0.0f, 0.0f, 0.0f))
			[
				CreateUnitSelector(CornerRadiusUnitModeHandle)
			]
		]
	]
	.ValueContent()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
		.AutoWidth()
		[
			CornerRadiusXHandle->CreatePropertyValueWidget()
		]
		+ SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
		.AutoWidth()
		[
			SNew(SBox)
			.IsEnabled_Lambda(CornerRadiusPropertyIsEnabledFunction)
			[
				CornerRadiusYHandle->CreatePropertyValueWidget()
			]
		]
		+ SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
		.AutoWidth()
		[
			SNew(SBox)
			.IsEnabled_Lambda(CornerRadiusPropertyIsEnabledFunction)
			[
				CornerRadiusZHandle->CreatePropertyValueWidget()
			]
		]
		+ SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
		.AutoWidth()
		[
			SNew(SBox)
			.IsEnabled_Lambda(CornerRadiusPropertyIsEnabledFunction)
			[
				CornerRadiusWHandle->CreatePropertyValueWidget()
			]
		]
	]
	;
}
void FLGUICanvasCustomClip_RoundedRect_Customization::ForceRefresh(IDetailLayoutBuilder* DetailBuilder)
{
	if (DetailBuilder)
	{
		DetailBuilder->ForceRefreshDetails();
	}
}
#undef LOCTEXT_NAMESPACE