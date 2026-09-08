// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "DetailCustomization/LexSpriteBaseCustomization.h"
#include "LexUIEditorUtils.h"
#include "LGUIEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "Core/LexUISpriteData_BaseObject.h"
#include "Core/Components/LexSpriteBase.h"
#include "Core/Components/LexWidget.h"

#define LOCTEXT_NAMESPACE "LexUISpriteBaseCustomization"
FLexSpriteBaseCustomization::FLexSpriteBaseCustomization()
{
}

FLexSpriteBaseCustomization::~FLexSpriteBaseCustomization()
{
	
}

TSharedRef<IDetailCustomization> FLexSpriteBaseCustomization::MakeInstance()
{
	return MakeShareable(new FLexSpriteBaseCustomization);
}
void FLexSpriteBaseCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptArray.Empty();
	for (auto item : targetObjects)
	{
		if (auto validItem = Cast<ULexSpriteBase>(item.Get()))
		{
			TargetScriptArray.Add(TWeakObjectPtr<ULexSpriteBase>(validItem));
			if (validItem->GetWorld() && validItem->GetWorld()->WorldType == EWorldType::Editor)
			{
				validItem->CheckSpriteData();
				validItem->GetWidget()->MarkCanvasUpdate(true);
			}
		}
	}
	if (TargetScriptArray.Num() == 0)
	{
		UE_LOG(LGUIEditor, Log, TEXT("[%s].%d Get TargetScript is null"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		return;
	}

	IDetailCategoryBuilder& category = DetailBuilder.EditCategory("LGUI");

	category.AddProperty(GET_MEMBER_NAME_CHECKED(ULexSpriteBase, Sprite));
	auto spriteHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexSpriteBase, Sprite));
	spriteHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([=, this, &DetailBuilder] {
		for (auto item : TargetScriptArray)
		{
			if (item.IsValid())
			{
				item->OnPostChangeSpriteProperty();
			}
		}
		DetailBuilder.ForceRefreshDetails();
	}));
	spriteHandle->SetOnPropertyValuePreChange(FSimpleDelegate::CreateLambda([=, this] {
		for (auto item : TargetScriptArray)
		{
			if (item.IsValid())
			{
				item->OnPreChangeSpriteProperty();
			}
		}
	}));
	ULexUISpriteData_BaseObject* spriteObject = nullptr;
	spriteHandle->GetValue(*(UObject**)&spriteObject);
	if (IsValid(spriteObject))
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
				if (!spriteObject->SupportReadPixel())
				{
					category.AddCustomRow(LOCTEXT("NotSupportVisiblePixelRaycast_Row", "NotSupportVisiblePixelRaycast"))
						.WholeRowContent()
						.VAlign(EVerticalAlignment::VAlign_Center)
						[
							SNew(STextBlock)
							.ColorAndOpacity(FLinearColor::Yellow)
							.Text(LOCTEXT("NotSupportVisiblePixelRaycast_Text", "Use RaycastType of VisiblePixel, but this Sprite does not support this type."))
							.Font(IDetailLayoutBuilder::GetDetailFont())
						]
					;
				}
			}
		}

		category.AddCustomRow(LOCTEXT("AdditionalButton", "AdditionalButton"))
		.ValueContent()
		[
			SNew(SButton)
			.HAlign(EHorizontalAlignment::HAlign_Center)
			.VAlign(EVerticalAlignment::VAlign_Center)
			.OnClicked_Lambda([=, this]()
			{
				GEditor->BeginTransaction(LOCTEXT("SpriteSnapSize_Transaction", "LexSprite snap size"));
				for (auto item : TargetScriptArray)
				{
					if (item.IsValid())
					{
						item->Modify();
						item->SetSizeFromSpriteData();
						FLexUIUtils::NotifyPropertyChanged(item.Get(), ULexWidget::GetPropertyName_AnchorData());
						item->GetWidget()->MarkCanvasUpdate(true);
					}
				}
				GEditor->EndTransaction();
				return FReply::Handled();
			})
			[
				SNew(STextBlock)
				.Text(LOCTEXT("SnapSize_ButtonText", "Snap Size"))
				.Font(IDetailLayoutBuilder::GetDetailFont())
			]
		];
	}
}
void FLexSpriteBaseCustomization::ForceRefresh(IDetailLayoutBuilder* DetailBuilder)
{
	if (DetailBuilder)
	{
		DetailBuilder->ForceRefreshDetails();
	}
}
#undef LOCTEXT_NAMESPACE