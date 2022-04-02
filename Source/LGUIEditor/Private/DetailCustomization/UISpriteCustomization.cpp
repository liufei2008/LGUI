﻿// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "DetailCustomization/UISpriteCustomization.h"
#include "LGUIEditorUtils.h"
#include "Core/ActorComponent/UISprite.h"
#include "Core/LGUISpriteData_BaseObject.h"
#include "LGUIEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"

#define LOCTEXT_NAMESPACE "UISpriteCustomization"
FUISpriteCustomization::FUISpriteCustomization()
{
}

FUISpriteCustomization::~FUISpriteCustomization()
{
	
}

TSharedRef<IDetailCustomization> FUISpriteCustomization::MakeInstance()
{
	return MakeShareable(new FUISpriteCustomization);
}
void FUISpriteCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptPtr = Cast<UUISprite>(targetObjects[0].Get());
	if (TargetScriptPtr != nullptr)
	{
		
	}
	else
	{
		UE_LOG(LGUIEditor, Log, TEXT("Get TargetScript is null"));
	}

	IDetailCategoryBuilder& category = DetailBuilder.EditCategory("LGUI");

	auto spriteTypeHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISprite, type));
	category.AddProperty(spriteTypeHandle);
	spriteTypeHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FUISpriteCustomization::ForceRefresh, &DetailBuilder));
	UISpriteType spriteType = TargetScriptPtr->type;
	if (spriteType == UISpriteType::Sliced || spriteType == UISpriteType::SlicedFrame)
	{
		if (TargetScriptPtr->sprite != nullptr)
		{
			if (TargetScriptPtr->sprite->GetSpriteInfo().HasBorder() == false)
			{
				category.AddCustomRow(LOCTEXT("NoBorderWarning", "NoBorderWarning"))
					.WholeRowContent()
					.MinDesiredWidth(300)
					[
						SNew(STextBlock)
						.AutoWrapText(true)
						.Text(LOCTEXT("Warning", "Target sprite does not have any border information!"))
						.ColorAndOpacity(FSlateColor(FLinearColor::Red))
						.Font(IDetailLayoutBuilder::GetDetailFont())
					];
			}
		}
	}
	else if (spriteType == UISpriteType::Filled)
	{
		auto fillMethodProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISprite, fillMethod));
		fillMethodProperty->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FUISpriteCustomization::ForceRefresh, &DetailBuilder));
		LGUIEditorUtils::CreateSubDetail(&category, &DetailBuilder, fillMethodProperty);
		UISpriteFillMethod fillMethod = TargetScriptPtr->fillMethod;
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUISprite, fillOrigin));
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUISprite, fillOriginType_Radial90));
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUISprite, fillOriginType_Radial180));
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUISprite, fillOriginType_Radial360));
		switch (fillMethod)
		{
		case UISpriteFillMethod::Horizontal:
		case UISpriteFillMethod::Vertical:
			break;
		case UISpriteFillMethod::Radial90:
		{
			TargetScriptPtr->fillOriginType_Radial90 = (UISpriteFillOriginType_Radial90)TargetScriptPtr->fillOrigin;
			auto originTypeRadialProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISprite, fillOriginType_Radial90));
			originTypeRadialProperty->SetPropertyDisplayName(LOCTEXT("FillOrigin", "    Fill Origin"));
			category.AddProperty(originTypeRadialProperty);
		}
			break;
		case UISpriteFillMethod::Radial180:
		{
			TargetScriptPtr->fillOriginType_Radial180 = (UISpriteFillOriginType_Radial180)TargetScriptPtr->fillOrigin;
			auto originTypeRadialProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISprite, fillOriginType_Radial180));
			originTypeRadialProperty->SetPropertyDisplayName(LOCTEXT("FillOrigin", "    Fill Origin"));
			category.AddProperty(originTypeRadialProperty);
		}
			break;
		case UISpriteFillMethod::Radial360:
		{
			TargetScriptPtr->fillOriginType_Radial360 = (UISpriteFillOriginType_Radial360)TargetScriptPtr->fillOrigin;
			auto originTypeRadialProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISprite, fillOriginType_Radial360));
			originTypeRadialProperty->SetPropertyDisplayName(LOCTEXT("FillOrigin", "    Fill Origin"));
			category.AddProperty(originTypeRadialProperty);
		}
			break;
		}
		LGUIEditorUtils::CreateSubDetail(&category, &DetailBuilder, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISprite, fillDirectionFlip)));
		LGUIEditorUtils::CreateSubDetail(&category, &DetailBuilder, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISprite, fillAmount)));
	}

	if (spriteType != UISpriteType::Filled)
	{
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUISprite, fillMethod));
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUISprite, fillOrigin));
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUISprite, fillDirectionFlip));
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUISprite, fillAmount));

		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUISprite, fillOriginType_Radial90));
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUISprite, fillOriginType_Radial180));
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUISprite, fillOriginType_Radial360));
	}
}
void FUISpriteCustomization::ForceRefresh(IDetailLayoutBuilder* DetailBuilder)
{
	if (TargetScriptPtr.IsValid() && DetailBuilder != nullptr)
	{
		DetailBuilder->ForceRefreshDetails();
	}
}
#undef LOCTEXT_NAMESPACE