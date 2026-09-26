// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "DetailCustomization/LexTextureBaseCustomization.h"
#include "LexUIEditorUtils.h"
#include "Core/Components/LexTextureBase.h"
#include "Utils/LexUIUtils.h"
#include "LGUIEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "Core/Components/LexWidget.h"

#define LOCTEXT_NAMESPACE "UITextureBaseCustomization"
FLexTextureBaseCustomization::FLexTextureBaseCustomization()
{
}

FLexTextureBaseCustomization::~FLexTextureBaseCustomization()
{
	
}

TSharedRef<IDetailCustomization> FLexTextureBaseCustomization::MakeInstance()
{
	return MakeShareable(new FLexTextureBaseCustomization);
}
void FLexTextureBaseCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptArray.Empty();
	for (auto item : targetObjects)
	{
		if (auto validItem = Cast<ULexTextureBase>(item.Get()))
		{
			TargetScriptArray.Add(TWeakObjectPtr<ULexTextureBase>(validItem));
			if (validItem->GetWorld() && validItem->GetWorld()->WorldType == EWorldType::Editor)
			{
				validItem->CheckTexture();
				validItem->GetWidget()->MarkCanvasUpdate(true);
			}
		}
	}
	if (TargetScriptArray.Num() == 0)
	{
		UE_LOG(LGUIEditor, Log, TEXT("[%s].%d Get TargetScript is null"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		return;
	}

	IDetailCategoryBuilder& category = DetailBuilder.EditCategory("LexUI");
	auto textureHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexTextureBase, Texture));
	textureHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLexTextureBaseCustomization::ForceRefresh, &DetailBuilder));
	category.AddProperty(GET_MEMBER_NAME_CHECKED(ULexTextureBase, Texture));
	UTexture* texture = nullptr;
	textureHandle->GetValue((*(UObject**)&texture));
	if(IsValid(texture))
	{
		ELexVisualRaycastType raycastType = ELexVisualRaycastType::Rect;
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
			if (raycastType == ELexVisualRaycastType::VisiblePixel)
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
						FLexUIUtils::NotifyPropertyChanged(item.Get(), ULexWidget::GetPropertyName_AnchorData());
						item->GetWidget()->MarkCanvasUpdate(true);
					}
				}
				GEditor->EndTransaction();
				return FReply::Handled();
			})
		];
	}
}
void FLexTextureBaseCustomization::ForceRefresh(IDetailLayoutBuilder* DetailBuilder)
{
	if (DetailBuilder)
	{
		DetailBuilder->ForceRefreshDetails();
	}
}
#undef LOCTEXT_NAMESPACE