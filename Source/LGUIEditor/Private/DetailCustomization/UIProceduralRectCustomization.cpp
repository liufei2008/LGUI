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

	auto CreateVectorPropertyWithUnitMode = [&](FName PropertyName, IDetailGroup& Group, FText PropertyDisplayName) {
		auto PropertyHandle = DetailBuilder.GetProperty(PropertyName);
		PropertyHandle->SetPropertyDisplayName(PropertyDisplayName);
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

#define TO_TEXT(x) #x

#define AddPropertyRowToGroup(PropertyName, DisplayName, Group)\
auto PropertyName##Handle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, PropertyName));\
PropertyName##Handle->SetPropertyDisplayName(LOCTEXT(TO_TEXT(PropertyName##_DisplayName), TO_TEXT(DisplayName)));\
Group.AddPropertyRow(PropertyName##Handle);

#define AddVectorPropertyRowToGroup(PropertyName, DisplayName, Group)\
CreateVectorPropertyWithUnitMode(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, PropertyName), Group, LOCTEXT(TO_TEXT(PropertyName##_DisplayName), TO_TEXT(DisplayName)));

	IDetailCategoryBuilder& LGUICategory = DetailBuilder.EditCategory("LGUI");
	
	DetailBuilder.HideCategory(TEXT("LGUI-ProceduralRect"));
	DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, bUniformSetCornerRadius));

	auto UniformSetCornerRadiusHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, bUniformSetCornerRadius));
	auto CornerRadiusUnitModeHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, CornerRadiusUnitMode));
	auto CornerRadiusHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, CornerRadius));
	auto CornerRadiusXHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, CornerRadius.X));
	auto CornerRadiusYHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, CornerRadius.Y));
	auto CornerRadiusZHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, CornerRadius.Z));
	auto CornerRadiusWHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, CornerRadius.W));
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
	LGUICategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, bSoftEdge)));

	auto EnableBodyHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, bEnableBody));
	EnableBodyHandle->SetPropertyDisplayName(LOCTEXT("EnableBody_DisplayName", "Body"));
	auto& BodyGroup = LGUICategory.AddGroup(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, bEnableBody), EnableBodyHandle->GetPropertyDisplayName(), false, true);
	BodyGroup.HeaderRow()
		.PropertyHandleList({ EnableBodyHandle })
		.NameContent()
		[
			EnableBodyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			EnableBodyHandle->CreatePropertyValueWidget()
		]
	;

	AddPropertyRowToGroup(BodyColor, Color, BodyGroup);
	auto EnableBodyGradientHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, bEnableBodyGradient));
	EnableBodyGradientHandle->SetPropertyDisplayName(LOCTEXT("EnableBodyGradient_DisplayName", "Gradient"));
	auto& BodyGradientGroup = BodyGroup.AddGroup(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, bEnableBodyGradient), EnableBodyGradientHandle->GetPropertyDisplayName(), true);
	BodyGradientGroup.HeaderRow()
		.PropertyHandleList({ EnableBodyGradientHandle })
		.NameContent()
		[
			EnableBodyGradientHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			EnableBodyGradientHandle->CreatePropertyValueWidget()
		]
	;
	AddPropertyRowToGroup(BodyGradientColor, Color, BodyGradientGroup);
	AddVectorPropertyRowToGroup(BodyGradientCenter, Center, BodyGradientGroup);
	AddVectorPropertyRowToGroup(BodyGradientRadius, Radius, BodyGradientGroup);
	AddPropertyRowToGroup(BodyGradientRotation, Rotation, BodyGradientGroup);
	auto BodyTextureHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BodyTexture));
	BodyTextureHandle->SetPropertyDisplayName(LOCTEXT("BodyTexture_DisplayName", "Texture"));
	auto& TextureGroup = BodyGroup.AddGroup(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BodyTexture), BodyTextureHandle->GetPropertyDisplayName(), true);
	TextureGroup.HeaderRow()
		.PropertyHandleList({ BodyTextureHandle })
		.NameContent()
		[
			BodyTextureHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			BodyTextureHandle->CreatePropertyValueWidget()
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

	AddPropertyRowToGroup(BodyTextureScaleMode, Scale Mode, TextureGroup);

	auto BorderHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, bEnableBorder));
	BorderHandle->SetPropertyDisplayName(LOCTEXT("bEnableBorder_DisplayName", "Border"));
	auto& BorderGroup = LGUICategory.AddGroup(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, bEnableBorder), BorderHandle->GetPropertyDisplayName(), false, true);
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
	AddVectorPropertyRowToGroup(BorderWidth, Width, BorderGroup);
	AddPropertyRowToGroup(BorderColor, Color, BorderGroup);

	auto BorderGradientHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, bEnableBorderGradient));
	BorderGradientHandle->SetPropertyDisplayName(LOCTEXT("bEnableBorderGradient_DisplayName", "Gradient"));
	auto& BorderGradientGroup = BorderGroup.AddGroup(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, bEnableBorderGradient), BorderGradientHandle->GetPropertyDisplayName(), true);
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
	AddPropertyRowToGroup(BorderGradientColor, Color, BorderGradientGroup);
	AddVectorPropertyRowToGroup(BorderGradientCenter, Center, BorderGradientGroup);
	AddVectorPropertyRowToGroup(BorderGradientRadius, Radius, BorderGradientGroup);
	AddPropertyRowToGroup(BorderGradientRotation, Rotation, BorderGradientGroup);

	auto InnerShadowHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, bEnableInnerShadow));
	InnerShadowHandle->SetPropertyDisplayName(LOCTEXT("bEnableInnerShadow_DisplayName", "Inner Shadow"));
	auto& InnerShadowGroup = LGUICategory.AddGroup(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, bEnableBorder), InnerShadowHandle->GetPropertyDisplayName(), false, true);
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
	AddPropertyRowToGroup(InnerShadowColor, Color, InnerShadowGroup);
	AddVectorPropertyRowToGroup(InnerShadowSize, Size, InnerShadowGroup);
	AddVectorPropertyRowToGroup(InnerShadowBlur, Blur, InnerShadowGroup);
	AddPropertyRowToGroup(InnerShadowAngle, Angle, InnerShadowGroup);
	AddVectorPropertyRowToGroup(InnerShadowDistance, Distance, InnerShadowGroup);

	auto OuterShadowHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, bEnableOuterShadow));
	OuterShadowHandle->SetPropertyDisplayName(LOCTEXT("EnableOuterShadow_DisplayName", "Outer Shadow"));
	auto& OuterShadowGroup = LGUICategory.AddGroup(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, bEnableOuterShadow), OuterShadowHandle->GetPropertyDisplayName(), false, true);
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
	AddPropertyRowToGroup(OuterShadowColor, Color, OuterShadowGroup);
	AddVectorPropertyRowToGroup(OuterShadowSize, Size, OuterShadowGroup);
	AddVectorPropertyRowToGroup(OuterShadowBlur, Blur, OuterShadowGroup);
	AddPropertyRowToGroup(OuterShadowAngle, Angle, OuterShadowGroup);
	AddVectorPropertyRowToGroup(OuterShadowDistance, Distance, OuterShadowGroup);

	auto RadialFillHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, bEnableRadialFill));
	RadialFillHandle->SetPropertyDisplayName(LOCTEXT("EnableRadialFill_DisplayName", "Radial Fill"));
	auto& RadialFillGroup = LGUICategory.AddGroup(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, bEnableRadialFill), RadialFillHandle->GetPropertyDisplayName(), false, true);
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

	AddVectorPropertyRowToGroup(RadialFillCenter, Center, RadialFillGroup);
	AddPropertyRowToGroup(RadialFillRotation, Rotation, RadialFillGroup);
	AddPropertyRowToGroup(RadialFillAngle, Angle, RadialFillGroup);

	auto TintColorHandle = DetailBuilder.GetProperty(UUIProceduralRect::GetColorPropertyName(), UUIBaseRenderable::StaticClass());
	TintColorHandle->SetPropertyDisplayName(LOCTEXT("TintColor", "Tint Color"));
	TintColorHandle->SetToolTipText(LOCTEXT("TintColorTooltip", "Known as \"Color\" property in other UI elements. This can tint all color of this UI element. Usually only set alpha value."));
	LGUICategory.AddProperty(TintColorHandle);
}
void FUIProceduralRectCustomization::ForceRefresh(IDetailLayoutBuilder* DetailBuilder)
{
	if (DetailBuilder)
	{
		DetailBuilder->ForceRefreshDetails();
	}
}
#undef LOCTEXT_NAMESPACE