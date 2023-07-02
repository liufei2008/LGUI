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

	IDetailCategoryBuilder& LGUICategory = DetailBuilder.EditCategory("LGUI");
	auto BlockDataHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData));
	BlockDataHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FUIProceduralRectCustomization::ForceRefresh, &DetailBuilder));
	
	DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData));
	DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, bUniformSetCornerRadius));
	auto ColorHandle = DetailBuilder.GetProperty(UUIProceduralRect::GetColorPropertyName(), UUIBaseRenderable::StaticClass());
	auto& BodyGroup = LGUICategory.AddGroup(TEXT("Body"), LOCTEXT("Body", "Body"), false, true);
	auto UniformSetCornerRadiusHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, bUniformSetCornerRadius));
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
	BodyGroup.AddWidgetRow()
	.PropertyHandleList({ CornerRadiusHandle })
	.NameContent()
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

	BodyGroup.AddPropertyRow(ColorHandle);
	BodyGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.bSoftEdge)));
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
	GradientGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.GradientCenter)));
	GradientGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.GradientRadius)));
	GradientGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.GradientRotation)));
	auto TextureHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.Texture));
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
						item->SetSizeFromTexture();
						LGUIUtils::NotifyPropertyChanged(item.Get(), UUIItem::GetAnchorDataPropertyName());
						item->EditorForceUpdate();
					}
				}
				GEditor->EndTransaction();
				return FReply::Handled();
			})
		]
	;
	TextureGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.TextureScaleMode)));

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
	BorderGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.BorderWidth)));
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
	BorderGradientGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.BorderGradientCenter)));
	BorderGradientGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.BorderGradientRadius)));
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
	InnerShadowGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.InnerShadowSize)));
	InnerShadowGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.InnerShadowBlur)));
	InnerShadowGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.InnerShadowOffset)));

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
	OuterShadowGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.OuterShadowSize)));
	OuterShadowGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.OuterShadowBlur)));
	OuterShadowGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.OuterShadowOffset)));

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
	RadialFillGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.RadialFillCenter)));
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