// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "DetailCustomization/LexTextureCustomization.h"
#include "LexUIEditorUtils.h"
#include "Core/Components/LexTexture.h"

#include "LGUIEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"

#define LOCTEXT_NAMESPACE "UITextureCustomization"
FLexTextureCustomization::FLexTextureCustomization()
{
}

FLexTextureCustomization::~FLexTextureCustomization()
{
	
}

TSharedRef<IDetailCustomization> FLexTextureCustomization::MakeInstance()
{
	return MakeShareable(new FLexTextureCustomization);
}
void FLexTextureCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptPtr = Cast<ULexTexture>(targetObjects[0].Get());
	if (TargetScriptPtr != nullptr)
	{
		
	}
	else
	{
		UE_LOG(LGUIEditor, Log, TEXT("[%s].%d Get TargetScript is null"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
	}

	IDetailCategoryBuilder& category = DetailBuilder.EditCategory("LGUI");

	auto spriteTypeHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexTexture, DrawType));
	category.AddProperty(spriteTypeHandle);
	spriteTypeHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLexTextureCustomization::ForceRefresh, &DetailBuilder));
	auto DrawType = TargetScriptPtr->DrawType;
	if (DrawType == ELexUISpriteDrawType::Normal)
	{
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(ULexTexture, PixelsPerUnitMultiplier));
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(ULexTexture, bFillCenter));
	}
	else if (DrawType == ELexUISpriteDrawType::Sliced)
	{
		if (TargetScriptPtr->Texture != nullptr)
		{
			if (TargetScriptPtr->SpriteInfo.HasBorder())
			{
				
			}
			else
			{
				category.AddCustomRow(LOCTEXT("NoBorderWarning", "NoBorderWarning"))
					.Visibility(TAttribute<EVisibility>::CreateSPLambda(this, [this]()
					{
						return (TargetScriptPtr->Texture && TargetScriptPtr->SpriteInfo.HasBorder()) ? EVisibility::Collapsed : EVisibility::Visible;
					}))
					.WholeRowContent()
					.MinDesiredWidth(300)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.AutoWrapText(true)
						.Text(LOCTEXT("Warning", "Sprite info does not have any border information!"))
						.ColorAndOpacity(FSlateColor(FLinearColor::Yellow))
						.Font(IDetailLayoutBuilder::GetDetailFont())
					];
			}
		}
		FLexUIEditorUtils::CreateSubDetail(&category, &DetailBuilder, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexTexture, bFillCenter)));
	}
	else if (DrawType == ELexUISpriteDrawType::Tiled)
	{
		FLexUIEditorUtils::CreateSubDetail(&category, &DetailBuilder, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexTexture, bFillCenter)));
	}
	if (DrawType == ELexUISpriteDrawType::Filled)
	{
		auto fillMethodProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexTexture, FillMethod));
		fillMethodProperty->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLexTextureCustomization::ForceRefresh, &DetailBuilder));
		FLexUIEditorUtils::CreateSubDetail(&category, &DetailBuilder, fillMethodProperty);
		ELexUISpriteFillMethod fillMethod = TargetScriptPtr->FillMethod;
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(ULexTexture, PixelsPerUnitMultiplier));
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(ULexTexture, bFillCenter));
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(ULexTexture, FillOrigin));
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(ULexTexture, FillOriginType_Radial90));
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(ULexTexture, FillOriginType_Radial180));
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(ULexTexture, FillOriginType_Radial360));
		switch (fillMethod)
		{
		case ELexUISpriteFillMethod::Horizontal:
		case ELexUISpriteFillMethod::Vertical:
			break;
		case ELexUISpriteFillMethod::Radial90:
		{
			TargetScriptPtr->FillOriginType_Radial90 = (ELexUISpriteFillOriginType_Radial90)TargetScriptPtr->FillOrigin;
			auto originTypeRadialProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexTexture, FillOriginType_Radial90));
			originTypeRadialProperty->SetPropertyDisplayName(LOCTEXT("FillOrigin", "    Fill Origin"));
			category.AddProperty(originTypeRadialProperty);
		}
			break;
		case ELexUISpriteFillMethod::Radial180:
		{
			TargetScriptPtr->FillOriginType_Radial180 = (ELexUISpriteFillOriginType_Radial180)TargetScriptPtr->FillOrigin;
			auto originTypeRadialProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexTexture, FillOriginType_Radial180));
			originTypeRadialProperty->SetPropertyDisplayName(LOCTEXT("FillOrigin", "    Fill Origin"));
			category.AddProperty(originTypeRadialProperty);
		}
			break;
		case ELexUISpriteFillMethod::Radial360:
		{
			TargetScriptPtr->FillOriginType_Radial360 = (ELexUISpriteFillOriginType_Radial360)TargetScriptPtr->FillOrigin;
			auto originTypeRadialProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexTexture, FillOriginType_Radial360));
			originTypeRadialProperty->SetPropertyDisplayName(LOCTEXT("FillOrigin", "    Fill Origin"));
			category.AddProperty(originTypeRadialProperty);
		}
			break;
		}
		FLexUIEditorUtils::CreateSubDetail(&category, &DetailBuilder, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexTexture, FillDirectionFlip)));
		FLexUIEditorUtils::CreateSubDetail(&category, &DetailBuilder, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexTexture, FillAmount)));
	}

	if (DrawType != ELexUISpriteDrawType::Filled)
	{
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(ULexTexture, FillMethod));
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(ULexTexture, FillOrigin));
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(ULexTexture, FillDirectionFlip));
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(ULexTexture, FillAmount));

		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(ULexTexture, FillOriginType_Radial90));
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(ULexTexture, FillOriginType_Radial180));
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(ULexTexture, FillOriginType_Radial360));
	}
}
void FLexTextureCustomization::ForceRefresh(IDetailLayoutBuilder* DetailBuilder)
{
	if (TargetScriptPtr.IsValid() && DetailBuilder != nullptr)
	{
		DetailBuilder->ForceRefreshDetails();
	}
}
#undef LOCTEXT_NAMESPACE