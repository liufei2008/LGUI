// Copyright 2019-Present LexLiu. All Rights Reserved.

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
	UTexture* texture = nullptr;
	textureHandle->GetValue((*(UObject**)&texture));
	if(IsValid(texture))
	{
		EUIRenderableRaycastType raycastType = EUIRenderableRaycastType::Rect;
		bool bGetRaycastTypeValue = true;
		for (int i = 0; i < TargetScriptArray.Num(); i++)
		{
			if (i == 0)
			{
				raycastType = TargetScriptArray[i]->GetRaycastType();
			}
			else
			{
				if (raycastType != TargetScriptArray[i]->GetRaycastType())
				{
					bGetRaycastTypeValue = false;
					break;
				}
			}
		}
		if (bGetRaycastTypeValue)
		{
			if (raycastType == EUIRenderableRaycastType::VisiblePixel)
			{
				if (texture->CompressionSettings != TextureCompressionSettings::TC_EditorIcon)
				{
					category.AddCustomRow(LOCTEXT("FixTextureSettingForHitTest_Row", "FixTextureSettingForHitTest"))
						.ValueContent()
						[
							SNew(SButton)
							.HAlign(EHorizontalAlignment::HAlign_Center)
							.VAlign(EVerticalAlignment::VAlign_Center)
							.OnClicked_Lambda([=, &DetailBuilder] {
								texture->CompressionSettings = TextureCompressionSettings::TC_EditorIcon;
								texture->UpdateResource();
								texture->MarkPackageDirty();
								DetailBuilder.ForceRefreshDetails();
								return FReply::Handled();
								})
							.ToolTipText(LOCTEXT("FixTextureSettingForHitTest_Tooltip", "\
	By default we can't access texture's pixel data, which is required for line trace.\
	Click this button to fix it by change texture settings.\
		"))
							[
								SNew(STextBlock)
								.Text(LOCTEXT("FixTextureForHitTest", "Fix texture for hit test"))
								.Font(IDetailLayoutBuilder::GetDetailFont())
							]
						]
						;
				}
			}
		}
	
		category.AddCustomRow(LOCTEXT("AdditionalButton_Row", "AdditionalButton"))
		.ValueContent()
		.MinDesiredWidth(160)
		[
			SNew(SButton)
			.HAlign(EHorizontalAlignment::HAlign_Center)
			.VAlign(EVerticalAlignment::VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("SnapSize_Button", "Snap Size"))
				.Font(IDetailLayoutBuilder::GetDetailFont())
			]
			.OnClicked_Lambda([=, this]()
			{
				GEditor->BeginTransaction(LOCTEXT("TextureSnapSize_Transaction", "UITexture snap size"));
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
}
void FUITextureBaseCustomization::ForceRefresh(IDetailLayoutBuilder* DetailBuilder)
{
	if (DetailBuilder)
	{
		DetailBuilder->ForceRefreshDetails();
	}
}
#undef LOCTEXT_NAMESPACE