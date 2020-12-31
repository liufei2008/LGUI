// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "DetailCustomization/UITextureBaseCustomization.h"
#include "LGUIEditorUtils.h"

#define LOCTEXT_NAMESPACE "UITextureBaseCustomization"
FUITextureBaseCustomization::FUITextureBaseCustomization()
{
}

FUITextureBaseCustomization::~FUITextureBaseCustomization()
{
	
}

TSharedRef<IDetailCustomization> FUITextureBaseCustomization::MakeInstance()
{
	return MakeShareable(new FUITextureBaseCustomization);
}
void FUITextureBaseCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptArray.Empty();
	for (auto item : targetObjects)
	{
		if (auto validItem = Cast<UUITextureBase>(item.Get()))
		{
			TargetScriptArray.Add(TWeakObjectPtr<UUITextureBase>(validItem));
			if (validItem->GetWorld() && validItem->GetWorld()->WorldType == EWorldType::Editor)
			{
				validItem->CheckTexture();
				validItem->EditorForceUpdateImmediately();
			}
		}
	}
	if (TargetScriptArray.Num() == 0)
	{
		UE_LOG(LGUIEditor, Log, TEXT("[UITextureCustomization]Get TargetScript is null"));
		return;
	}

	IDetailCategoryBuilder& category = DetailBuilder.EditCategory("LGUI");
	auto textureHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUITextureBase, texture));
	textureHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FUITextureBaseCustomization::ForceRefresh, &DetailBuilder));
	category.AddProperty(GET_MEMBER_NAME_CHECKED(UUITextureBase, texture));
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
				item->SetSizeFromTexture();
				item->EditorForceUpdateImmediately();
			}
			return FReply::Handled();
		})
	];
}
void FUITextureBaseCustomization::ForceRefresh(IDetailLayoutBuilder* DetailBuilder)
{
	if (DetailBuilder)
	{
		DetailBuilder->ForceRefreshDetails();
	}
}
#undef LOCTEXT_NAMESPACE