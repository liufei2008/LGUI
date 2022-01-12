// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "DetailCustomization/UITextureCustomization.h"
#include "LGUIEditorUtils.h"
#include "Core/ActorComponent/UITexture.h"

#include "LGUIEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"

#define LOCTEXT_NAMESPACE "UITextureCustomization"
FUITextureCustomization::FUITextureCustomization()
{
}

FUITextureCustomization::~FUITextureCustomization()
{
	
}

TSharedRef<IDetailCustomization> FUITextureCustomization::MakeInstance()
{
	return MakeShareable(new FUITextureCustomization);
}
void FUITextureCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptPtr = Cast<UUITexture>(targetObjects[0].Get());
	if (TargetScriptPtr != nullptr)
	{
		
	}
	else
	{
		UE_LOG(LGUIEditor, Log, TEXT("Get TargetScript is null"));
	}

	IDetailCategoryBuilder& category = DetailBuilder.EditCategory("LGUI");

	auto spriteTypeHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUITexture, type));
	category.AddProperty(spriteTypeHandle);
	spriteTypeHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FUITextureCustomization::ForceRefresh, &DetailBuilder));
	UITextureType spriteType = TargetScriptPtr->type;
	if (spriteType == UITextureType::Filled)
	{
		auto fillMethodProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUITexture, fillMethod));
		fillMethodProperty->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FUITextureCustomization::ForceRefresh, &DetailBuilder));
		LGUIEditorUtils::CreateSubDetail(&category, &DetailBuilder, fillMethodProperty);
		UISpriteFillMethod fillMethod = TargetScriptPtr->fillMethod;
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUITexture, fillOrigin));
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUITexture, fillOriginType_Radial90));
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUITexture, fillOriginType_Radial180));
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUITexture, fillOriginType_Radial360));
		switch (fillMethod)
		{
		case UISpriteFillMethod::Horizontal:
		case UISpriteFillMethod::Vertical:
			break;
		case UISpriteFillMethod::Radial90:
		{
			TargetScriptPtr->fillOriginType_Radial90 = (UISpriteFillOriginType_Radial90)TargetScriptPtr->fillOrigin;
			auto originTypeRadialProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUITexture, fillOriginType_Radial90));
			originTypeRadialProperty->SetPropertyDisplayName(LOCTEXT("FillOrigin", "    Fill Origin"));
			category.AddProperty(originTypeRadialProperty);
		}
			break;
		case UISpriteFillMethod::Radial180:
		{
			TargetScriptPtr->fillOriginType_Radial180 = (UISpriteFillOriginType_Radial180)TargetScriptPtr->fillOrigin;
			auto originTypeRadialProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUITexture, fillOriginType_Radial180));
			originTypeRadialProperty->SetPropertyDisplayName(LOCTEXT("FillOrigin", "    Fill Origin"));
			category.AddProperty(originTypeRadialProperty);
		}
			break;
		case UISpriteFillMethod::Radial360:
		{
			TargetScriptPtr->fillOriginType_Radial360 = (UISpriteFillOriginType_Radial360)TargetScriptPtr->fillOrigin;
			auto originTypeRadialProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUITexture, fillOriginType_Radial360));
			originTypeRadialProperty->SetPropertyDisplayName(LOCTEXT("FillOrigin", "    Fill Origin"));
			category.AddProperty(originTypeRadialProperty);
		}
			break;
		}
		LGUIEditorUtils::CreateSubDetail(&category, &DetailBuilder, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUITexture, fillDirectionFlip)));
		LGUIEditorUtils::CreateSubDetail(&category, &DetailBuilder, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUITexture, fillAmount)));
	}

	if (spriteType != UITextureType::Filled)
	{
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUITexture, fillMethod));
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUITexture, fillOrigin));
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUITexture, fillDirectionFlip));
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUITexture, fillAmount));

		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUITexture, fillOriginType_Radial90));
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUITexture, fillOriginType_Radial180));
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUITexture, fillOriginType_Radial360));
	}
}
void FUITextureCustomization::ForceRefresh(IDetailLayoutBuilder* DetailBuilder)
{
	if (TargetScriptPtr.IsValid() && DetailBuilder != nullptr)
	{
		DetailBuilder->ForceRefreshDetails();
	}
}
#undef LOCTEXT_NAMESPACE