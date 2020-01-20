// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "DetailCustomization/UISpriteCustomization.h"
#include "LGUIEditorUtils.h"

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
	uint8 spriteTypeInt;
	spriteTypeHandle->GetValue(spriteTypeInt);
	UISpriteType spriteType = (UISpriteType)spriteTypeInt;
	if (spriteType == UISpriteType::Sliced || spriteType == UISpriteType::SlicedFrame)
	{
		if (TargetScriptPtr->sprite != nullptr)
		{
			if (TargetScriptPtr->sprite->InitAndGetSpriteInfo().HasBorder() == false)
			{
				category.AddCustomRow(LOCTEXT("AdditionalButton", "AdditionalButton"))
					.WholeRowContent()
					.MinDesiredWidth(300)
					[
						SNew(STextBlock)
						.AutoWrapText(true)
						.Text(LOCTEXT("Warning", "Target sprite does not have any border information!"))
						.ColorAndOpacity(FSlateColor(FLinearColor::Red))
					];
			}
		}
	}
}
void FUISpriteCustomization::ForceRefresh(IDetailLayoutBuilder* DetailBuilder)
{
	if (TargetScriptPtr.Get() != nullptr && DetailBuilder != nullptr)
	{
		DetailBuilder->ForceRefreshDetails();
	}
}
#undef LOCTEXT_NAMESPACE