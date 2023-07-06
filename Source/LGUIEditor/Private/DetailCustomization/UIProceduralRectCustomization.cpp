// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "DetailCustomization/UIProceduralRectCustomization.h"
#include "LGUIEditorUtils.h"
#include "Core/ActorComponent/UIProceduralRect.h"
#include "Utils/LGUIUtils.h"
#include "LGUIEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "IDetailGroup.h"

#define LOCTEXT_NAMESPACE "UIProceduralRectCustomization"
FUIProceduralRectCustomization::FUIProceduralRectCustomization()
{
}

FUIProceduralRectCustomization::~FUIProceduralRectCustomization()
{
	
}

TSharedRef<IDetailCustomization> FUIProceduralRectCustomization::MakeInstance()
{
	return MakeShareable(new FUIProceduralRectCustomization);
}
void FUIProceduralRectCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptArray.Empty();
	for (auto item : targetObjects)
	{
		if (auto validItem = Cast<UUIProceduralRect>(item.Get()))
		{
			TargetScriptArray.Add(TWeakObjectPtr<UUIProceduralRect>(validItem));
			if (validItem->GetWorld() && validItem->GetWorld()->WorldType == EWorldType::Editor)
			{
				validItem->EditorForceUpdate();
			}
		}
	}
	if (TargetScriptArray.Num() == 0)
	{
		UE_LOG(LGUIEditor, Log, TEXT("[UIProceduralRectCustomization]Get TargetScript is null"));
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
				PropertyHandle->SetValue((uint8)EUIProceduralRectUnitMode::Value);
				})
			.IsChecked_Lambda([=] {
				uint8 Value;
				PropertyHandle->GetValue(Value);
				return Value == (uint8)EUIProceduralRectUnitMode::Value ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
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
			PropertyHandle->SetValue((uint8)EUIProceduralRectUnitMode::Percentage);
				})
			.IsChecked_Lambda([=] {
				uint8 Value;
				PropertyHandle->GetValue(Value);
				return Value == (uint8)EUIProceduralRectUnitMode::Percentage ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
				})
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Percentage", "%"))
				.Font(IDetailLayoutBuilder::GetDetailFont())
			]
		]
		;
	};

	auto CreateVectorPropertyWithUnitMode = [&](FName PropertyName, IDetailGroup& Group) {
		auto PropertyHandle = DetailBuilder.GetProperty(PropertyName);
		auto PropertyUnitHandle = DetailBuilder.GetProperty(FName(*(PropertyName.ToString() + TEXT("UnitMode"))));
		auto ValueHorizontalBox = SNew(SHorizontalBox);
		uint32 NumChildren = 0;
		PropertyHandle->GetNumChildren(NumChildren);
		if (NumChildren == 0)
		{
			ValueHorizontalBox->AddSlot()
			[
				PropertyHandle->CreatePropertyValueWidget()
			];
		}
		else
		{
			for (uint32 i = 0; i < NumChildren; i++)
			{
				ValueHorizontalBox->AddSlot()
					[
						PropertyHandle->GetChildHandle(i)->CreatePropertyValueWidget()
					];
			}
		}
		Group.AddWidgetRow()
		.PropertyHandleList({ PropertyHandle })
		.NameContent()
		[
			SNew(SBox)
			.MinDesiredWidth(1000)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				[
					PropertyHandle->CreatePropertyNameWidget()
				]
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				[
					CreateUnitSelector(PropertyUnitHandle)
				]
			]
		]
		.ValueContent()
		[
			ValueHorizontalBox
		]
	;
	};

	IDetailCategoryBuilder& LGUICategory = DetailBuilder.EditCategory("LGUI");
	auto BlockDataHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData));
	BlockDataHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FUIProceduralRectCustomization::ForceRefresh, &DetailBuilder));
	
	DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData));
	DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, bUniformSetCornerRadius));

	auto TintColorHandle = DetailBuilder.GetProperty(UUIProceduralRect::GetColorPropertyName(), UUIBaseRenderable::StaticClass());
	LGUICategory.AddProperty(TintColorHandle);

	auto UniformSetCornerRadiusHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, bUniformSetCornerRadius));
	auto CornerRadiusUnitModeHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.CornerRadiusUnitMode));
	auto CornerRadiusHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.CornerRadius));
	auto CornerRadiusXHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.CornerRadius.X));
	auto CornerRadiusYHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.CornerRadius.Y));
	auto CornerRadiusZHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.CornerRadius.Z));
	auto CornerRadiusWHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.CornerRadius.W));
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
	LGUICategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.bSoftEdge)));

	auto BodyHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.bEnableBody));
	auto& BodyGroup = LGUICategory.AddGroup(TEXT("Body"), LOCTEXT("Body", "Body"), false, true);
	BodyGroup.HeaderRow()
		.PropertyHandleList({ BodyHandle })
		.NameContent()
		[
			BodyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			BodyHandle->CreatePropertyValueWidget()
		]
	;

	auto BodyColorHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.BodyColor));
	BodyGroup.AddPropertyRow(BodyColorHandle);
	auto GradientHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.bEnableGradient));
	auto& GradientGroup = BodyGroup.AddGroup(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.bEnableGradient), GradientHandle->GetPropertyDisplayName(), true);
	GradientGroup.HeaderRow()
		.PropertyHandleList({ GradientHandle })
		.NameContent()
		[
			GradientHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			GradientHandle->CreatePropertyValueWidget()
		]
	;
	GradientGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.GradientColor)));
	CreateVectorPropertyWithUnitMode(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.GradientCenter), GradientGroup);
	CreateVectorPropertyWithUnitMode(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.GradientRadius), GradientGroup);
	GradientGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.GradientRotation)));
	auto TextureHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.BodyTexture));
	auto& TextureGroup = BodyGroup.AddGroup(TEXT("Texture"), LOCTEXT("Texture", "Texture"), true);
	TextureGroup.HeaderRow()
		.PropertyHandleList({ TextureHandle })
		.NameContent()
		[
			TextureHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			TextureHandle->CreatePropertyValueWidget()
		]
	;
	TextureGroup.AddWidgetRow()
		.ValueContent()
		[
			SNew(SButton)
			.HAlign(EHorizontalAlignment::HAlign_Center)
			.VAlign(EVerticalAlignment::VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("SnapSize_Button", "Snap Size"))
				.Font(IDetailLayoutBuilder::GetDetailFont())
			]
			.OnClicked_Lambda([=]()
			{
				GEditor->BeginTransaction(LOCTEXT("TextureSnapSize_Transaction", "UIProceduralRect texture snap size"));
				for (auto item : TargetScriptArray)
				{
					if (item.IsValid())
					{
						item->Modify();
						item->SetSizeFromBodyTexture();
						LGUIUtils::NotifyPropertyChanged(item.Get(), UUIItem::GetAnchorDataPropertyName());
						item->EditorForceUpdate();
					}
				}
				GEditor->EndTransaction();
				return FReply::Handled();
			})
		]
	;
	TextureGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.BodyTextureScaleMode)));

	auto BorderHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.bEnableBorder));
	auto& BorderGroup = LGUICategory.AddGroup(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.bEnableBorder), BorderHandle->GetPropertyDisplayName(), false, true);
	BorderGroup.HeaderRow()
		.PropertyHandleList({ BorderHandle })
		.NameContent()
		[
			BorderHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			BorderHandle->CreatePropertyValueWidget()
		]
	;
	CreateVectorPropertyWithUnitMode(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.BorderWidth), BorderGroup);
	BorderGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.BorderColor)));

	auto BorderGradientHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.bEnableBorderGradient));
	auto& BorderGradientGroup = BorderGroup.AddGroup(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.bEnableBorderGradient), BorderGradientHandle->GetPropertyDisplayName(), true);
	BorderGradientGroup.HeaderRow()
		.PropertyHandleList({ BorderGradientHandle })
		.NameContent()
		[
			BorderGradientHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			BorderGradientHandle->CreatePropertyValueWidget()
		]
	;
	BorderGradientGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.BorderGradientColor)));
	CreateVectorPropertyWithUnitMode(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.BorderGradientCenter), BorderGradientGroup);
	CreateVectorPropertyWithUnitMode(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.BorderGradientRadius), BorderGradientGroup);
	BorderGradientGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.BorderGradientRotation)));

	auto InnerShadowHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.bEnableInnerShadow));
	auto& InnerShadowGroup = LGUICategory.AddGroup(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.bEnableBorder), InnerShadowHandle->GetPropertyDisplayName(), false, true);
	InnerShadowGroup.HeaderRow()
		.PropertyHandleList({ InnerShadowHandle })
		.NameContent()
		[
			InnerShadowHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			InnerShadowHandle->CreatePropertyValueWidget()
		]
	;
	InnerShadowGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.InnerShadowColor)));
	CreateVectorPropertyWithUnitMode(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.InnerShadowSize), InnerShadowGroup);
	CreateVectorPropertyWithUnitMode(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.InnerShadowBlur), InnerShadowGroup);
	InnerShadowGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.InnerShadowAngle)));
	CreateVectorPropertyWithUnitMode(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.InnerShadowDistance), InnerShadowGroup);

	auto OuterShadowHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.bEnableOuterShadow));
	auto& OuterShadowGroup = LGUICategory.AddGroup(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.bEnableOuterShadow), OuterShadowHandle->GetPropertyDisplayName(), false, true);
	OuterShadowGroup.HeaderRow()
		.PropertyHandleList({ OuterShadowHandle })
		.NameContent()
		[
			OuterShadowHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			OuterShadowHandle->CreatePropertyValueWidget()
		]
	;
	OuterShadowGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.OuterShadowColor)));
	CreateVectorPropertyWithUnitMode(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.OuterShadowSize), OuterShadowGroup);
	CreateVectorPropertyWithUnitMode(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.OuterShadowBlur), OuterShadowGroup);
	OuterShadowGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.OuterShadowAngle)));
	CreateVectorPropertyWithUnitMode(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.OuterShadowDistance), OuterShadowGroup);

	auto RadialFillHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.bEnableRadialFill));
	auto& RadialFillGroup = LGUICategory.AddGroup(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.bEnableRadialFill), RadialFillHandle->GetPropertyDisplayName(), false, true);
	RadialFillGroup.HeaderRow()
		.PropertyHandleList({ RadialFillHandle })
		.NameContent()
		[
			RadialFillHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			RadialFillHandle->CreatePropertyValueWidget()
		]
	;
	CreateVectorPropertyWithUnitMode(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.RadialFillCenter), RadialFillGroup);
	RadialFillGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.RadialFillRotation)));
	RadialFillGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.RadialFillAngle)));
}
void FUIProceduralRectCustomization::ForceRefresh(IDetailLayoutBuilder* DetailBuilder)
{
	if (DetailBuilder)
	{
		DetailBuilder->ForceRefreshDetails();
	}
}
#undef LOCTEXT_NAMESPACE