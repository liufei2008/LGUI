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
	auto ColorHandle = DetailBuilder.GetProperty(UUIProceduralRect::GetColorPropertyName(), UUIBaseRenderable::StaticClass());
	//auto TextureHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.Texture));
	auto& BodyGroup = LGUICategory.AddGroup(TEXT("Body"), LOCTEXT("Body", "Body"), false, true);
	BodyGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.CornerRadius)));
	BodyGroup.AddPropertyRow(ColorHandle);
	BodyGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.bSoftEdge)));
	auto GradientHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.bEnableGradient));
	auto& GradientGroup = BodyGroup.AddGroup(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.bEnableGradient), GradientHandle->GetPropertyDisplayName(), true);
	GradientGroup.HeaderRow()
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
	BodyGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.Texture)));
	BodyGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.TextureScaleMode)));

	auto BorderHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.bEnableBorder));
	auto& BorderGroup = LGUICategory.AddGroup(GET_MEMBER_NAME_CHECKED(UUIProceduralRect, BlockData.bEnableBorder), BorderHandle->GetPropertyDisplayName(), false, true);
	BorderGroup.HeaderRow()
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