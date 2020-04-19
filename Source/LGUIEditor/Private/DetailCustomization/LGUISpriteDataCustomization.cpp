﻿// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "DetailCustomization/LGUISpriteDataCustomization.h"
#include "DesktopPlatformModule.h"
#include "Core/LGUISettings.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Core/LGUIAtlasData.h"
#include "LGUIEditorPCH.h"

#define LOCTEXT_NAMESPACE "LGUISpriteDataCustomization"

TSharedRef<IDetailCustomization> FLGUISpriteDataCustomization::MakeInstance()
{
	return MakeShareable(new FLGUISpriteDataCustomization);
}

void FLGUISpriteDataCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptPtr = Cast<ULGUISpriteData>(targetObjects[0].Get());
	if (TargetScriptPtr == nullptr)
	{
		UE_LOG(LGUIEditor, Log, TEXT("Get TargetScript is null"));
		return;
	}
	DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(ULGUISpriteData, spriteInfo));
	IDetailCategoryBuilder& lguiCategory = DetailBuilder.EditCategory("LGUI");
	lguiCategory.AddProperty(GET_MEMBER_NAME_CHECKED(ULGUISpriteData, spriteTexture));
	lguiCategory.AddCustomRow(LOCTEXT("ReloadTexture", "ReloadTexture"))
		.ValueContent()
		[
			SNew(SButton)
			.Text(FText::FromString("ReloadTexture"))
			.HAlign(EHorizontalAlignment::HAlign_Center)
			.OnClicked_Lambda([=]{
				TargetScriptPtr->ReloadTexture();
				return FReply::Handled();
			})
		];
	lguiCategory.AddProperty(GET_MEMBER_NAME_CHECKED(ULGUISpriteData, spriteInfo.width));
	lguiCategory.AddProperty(GET_MEMBER_NAME_CHECKED(ULGUISpriteData, spriteInfo.height));
	lguiCategory.AddProperty(GET_MEMBER_NAME_CHECKED(ULGUISpriteData, packingTag));
	
	//if change packingTag, clear all sprites and repack
	auto packingTagHangle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUISpriteData, packingTag));
	packingTagHangle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([] {ULGUISpriteData::MarkAllSpritesNeedToReinitialize(); }));
	//if change spriteTexture, clear all sprites and repack
	auto spriteTextureHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUISpriteData, spriteTexture));
	spriteTextureHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&DetailBuilder] {ULGUISpriteData::MarkAllSpritesNeedToReinitialize(); DetailBuilder.ForceRefreshDetails(); }));
	UObject* spriteTextureObject;
	spriteTextureHandle->GetValue(spriteTextureObject);
	if (auto spriteTexture = Cast<UTexture2D>(spriteTextureObject))
	{
		int32 atlasPadding = ULGUISettings::GetAtlasTexturePadding(TargetScriptPtr->packingTag);
		if (TargetScriptPtr->spriteTexture->GetSurfaceWidth() + atlasPadding * 2 > WARNING_ATLAS_SIZE || TargetScriptPtr->spriteTexture->GetSurfaceWidth() + atlasPadding * 2 > WARNING_ATLAS_SIZE)
		{
			UE_LOG(LGUIEditor, Error, TEXT("Target texture width or height is too large! Consider use UITexture to render this texture."));
			FNotificationInfo Info(LOCTEXT("TextureSizeError", "Target texture width or height is too large! Consider use UITexture to render this texture."));
			Info.Image = FEditorStyle::GetBrush(TEXT("LevelEditor.RecompileGameCode"));
			Info.FadeInDuration = 0.1f;
			Info.FadeOutDuration = 0.5f;
			Info.ExpireDuration = 5.0f;
			Info.bUseThrobber = false;
			Info.bUseSuccessFailIcons = true;
			Info.bUseLargeFont = true;
			Info.bFireAndForget = false;
			Info.bAllowThrottleWhenFrameRateIsLow = false;
			auto NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);
			NotificationItem->SetCompletionState(SNotificationItem::CS_Success);
			NotificationItem->ExpireAndFadeout();

			auto CompileFailSound = LoadObject<USoundBase>(NULL, TEXT("/Engine/EditorSounds/Notifications/CompileFailed_Cue.CompileFailed_Cue"));
			GEditor->PlayEditorSound(CompileFailSound);
			spriteTextureObject = nullptr;
			spriteTextureHandle->SetValue(spriteTextureObject);
			TargetScriptPtr->isInitialized = false;
		}
	}

	if(TargetScriptPtr->spriteTexture != nullptr)
	{
		IDetailCategoryBuilder& borderEditorCategory = DetailBuilder.EditCategory("BorderEditor");
		spriteSlateBrush = TSharedPtr<FSlateBrush>(new FSlateBrush);
		spriteSlateBrush->SetResourceObject(TargetScriptPtr->spriteTexture);
		borderEditorCategory.AddProperty(GET_MEMBER_NAME_CHECKED(ULGUISpriteData, spriteInfo.borderLeft));
		borderEditorCategory.AddProperty(GET_MEMBER_NAME_CHECKED(ULGUISpriteData, spriteInfo.borderRight));
		borderEditorCategory.AddProperty(GET_MEMBER_NAME_CHECKED(ULGUISpriteData, spriteInfo.borderTop));
		borderEditorCategory.AddProperty(GET_MEMBER_NAME_CHECKED(ULGUISpriteData, spriteInfo.borderBottom));
		borderEditorCategory.AddCustomRow(LOCTEXT("BorderEditor", "BorderEditor"))
		.WholeRowContent()
		[
			SAssignNew(imageBorder, SBorder)
			[
				SAssignNew(ImageBox, SBox)
				//.MinDesiredHeight(this, &FLGUISpriteDataCustomization::GetMinDesiredHeight, &DetailBuilder)
				.HeightOverride(this, &FLGUISpriteDataCustomization::GetMinDesiredHeight, &DetailBuilder)
				//.MinDesiredHeight(4096)
				//.MinDesiredWidth(4096)
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.AutoHeight()
					.VAlign(EVerticalAlignment::VAlign_Center)
					.HAlign(EHorizontalAlignment::HAlign_Center)
					[
						SNew(SHorizontalBox)
						+SHorizontalBox::Slot()
						.AutoWidth()
						.HAlign(EHorizontalAlignment::HAlign_Center)
						.VAlign(EVerticalAlignment::VAlign_Center)
						[
							SNew(SOverlay)
							//image background
							+SOverlay::Slot()
							[
								SNew(SImage)
								.Image(FEditorStyle::GetBrush("Checkerboard"))
								.ColorAndOpacity(FSlateColor(FLinearColor(0.15f, 0.15f, 0.15f)))
							]
							//image display
							+SOverlay::Slot()
							[
								SNew(SBox)
								.WidthOverride(this, &FLGUISpriteDataCustomization::GetImageWidth)
								.HeightOverride(this, &FLGUISpriteDataCustomization::GetImageHeight)
								[
									SNew(SImage)
									.Image(spriteSlateBrush.Get())
								]
							]
							//left splitter
							+SOverlay::Slot()
							[
								SNew(SHorizontalBox)
								+SHorizontalBox::Slot()
								.HAlign(EHorizontalAlignment::HAlign_Left)
								[
									SNew(SBox)
									.WidthOverride(this, &FLGUISpriteDataCustomization::GetBorderLeftSize)
									[
										SNew(SBox)
										.HAlign(EHorizontalAlignment::HAlign_Right)
										.WidthOverride(1)
										[
											SNew(SImage)
											.Image(FEditorStyle::GetBrush("PropertyEditor.VerticalDottedLine"))
										]
									]
								]
							]
							//right splitter
							+SOverlay::Slot()
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.HAlign(EHorizontalAlignment::HAlign_Right)
								[
									SNew(SBox)
									.WidthOverride(this, &FLGUISpriteDataCustomization::GetBorderRightSize)
									[
										SNew(SBox)
										.HAlign(EHorizontalAlignment::HAlign_Left)
										.WidthOverride(1)
										[
											SNew(SImage)
											.Image(FEditorStyle::GetBrush("PropertyEditor.VerticalDottedLine"))
										]
									]
								]
							]
							//top splitter
							+SOverlay::Slot()
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot()
								.VAlign(EVerticalAlignment::VAlign_Top)
								[
									SNew(SBox)
									.HeightOverride(this, &FLGUISpriteDataCustomization::GetBorderTopSize)
									[
										SNew(SBox)
										.VAlign(EVerticalAlignment::VAlign_Bottom)
										.HeightOverride(1)
										[
											SNew(SImage)
											.Image(FEditorStyle::GetBrush("PropertyEditor.HorizontalDottedLine"))
										]
									]
								]
							]
							//bottom splitter
							+SOverlay::Slot()
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot()
								.VAlign(EVerticalAlignment::VAlign_Bottom)
								[
									SNew(SBox)
									.HeightOverride(this, &FLGUISpriteDataCustomization::GetBorderBottomSize)
									[
										SNew(SBox)
										.VAlign(EVerticalAlignment::VAlign_Top)
										.HeightOverride(1)
										[
											SNew(SImage)
											.Image(FEditorStyle::GetBrush("PropertyEditor.HorizontalDottedLine"))
										]
									]
								]
							]
						]
					]
				]
			]
		];
	}
}

