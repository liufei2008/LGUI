// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "DetailCustomization/LGUIStaticSpriteAtlasDataCustomization.h"
#include "Core/LGUIStaticSpriteAtlasData.h"
#include "Core/LGUISpriteData.h"
#include "Utils/LGUIUtils.h"
#include "LGUIEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"

#define LOCTEXT_NAMESPACE "LGUIStaticSpriteAtlasDataCustomization"

TSharedRef<IDetailCustomization> FLGUIStaticSpriteAtlasDataCustomization::MakeInstance()
{
	return MakeShareable(new FLGUIStaticSpriteAtlasDataCustomization);
}

void FLGUIStaticSpriteAtlasDataCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptPtr = Cast<ULGUIStaticSpriteAtlasData>(targetObjects[0].Get());
	if (TargetScriptPtr == nullptr)
	{
		UE_LOG(LGUIEditor, Log, TEXT("Get TargetScript is null"));
		return;
	}
	IDetailCategoryBuilder& LguiCategory = DetailBuilder.EditCategory("LGUI");
	auto spriteArrayHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUIStaticSpriteAtlasData, spriteArray));
	spriteArrayHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&DetailBuilder] {
		DetailBuilder.ForceRefreshDetails();
		}));
	LguiCategory.AddProperty(spriteArrayHandle);

	//check spriteData's packingAtlas
	if (TargetScriptPtr->CheckInvalidSpriteData())
	{
		auto ErrMsg = LOCTEXT("CheckSpriteDataError", "Some spriteData in spriteArray is not valid! Click \"Cleanup\" button to clear invalid spriteData.");
		UE_LOG(LGUIEditor, Error, TEXT("%s"), *ErrMsg.ToString());
		LGUIUtils::EditorNotification(ErrMsg, 10.0f);
		LguiCategory.AddCustomRow(LOCTEXT("Error_Row", "Error"))
			.NameContent()
			[
				SNew(STextBlock)
				.Text(ErrMsg)
				.ColorAndOpacity(FLinearColor(FColor::Red))
				.AutoWrapText(true)
			]
			.ValueContent()
			[
				SNew(SButton)
				.Text(LOCTEXT("CleanupButtonText", "Cleanup"))
				.HAlign(EHorizontalAlignment::HAlign_Center)
				.OnClicked_Lambda([this, &DetailBuilder] {
					TargetScriptPtr->CleanupInvalidSpriteData();
					DetailBuilder.ForceRefreshDetails();
					return FReply::Handled();
					})
			]
		;
	}

	LguiCategory.AddCustomRow(LOCTEXT("PackAtlasButtonRow", "Pack Atlas"))
		.ValueContent()
		[
			SNew(SButton)
			.Text(LOCTEXT("PackAtlasButton", "Pack Atlas"))
			.HAlign(EHorizontalAlignment::HAlign_Center)
			.OnClicked_Lambda([this] {
				TargetScriptPtr->MarkNotInitialized();
				TargetScriptPtr->InitCheck();
				TargetScriptPtr->MarkPackageDirty();
				return FReply::Handled();
			})
		];

	auto TextureMipDataHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUIStaticSpriteAtlasData, textureMipData));
	auto TextureMipDataBufferSize = TargetScriptPtr->textureMipData.Num();
	auto TextureMipDataBufferSize_kb = (double)TextureMipDataBufferSize / 1024;
	auto TextureMipDataBufferSize_mb = TextureMipDataBufferSize_kb / 1024;
	FString DisplyTextureMipDataBufferSize;
	if (TextureMipDataBufferSize_mb >= 1)
	{
		DisplyTextureMipDataBufferSize = FString::Printf(TEXT("%d.%d mb")
			, FMath::FloorToInt(TextureMipDataBufferSize_mb)
			, FMath::RoundToInt(FMath::Fractional(TextureMipDataBufferSize_mb) * 100)
		);
	}
	else if (TextureMipDataBufferSize_kb >= 1)
	{
		DisplyTextureMipDataBufferSize = FString::Printf(TEXT("%d.%d kb")
			, FMath::FloorToInt(TextureMipDataBufferSize_kb)
			, FMath::RoundToInt(FMath::Fractional(TextureMipDataBufferSize_kb) * 100)
		);
	}
	else
	{
		DisplyTextureMipDataBufferSize = FString::Printf(TEXT("%d"), TextureMipDataBufferSize);
	}
	LguiCategory.AddCustomRow(LOCTEXT("PackedDataSize_Row", "Packed Data Size"), true)
		.NameContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("PackedDataSize_Name", "Packed Data Size"))
			.Font(DetailBuilder.GetDetailFont())
			.ToolTipText(LOCTEXT("PackedDataSize_Tooltip", "Store texture mip data, so we can recreate atlas texture with this data."))
		]
		.ValueContent()
		[
			SNew(STextBlock)
			.Text(FText::FromString(FString::Printf(TEXT("%s"), *DisplyTextureMipDataBufferSize)))
			.Font(DetailBuilder.GetDetailFont())
		];
}

#undef LOCTEXT_NAMESPACE