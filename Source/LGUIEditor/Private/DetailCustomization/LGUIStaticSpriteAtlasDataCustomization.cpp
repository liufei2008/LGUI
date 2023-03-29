// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "DetailCustomization/LGUIStaticSpriteAtlasDataCustomization.h"
#include "Core/LGUIStaticSpriteAtlasData.h"
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
	LguiCategory.AddProperty(GET_MEMBER_NAME_CHECKED(ULGUIStaticSpriteAtlasData, spriteArray));
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
	LguiCategory.AddCustomRow(LOCTEXT("PackPreviewButtonRow", "Pack Preview"))
		.ValueContent()
		[
			SNew(SButton)
			.Text(LOCTEXT("PackPreviewButton", "Pack Preview"))
			.HAlign(EHorizontalAlignment::HAlign_Center)
			.OnClicked_Lambda([=] {
				TargetScriptPtr->MarkNotInitialized();
				TargetScriptPtr->InitCheck();
				TargetScriptPtr->MarkPackageDirty();
				return FReply::Handled();
			})
		];
	DetailBuilder.HideProperty(TextureMipDataHandle);
	LguiCategory.AddCustomRow(LOCTEXT("PackedDataSize_Row", "Packed Data Size"), true)
		.NameContent()
		[
			//TextureMipDataHandle->CreatePropertyNameWidget()
			SNew(STextBlock)
			.Text(LOCTEXT("PackedDataSize_Name", "Packed Data Size"))
			.Font(DetailBuilder.GetDetailFont())
			.ToolTipText(TextureMipDataHandle->GetToolTipText())
		]
		.ValueContent()
		[
			SNew(STextBlock)
			.Text(FText::FromString(FString::Printf(TEXT("%s"), *DisplyTextureMipDataBufferSize)))
			.Font(DetailBuilder.GetDetailFont())
		];
}

#undef LOCTEXT_NAMESPACE