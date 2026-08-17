// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "DetailCustomization/LexSpriteCustomization.h"
#include "LexUIEditorUtils.h"
#include "Core/Components/LexSprite.h"
#include "Core/LexUISpriteData_BaseObject.h"
#include "LGUIEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"

#define LOCTEXT_NAMESPACE "UISpriteCustomization"
FLexSpriteCustomization::FLexSpriteCustomization()
{
}

FLexSpriteCustomization::~FLexSpriteCustomization()
{
	
}

TSharedRef<IDetailCustomization> FLexSpriteCustomization::MakeInstance()
{
	return MakeShareable(new FLexSpriteCustomization);
}
void FLexSpriteCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptPtr = Cast<ULexSprite>(targetObjects[0].Get());
	if (TargetScriptPtr != nullptr)
	{
		
	}
	else
	{
		UE_LOG(LGUIEditor, Log, TEXT("[%s].%d Get TargetScript is null"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
	}

	IDetailCategoryBuilder& category = DetailBuilder.EditCategory("LGUI");

	auto spriteTypeHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexSprite, DrawType));
	category.AddProperty(spriteTypeHandle);
	spriteTypeHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLexSpriteCustomization::ForceRefresh, &DetailBuilder));
	auto DrawType = TargetScriptPtr->DrawType;
	if (DrawType == ELexUISpriteDrawType::Normal)
	{
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(ULexSprite, PixelsPerUnitMultiplier));
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(ULexSprite, bFillCenter));
	}
	else if (DrawType == ELexUISpriteDrawType::Sliced)
	{
		if (TargetScriptPtr->Sprite != nullptr)
		{
			if (TargetScriptPtr->Sprite->GetSpriteInfo().HasBorder())
			{
				
			}
			else
			{
				category.AddCustomRow(LOCTEXT("NoBorderWarning", "NoBorderWarning"))
					.Visibility(TAttribute<EVisibility>::CreateSPLambda(this, [this]()
					{
						return (TargetScriptPtr->Sprite && TargetScriptPtr->Sprite->GetSpriteInfo().HasBorder()) ? EVisibility::Collapsed : EVisibility::Visible;
					}))
					.WholeRowContent()
					.MinDesiredWidth(300)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.AutoWrapText(true)
						.Text(LOCTEXT("Warning", "Target Sprite does not have any border information!"))
						.ColorAndOpacity(FSlateColor(FLinearColor::Yellow))
						.Font(IDetailLayoutBuilder::GetDetailFont())
					];
			}
		}
		FLexUIEditorUtils::CreateSubDetail(&category, &DetailBuilder, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexSprite, bFillCenter)));
	}
	else if (DrawType == ELexUISpriteDrawType::Tiled)
	{
		FLexUIEditorUtils::CreateSubDetail(&category, &DetailBuilder, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexSprite, bFillCenter)));
	}
	else if (DrawType == ELexUISpriteDrawType::Filled)
	{
		auto fillMethodProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexSprite, FillMethod));
		fillMethodProperty->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLexSpriteCustomization::ForceRefresh, &DetailBuilder));
		FLexUIEditorUtils::CreateSubDetail(&category, &DetailBuilder, fillMethodProperty);
		ELexUISpriteFillMethod fillMethod = TargetScriptPtr->FillMethod;
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(ULexSprite, PixelsPerUnitMultiplier));
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(ULexSprite, bFillCenter));
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(ULexSprite, FillOrigin));
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(ULexSprite, FillOriginType_Radial90));
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(ULexSprite, FillOriginType_Radial180));
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(ULexSprite, FillOriginType_Radial360));
		switch (fillMethod)
		{
		case ELexUISpriteFillMethod::Horizontal:
		case ELexUISpriteFillMethod::Vertical:
			break;
		case ELexUISpriteFillMethod::Radial90:
		{
			TargetScriptPtr->FillOriginType_Radial90 = (ELexUISpriteFillOriginType_Radial90)TargetScriptPtr->FillOrigin;
			auto originTypeRadialProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexSprite, FillOriginType_Radial90));
			originTypeRadialProperty->SetPropertyDisplayName(LOCTEXT("FillOrigin", "    Fill Origin"));
			category.AddProperty(originTypeRadialProperty);
		}
			break;
		case ELexUISpriteFillMethod::Radial180:
		{
			TargetScriptPtr->FillOriginType_Radial180 = (ELexUISpriteFillOriginType_Radial180)TargetScriptPtr->FillOrigin;
			auto originTypeRadialProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexSprite, FillOriginType_Radial180));
			originTypeRadialProperty->SetPropertyDisplayName(LOCTEXT("FillOrigin", "    Fill Origin"));
			category.AddProperty(originTypeRadialProperty);
		}
			break;
		case ELexUISpriteFillMethod::Radial360:
		{
			TargetScriptPtr->FillOriginType_Radial360 = (ELexUISpriteFillOriginType_Radial360)TargetScriptPtr->FillOrigin;
			auto originTypeRadialProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexSprite, FillOriginType_Radial360));
			originTypeRadialProperty->SetPropertyDisplayName(LOCTEXT("FillOrigin", "    Fill Origin"));
			category.AddProperty(originTypeRadialProperty);
		}
			break;
		}
		FLexUIEditorUtils::CreateSubDetail(&category, &DetailBuilder, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexSprite, FillDirectionFlip)));
		FLexUIEditorUtils::CreateSubDetail(&category, &DetailBuilder, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexSprite, FillAmount)));
	}

	if (DrawType != ELexUISpriteDrawType::Filled)
	{
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(ULexSprite, FillMethod));
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(ULexSprite, FillOrigin));
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(ULexSprite, FillDirectionFlip));
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(ULexSprite, FillAmount));

		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(ULexSprite, FillOriginType_Radial90));
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(ULexSprite, FillOriginType_Radial180));
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(ULexSprite, FillOriginType_Radial360));
	}
}
void FLexSpriteCustomization::ForceRefresh(IDetailLayoutBuilder* DetailBuilder)
{
	if (TargetScriptPtr.IsValid() && DetailBuilder != nullptr)
	{
		DetailBuilder->ForceRefreshDetails();
	}
}
#undef LOCTEXT_NAMESPACE