// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "DetailCustomization/UITextureBaseCustomization.h"
#include "LGUIEditorUtils.h"
#include "Core/ActorComponent/UITextureBase.h"
#include "Utils/LGUIUtils.h"
#include "LGUIEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"

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
				validItem->EditorForceUpdate();
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
	.MinDesiredWidth(160)
	[
		SNew(SButton)
		.Text(LOCTEXT("MakePixelPerfectButton", "Snap Size"))
		.OnClicked_Lambda([=]()
		{
			GEditor->BeginTransaction(LOCTEXT("TextureSnapSize", "UITexture snap size"));
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