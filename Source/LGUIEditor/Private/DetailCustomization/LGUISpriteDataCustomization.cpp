// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "DetailCustomization/LGUISpriteDataCustomization.h"
#include "DesktopPlatformModule.h"
#include "Core/LGUISettings.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Core/LGUIDynamicSpriteAtlasData.h"
#include "Core/LGUIStaticSpriteAtlasData.h"
#include "Core/LGUIManager.h"
#include "Sound/SoundCue.h"
#include "Core/LGUISpriteData.h"

#include "LGUIEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "LGUIEditorTools.h"

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
	if (IsValid(TargetScriptPtr->packingAtlas))
	{
		lguiCategory.AddCustomRow(LOCTEXT("TextureRecommend_Row", "TextureRecommend"))
		.WholeRowContent()
		.VAlign(EVerticalAlignment::VAlign_Center)
		[
			SNew(STextBlock)
			.Font(DetailBuilder.GetDetailFont())
			.AutoWrapText(true)
			.Text(LOCTEXT("TextureRecommend_Text", "Since this SpriteData use AtlasPacking, \
it is recommended to move the SpriteTexture to a folder that is \"Directories to nevert cook\", \
this can reduce the package size, because AtlasPacking already pack this SpriteTexture to a atlas texture. \
\nIgnore this message if you already make the change."))
		];
	}
	lguiCategory.AddCustomRow(LOCTEXT("ReloadTexture_Row", "ReloadTexture"))
		.ValueContent()
		[
			SNew(SButton)
			.Text(LOCTEXT("ReloadTexture_Button", "ReloadTexture"))
			.HAlign(EHorizontalAlignment::HAlign_Center)
			.OnClicked_Lambda([=]{
				TargetScriptPtr->ReloadTexture();
				TargetScriptPtr->MarkPackageDirty();
				ULGUIManagerWorldSubsystem::RefreshAllUI();
				return FReply::Handled();
			})
		];
	lguiCategory.AddProperty(GET_MEMBER_NAME_CHECKED(ULGUISpriteData, spriteInfo.width));
	lguiCategory.AddProperty(GET_MEMBER_NAME_CHECKED(ULGUISpriteData, spriteInfo.height));
	IDetailCategoryBuilder& atlasPackingCategory = DetailBuilder.EditCategory("AtlasPacking");
	auto PackingAtlasProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUISpriteData, packingAtlas));
	atlasPackingCategory.AddProperty(PackingAtlasProperty);
	PackingAtlasProperty->SetOnPropertyValuePreChange(FSimpleDelegate::CreateLambda([=] {
		if (IsValid(TargetScriptPtr->GetPackingAtlas()))
		{
			TargetScriptPtr->GetPackingAtlas()->RemoveSpriteData(TargetScriptPtr.Get());
		}
		}));
	PackingAtlasProperty->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([=, &DetailBuilder] {
		if (IsValid(TargetScriptPtr->GetPackingAtlas()))
		{
			TargetScriptPtr->GetPackingAtlas()->AddSpriteData(TargetScriptPtr.Get());
		}
		CheckSprite();
		TargetScriptPtr->InitSpriteData();
		DetailBuilder.ForceRefreshDetails();
		}));
	auto PackingTagProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUISpriteData, packingTag));
	DetailBuilder.HideProperty(PackingTagProperty);
	
	RefreshNameList(nullptr);
	if (ULGUIDynamicSpriteAtlasManager::Instance != nullptr)
	{
		ULGUIDynamicSpriteAtlasManager::Instance->OnAtlasMapChanged.AddSP(this, &FLGUISpriteDataCustomization::RefreshNameList, &DetailBuilder);
	}
	atlasPackingCategory.AddCustomRow(LOCTEXT("PackingTag", "Packing Tag"))
	.NameContent()
	[
		PackingTagProperty->CreatePropertyNameWidget()
	]
	.ValueContent()
	[
		SNew(SBox)
		.IsEnabled_Lambda([=] {
			return !IsValid(TargetScriptPtr->GetPackingAtlas());
			})
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SBox)
				.MinDesiredWidth(120)
				.Padding(FMargin(0, 2, 0, 2))
				[
					//PackingTagProperty->CreatePropertyValueWidget()
					SNew(SComboButton)
					.HasDownArrow(true)
					.ButtonContent()
					[
						SNew(SHorizontalBox)
						+SHorizontalBox::Slot()
						[
							SNew(SEditableText)
							.OnTextCommitted(this, &FLGUISpriteDataCustomization::OnPackingTagTextCommited, PackingTagProperty, &DetailBuilder)
							.Text(this, &FLGUISpriteDataCustomization::GetPackingTagText, PackingTagProperty)
						]
					]
					.MenuContent()
					[
						SNew(SVerticalBox)
						+SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SListView<TSharedPtr<FName>>)
							.ListItemsSource(&NameList)
							.OnGenerateRow(this, &FLGUISpriteDataCustomization::GenerateComboItem)
							.OnSelectionChanged(this, &FLGUISpriteDataCustomization::HandleRequiredParamComboChanged, PackingTagProperty, &DetailBuilder)
						]
					]
				]
			]
			+SHorizontalBox::Slot()
			.Padding(FMargin(2))
			.AutoWidth()
			[
				SNew(SButton)
				.VAlign(EVerticalAlignment::VAlign_Center)
				.Text(LOCTEXT("OpenAtals", "Open Atals Viewer"))
				.OnClicked_Lambda([]() {
				LGUIEditorTools::OpenAtlasViewer_Impl();
				return FReply::Handled();
					})
			]
		]
	]
	;
	atlasPackingCategory.AddProperty(GET_MEMBER_NAME_CHECKED(ULGUISpriteData, useEdgePixelPadding));

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
		if (TargetScriptPtr->spriteTexture->GetSurfaceWidth() + atlasPadding * 2 > WARNING_ATLAS_SIZE || TargetScriptPtr->spriteTexture->GetSurfaceHeight() + atlasPadding * 2 > WARNING_ATLAS_SIZE)
		{
			UE_LOG(LGUIEditor, Error, TEXT("Target texture width or height is too large! Consider use UITexture to render this texture."));
			FNotificationInfo Info(LOCTEXT("TextureSizeError", "Target texture width or height is too large! Consider use UITexture to render this texture."));
			Info.Image = FAppStyle::GetBrush(TEXT("LevelEditor.RecompileGameCode"));
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
		IDetailCategoryBuilder& paddingCategory = DetailBuilder.EditCategory("Padding");
		paddingCategory.AddProperty(GET_MEMBER_NAME_CHECKED(ULGUISpriteData, spriteInfo.paddingLeft));
		paddingCategory.AddProperty(GET_MEMBER_NAME_CHECKED(ULGUISpriteData, spriteInfo.paddingRight));
		paddingCategory.AddProperty(GET_MEMBER_NAME_CHECKED(ULGUISpriteData, spriteInfo.paddingTop));
		paddingCategory.AddProperty(GET_MEMBER_NAME_CHECKED(ULGUISpriteData, spriteInfo.paddingBottom));

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
			SNew(SBorder)
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
								.Image(FAppStyle::GetBrush("Checkerboard"))
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
											.Image(FAppStyle::GetBrush("PropertyEditor.VerticalDottedLine"))
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
											.Image(FAppStyle::GetBrush("PropertyEditor.VerticalDottedLine"))
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
											.Image(FAppStyle::GetBrush("PropertyEditor.HorizontalDottedLine"))
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
											.Image(FAppStyle::GetBrush("PropertyEditor.HorizontalDottedLine"))
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

void FLGUISpriteDataCustomization::CheckSprite()
{
	//check invalid RenderSprite in atlas
	if (IsValid(TargetScriptPtr->packingAtlas))
	{
		TargetScriptPtr->packingAtlas->CheckSprite();
	}
	if (auto spriteAtlasData = ULGUIDynamicSpriteAtlasManager::Find(TargetScriptPtr->packingTag))
	{
		spriteAtlasData->CheckSprite(TargetScriptPtr->packingTag);
	}
}

void FLGUISpriteDataCustomization::RefreshNameList(IDetailLayoutBuilder* DetailBuilder)
{
	NameList.Reset();
	NameList.Add(MakeShareable(new FName(NAME_None)));
	if (ULGUIDynamicSpriteAtlasManager::Instance != nullptr)
	{
		auto& AtlasMap = ULGUIDynamicSpriteAtlasManager::Instance->GetAtlasMap();
		for (auto KeyValue : AtlasMap)
		{
			NameList.Add(TSharedPtr<FName>(new FName(KeyValue.Key)));
		}
	}
	if (DetailBuilder != nullptr)
	{
		DetailBuilder->ForceRefreshDetails();
	}
}

void FLGUISpriteDataCustomization::OnPackingTagTextCommited(const FText& InText, ETextCommit::Type CommitType, TSharedRef<IPropertyHandle> InProperty, IDetailLayoutBuilder* DetailBuilder)
{
	FName packingTag = FName(InText.ToString());
	InProperty->SetValue(packingTag);
	TargetScriptPtr->ReloadTexture();
	TargetScriptPtr->InitSpriteData();
	DetailBuilder->ForceRefreshDetails();

	CheckSprite();
}

TSharedRef<ITableRow> FLGUISpriteDataCustomization::GenerateComboItem(TSharedPtr<FName> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<TSharedPtr<FName>>, OwnerTable)
		[
			SNew(STextBlock).Text(FText::FromName(*InItem))
		];
}

void FLGUISpriteDataCustomization::HandleRequiredParamComboChanged(TSharedPtr<FName> Item, ESelectInfo::Type SelectInfo, TSharedRef<IPropertyHandle> InProperty, IDetailLayoutBuilder* DetailBuilder)
{
	InProperty->SetValue(*Item.Get());
	TargetScriptPtr->ReloadTexture();
	TargetScriptPtr->InitSpriteData();
	DetailBuilder->ForceRefreshDetails();

	CheckSprite();
}

FText FLGUISpriteDataCustomization::GetPackingTagText(TSharedRef<IPropertyHandle> InProperty)const
{
	FName packingTag;
	InProperty->GetValue(packingTag);
	return FText::FromName(packingTag);
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
	return DetailBuilder->GetDetailsView()->GetCachedGeometry().GetLocalSize().Y - 400;
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