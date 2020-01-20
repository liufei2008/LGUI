// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "DetailCustomization/UISpriteBaseCustomization.h"
#include "LGUIEditorUtils.h"

#define LOCTEXT_NAMESPACE "UISpriteBaseCustomization"
FUISpriteBaseCustomization::FUISpriteBaseCustomization()
{
}

FUISpriteBaseCustomization::~FUISpriteBaseCustomization()
{
	
}

TSharedRef<IDetailCustomization> FUISpriteBaseCustomization::MakeInstance()
{
	return MakeShareable(new FUISpriteBaseCustomization);
}
void FUISpriteBaseCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptArray.Empty();
	for (auto item : targetObjects)
	{
		if (auto validItem = Cast<UUISpriteBase>(item.Get()))
		{
			TargetScriptArray.Add(TWeakObjectPtr<UUISpriteBase>(validItem));
			if (validItem->GetWorld() && validItem->GetWorld()->WorldType == EWorldType::Editor)
			{
				validItem->CheckSpriteData();
				validItem->EditorForceUpdateImmediately();
			}
		}
	}
	if (TargetScriptArray.Num() == 0)
	{
		UE_LOG(LGUIEditor, Log, TEXT("[UISpriteBaseCustomization]Get TargetScript is null"));
		return;
	}

	IDetailCategoryBuilder& category = DetailBuilder.EditCategory("LGUI");

	category.AddProperty(GET_MEMBER_NAME_CHECKED(UUISpriteBase, sprite));
	auto spriteHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUISpriteBase, sprite));
	spriteHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([=, &DetailBuilder] {
		for (auto item : TargetScriptArray)
		{
			item->OnPostChangeSpriteProperty();
		}
		DetailBuilder.ForceRefreshDetails();
	}));
	spriteHandle->SetOnPropertyValuePreChange(FSimpleDelegate::CreateLambda([=] {
		for (auto item : TargetScriptArray)
		{
			item->OnPreChangeSpriteProperty();
		}
	}));
	UObject* spriteObject;
	spriteHandle->GetValue(spriteObject);
	if (spriteObject != nullptr)
	{
		category.AddCustomRow(LOCTEXT("AdditionalButton", "AdditionalButton"))
		.ValueContent()
		.MinDesiredWidth(300)
		[
			SNew(SButton)
			.Text(LOCTEXT("MakePixelPerfectButton", "Snap Size"))
			.OnClicked_Lambda([=]()
			{
				for (auto item : TargetScriptArray)
				{
					item->SetSizeFromSpriteData();
					item->EditorForceUpdateImmediately();
				}
				return FReply::Handled();
			})
		];
	}
}
void FUISpriteBaseCustomization::ForceRefresh(IDetailLayoutBuilder* DetailBuilder)
{
	if (DetailBuilder)
	{
		DetailBuilder->ForceRefreshDetails();
	}
}
#undef LOCTEXT_NAMESPACE