FOptionalSize FLGUISpriteDataCustomization::GetImageWidth()const
{
	float imageAspect = (float)(TargetScriptPtr->spriteTexture->GetSurfaceWidth()) / TargetScriptPtr->spriteTexture->GetSurfaceHeight();
	auto imageBoxSize = ImageBox->GetCachedGeometry().GetLocalSize();
	float imageBoxAspect = (float)(imageBoxSize.X / imageBoxSize.Y);
	if (imageAspect > imageBoxAspect)
	{
		return imageBoxSize.X;
	}
	else
	{
		return imageBoxSize.Y * imageAspect;
	}
}
FOptionalSize FLGUISpriteDataCustomization::GetImageHeight()const
{
	float imageAspect = (float)(TargetScriptPtr->spriteTexture->GetSurfaceWidth()) / TargetScriptPtr->spriteTexture->GetSurfaceHeight();
	auto imageBoxSize = ImageBox->GetCachedGeometry().GetLocalSize();
	float imageBoxAspect = (float)(imageBoxSize.X / imageBoxSize.Y);
	if (imageAspect > imageBoxAspect)
	{
		return imageBoxSize.X / imageAspect;
	}
	else
	{
		return imageBoxSize.Y;
	}
}
FOptionalSize FLGUISpriteDataCustomization::GetMinDesiredHeight(IDetailLayoutBuilder* DetailBuilder)const
{
	return DetailBuilder->GetDetailsView()->GetCachedGeometry().GetLocalSize().Y - 480;
}
FOptionalSize FLGUISpriteDataCustomization::GetBorderLeftSize()const
{
	if (TargetScriptPtr.Get() == nullptr)return 0;
	float imageAspect = (float)(TargetScriptPtr->spriteTexture->GetSurfaceWidth()) / TargetScriptPtr->spriteTexture->GetSurfaceHeight();
	auto imageBoxSize = ImageBox->GetCachedGeometry().GetLocalSize();
	float imageBoxAspect = (float)(imageBoxSize.X / imageBoxSize.Y);
	if (imageAspect > imageBoxAspect)
	{
		return TargetScriptPtr->spriteInfo.borderLeft * imageBoxSize.X / TargetScriptPtr->spriteTexture->GetSurfaceWidth();
	}
	else
	{
		return TargetScriptPtr->spriteInfo.borderLeft * imageBoxSize.Y / TargetScriptPtr->spriteTexture->GetSurfaceHeight();
	}
}
FOptionalSize FLGUISpriteDataCustomization::GetBorderRightSize()const
{
	if (TargetScriptPtr.Get() == nullptr)return 0;
	float imageAspect = (float)(TargetScriptPtr->spriteTexture->GetSurfaceWidth()) / TargetScriptPtr->spriteTexture->GetSurfaceHeight();
	auto imageBoxSize = ImageBox->GetCachedGeometry().GetLocalSize();
	float imageBoxAspect = (float)(imageBoxSize.X / imageBoxSize.Y);
	if (imageAspect > imageBoxAspect)
	{
		return TargetScriptPtr->spriteInfo.borderRight * imageBoxSize.X / TargetScriptPtr->spriteTexture->GetSurfaceWidth();
	}
	else
	{
		return TargetScriptPtr->spriteInfo.borderRight * imageBoxSize.Y / TargetScriptPtr->spriteTexture->GetSurfaceHeight();
	}
}
FOptionalSize FLGUISpriteDataCustomization::GetBorderTopSize()const
{
	if (TargetScriptPtr.Get() == nullptr)return 0;
	float imageAspect = (float)(TargetScriptPtr->spriteTexture->GetSurfaceWidth()) / TargetScriptPtr->spriteTexture->GetSurfaceHeight();
	auto imageBoxSize = ImageBox->GetCachedGeometry().GetLocalSize();
	float imageBoxAspect = (float)(imageBoxSize.X / imageBoxSize.Y);
	if (imageAspect > imageBoxAspect)
	{
		return TargetScriptPtr->spriteInfo.borderTop * imageBoxSize.X / TargetScriptPtr->spriteTexture->GetSurfaceWidth();
	}
	else
	{
		return TargetScriptPtr->spriteInfo.borderTop * imageBoxSize.Y / TargetScriptPtr->spriteTexture->GetSurfaceHeight();
	}
}
FOptionalSize FLGUISpriteDataCustomization::GetBorderBottomSize()const
{
	if (TargetScriptPtr.Get() == nullptr)return 0;
	float imageAspect = (float)(TargetScriptPtr->spriteTexture->GetSurfaceWidth()) / TargetScriptPtr->spriteTexture->GetSurfaceHeight();
	auto imageBoxSize = ImageBox->GetCachedGeometry().GetLocalSize();
	float imageBoxAspect = (float)(imageBoxSize.X / imageBoxSize.Y);
	if (imageAspect > imageBoxAspect)
	{
		return TargetScriptPtr->spriteInfo.borderBottom * imageBoxSize.X / TargetScriptPtr->spriteTexture->GetSurfaceWidth();
	}
	else
	{
		return TargetScriptPtr->spriteInfo.borderBottom * imageBoxSize.Y / TargetScriptPtr->spriteTexture->GetSurfaceHeight();
	}
}

#undef LOCTEXT_NAMESPACE